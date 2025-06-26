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
// Name:        download_task.h
// Description: Class that manages the lifecycle of a Download command

#ifndef DOWNLOAD_TASK_H_
#define DOWNLOAD_TASK_H_

#include "task.h"
#include "transaction.h"

class DownloadTask : public Task {
 public:
  DownloadTask(const DownloadGroupItem &item);

  /**
   * DownloadTask constructor - to be used only for unit testing.
   * @param DownloadGroupItem
   * @param TaskDownloaderPtr object
   */
  DownloadTask(const DownloadGroupItem &item, AdDownloadStaticLib::TaskDownloaderPtr downloader);

  const std::string &GetId() const override;
  int Start() override;
  int Stop() override;
  int Pause() override;
  int Resume() override;
  int Cancel() override;
  unsigned long long GetPriority() const override;
  void SetPriority(unsigned long long priority) override;

  DownloadGroupItem GetDownloadGroupItem() const override;
  TaskType GetType() const override;
  TaskState GetState() const override;

  ~DownloadTask() override;

  void OnStateChange(const int state);
  void OnProgress(const DownloadGroupItem &item);

  int CreateItemToDownload();

  virtual TaskProgress GetTaskProgress() const override;

 private:
  void DeleteIncompleteFile();

 private:
  DownloadGroupItem m_downloadGroupItem;
  AdDownloadStaticLib::TaskDownloaderPtr m_downloader;
  TaskState m_state{TaskState::NEW};
  std::mutex m_stateMutex;
  std::condition_variable m_stateConditionVar;
};

typedef std::shared_ptr<DownloadTask> DownloadTaskPtr;

#endif
