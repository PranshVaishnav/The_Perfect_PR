//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright 2018 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary to Autodesk,
// Inc., and considered a trade secret as defined in section 499C of the
// penal code of the State of California.  Use of this information by anyone
// other than authorized employees of Autodesk, Inc. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
//////////////////////////////////////////////////////////////////////////////
//
// Name:        transaction.h
// Description: Declares all internal class for representing transaction

#ifndef TRANSACTION_H_
#define TRANSACTION_H_

#include <memory>
#include <set>
#include <string>

#include <iostream>

#include "download_group_item_wrapper.h"
#include "download_protocol.h"
#include "downloader.h"
#include "ipc_request_channel.h"
#include "task_progress_listener.h"

enum TransactionalState {
  TRANSACTION_NEW,
  TRANSACTION_ACTIVATED,
  TRANSACTION_PAUSED,
  TRANSACTION_CANCELED,
  TRANSACTION_STOPPED
};

struct DownloadItemGroupComparator {
  bool operator()(const DownloadGroupItemWrapperPtr& lhs, const DownloadGroupItemWrapperPtr& rhs) const {
    DownloadGroupItem& rhsItem = rhs->GetDownloadItem();
    DownloadGroupItem& lhsItem = lhs->GetDownloadItem();
    if (lhsItem.priority == rhsItem.priority) {
      return lhsItem.url < rhsItem.url;
    } else {
      return lhsItem.priority < rhsItem.priority;
    }
  }
};

class Transaction {
 public:
  Transaction(const DownloadGroupRequest&, int, std::shared_ptr<ChannelBrokerBase>);
  Transaction(const DownloadGroupIdRequest&, int, std::shared_ptr<ChannelBrokerBase>);
  Transaction() = default;

  const std::string& GetId() const;
  const std::set<DownloadGroupItemWrapperPtr, DownloadItemGroupComparator>& GetDownloadItems() const;
  TransactionalState GetState() const;
  int GetLastErrorCode() const;
  std::chrono::milliseconds GetCreateTime() const;
  std::vector<DownloadGroupItem> GetDownloadGroupItems() const;
  adustring GetDecompressLocation() const;
  void SetLastErrorCode(int errorCode);
  void SetState(TransactionalState state);
  bool ContainsItem(const std::string& id);
  void ItemProgressChanged();

 private:
  std::string m_downloadGroupId;
  adustring m_decompressLocation;
  std::set<DownloadGroupItemWrapperPtr, DownloadItemGroupComparator> m_downloadItems;
  TransactionalState m_transactionalState{TransactionalState::TRANSACTION_NEW};
  std::chrono::milliseconds m_createTime;
  int m_lastErrorCode{};
  std::mutex m_progressMutex;
  std::shared_ptr<ChannelBrokerBase> m_channelBroker;
};
typedef std::shared_ptr<Transaction> TransactionPtr;

#endif
