#include "task_cache.h"
#include "decompress_task.h"
#include "download_task.h"
#include "transaction.h"

TaskPtr TaskCache::CreateDownloadTask(const DownloadGroupItem& downloadGroupItem) {
  auto existingDG = m_downloadTasks.find(downloadGroupItem.GetId());
  if (existingDG != m_downloadTasks.end()) {
    return existingDG->second;
  }
  auto downloadTask = std::make_shared<DownloadTask>(downloadGroupItem);
#ifndef _LINUX
  downloadTask->CreateItemToDownload();
#endif
  AddTaskToCache(downloadTask);
  return downloadTask;
}

TaskPtr TaskCache::CreateDecompressTask(const DownloadGroupItem& downloadGroupItem,
                                        const adustring& decompressLocation /*=""*/) {
  auto existingTask = m_decompressTasks.find(downloadGroupItem.GetId());
  if (existingTask != m_decompressTasks.end()) {
    return existingTask->second;
  }
  auto decompressTask = std::make_shared<DecompressTask>(downloadGroupItem, decompressLocation);
  AddTaskToCache(decompressTask);
  return decompressTask;
}

void TaskCache::AddTaskToCache(const TaskPtr& task) {
  if (task->GetType() == TaskType::DownloadTask) {
    LOG_DEBUG("Download Task [", task->GetId(), "] added to cache");
    m_downloadTasks.insert({task->GetId(), task});
  } else {
    LOG_DEBUG("Decompress Task [", task->GetId(), "] added to cache");
    m_decompressTasks.insert({task->GetId(), task});
  }
}

TaskPtr TaskCache::GetTask(const DownloadGroupItem& downloadGroupItem) {
  auto existingTask = m_downloadTasks.find(downloadGroupItem.GetId());
  if (existingTask == m_downloadTasks.end()) {
    existingTask = m_decompressTasks.find(downloadGroupItem.GetId());
    if (existingTask != m_decompressTasks.end()) {
      LOG_DEBUG("Retrieving Decompress Task [", existingTask->second->GetId(), "] from cache");
      return existingTask->second;
    }
  } else {
    LOG_DEBUG("Retrieving Download Task [", existingTask->second->GetId(), "] from cache");
    return existingTask->second;
  }
  return nullptr;
}

void TaskCache::RemoveTask(const Task& task) {
  if (task.GetType() == TaskType::DownloadTask) {
    LOG_DEBUG("Download Task [", task.GetId(), "] removed from cache");
    m_downloadTasks.erase(task.GetId());
  } else if (task.GetType() == TaskType::DecompressTask) {
    LOG_DEBUG("Decompress Task [", task.GetId(), "] removed from cache");
    m_decompressTasks.erase(task.GetId());
  }
}

void TaskCache::Clear() {
  m_downloadTasks.clear();
  m_decompressTasks.clear();
}

bool TaskCache::IsEmpty() { return m_downloadTasks.empty() && m_decompressTasks.empty(); }