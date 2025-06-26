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
// Name:        task_state_change_listener.h
// Description: Interface for classes interested in listening to state changes

#ifndef TASK_STATE_LISTENER_H_
#define TASK_STATE_LISTENER_H_

#include "task.h"

class TaskStateListener {
 public:
  virtual void TaskStateChanged(const Task& task) = 0;
  virtual ~TaskStateListener() {}
};

typedef std::shared_ptr<TaskStateListener> TaskStateListenerPtr;

#endif