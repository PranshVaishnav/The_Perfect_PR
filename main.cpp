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
// Name:        main.cpp
// Description: Main function to run as executable and perform action based on
//              arguments received.

#include <boost/compute/detail/getenv.hpp>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <iostream>
#include <regex>

#if defined(__linux__) || defined(__APPLE__)
#include <signal.h>
#endif

#include <../source/cer_utils/cer_wrapper.h>
#include "command_cache.h"
#include "downloader.h"
#include "error_codes.h"
#include "ipc_request_channel.h"
#include "logger.h"
#include "secure_settings_api_wrapper.h"
#include "task_scheduler.h"
#include "transaction_manager.h"
#ifdef _WIN32
#include "dda_utils/file_utils.h"
#include "dda_utils/path_utils.h"
#include "dda_utils/string_utils.h"
#include "xplat/xplat_feature_const.h"
#include "xplat/xplat_feature_manager.h"
#endif
#include <boost/algorithm/string.hpp>
#include "dlm_version.h"
#include "string_utils.h"

using namespace Autodesk::SecureSettings;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace dll = boost::dll;

const std::string kExec = "exec";
const std::string kUrl = "url";
const std::string kFilename = "filename";
const std::string kDownloadfolder = "downloadfolder";
const std::string kFullpath = "fullpath";
const std::string kDownload = "download";
const std::string kNoOptimizer = "nooptimizer";
const std::string kDownloadWorker = "downloadworker";
const std::string kDecompressWorker = "decompressworker";
const std::string kMaxDownloadTimeInS = "maxtime";
const std::string kIPCChannelName = "IPCChannelName";
const std::string kLogLevel = "LogLevel";
const std::string kLogPath = "LogPath";
const std::string kSilent = "silent";
const std::string kProductName = "productname";
const std::string kProductVersion = "productversion";
const std::string kFileCheckSum = "fileChecksum";
const std::string kEnableVerboseLog = "ADDOWNLOADLIB_FORCE_VERBOSE_LOG";

class ScopeLogger {
 public:
  ScopeLogger() { m_opened = false; }
  void InitLog(const std::string& fileLocation, int level) {
    Logger::Instance().AddLogFile(L"DLM.log", Autodesk::DDAUtils::StringUtils::StringToWString(fileLocation));
    Logger::Instance().ToggleLogLevel(static_cast<LOG_LEVEL>(level));
    LOG_INFO("Download Manager Execution - Begin.");
    m_opened = true;
  }
  ~ScopeLogger() {
    if (m_opened) {
      LOG_INFO("Download Manager Execution - End.");
      m_opened = false;
    }
  }
  bool LogOpened() { return m_opened; }

 private:
  bool m_opened;
};

int PerformURLDownload(const std::shared_ptr<AdDownloadStaticLib::DefaultDownloader>& downloader) {
  int retVal = AdDownloadStaticLib::kNO_ERROR;
  if (downloader == nullptr) {
    return AdDownloadStaticLib::kDOWNLOAD_INIT_FAILURE;
  }

  retVal = downloader->Open();
  if (retVal != AdDownloadStaticLib::kNO_ERROR) {
    LOG_ERROR("Failed to initialize. Error occured: ", retVal);
    return retVal;
  }
  retVal = downloader->Wait();
  if (retVal != AdDownloadStaticLib::kNO_ERROR) {
    LOG_ERROR("Failed to download. Error occured: ", retVal);
    return retVal;
  }
  return retVal;
}

bool CheckIfDownloadRequired(const adustring& fileLocation, const adustring& fileChksum) {
  adustring calcChksum;
  GetSHA256Hash(fileLocation, calcChksum);
  if (boost::iequals(calcChksum, fileChksum)) {
    LOG_INFO("checksum for ", fileLocation, " exist. Skip downloading");
    return true;
  } else {
    LOG_INFO("checksum for ", fileLocation, " does not exist. Continue downloading.");
  }
  return false;
}

// POSIX shells canot handle exit codes not in range 0-255
// Translate error code to squeeze in this range.
// CURL error (1001 - 1094) -> (101 - 194)
// DLM error (-101 - -199) -> (1 - 99)
// HTTP error (2000 - 2999) -> 200
int TranslateToExitCode(int errorCode) {
  int exitCode = errorCode;
  if (errorCode > 1000 && errorCode < 1099) {
    exitCode = errorCode - 900;
  } else if (errorCode < -100 && errorCode > -199) {
    exitCode = std::abs(errorCode) - 100;
  } else if (errorCode >= 2000) {
    exitCode = 200;
  }
  LOG_INFO("Translating error code ", errorCode, " to ", exitCode);
  return exitCode;
}

#if WIN32
#define WSTRING_TYPE_VALUE po::wvalue<std::wstring>()
int wmain(int argc, wchar_t* argv[]) {
#else
#define WSTRING_TYPE_VALUE po::value<std::string>()
int main(int argc, char** argv) {
#endif
  ScopeLogger log;
  int loglvl = LOG_LEVEL::info;
#ifdef _DEBUG
  loglvl = LOG_LEVEL::trace;
#endif

#if defined(__linux__) || defined(__APPLE__)
  signal(SIGPIPE, SIG_IGN);
#endif

  std::string logpath;
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  adstring ipcChannelName = kDLMChannelName;
  boost::filesystem::path rootPath = boost::dll::program_location().parent_path();
  Autodesk::DDA::CERWrapper::Instance()->Init("DLM", rootPath.native());
#if _WIN32
  auto currentRootPath = Autodesk::DDA::PathUtils::GetCurrentModuleDirectory();
  AdDataType::adOSString secureSettingLibPathStr = currentRootPath.native();
  SecureSettingsAPI::GetInstance().Initialize(secureSettingLibPathStr);
#endif  // _WIN32
  try {
    po::options_description desc("Usage");
    auto&& addOptions = desc.add_options();
    addOptions("help,h", "print help message.");
    addOptions("exec,e", "run download module as a event loop.");
    addOptions((kUrl + std::string(",u")).c_str(), po::value<std::string>()->required(), "download url.");
    addOptions((kFilename + std::string(",f")).c_str(), po::value<std::string>()->required(), "download file name");
    addOptions((kDownloadfolder + std::string(",r")).c_str(), WSTRING_TYPE_VALUE->required(),
               "download directory that shall be created in temp folder");
    addOptions((kFullpath + std::string(",p")).c_str(), WSTRING_TYPE_VALUE->required(),
               "download file name with full path");
    addOptions(kDownloadWorker.c_str(), po::value<int>()->required(), "download worker count.");
    addOptions(kDecompressWorker.c_str(), po::value<int>()->required(), "decompress worker count.");
    addOptions((kIPCChannelName + std::string(",i")).c_str(), po::value<std::string>()->required(), "IPC Channel Name");
    addOptions((kLogLevel + std::string(",l")).c_str(), po::value<int>()->required(),
               "log level: trace 0, debug 1, info 2, warning 3, error 4, fatal 5");
    addOptions((kLogPath + std::string(",g")).c_str(), po::value<std::string>()->required(), "DLM log file path");
    addOptions((kNoOptimizer + std::string(",n")).c_str(), "disable resource optimizer");
    addOptions((kSilent + ",s").c_str(), po::bool_switch()->default_value(false), "silent mode");
    addOptions(kMaxDownloadTimeInS.c_str(), po::value<int>()->required(), "max total download time in seconds");
    addOptions(kProductName.c_str(), po::value<std::string>()->required(), "the name of product calling DLM");
    addOptions(kProductVersion.c_str(), po::value<std::string>()->required(), "the version of product calling DLM");
    addOptions(kFileCheckSum.c_str(), po::value<std::string>()->required(), "checksum of file to download");

    po::variables_map vm;
#if WIN32
    po::store(po::wcommand_line_parser(argc, argv).options(desc).run(), vm);
#else
    po::store(po::parse_command_line(argc, argv, desc), vm);
#endif

    if (vm.count("help") || argc < 2) {
      std::cout << desc << std::endl;
      return 0;
    }

    if (vm[kSilent].as<bool>()) {
      Autodesk::DDA::CERWrapper::Instance()->Mute();
    }

    if (vm.count(kLogLevel)) {
      int defaultLvl = loglvl;
      loglvl = vm[kLogLevel].as<int>();
      if ((loglvl < LOG_LEVEL::trace) || (loglvl > LOG_LEVEL::fatal)) {
        loglvl = defaultLvl;
      }
    }
    if (vm.count(kLogPath)) {
      logpath = vm[kLogPath].as<std::string>();
    }
    log.InitLog(logpath, loglvl);

    LOG_INFO("DLM Version [", DLMVersion, "]");

#if WIN32
    if (vm.count(kProductName) && vm.count(kProductVersion)) {
      std::string prodName = vm[kProductName].as<std::string>();
      std::string prodVersion = vm[kProductVersion].as<std::string>();
      Autodesk::DDA::CERWrapper::Instance()->SetAppName(prodName);
      LOG_INFO("Caller's productname: ", prodName, " productversion: ", prodVersion);
    }
#endif

    if (vm.count(kExec)) {
      if (vm.count(kIPCChannelName)) {
        ipcChannelName = vm[kIPCChannelName].as<std::string>();
      }
      std::shared_ptr<Channel> channel(ChannelFactory::CreateAsClient(ipcChannelName));
      LOG_INFO("Creating client IPC channel with name: ", ipcChannelName);
      std::shared_ptr<ChannelBrokerBase> channelBroker(std::make_shared<ChannelBroker>(channel));
      std::shared_ptr<TaskScheduler> taskScheduler = std::make_shared<TaskScheduler>();

      if (!vm.count(kNoOptimizer)) {
        taskScheduler->InitResourceOptimizer();
      }

      auto transactionManager = std::make_shared<TransactionManager>(taskScheduler);
      CommandCache commandCache(transactionManager);
      std::shared_ptr<DLMIPCHandler> spRequestHandler = std::make_shared<DLMIPCHandler>();

      if (vm.count(kDownloadWorker)) {
        const int downloadCnt = vm[kDownloadWorker].as<int>();
        taskScheduler->SetDownloadThreadCount(downloadCnt);
      }

      if (vm.count(kDecompressWorker)) {
        const int decompressCnt = vm[kDecompressWorker].as<int>();
        taskScheduler->SetDecompressSSDThreadCount(decompressCnt);
      }

      spRequestHandler->new_request.connect([&](const std::string& request, std::string& response) {
        commandCache.ProcessRequest(request, response, channelBroker);
      });

      transactionManager->break_connection.connect([&]() { spRequestHandler->BreakConnection(); });

      channel->AddReactor(spRequestHandler);
      if (ChannelFeatureController* channelController = dynamic_cast<ChannelFeatureController*>(channel.get())) {
        channelController->SetSkipUnlinkServerSocket(true);
      } else {
        LOG_ERROR("Casting from Channel to ChannelFeatureController failed.");
      }
      channel->Connect();
      spRequestHandler->WaitForConnection();
      taskScheduler->PrepareShutdown();
      transactionManager->StopAllTransactions();
      taskScheduler.reset();
      channel->Destroy();
      LOG_INFO("Download manager exit...");
      return TranslateToExitCode(0);
    }

    if (!SecureSettingsAPI::GetInstance().IsInternetConnectionAllowed()) {
      LOG_ERROR("Internet connection is disallowed by trust");
      exit(1);
    }

    int errorCode = 0;

    const int maxTime = vm.count(kMaxDownloadTimeInS) ? vm[kMaxDownloadTimeInS].as<int>() : 0;
#ifdef WIN32
    bool bVerboseDebugLog = boost::compute::detail::getenv(kEnableVerboseLog.c_str()) != nullptr;
#else
    bool bVerboseDebugLog = std::getenv(kEnableVerboseLog.c_str()) != nullptr;
#endif

    if (vm.count(kUrl) && vm.count(kFullpath)) {
      const std::string url = vm[kUrl].as<std::string>();

#if WIN32
      const std::wstring wfullpath = vm[kFullpath].as<std::wstring>();
      const std::string fullpath = Autodesk::DDAUtils::StringUtils::OSStringToString(wfullpath);
#else
      const std::string fullpath = vm[kFullpath].as<std::string>();
#endif
      if (vm.count(kFileCheckSum)) {
        adustring fileChksum = vm[kFileCheckSum].as<adustring>();
        adustring fileLocation = fullpath;
        if (CheckIfDownloadRequired(fileLocation, fileChksum)) {
          return TranslateToExitCode(0);
        }
      }

      LOG_INFO("Performing URL download, url: ", url, " fullpath:", fullpath, " maxTime: ", maxTime);
      auto pd = std::make_shared<AdDownloadStaticLib::DefaultDownloader>(
          url, fullpath, maxTime, bVerboseDebugLog ? &(Logger::Instance()) : nullptr);
      errorCode = PerformURLDownload(pd);
      if (errorCode == AdDownloadStaticLib::kDOWNLOAD_TIMEOUT) {
        LOG_ERROR("Download timeout happened, exiting download.");
        exit(1);
      }
      return TranslateToExitCode(errorCode);
    } else if (vm.count(kUrl) && vm.count(kFilename) && vm.count(kDownloadfolder)) {
      std::regex validFolderRegex("^[a-zA-Z0-9_.-]*$");

#if WIN32
      const std::wstring wdownloadfolder = vm[kDownloadfolder].as<std::wstring>();
      const std::string downloadfolder = Autodesk::DDAUtils::StringUtils::OSStringToString(wdownloadfolder);
#else
      const std::string downloadfolder = vm[kDownloadfolder].as<std::string>();
#endif

      if (!std::regex_match(downloadfolder, validFolderRegex)) {
        LOG_ERROR("Invalid folder name: ", downloadfolder);
        return TranslateToExitCode(-1);
      }
      const std::string url = vm[kUrl].as<std::string>();
      const std::string filename = vm[kFilename].as<std::string>();
      auto pd = std::make_shared<AdDownloadStaticLib::DefaultDownloader>(
          url, downloadfolder, filename, maxTime, bVerboseDebugLog ? &(Logger::Instance()) : nullptr);
      errorCode = PerformURLDownload(pd);
      if (errorCode == AdDownloadStaticLib::kDOWNLOAD_TIMEOUT) {
        LOG_ERROR("Download timeout happened, exiting download.");
        exit(1);
      }
      return TranslateToExitCode(errorCode);
    }
  } catch (po::error& error) {
    if (!log.LogOpened()) Logger::Instance().AddLogFile(L"DLM.log");
    LOG_ERROR("Error occured. Details: ", error.what());
  }

  return TranslateToExitCode(0);
}