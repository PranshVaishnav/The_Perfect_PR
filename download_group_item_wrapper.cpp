#include "download_group_item_wrapper.h"
#include "error_codes.h"
#include "transaction.h"

DownloadGroupItemWrapper::DownloadGroupItemWrapper(Transaction& transaction, const DownloadGroupItem& item)
    : m_transaction{transaction}, m_downloadItem{item} {}

void DownloadGroupItemWrapper::TaskProgressChanged(const Task& task) {
  std::lock_guard<std::mutex> lock(m_progressMutex);
  if ((m_transaction.GetState() != TransactionalState::TRANSACTION_ACTIVATED) &&
      (m_transaction.GetState() != TransactionalState::TRANSACTION_PAUSED)) {
    return;
  }
  if (task.GetType() == TaskType::DownloadTask) {
    m_downloadItem.downloadedBlock = task.GetTaskProgress().GetCompleted();
    m_downloadItem.decompressedBlock = 0;
  } else {
    m_downloadItem.downloadedBlock = m_downloadItem.totalBlock;
    m_downloadItem.decompressedBlock = task.GetTaskProgress().GetCompleted();
  }
  m_downloadItem.totalBlock = task.GetTaskProgress().GetTotal();
  m_downloadItem.fileLocation = task.GetDownloadGroupItem().fileLocation;
  m_downloadItem.errorCode = task.GetDownloadGroupItem().errorCode;
  m_downloadItem.maxTime = task.GetDownloadGroupItem().maxTime;
  if (m_downloadItem.errorCode != AdDownloadStaticLib::kNO_ERROR && !m_downloadItem.ignoreError) {
    m_transaction.SetLastErrorCode(task.GetDownloadGroupItem().errorCode < 0
                                       ? task.GetDownloadGroupItem().errorCode
                                       : AdDownloadStaticLib::kDOWNLOAD_FATAL_ERROR);
  }
  m_transaction.ItemProgressChanged();
}

void DownloadGroupItemWrapper::SetTask(TaskPtr& taskPtr) {
  Deregister();
  m_taskPtr = taskPtr;
  m_taskPtr->RegisterProgressListener(shared_from_this());
}

DownloadGroupItem& DownloadGroupItemWrapper::GetDownloadItem() const {
  return const_cast<DownloadGroupItem&>(m_downloadItem);
}

DownloadGroupItemWrapper::~DownloadGroupItemWrapper() { Deregister(); }

void DownloadGroupItemWrapper::Deregister() {
  if (m_taskPtr) {
    m_taskPtr->DeregisterProgressListener(shared_from_this());
  }
}
