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
// Name:        command_cache.h
// Description: Stores all the commands for execution

#ifndef COMMAN_CACHE_H_
#define COMMAN_CACHE_H_

#include "command.h"
#include "common_typedefs.h"
#include "json_serializer.h"

#include <map>

class TransactionManager;
class BaseCommand;

class CommandCache {
 public:
  CommandCache(const std::shared_ptr<TransactionManager> &transactionManager);
  ~CommandCache() = default;

  void ProcessRequest(const adustring &request, adustring &response, std::shared_ptr<ChannelBrokerBase> &);

 private:
#if 0
  template <typename T>
  bool ValidateRequest(BasicRequestMessageEnvelope<T> &requestMessageEnvelope,
                       BasicResponseMessageEnvelope<DownloadGroupResponse> &responseMessage);
#endif
  std::map<std::string, std::shared_ptr<BaseCommand>> m_commands;
};

#endif  // COMMAN_CACHE_H_
