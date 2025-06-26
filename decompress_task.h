//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright 2019 by Autodesk, Inc.
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
// Name:        decompress_task.h
// Description: Class that manages the Decompress task

#ifndef DECOMPRESS_TASK_H_
#define DECOMPRESS_TASK_H_

#include "common_typedefs.h"
#include "common_utils.h"
#include "task.h"
#include "transaction.h"

typedef std::shared_ptr<ProcessHandle> ProcessHandlePtr;

class BaseDecompressExternalCalls {
 public:
  virtual bool ExtCreatePathIfNotExist(const adustring& path) = 0;
  virtual ProcessHandlePtr ExtSystemCommand(const adustring& cmd) = 0;
  virtual ProcessHandlePtr ExtSystemCommand(const adustring& cmd0, const adustring& cmd1) = 0;
  virtual ~BaseDecompressExternalCalls() {}
};

class DecompressExternalCalls : public BaseDecompressExternalCalls {
 public:
  virtual bool ExtCreatePathIfNotExist(const adustring& path) override;
  virtual ProcessHandlePtr ExtSystemCommand(const adustring& cmd) override;
  virtual ProcessHandlePtr ExtSystemCommand(const adustring& cmd0, const adustring& cmd1) override;
};

class DecompressTask : public Task {
 public:
  DecompressTask(const DownloadGroupItem& item, const adustring& decompressLocation = "");

  /**
   * DecompressTask constructor - to be used only for unit testing.
   * @param DownloadGroupItem
   * @param BaseDecompressExternalCalls object
   */
  DecompressTask(const DownloadGroupItem& item, const std::shared_ptr<BaseDecompressExternalCalls>& externalCalls,
                 const adustring& decompressLocation = "");

  ~DecompressTask() = default;
  const std::string& GetId() const override;
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

  virtual TaskProgress GetTaskProgress() const override;

 private:
  enum FileType { COMPRESSED, UNCOMPRESSED, INVALID };
  int DoStop(TaskState);
  void SetState(TaskState);
  void SendNotification(const std::string&, unsigned long long, int);
  FileType IsValidExtension(const adustring&);
 private:
  DownloadGroupItem m_downloadGroupItem;
  adustring m_decompressLocation;
  std::atomic<TaskState> m_state{TaskState::NEW};
  std::shared_ptr<BaseDecompressExternalCalls> m_externalCalls;
  ProcessHandlePtr m_decompressProcess{nullptr};
  int m_lastBlockNumber{0};
  std::mutex m_stateMutex;
  std::condition_variable m_stateConditionVar;
};

#endif
