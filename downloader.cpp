#include <iostream>
#include <mutex>

#include "downloader.h"
#include "logger.h"

const std::string IgnoreChecksum = "*";

namespace AdDownloadStaticLib {

DefaultDownloader::DefaultDownloader(const std::string &url, const std::string &downloadPath, const int maxTime,
                                     void *logPtr)
    : BaseDownloader(url, downloadPath, logPtr, nullptr, maxTime) {}

DefaultDownloader::DefaultDownloader(const std::string &url, const std::string &downloadDir,
                                     const std::string &fileName, const int maxTime, void *logPtr)
    : BaseDownloader(url, downloadDir, fileName, logPtr, nullptr, maxTime) {}

void DefaultDownloader::Notify(StatusEvent *e) {
  static std::mutex display_mutex;
  switch (e->type) {
    case StatusEvent::THREAD_WRITER_DONE: {
      std::lock_guard<std::mutex> guard(display_mutex);
      LOG_INFO("Downloading ", e->m_downloadedBlocks, " of ", e->m_totalBlockNumber);
    } break;
    default:
      break;
  }
}

TaskDownloader::TaskDownloader(const DownloadGroupItem &item)
    : BaseDownloader(item.url, kTempFolder, item.fileName, &(Logger::Instance()), nullptr, item.maxTime),
      m_downloadItem(item) {}

void TaskDownloader::Notify(StatusEvent *e) {
  switch (e->type) {
    case StatusEvent::THREAD_WRITER_DONE: {
      m_downloadItem.downloadedBlock = e->m_downloadedBlocks;
      m_downloadItem.totalBlock = e->m_totalBlockNumber;
      notify(m_downloadItem);
      stateChange(e->type);
    } break;
    case StatusEvent::DOWNLOAD_FATAL_ERROR: {
      m_downloadItem.errorCode = e->m_curlErrorCode;
      notify(m_downloadItem);
    } break;
    case StatusEvent::INITIALIZATION_ERROR:
      m_downloadItem.errorCode = e->m_curlErrorCode;
      notify(m_downloadItem);
      stateChange(e->type);
      break;
    case StatusEvent::DOWNLOAD_COMPLETE:
      m_downloadItem.downloadedBlock = e->m_downloadedBlocks;
      if (m_downloadItem.totalBlock != e->m_totalBlockNumber) {
        m_downloadItem.totalBlock = e->m_totalBlockNumber;
        notify(m_downloadItem);
      }
      if (m_downloadItem.fileChecksum != IgnoreChecksum) {
        m_downloadItem.errorCode = SHA256ChecksumCheck(m_downloadItem.fileChecksum);

        if (m_downloadItem.errorCode == kNoError) {
          m_downloadItem.downloadedBlock = m_downloadItem.totalBlock;
        } else {
          notify(m_downloadItem);
        }
      }
    case StatusEvent::DOWNLOAD_CANCELLED:
    case StatusEvent::DOWNLOAD_STOPPED:
    case StatusEvent::THREAD_DOWNLOAD_GET_NEW_BLOCK: {
      stateChange(e->type);
    } break;
    default:
      break;
  }
}

int TaskDownloader::GetTotalBlock() const { return m_downloadItem.totalBlock; }

int TaskDownloader::GetDownloadedBlock() const { return m_downloadItem.downloadedBlock; }

}  // namespace AdDownloadStaticLib
