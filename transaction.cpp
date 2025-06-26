#include "transaction.h"

Transaction::Transaction(const DownloadGroupRequest& request, int lastErrorCode,
  std::shared_ptr<ChannelBrokerBase> channelBroker)
    : m_downloadGroupId{request.downloadGroupId},
      m_decompressLocation{request.downloadGroupLocation},
  m_createTime(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())),
      m_lastErrorCode{lastErrorCode}, m_channelBroker{channelBroker} {
  for (const DownloadGroupItem& item : request.downloadGroup) {
    m_downloadItems.insert(std::make_shared<DownloadGroupItemWrapper>(*this, item));
  }
}

Transaction::Transaction(const DownloadGroupIdRequest& request, int lastErrorCode,
                         std::shared_ptr<ChannelBrokerBase> channelBroker)
    : m_downloadGroupId{request.downloadGroupId},
      m_createTime(
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())),
      m_lastErrorCode{lastErrorCode},
      m_channelBroker{channelBroker} {
}

const std::string& Transaction::GetId() const {
  return m_downloadGroupId;
}

const std::set<DownloadGroupItemWrapperPtr, DownloadItemGroupComparator>& Transaction::GetDownloadItems() const {
  return m_downloadItems;
}

TransactionalState Transaction::GetState() const {
  return m_transactionalState;
}

int Transaction::GetLastErrorCode() const {
  return m_lastErrorCode;
}

std::chrono::milliseconds Transaction::GetCreateTime() const {
  return m_createTime;
}

void Transaction::SetLastErrorCode(int errorCode) {
  m_lastErrorCode = errorCode;
}

void Transaction::SetState(TransactionalState state) {
  m_transactionalState = state;
}

bool Transaction::ContainsItem(const std::string& id) {
  auto found =
    std::find_if(m_downloadItems.begin(), m_downloadItems.end(),
      [id](const DownloadGroupItemWrapperPtr& item) {
        return item->GetDownloadItem().GetId() == id;
      });
  return found != m_downloadItems.end();
}

std::vector<DownloadGroupItem> Transaction::GetDownloadGroupItems() const {
  std::vector<DownloadGroupItem> items;
  for (const DownloadGroupItemWrapperPtr wrapper : m_downloadItems) {
    items.push_back(wrapper->GetDownloadItem());
  }
  return items;
}

adustring Transaction::GetDecompressLocation() const { return m_decompressLocation; }

void Transaction::ItemProgressChanged() {
  std::lock_guard<std::mutex> lock(m_progressMutex);
  if (m_channelBroker && m_transactionalState == TransactionalState::TRANSACTION_ACTIVATED) {
    m_channelBroker->Notify(*this);
  }
}