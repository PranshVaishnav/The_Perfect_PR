#include "download_task.h"
#include "downloader_error_codes.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "file_utils.h"

using namespace AdDownloadStaticLib;

DownloadTask::DownloadTask(const DownloadGroupItem& item) : m_downloadGroupItem{item} {
  m_downloader = std::make_shared<TaskDownloader>(item);
  m_downloader->stateChange.connect(boost::bind(&DownloadTask::OnStateChange, this, boost::placeholders::_1));
  m_downloader->notify.connect(boost::bind(&DownloadTask::OnProgress, this, boost::placeholders::_1));
}

DownloadTask::DownloadTask(const DownloadGroupItem& item, AdDownloadStaticLib::TaskDownloaderPtr downloader)
    : m_downloadGroupItem{item}, m_downloader{downloader} {}

const std::string& DownloadTask::GetId() const { return m_downloadGroupItem.GetId(); }

int DownloadTask::CreateItemToDownload() {
  m_downloadGroupItem.fileLocation = m_downloader->DownloadLocation();
  return m_downloader->CreateDownloadFile();
}

int DownloadTask::Start() {
  if (m_state == TaskState::CANCELED) {
    LOG_INFO("Task already canceled. Can't start");
    return kNoError;
  }

  m_state = TaskState::RUNNING;
  m_downloadGroupItem.fileLocation = m_downloader->DownloadLocation();
  int errorCode = m_downloader->Open();
  if (errorCode == kNoError) {
    errorCode = m_downloader->Wait();
  }
  if (errorCode != kNoError && m_state == TaskState::RUNNING) {
    m_state = TaskState::STOPPED;
    m_downloadGroupItem.errorCode = errorCode;
    LOG_DEBUG("Task ", GetId(), " stopped with error ", errorCode);
    OnProgress(m_downloadGroupItem);
  }

  return errorCode;
}

int DownloadTask::Stop() {
  std::unique_lock<std::mutex> lock(m_stateMutex);
  int errorCode = kNoError;
  while (m_state == TaskState::RUNNING) {
    errorCode = m_downloader->Stop();
    m_stateConditionVar.wait(lock);
  }
  m_state = TaskState::STOPPED;
  return errorCode;
}

int DownloadTask::Pause() { return Stop(); }

int DownloadTask::Resume() {
  std::unique_lock<std::mutex> lock(m_stateMutex);
  int errorCode = kNoError;
  while (m_state != TaskState::RUNNING) {
    errorCode = Start();
    m_stateConditionVar.wait(lock);
  }
  m_state = TaskState::RUNNING;
  return errorCode;
}

void DownloadTask::DeleteIncompleteFile() {
  if (!m_downloadGroupItem.fileLocation.empty()) {
    int totalBlock = 0, downloadedBlocks = 0;
    auto infoFileLocation = m_downloadGroupItem.fileLocation + ".info";
    if (boost::filesystem::exists(infoFileLocation)) {
      m_downloader->LoadBlockInfo(totalBlock, downloadedBlocks);
    }

    if (downloadedBlocks < totalBlock || downloadedBlocks == 0) {
      FileUtil::Remove(m_downloadGroupItem.fileLocation);
      FileUtil::Remove(infoFileLocation);
    }
  }
}

int DownloadTask::Cancel() {
  std::unique_lock<std::mutex> lock(m_stateMutex);
  int errorCode = kNoError;
  if (m_state == TaskState::NEW) {
    DeleteIncompleteFile();
  } else {
    while (m_state == TaskState::RUNNING) {
      errorCode = m_downloader->Cancel();
      m_stateConditionVar.wait(lock);
    }
  }
  m_state = TaskState::CANCELED;
  return errorCode;
}

unsigned long long DownloadTask::GetPriority() const { return m_priority; }

void DownloadTask::SetPriority(unsigned long long priority) { m_priority = priority; }

DownloadGroupItem DownloadTask::GetDownloadGroupItem() const { return m_downloadGroupItem; }

TaskType DownloadTask::GetType() const { return TaskType::DownloadTask; }

DownloadTask::~DownloadTask() { LOG_DEBUG("DownloadTask deconstructor"); }

void DownloadTask::OnStateChange(const int state) {
  switch (state) {
    case StatusEvent::THREAD_WRITER_DONE:
      m_state = TaskState::RUNNING;
      m_stateConditionVar.notify_one();
      break;
    case StatusEvent::DOWNLOAD_COMPLETE:
      m_state = (GetDownloadGroupItem().errorCode) ? TaskState::STOPPED : TaskState::COMPLETED;
      if (m_state == TaskState::COMPLETED) {
        NotifyProgress(*this);
      }
      m_stateConditionVar.notify_one();
      break;
    case StatusEvent::DOWNLOAD_CANCELLED:
      m_state = TaskState::CANCELED;
      m_stateConditionVar.notify_one();
      break;
    case StatusEvent::DOWNLOAD_STOPPED:
    case StatusEvent::INITIALIZATION_ERROR:
      m_state = TaskState::STOPPED;
      m_stateConditionVar.notify_one();
      break;
    case StatusEvent::DOWNLOAD_PAUSED:
      m_state = TaskState::PAUSED;
      m_stateConditionVar.notify_one();
      break;
  };
  NotifyStateChange(*this);
}

void DownloadTask::OnProgress(const DownloadGroupItem& item) {
  m_downloadGroupItem.errorCode = item.errorCode;
  m_downloadGroupItem.totalBlock = item.totalBlock;
  m_downloadGroupItem.downloadedBlock = item.downloadedBlock;
  if (m_downloader->GetDownloadedBlock() < m_downloader->GetTotalBlock()) {
    NotifyProgress(*this);
  }
}

TaskProgress DownloadTask::GetTaskProgress() const {
  return TaskProgress(m_downloader->GetDownloadedBlock(), m_downloader->GetTotalBlock());
}

TaskState DownloadTask::GetState() const { return m_state; }
