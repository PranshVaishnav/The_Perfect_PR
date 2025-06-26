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
// Name:        task_scheduler.cpp
// Description: Defines class to handle messages received and perform actions

#include "task_scheduler.h"
#include "common_utils.h"
#include "download_protocol.h"
#include "file_utils.h"
#include "string_utils.h"
#include "task.h"

#ifdef ENABLE_LOG_
#include "logger.h"
#endif

using namespace AdResourceMonitor;

TaskScheduler::TaskScheduler() {}

void TaskScheduler::InitResourceOptimizer() {
  ResourceMonitor& resourceMonitor = ResourceMonitor::GetResourceMonitorObject();
  resourceMonitor.RegisterCallback(std::static_pointer_cast<INetworkObserver>(shared_from_this()));
  resourceMonitor.RegisterCallback(std::static_pointer_cast<ICPUObserver>(shared_from_this()));
  m_trafficControl.StartReporter();
  m_trafficControl.sigDownloadPool.connect([this](aduint count) { SetDownloadThreadCount(count); });
  m_trafficControl.sigDecompressPool.connect([this](aduint count) { SetDecompressSSDThreadCount(count); });
}

/*virtual*/ void TaskScheduler::NetworkSpeedUpdated(adull currentSpeed) /*override*/ {
  LOG_INFO("current network speed is : ", currentSpeed);
  m_trafficControl.SubmitDownloadThreadPoolProperties(m_downloadThreadPool.GetThreadCount(),
                                                      m_downloadThreadPool.GetActiveThreadCount(),
                                                      m_downloadThreadPool.GetPendingJobsCount());
  m_trafficControl.SubmitNetworkSpeed(currentSpeed);
}

/*virtual*/ void TaskScheduler::CPUUtilUpdated(unsigned int currentUtilization) /*override*/ {
  LOG_INFO("Current CPU utilization = ", currentUtilization);
  m_trafficControl.SubmitDecompressThreadPoolProperties(m_decompressSSDThreadPool.GetThreadCount(),
                                                        m_decompressSSDThreadPool.GetPendingJobsCount());
  m_trafficControl.SubmitCPUUtilization(currentUtilization);
}

void TaskScheduler::AddTask(const std::shared_ptr<Task>& task) {
  if (task->GetType() == TaskType::DownloadTask) {
    m_downloadThreadPool.Push(task->GetPriority(), std::bind(&TaskScheduler::ExecuteInThreadPool, this, task));
  } else if (task->GetType() == TaskType::DecompressTask) {
    m_decompressSSDThreadPool.Push(task->GetPriority(), std::bind(&TaskScheduler::ExecuteInThreadPool, this, task));
  }
}

void TaskScheduler::AddHDDDecompressTask(const std::shared_ptr<Task>& task) {
  if (task->GetType() == TaskType::DecompressTask) {
    m_decompressHDDThreadPool.Push(task->GetPriority(), std::bind(&TaskScheduler::ExecuteInThreadPool, this, task));
  }
}

void TaskScheduler::ExecuteInThreadPool(const std::weak_ptr<Task>& wpTask) {
  if (auto spTask = wpTask.lock()) {
    if (spTask->GetState() == TaskState::NEW) {
      spTask->Start();
    }
  }
}

void TaskScheduler::SetDownloadThreadCount(const int& download_count) {
  m_downloadThreadPool.SetThreadPoolSize(download_count);
}

void TaskScheduler::SetDecompressSSDThreadCount(const int& decompress_count) {
  m_decompressSSDThreadPool.SetThreadPoolSize(decompress_count);
}

void TaskScheduler::PrepareShutdown() {
  m_trafficControl.ShutdownReporter();
  m_downloadThreadPool.PrepareShutdown();
  m_decompressSSDThreadPool.PrepareShutdown();
  m_decompressHDDThreadPool.PrepareShutdown();
  m_trafficControl.ShutdownReporter();
}
