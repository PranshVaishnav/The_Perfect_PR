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
// Name:        decompress_task.cpp
// Description: Class that manages the Decompress task

#include <algorithm>
#include <boost/dll.hpp>

#include "decompress_task.h"
#include "error_codes.h"
#include "file_utils.h"
#include "logger.h"
#include "string_utils.h"

const adustring kCompressExtension = ".tar.xz";
const adustring kNonCompressExtension = ".tar";
const adustring kPackageBuilderProcess = "PackageBuilder";
const adustring kProgressValueIdentifier = "Bytes processed:";
#if WIN32
const char kPercentIndicator = '%';
#endif

bool DecompressExternalCalls::ExtCreatePathIfNotExist(const adustring& path) {
  std::wstring wPath(StringUtils::GetWStringFromString(path));
  bool isPathCreated(false);
  bool bRet = FileUtil::CreatePathIfNotExist(wPath, isPathCreated);
  if (bRet && isPathCreated)
      boost::filesystem::permissions(wPath, boost::filesystem::all_all);
  return bRet;
}

ProcessHandlePtr DecompressExternalCalls::ExtSystemCommand(const adustring& cmd) {
  try {
    LOG_INFO("Executing decompression command: ", cmd);
    return std::make_shared<BoostProcessHandle>(cmd);
  } catch (const std::exception& ex) {
    LOG_ERROR("Error trying to execute: (", cmd, "). Details:", ex.what());
    return std::make_shared<BoostProcessHandle>("");
  }
}

ProcessHandlePtr DecompressExternalCalls::ExtSystemCommand(const adustring& cmd0, const adustring& cmd1) {
#if WIN32
  try {
    std::size_t hashVal = std::hash<adustring>{}(cmd0);
    LOG_INFO("Executing decompression command0 hash value: ", hashVal);
    LOG_INFO("Executing decompression command0: ", cmd0, " ; command1: ", cmd1);
    auto processHandle = std::make_shared<ChainedBoostProcessHandle2>(std::to_string(hashVal), cmd0, cmd1);
    if (processHandle == nullptr || !processHandle->Valid()) {
      throw std::runtime_error("Failed to execute chained decompression commands.");
    }
    return processHandle;
  } catch (const std::exception& ex) {
    LOG_ERROR("Error trying to execute: (command0 :", cmd0, " ; command1 :", cmd1, "). Details:", ex.what());
    return std::make_shared<BoostProcessHandle>("");
  }
#else
  return std::make_shared<BoostProcessHandle>("");
#endif
}

DecompressTask::DecompressTask(const DownloadGroupItem& item, const adustring& decompressLocation /*=""*/)
    : m_downloadGroupItem{item}, m_decompressLocation{decompressLocation} {
  m_externalCalls = std::make_shared<DecompressExternalCalls>();
}

DecompressTask::DecompressTask(const DownloadGroupItem& item,
                               const std::shared_ptr<BaseDecompressExternalCalls>& externalCalls,
                               const adustring& decompressLocation /*= ""*/)
    : m_downloadGroupItem{item}, m_decompressLocation{decompressLocation}, m_externalCalls{externalCalls} {}

const std::string& DecompressTask::GetId() const { return m_downloadGroupItem.GetId(); }

double GetPercentComplete(const std::string& outputString, unsigned long long totalSize) {
  double percentComplete = -1.0;
#if WIN32
  bool isProgressReport = outputString.length() > 1 && outputString[outputString.length() - 1] == kPercentIndicator;
  if (isProgressReport) {
    try {
      std::string percentNumber = outputString.substr(0, outputString.length() - 1);
      percentComplete = boost::lexical_cast<int>(percentNumber);
      percentComplete /= 100;
    } catch (boost::bad_lexical_cast&) {
    }
  }
#else
  auto index = outputString.find(kProgressValueIdentifier);
  if (index != std::string::npos) {
    auto size = std::stoull(outputString.substr(index + kProgressValueIdentifier.length()));
    percentComplete = (totalSize <= 0) ? 0.0 : double(size) / totalSize;
  }
#endif
  return percentComplete;
}

void DecompressTask::SendNotification(const std::string& outputString, unsigned long long totalSize, int totalBlocks) {
  double percentComplete = GetPercentComplete(outputString, totalSize);
  if (percentComplete >= 0.0) {
    int completedBlocks = int(totalBlocks * percentComplete);
    completedBlocks = std::min<int>(completedBlocks, totalBlocks - 1);
    if (completedBlocks != m_lastBlockNumber) {
      m_lastBlockNumber = completedBlocks;
      NotifyProgress(*this);
    }
  }
}

DecompressTask::FileType DecompressTask::IsValidExtension(const adustring&) {
  if (FileUtil::HasExtension(m_downloadGroupItem.fileLocation, kCompressExtension)) {
    return DecompressTask::FileType::COMPRESSED;
  } else if (FileUtil::HasExtension(m_downloadGroupItem.fileLocation, kNonCompressExtension)){
    return DecompressTask::FileType::UNCOMPRESSED;
  }
  return DecompressTask::FileType::INVALID;
}

void DecompressTask::SetState(TaskState newState) {
  std::lock_guard<std::mutex> lock(m_stateMutex);
  m_state = newState;
}

int DecompressTask::Start() {
  int errorCode = kNoError;

  const std::string sourceLocation = FileUtil::GetParentPath(m_downloadGroupItem.fileLocation);
  const std::string decompressLocation = m_decompressLocation.empty() ? sourceLocation : m_decompressLocation;
  DecompressTask::FileType fileType = IsValidExtension(m_downloadGroupItem.fileLocation);
  if (fileType != DecompressTask::FileType::INVALID) {
    if (m_externalCalls->ExtCreatePathIfNotExist(decompressLocation)) {
#if WIN32
      std::wstring zpath = boost::dll::program_location().wstring();
      adustring exe =
          (boost::format("\"%1%/7za.exe\"") % StringUtils::GetStringFromWString(FileUtil::GetParentPath(zpath))).str();
      adustring stage1;
      adustring stage2;
      if (fileType == DecompressTask::FileType::COMPRESSED) {
        stage1 = (boost::format("x -txz \"%1%\" -bsp2 -so") % m_downloadGroupItem.fileLocation).str();
        stage2 = (boost::format("x -ttar -si -aoa -bsp2 -o\"%1%\"") % decompressLocation).str();
      } else {
        stage2 = (boost::format("x -ttar \"%1%\" -aoa -bsp2 -o\"%2%\"") % m_downloadGroupItem.fileLocation %
                  decompressLocation)
                     .str();
      }

#else
      adustring decompressCommand = FileUtil::GetParentPath("\"" + boost::dll::program_location().string()) + "/" +
                                    kPackageBuilderProcess + "\" \"" + m_downloadGroupItem.fileLocation + "\" \"" +
                                    decompressLocation + "\" -p";
#endif

      std::unique_lock<std::mutex> lock(m_stateMutex);
      if (m_state == TaskState::NEW) {
        m_state = TaskState::RUNNING;

#if WIN32
        if (fileType == DecompressTask::FileType::COMPRESSED) {
          m_decompressProcess = m_externalCalls->ExtSystemCommand(exe + " " + stage1, exe + " " + stage2);
        } else {
          m_decompressProcess = m_externalCalls->ExtSystemCommand(exe + " " + stage2);
        }

#else
        m_decompressProcess = m_externalCalls->ExtSystemCommand(decompressCommand);
#endif

      } else {
        m_decompressProcess = std::make_shared<InvalidProcessHandle>();
      }
      lock.unlock();

      if (m_decompressProcess->Valid()) {
        while (m_decompressProcess->Valid() && m_decompressProcess->Running()) {
          std::string output = m_decompressProcess->GetOutput();
          SendNotification(output, m_downloadGroupItem.uncompressedFileSize, m_downloadGroupItem.totalBlock);
        }
        m_decompressProcess->Wait();
      } else {
        SetState(TaskState::STOPPED);
      }
      errorCode = m_decompressProcess->ExitCode();
    } else {
      LOG_ERROR("Invalid decompress location");
      errorCode = -1;
    }
  } else {
    LOG_INFO("Decompression not supported for this file type. Ignoring.");

    if (decompressLocation != sourceLocation) {
      m_externalCalls->ExtCreatePathIfNotExist(decompressLocation);

      boost::filesystem::path dest = StringUtils::GetWStringFromString(decompressLocation);
      boost::filesystem::path fileName = boost::filesystem::path(StringUtils::GetWStringFromString(m_downloadGroupItem.fileLocation)).filename();
      dest /= fileName;

      try {
        //boost::filesystem::copy_file(m_downloadGroupItem.fileLocation, dest, boost::filesystem::copy_option::overwrite_if_exists);
        boost::filesystem::copy_file(m_downloadGroupItem.fileLocation, dest, boost::filesystem::copy_options::overwrite_existing);
      } catch (const boost::filesystem::filesystem_error& e) {
        LOG_ERROR("Copy file into download location failed. What = ", e.what());
      }
    }
  }

  if (errorCode == kNoError) {
    SetState(TaskState::COMPLETED);
    m_downloadGroupItem.decompressedBlock = m_downloadGroupItem.totalBlock;
  } else {
    if (m_state == TaskState::RUNNING || m_state == TaskState::NEW) {
      LOG_ERROR("Decompress failed, return code = ", errorCode);
      m_downloadGroupItem.errorCode = AdDownloadStaticLib::kDECOMPRESS_FAILURE;
      SetState(TaskState::STOPPED);
    }
    m_downloadGroupItem.decompressedBlock = 0;
  }

  if (m_state == TaskState::COMPLETED) {
    NotifyProgress(*this);
  }
  m_stateConditionVar.notify_one();
  NotifyStateChange(*this);
  return errorCode;
}

int DecompressTask::DoStop(TaskState newState) {
  std::unique_lock<std::mutex> lock(m_stateMutex);
  if (m_state == TaskState::RUNNING && m_decompressProcess && m_decompressProcess->Valid()) {
    if (m_decompressProcess->Running()) {
      m_state = newState;
      m_decompressProcess->Stop();
      m_stateConditionVar.wait(lock, [this]() { return m_state != TaskState::RUNNING; });
    }
  }
  m_state = newState;
  m_lastBlockNumber = 0;
  return 0;
}

int DecompressTask::Stop() {
  DoStop(TaskState::STOPPED);
  return 0;
}

int DecompressTask::Pause() {
  DoStop(TaskState::PAUSED);
  return 0;
}

int DecompressTask::Resume() { return Start(); }

int DecompressTask::Cancel() {
  DoStop(TaskState::CANCELED);
  return 0;
}

unsigned long long DecompressTask::GetPriority() const { return m_priority; }

void DecompressTask::SetPriority(unsigned long long priority) { m_priority = priority; }

DownloadGroupItem DecompressTask::GetDownloadGroupItem() const { return m_downloadGroupItem; }

void AddParent(TransactionPtr) {}

TaskType DecompressTask::GetType() const { return TaskType::DecompressTask; }

TaskState DecompressTask::GetState() const { return m_state; }

TaskProgress DecompressTask::GetTaskProgress() const {
  TaskProgress taskProgess(m_lastBlockNumber, m_downloadGroupItem.totalBlock);
  if (m_state == TaskState::COMPLETED) {
    taskProgess.SetCompleted(m_downloadGroupItem.totalBlock);
  }
  return taskProgess;
}
