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
// Name:        task_progress_provider.h
// Description: Common code for classes that provide progress notifications

#ifndef TASK_PROGRESS_PROVIDER_H_
#define TASK_PROGRESS_PROVIDER_H_

#include <set>
#include <thread>

#include "task_progress_listener.h"

class TaskProgressProvider {
 public:
  void RegisterProgressListener(const TaskProgressListenerPtr& listener) {
    m_listeners.insert(const_cast<TaskProgressListenerPtr&>(listener));
  }

  void DeregisterProgressListener(const TaskProgressListenerPtr& listener) {
    m_listeners.erase(listener); 
  }

 protected:
  void NotifyProgress(const Task& task) {
    for (TaskProgressListenerPtr listener : m_listeners) {
      listener->TaskProgressChanged(task);
    }
  }

 private:
  std::set<TaskProgressListenerPtr> m_listeners;
};

#endif
