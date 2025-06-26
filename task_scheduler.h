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
// Name:        task_scheduler.h
// Description: Declares class to handle messages received and perform actions

#ifndef TASK_SCHEDULER_H_
#define TASK_SCHEDULER_H_

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "command.h"
#include "common_typedefs.h"
#include "downloader_error_codes.h"
#include "dynamic_thread_pool.h"
#include "resource_monitor.h"
#include "traffic_control.h"
#include "transaction.h"

class Task;

class TaskScheduler : public AdResourceMonitor::INetworkObserver,
                      public AdResourceMonitor::ICPUObserver,
                      public std::enable_shared_from_this<TaskScheduler> {
 public:
  TaskScheduler();

  /**
   * Initializes resource optimizer
   */
  void InitResourceOptimizer();

  /**
   * Function to be called back when Network speed is updated
   * @param current network speed after update
   */
  virtual void NetworkSpeedUpdated(adull currentSpeed) override;

  /**
   * Function to be called back when CPU speed is updated
   * @param current CPU speed after update
   */
  virtual void CPUUtilUpdated(unsigned int currentUtilization) override;

  void AddTask(const std::shared_ptr<Task>& task);
  void AddHDDDecompressTask(const std::shared_ptr<Task>& task);
  void PrepareShutdown();

  void SetDownloadThreadCount(const int& download_count);
  void SetDecompressSSDThreadCount(const int& decompress_count);

 private:

  void ExecuteInThreadPool(const std::weak_ptr<Task>& wpTask);
  DynamicThreadPool m_downloadThreadPool;
  DynamicThreadPool m_decompressHDDThreadPool;
  DynamicThreadPool m_decompressSSDThreadPool;
  TrafficControl m_trafficControl;
};

typedef std::shared_ptr<TaskScheduler> TaskSchedulerPtr;

#endif
