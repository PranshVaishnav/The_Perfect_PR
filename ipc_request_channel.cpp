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
// Name:        ipc_request_channel.cpp
// Description: Defines ChannelBroker class that sends async response to server

#include "ipc_request_channel.h"
#include "string_utils.h"
#include "transaction.h"
#include "util_base.h"

void ChannelBroker::Notify(const Transaction &transaction) {
  BasicResponseMessageEnvelope<DownloadGroupItemsResponse> responseMessage;

  responseMessage.withCommand(kCommandNotify).withTarget(kDDASource).withErrorCode(transaction.GetLastErrorCode());
  responseMessage.response.downloadGroupId = transaction.GetId();
  responseMessage.response.downloadGroup = transaction.GetDownloadGroupItems();

  std::string response = Serialize(&responseMessage);
  LOG_INFO(kCommandNotify, " message: ", response);
  m_channel->Send(response);
}

void ChannelBroker::Notify(const Transaction &transaction, const std::string &command) {
  BasicResponseMessageEnvelope<DownloadGroupItemsResponse> responseMessage;

  responseMessage.withCommand(command).withTarget(kDDASource).withErrorCode(transaction.GetLastErrorCode());

  responseMessage.response.downloadGroupId = transaction.GetId();
  responseMessage.response.downloadGroup = transaction.GetDownloadGroupItems();

  std::string response = Serialize(&responseMessage);
  LOG_INFO(command, " message: ", response);
  m_channel->Send(response);
}

void ChannelBroker::Run() {}

void ChannelBroker::DestroyChannel() { m_channel->Destroy(); }

ChannelBroker::ChannelBroker(std::shared_ptr<Channel> channel) : m_channel(channel), m_loop(true) {}
