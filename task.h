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

#ifndef TASK_H_
#define TASK_H_

#include <string>
#include "download_protocol.h"
#include "task_progress_provider.h"
#include "task_state_change_provider.h"

enum class TaskType { DownloadTask = 0, DecompressTask };
enum class TaskState { RUNNING = 0, COMPLETED, STOPPED, PAUSED, CANCELED, NEW };

class TaskProgress {
 public:
  TaskProgress(int completed, int total) : m_completed(completed), m_total(total) { UpdatePercentage(); }

  void SetCompleted(int completed) {
    m_completed = completed;
    UpdatePercentage();
  }

  void SetTotal(int total) {
    m_total = total;
    UpdatePercentage();
  }

  float GetPercentage() const { return m_percentage; }
  int GetCompleted() const { return m_completed; }
  int GetTotal() const { return m_total; }

 private:
  void UpdatePercentage() {
    m_percentage = (m_total == 0) ? 0.0f : (static_cast<float>(m_completed) / static_cast<float>(m_total)) * 100;
  }

 private:
  float m_percentage{0.0};
  int m_completed{0};
  int m_total{0};
};

class Task : public TaskStateChangeProvider, public TaskProgressProvider {
 public:
  Task() = default;
  virtual ~Task() {}

  virtual const std::string& GetId() const = 0;
  virtual int Start() = 0;
  virtual int Stop() = 0;
  virtual int Pause() = 0;
  virtual int Resume() = 0;
  virtual int Cancel() = 0;
  virtual unsigned long long GetPriority() const = 0;
  virtual void SetPriority(unsigned long long priority) = 0;
  virtual DownloadGroupItem GetDownloadGroupItem() const = 0;
  virtual TaskState GetState() const = 0;
  virtual TaskType GetType() const = 0;
  virtual TaskProgress GetTaskProgress() const = 0;

 protected:
  unsigned long long m_priority;
};

typedef std::shared_ptr<Task> TaskPtr;

#endif
