#include "transaction_manager.h"
#include "downloader_error_codes.h"
#include "file_utils.h"
#include "logger.h"

using namespace AdResourceMonitor;
static const std::set<TransactionPtr> kEmptySet;
const int kMaxPriority = 100;

TransactionManager::TransactionManager(const TaskSchedulerPtr& taskScheduler,
                                       const std::shared_ptr<TaskCache>& taskCache)
    : m_taskScheduler{taskScheduler}, m_taskCache{taskCache} {}

TransactionManager::TransactionManager(const TaskSchedulerPtr& taskScheduler)
    : m_taskScheduler{taskScheduler}, m_taskCache{std::make_shared<TaskCache>()} {}

void TransactionManager::Start(TransactionPtr& transaction) {
  for (const DownloadGroupItemWrapperPtr& downloadItemWrapper : transaction->GetDownloadItems()) {
    DownloadGroupItem& downloadItem = downloadItemWrapper->GetDownloadItem();
    LOG_DEBUG("Querying from cache for Download Item ", downloadItem.GetId());
    TaskPtr downloadTask = m_taskCache->CreateDownloadTask(downloadItem);
    const_cast<DownloadGroupItemWrapperPtr&>(downloadItemWrapper)->SetTask(downloadTask);
    downloadTask->SetPriority((transaction->GetCreateTime().count() * kMaxPriority) + downloadItem.priority);
    RegisterParent(transaction, downloadTask);
    downloadTask->RegisterStateChangeListener(shared_from_this());
    m_taskScheduler->AddTask(downloadTask);
  }
  transaction->SetState(TransactionalState::TRANSACTION_ACTIVATED);
}

bool TransactionManager::ShouldExecute(const TaskPtr& task, const TransactionPtr& currentParent) {
  for (const TransactionPtr& parent : GetParents(*task)) {
    if (parent->GetId() != currentParent->GetId() &&
        (parent->GetState() == TransactionalState::TRANSACTION_PAUSED ||
         parent->GetState() == TransactionalState::TRANSACTION_ACTIVATED)) {
      LOG_DEBUG("Request ignored for task ", task->GetId(), " with parent transaction: ", currentParent->GetId(),
                " as another parent ", parent->GetId(), " is still in active/paused state");
      return false;
    }
  }
  return true;
}

int TransactionManager::StartTransaction(TransactionPtr& transaction) {
  LOG_INFO("Start request received for Transaction ", transaction->GetId());
  std::lock_guard<std::mutex> lock(m_transactionMutex);
  int errorCode = m_transactionValidator.ValidateStartRequest(m_transactions, transaction);
  if (errorCode == kNoError) {
    m_transactions[transaction->GetId()] = transaction;
    Start(transaction);
  }
  return errorCode;
}

int TransactionManager::StopTransaction(TransactionPtr& transaction) {
  LOG_INFO("Stop request received for Transaction ", transaction->GetId());
  std::lock_guard<std::mutex> lock(m_transactionMutex);
  int errorCode = m_transactionValidator.ValidateStopRequest(m_transactions, transaction);
  if (errorCode == kNoError) {
    TransactionPtr currentTransaction = m_transactions[transaction->GetId()];
    for (const DownloadGroupItemWrapperPtr& downloadItemWrapper : currentTransaction->GetDownloadItems()) {
      DownloadGroupItem& downloadItem = downloadItemWrapper->GetDownloadItem();
      TaskPtr task = m_taskCache->GetTask(downloadItem);
      if (!task) {
        continue;
      }
      if (ShouldExecute(task, currentTransaction)) {
        task->Stop();
      }
      DeregisterParent(currentTransaction, (*task));
      RemoveTaskIfOrphan(*task);
    }
    currentTransaction->SetState(TransactionalState::TRANSACTION_STOPPED);
    DeleteTransaction(currentTransaction);
  }
  return errorCode;
}

int TransactionManager::CancelTransaction(TransactionPtr& transaction) {
  LOG_INFO("Cancel request received for Transaction ", transaction->GetId());
  std::lock_guard<std::mutex> lock(m_transactionMutex);
  int errorCode = m_transactionValidator.ValidateCancelRequest(m_transactions, transaction);
  if (errorCode == kNoError) {
    TransactionPtr currentTransaction = m_transactions[transaction->GetId()];
    for (const DownloadGroupItemWrapperPtr& downloadItemWrapper : currentTransaction->GetDownloadItems()) {
      DownloadGroupItem& downloadItem = downloadItemWrapper->GetDownloadItem();
      TaskPtr task = m_taskCache->GetTask(downloadItem);
      if (!task) {
        task = m_taskCache->CreateDownloadTask(downloadItem);
      }
      if (GetParents(*task).size() <= 1) {
        task->Cancel();
      }
      DeregisterParent(currentTransaction, *task);
      RemoveTaskIfOrphan(*task);
    }
    currentTransaction->SetState(TransactionalState::TRANSACTION_CANCELED);
    DeleteTransaction(currentTransaction);
  }
  return errorCode;
}

int TransactionManager::PauseTransaction(TransactionPtr& transaction) {
  LOG_INFO("Pause request received for Transaction ", transaction->GetId());
  std::lock_guard<std::mutex> lock(m_transactionMutex);
  int errorCode = m_transactionValidator.ValidatePauseRequest(m_transactions, transaction);
  if (errorCode == kNoError) {
    TransactionPtr currentTransaction = m_transactions[transaction->GetId()];
    for (const DownloadGroupItemWrapperPtr& downloadItemWrapper : currentTransaction->GetDownloadItems()) {
      DownloadGroupItem& downloadItem = downloadItemWrapper->GetDownloadItem();
      TaskPtr task = m_taskCache->GetTask(downloadItem);
      if (!task) {
        continue;
      }
      if (ShouldExecute(task, currentTransaction)) {
        task->Pause();
      }
      DeregisterParent(currentTransaction, *task);
      RemoveTaskIfOrphan(*task);
    }
    currentTransaction->SetState(TransactionalState::TRANSACTION_PAUSED);
  }
  return errorCode;
}

int TransactionManager::ResumeTransaction(TransactionPtr& transaction) {
  LOG_INFO("Resume request received for Transaction ", transaction->GetId());
  std::lock_guard<std::mutex> lock(m_transactionMutex);
  int errorCode = m_transactionValidator.ValidateResumeRequest(m_transactions, transaction);
  if (errorCode == kNoError) {
    TransactionPtr currentTransaction = m_transactions[transaction->GetId()];
    Start(currentTransaction);
  }
  return errorCode;
}

void TransactionManager::StopAllTransactions() {
  LOG_INFO("StopAllTransactions request received");
  m_stopAllTransactions = true;
  std::lock_guard<std::mutex> guard(m_transactionMutex);
  for (auto mapPair : m_transactions) {
    auto transaction = mapPair.second;
    for (const DownloadGroupItemWrapperPtr& downloadItemWrapper : transaction->GetDownloadItems()) {
      DownloadGroupItem& downloadItem = downloadItemWrapper->GetDownloadItem();
      auto task = m_taskCache->GetTask(downloadItem);
      if (task) {
        task->Stop();
      }
    }
    transaction->SetState(TransactionalState::TRANSACTION_STOPPED);
  }
}

int TransactionManager::Exit() {
  break_connection();
  return kNoError;
}

void TransactionManager::DeleteTransaction(const TransactionPtr& transaction) {
  LOG_DEBUG("Parent transaction [", transaction->GetId(), "] deleted from cache");
  m_transactions.erase(transaction->GetId());
}

void TransactionManager::RemoveTaskIfOrphan(const Task& task) {
  if (GetParents(task).empty()) {
    m_taskCache->RemoveTask(task);
  }
}

void TransactionManager::RegisterParent(const TransactionPtr& transaction, const TaskPtr& task) {
  LOG_DEBUG("Added parent [", transaction->GetId(), "] for task [", task->GetId(), "]");
  m_taskParentMap[task->GetId()].insert(transaction);
}

void TransactionManager::DeregisterParent(const TransactionPtr& transaction, const Task& task) {
  auto existing = m_taskParentMap.find(task.GetId());
  if (existing != m_taskParentMap.end()) {
    LOG_DEBUG("Removed parent [", transaction->GetId(), "] for task [", task.GetId(), "]");
    existing->second.erase(transaction);
  }
}

const std::set<TransactionPtr>& TransactionManager::GetParents(const Task& task) {
  auto existing = m_taskParentMap.find(task.GetId());
  if (existing != m_taskParentMap.end()) {
    return existing->second;
  }
  LOG_DEBUG("No parent registered for task [", task.GetId(), "]");
  return kEmptySet;
}

bool IsItemCompleted(const DownloadGroupItem& item) {
  if (item.errorCode == kNoError) {
    if (item.totalBlock != 0 && item.totalBlock == item.decompressedBlock) {
      LOG_DEBUG("Download item [", item.GetId(), "] marked completed");
      return true;
    }
    LOG_DEBUG("Download item [", item.GetId(), "] marked NOT completed");
    return false;
  } else {
    LOG_DEBUG("Download item [", item.GetId(), "] marked NOT completed due to error");
    return false;
  }
  return false;
}

bool AllItemsCompleted(const std::vector<DownloadGroupItem>& items) {
  for (auto itemIt = items.begin(); itemIt != items.end(); ++itemIt) {
    if (!IsItemCompleted(*itemIt)) {
      return false;
    }
  }
  return true;
}

void TransactionManager::DeregisterActiveParents(const Task& task) {
  std::set<TransactionPtr>& parents = const_cast<std::set<TransactionPtr>&>(GetParents(task));
  std::set<TransactionPtr> deregisteredParents;
  for (auto it = parents.begin(); it != parents.end(); ++it) {
    if ((*it)->GetState() == TransactionalState::TRANSACTION_ACTIVATED) {
      deregisteredParents.insert(*it);
    }
  }
  for (auto it = deregisteredParents.begin(); it != deregisteredParents.end(); ++it) {
    std::vector<DownloadGroupItem> items = (*it)->GetDownloadGroupItems();
    if (AllItemsCompleted(items)) {
      DeleteTransaction(*it);
    }
  }
  for (auto deregisteredParent : deregisteredParents) {
    LOG_DEBUG("Deregistering parent [", deregisteredParent->GetId(), "] for task [", task.GetId(), "]");
    parents.erase(deregisteredParent);
  }
}

void TransactionManager::TaskStateChanged(const Task& task) {
  if (m_stopAllTransactions) {
    LOG_DEBUG("stopAllTransactions already received.");
    return;
  }

  if (task.GetState() == TaskState::COMPLETED) {
    std::lock_guard<std::mutex> lock(m_transactionMutex);
    if (task.GetType() == TaskType::DownloadTask) {
      LOG_DEBUG("Download task [", task.GetId(), "] completed");
      for (const TransactionPtr& transaction : GetParents(task)) {
        auto decompressLocation = transaction->GetDecompressLocation();
        if (decompressLocation.empty()) {
          decompressLocation = FileUtil::GetParentPath(task.GetDownloadGroupItem().fileLocation);
        }

        TaskPtr decompressTask = m_taskCache->CreateDecompressTask(task.GetDownloadGroupItem(), decompressLocation);
        decompressTask->SetPriority(task.GetPriority());
        decompressTask->RegisterStateChangeListener(shared_from_this());
        for (const DownloadGroupItemWrapperPtr& item : transaction->GetDownloadItems()) {
          if (item->GetDownloadItem().GetId() == task.GetId()) {
            item->SetTask(decompressTask);
          }
        }

        m_taskScheduler->AddTask(decompressTask);
      }
      m_taskCache->RemoveTask(task);
    } else if (task.GetType() == TaskType::DecompressTask) {
      LOG_DEBUG("Decompress task [", task.GetId(), "] completed");
      DeregisterActiveParents(task);
      RemoveTaskIfOrphan(task);
    }
  }
  if (task.GetState() == TaskState::STOPPED) {
    StopTransactionsIfError(task);
  }
}

void TransactionManager::StopTransactionsIfError(const Task& task) {
  int taskErrorCode = task.GetDownloadGroupItem().errorCode;
  std::set<TransactionPtr> stopTransactions;
  if (taskErrorCode != kNoError) {
    std::lock_guard<std::mutex> lock(m_transactionMutex);
    for (auto mapPair : m_transactions) {
      auto transaction = mapPair.second;
      for (const DownloadGroupItemWrapperPtr& item : transaction->GetDownloadItems()) {
        if (item->GetDownloadItem().GetId() == task.GetId()) {
          if (!item->GetDownloadItem().ignoreError) {
            int currentTranError = transaction->GetLastErrorCode() ? transaction->GetLastErrorCode() : taskErrorCode;
            item->GetDownloadItem().errorCode = taskErrorCode;
            transaction->SetLastErrorCode(currentTranError);
            transaction->ItemProgressChanged();
            stopTransactions.insert(transaction);
            break;
          } else {
            DeregisterParent(transaction, (task));
            RemoveTaskIfOrphan(task);
          }
        }
      }
    }
  }
  for (TransactionPtr transaction : stopTransactions) {
    StopTransaction(transaction);
  }
}
