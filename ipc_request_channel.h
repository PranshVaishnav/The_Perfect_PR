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
// Name:        ipc_request_channel.h
// Description: Declares ChannelBroker class that sends async response to server

#ifndef IPC_REQUEST_CHANNEL_H_
#define IPC_REQUEST_CHANNEL_H_

#include <atomic>
#include <string>

#include "ipc_request_handler.h"

class Transaction;

class ChannelBrokerBase {
public:
  virtual void Notify(const Transaction& transaction) = 0;
  virtual void Notify(const Transaction& transaction, const std::string& command) = 0;
  virtual void Run() = 0;
  virtual void DestroyChannel() = 0;
  virtual ~ChannelBrokerBase() {}
};

class ChannelBroker : public ChannelBrokerBase {
 public:
  virtual void Notify(const Transaction &transaction) override;

  virtual void Notify(const Transaction &transaction, const std::string &command) override;

  virtual void Run() override;

  virtual void DestroyChannel() override;

  ChannelBroker(std::shared_ptr<Channel> channel);

 protected:
  std::shared_ptr<Channel> m_channel;
  std::atomic_bool m_loop;
};

#endif
