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
// Name:        download_group_item_wrapper.h
// Description: Declares all internal class for representing transaction\task

#ifndef DOWNLOAD_GROUP_ITEM_WRAPPER_
#define DOWNLOAD_GROUP_ITEM_WRAPPER_

#include <mutex>
#include "task.h"

class Transaction;

class DownloadGroupItemWrapper : public TaskProgressListener,
  public std::enable_shared_from_this<DownloadGroupItemWrapper> {
public:
  DownloadGroupItemWrapper(Transaction&, const DownloadGroupItem&);
  ~DownloadGroupItemWrapper();
  void TaskProgressChanged(const Task&);
  void SetTask(TaskPtr& taskPtr);
  DownloadGroupItem& GetDownloadItem() const;

private:
  void Deregister();

private:
  Transaction& m_transaction;
  DownloadGroupItem m_downloadItem;
  TaskPtr m_taskPtr{ nullptr };
  std::mutex m_progressMutex;
};

typedef std::shared_ptr<DownloadGroupItemWrapper> DownloadGroupItemWrapperPtr;

#endif