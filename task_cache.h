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
// Name:        task.h
// Description: Class to define the interface exposed by any task

#ifndef TASK_CACHE_H_
#define TASK_CACHE_H_

#include "task.h"
#include "transaction.h"
#include "common_typedefs.h"
#include <map>

class TaskCache {
 public:
  virtual TaskPtr CreateDownloadTask(const DownloadGroupItem&);
  virtual TaskPtr CreateDecompressTask(const DownloadGroupItem&, const adustring& = "");
  virtual TaskPtr GetTask(const DownloadGroupItem&);
  virtual void RemoveTask(const Task&);
  virtual void Clear();
  virtual bool IsEmpty();
  virtual ~TaskCache() {}

protected:
  void AddTaskToCache(const TaskPtr& task);
  std::map<std::string, TaskPtr> m_downloadTasks;
  std::map<std::string, TaskPtr> m_decompressTasks;
};

#endif
