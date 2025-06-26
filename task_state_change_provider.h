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
// Name:        task_state_change_provider.h
// Description: Common code for classes that provide state change notifications

#ifndef TASK_STATE_CHANGE_PROVIDER_H_
#define TASK_STATE_CHANGE_PROVIDER_H_

#include <thread>
#include <set>

#include "task.h"
#include "task_state_listener.h"

class TaskStateChangeProvider {
 public:
  void RegisterStateChangeListener(const TaskStateListenerPtr& listener) {
    m_listeners.insert(const_cast<TaskStateListenerPtr&>(listener));
  }

  void DeregisterStateChangeListener(const TaskStateListenerPtr& listener) {
    m_listeners.erase(listener);
  }

 protected:
  void NotifyStateChange(const Task& task) {
    for (const TaskStateListenerPtr& listener : m_listeners) {
      listener->TaskStateChanged(task);
    }
  }

 private:
  std::set<TaskStateListenerPtr> m_listeners;
};

#endif
