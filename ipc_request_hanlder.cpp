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
// Name:        ipc_request_handler.cpp
// Description: Defines class to handle messages received and perform actions
#include <boost/format.hpp> 
#include <sstream>
#include <string>

#include "logger.h"
#include "string_utils.h"

#include "ipc_request_handler.h"
#include "task_scheduler.h"
namespace pt = boost::property_tree;

/*virtual*/ void DLMIPCHandler::Connected() /*override*/ { LOG_DEBUG(__FUNCTION__); }

/*virtual*/ void DLMIPCHandler::Disconnected() /*override*/ {
  LOG_DEBUG(__FUNCTION__);
  {
    std::lock_guard<std::mutex> guard(m_connectionMutex);
    m_connectionBroken = true;
  }
  m_conditionConnected.notify_all();
}

/*virtual*/ void DLMIPCHandler::NewMessageAvailable(const Message& request, Message& response) /*override*/ {
  LOG_DEBUG(__FUNCTION__);
  Message req = request;
  sstring body = req.GetBody();

  LOG_INFO("Received message: ", FormatJson(body, false));

  adustring sResponseBody;
  new_request(body, sResponseBody);

  LOG_INFO("Processed message: ", sResponseBody);
  sResponseBody = FormatJson(sResponseBody, true);

  response.SetBody(sResponseBody);
}

void DLMIPCHandler::WaitForConnection() {
  std::unique_lock<std::mutex> lock(m_connectionMutex);
  m_conditionConnected.wait(lock, [this] { return m_connectionBroken; });
  LOG_INFO("Finish waiting for connection...");
}

void DLMIPCHandler::BreakConnection() {
  LOG_DEBUG(__FUNCTION__);
  {
    std::lock_guard<std::mutex> guard(m_connectionMutex);
    m_connectionBroken = true;
  }
  m_conditionConnected.notify_all();
}

adustring DLMIPCHandler::FormatJson(adustring jsonData, bool newLine) {
  pt::ptree node;
  std::stringstream ss(jsonData), dd;
  try {
    pt::json_parser::read_json(ss, node);
    pt::json_parser::write_json(dd, node, newLine);
  } catch (pt::json_parser::json_parser_error e) {
    // Logger::Instance().LogError(str(boost::format("Failed to Format Json. Exception occured: [%1%]") % e.message()));
    std::ostringstream oss;
    oss << boost::format("Failed to Format Json. Exception occurred: [%1%]") % e.what();
    Logger::Instance().LogError(oss.str());
    return " ";
  }
  return dd.str();
}
