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
// Name:        ipc_request_handler.h
// Description: Declares class to handle messages received and perform actions

#ifndef IPC_REQUEST_HANDLER_H_
#define IPC_REQUEST_HANDLER_H_

#include <boost/signals2.hpp>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "channel_factory.h"
#include "common_typedefs.h"
#include "download_protocol.h"

using namespace Autodesk::ipc;
using namespace Autodesk;

class DLMIPCHandler : public ChannelReactor {
 public:
  virtual ~DLMIPCHandler() {}
  virtual void Connected() override;
  virtual void Disconnected() override;
  virtual void NewMessageAvailable(const Message& request, Message& response) override;

  void BreakConnection();
  void WaitForConnection();

 public:
  boost::signals2::signal<void(const adustring& request, adustring& response)> new_request;

 private:
  std::condition_variable m_conditionConnected;
  std::mutex m_connectionMutex;
  adustring FormatJson(adustring jsonData, bool newLine);
  bool m_connectionBroken{};
};

#endif  // IPC_REQUEST_HANDLER_H_
