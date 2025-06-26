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
// Name:        task_progress_listener.h
// Description: Interface for classes interested in listening to progress changes

#ifndef TASK_PROGRESS_LISTENER_H_
#define TASK_PROGRESS_LISTENER_H_

class Task;

class TaskProgressListener {
 public:
  virtual void TaskProgressChanged(const Task& task) = 0;
  virtual ~TaskProgressListener() {}
};

typedef std::shared_ptr<TaskProgressListener> TaskProgressListenerPtr;

#endif