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
// Name:        command_cache.cpp
// Description: Stores all the commands for execution

#include "command_cache.h"
#include "command.h"
#include "download_protocol.h"
#include "logger.h"
#include "transaction_manager.h"

CommandCache::CommandCache(const std::shared_ptr<TransactionManager> &transactionManager) {
  // Create commands
  std::shared_ptr<BaseCommand> initCommand = std::make_shared<InitDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandInitDownload, initCommand));
  std::shared_ptr<BaseCommand> startCommand = std::make_shared<StartDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandStartDownload, startCommand));
  std::shared_ptr<BaseCommand> pauseCommand = std::make_shared<PauseDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandPauseDownload, pauseCommand));
  std::shared_ptr<BaseCommand> cancelCommand = std::make_shared<CancelDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandCancelDownload, cancelCommand));
  std::shared_ptr<BaseCommand> stopCommand = std::make_shared<StopDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandStopDownload, stopCommand));
  std::shared_ptr<BaseCommand> resumeCommand = std::make_shared<ResumeDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandResumeDownload, resumeCommand));
  std::shared_ptr<BaseCommand> exitCommand = std::make_shared<ExitDownloadCommand>(transactionManager);
  m_commands.insert(std::make_pair<>(kCommandExitDownload, exitCommand));
}

void CommandCache::ProcessRequest(const adustring &request, adustring &response,
                                  std::shared_ptr<ChannelBrokerBase> &channel) {
  BasicRequestMessageEnvelope<DummyClass> requestMessageEnvelope;
  BasicResponseMessageEnvelope<DownloadGroupResponse> responseMessage;

  if (!Deserialize(request, requestMessageEnvelope)) {
    responseMessage.withErrorCode(kErrorInvalidRequest);
    response = Serialize(&responseMessage);
    return;
  }

  LOG_INFO("Executing command: ", requestMessageEnvelope.command);

  if (requestMessageEnvelope.command == "CRASH_DOWNLOAD") {
      std::string *p = nullptr;
      p->append("X");
  }

  if (!m_commands.count(requestMessageEnvelope.command)) {
    responseMessage.withErrorCode(kErrorInvalidCommand).withCommand(requestMessageEnvelope.command);
    response = Serialize(&responseMessage);
    return;
  }
  m_commands.at(requestMessageEnvelope.command)->Process(request, response, channel);
}
