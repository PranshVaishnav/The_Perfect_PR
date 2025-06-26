#include "command.h"

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "error_codes.h"
#include "file_utils.h"
#include "logger.h"
#include "secure_settings_api_wrapper.h"
#include "transaction.h"
#include "transaction_manager.h"

using namespace Autodesk::SecureSettings;

static const std::string IgnoreChecksum = "*";

template <typename REQ, typename RES>
bool _Process(const std::string& command, const std::string& request, BasicRequestMessageEnvelope<REQ>& requestMessage,
              std::string& response, BasicResponseMessageEnvelope<RES>& responseMessage, std::function<void()> f) {
  responseMessage.withErrorCode(kNoError);

  if (!Deserialize(request, requestMessage)) {
    responseMessage.withTarget(requestMessage.source)
        .withSource(kDLMSource)
        .withCommand(command)
        .withErrorCode(kErrorInvalidRequest);
    response = Serialize(&responseMessage);
    return false;
  }

  auto errorCode = Validate(requestMessage);
  if (errorCode != kNoError) {
    responseMessage.withTarget(requestMessage.source)
        .withSource(kDLMSource)
        .withCommand(command)
        .withErrorCode(errorCode);
    response = Serialize(&responseMessage);
    return false;
  }

  responseMessage.withTarget(requestMessage.source).withSource(kDLMSource).withCommand(command).withErrorCode(kNoError);
  f();

  response = Serialize(&responseMessage);
  return ((responseMessage.errorCode) ? false : true);
}

bool StartDownloadCommand::Process(const std::string& request, std::string& response,
                                   std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupItemsResponse> responseMessage;

  return _Process(kCommandStartDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

int StartDownloadCommand::Process(const DownloadGroupRequest& request, DownloadGroupItemsResponse& response,
                                  std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  auto downloadGroupItems = transaction->GetDownloadItems();
  int err = kNoError;

  if (SecureSettingsAPI::GetInstance().IsInternetConnectionAllowed()) {
    for (auto itemWrapper : downloadGroupItems) {
      DownloadGroupItem& item = itemWrapper->GetDownloadItem();
      AdDownloadStaticLib::TaskDownloader downloader(item);
      if (item.fileChecksum == IgnoreChecksum) {
        downloader.DeleteDownloadedData();
      }
      item.errorCode = downloader.LoadBlockInfo(item.totalBlock, item.downloadedBlock);
      if (item.errorCode != AdDownloadStaticLib::kNO_ERROR && !item.ignoreError) {
        err = IsInternalLibraryError(item.errorCode) ? item.errorCode : AdDownloadStaticLib::kDOWNLOAD_FATAL_ERROR;
        transaction->SetLastErrorCode(err);
        break;
      } else {
        transaction->SetLastErrorCode(kNoError);
      }
    }

    if (err == kNoError) {
      err = m_transactionManager->StartTransaction(transaction);
    }
  } else {
    err = kErrorInternetConnectionDenied;
    transaction->SetLastErrorCode(err);
  }

  response.downloadGroup = transaction->GetDownloadGroupItems();
  response.downloadGroupId = request.downloadGroupId;
  response.downloadGroupLocation = request.downloadGroupLocation;

  return err;
}

StartDownloadCommand::StartDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

BaseCommand::BaseCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : m_transactionManager{transactionManager} {}

bool BaseCommand::IsInternalLibraryError(int errorCode) { return errorCode < 0; }

bool InitDownloadCommand::Process(const std::string& request, std::string& response,
                                  std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupItemsInitResponse> responseMessage;

  return _Process(kCommandInitDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

InitDownloadCommand::InitDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int InitDownloadCommand::Process(const DownloadGroupRequest& request, DownloadGroupItemsInitResponse& response,
                                 std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  unsigned long long consumedDiskSpace = 0;
  int errorCode = 0;

  if (SecureSettingsAPI::GetInstance().IsInternetConnectionAllowed()) {
    auto downloadGroupItems = transaction->GetDownloadItems();
    for (auto itemWrapper : downloadGroupItems) {
      DownloadGroupItem& item = itemWrapper->GetDownloadItem();
      AdDownloadStaticLib::TaskDownloader downloader(item);
      adustring itemLocation = downloader.DownloadLocation();
#ifndef _LINUX
      if (!itemLocation.empty() && !boost::filesystem::exists(itemLocation)) {
        consumedDiskSpace += item.compressedFileSize;
        if (m_objDiskInformation.GetAvailableSpace(FileUtil::GetParentPath(itemLocation)) <= consumedDiskSpace) {
          errorCode = AdDownloadStaticLib::kNOT_ENOUGH_SPACE;
          return errorCode;
        }
      }
#endif

      item.errorCode = downloader.LoadBlockInfo(item.totalBlock, item.downloadedBlock);
      if (item.errorCode != AdDownloadStaticLib::kNO_ERROR && !item.ignoreError) {
        transaction->SetLastErrorCode(
            IsInternalLibraryError(item.errorCode) ? item.errorCode : AdDownloadStaticLib::kDOWNLOAD_INIT_FAILURE);
        break;
      } else {
        transaction->SetLastErrorCode(kNoError);
      }
    }

    errorCode = transaction->GetLastErrorCode();
  } else {
    errorCode = kErrorInternetConnectionDenied;
    transaction->SetLastErrorCode(errorCode);
  }

  response.downloadGroupId = request.downloadGroupId;
  response.downloadGroup = transaction->GetDownloadGroupItems();

  return errorCode;
}

bool PauseDownloadCommand::Process(const std::string& request, std::string& response,
                                   std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupIdRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupIdResponse> responseMessage;

  return _Process(kCommandPauseDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

PauseDownloadCommand::PauseDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int PauseDownloadCommand::Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
                                  std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  response.downloadGroupId = request.downloadGroupId;
  return m_transactionManager->PauseTransaction(transaction);
}

bool CancelDownloadCommand::Process(const std::string& request, std::string& response,
                                    std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupIdRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupIdResponse> responseMessage;

  return _Process(kCommandCancelDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

CancelDownloadCommand::CancelDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int CancelDownloadCommand::Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
                                   std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  response.downloadGroupId = request.downloadGroupId;
  return m_transactionManager->CancelTransaction(transaction);
}

bool StopDownloadCommand::Process(const std::string& request, std::string& response,
                                  std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupIdRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupIdResponse> responseMessage;

  return _Process(kCommandStopDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

StopDownloadCommand::StopDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int StopDownloadCommand::Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
                                 std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  response.downloadGroupId = request.downloadGroupId;
  return m_transactionManager->StopTransaction(transaction);
}

bool ResumeDownloadCommand::Process(const std::string& request, std::string& response,
                                    std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupIdRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupIdResponse> responseMessage;

  return _Process(kCommandResumeDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

ResumeDownloadCommand::ResumeDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int ResumeDownloadCommand::Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
                                   std::shared_ptr<ChannelBrokerBase>& channel) {
  TransactionPtr transaction = std::make_shared<Transaction>(request, 0, channel);
  response.downloadGroupId = request.downloadGroupId;
  return m_transactionManager->ResumeTransaction(transaction);
}

bool ExitDownloadCommand::Process(const std::string& request, std::string& response,
                                  std::shared_ptr<ChannelBrokerBase>& channel) {
  BasicRequestMessageEnvelope<DownloadGroupIdRequest> requestMessage;
  BasicResponseMessageEnvelope<DownloadGroupIdResponse> responseMessage;

  return _Process(kCommandExitDownload, request, requestMessage, response, responseMessage, [&]() {
    responseMessage.errorCode = Process(requestMessage.request, responseMessage.response, channel);
  });
}

ExitDownloadCommand::ExitDownloadCommand(const std::shared_ptr<TransactionManager>& transactionManager)
    : BaseCommand(transactionManager) {}

int ExitDownloadCommand::Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
                                 std::shared_ptr<ChannelBrokerBase>& channel) {
  return m_transactionManager->Exit();
}