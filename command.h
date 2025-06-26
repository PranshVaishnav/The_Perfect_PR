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
// Name:        command.h
// Description: Declares class to process the incoming message

#ifndef COMMAND_H_
#define COMMAND_H_

#include <memory>
#include "disk_information.h"
#include "download_protocol.h"

class TransactionManager;
class ChannelBrokerBase;

class BaseCommand {
 public:
  virtual bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>&) = 0;

  BaseCommand(const std::shared_ptr<TransactionManager>&);
  virtual ~BaseCommand() {}

 protected:
  bool IsInternalLibraryError(int errorCode);
  std::shared_ptr<TransactionManager> m_transactionManager;
};

class StartDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  StartDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupRequest& request, DownloadGroupItemsResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

class InitDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  InitDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupRequest& request, DownloadGroupItemsInitResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
  AdResourceMonitor::DiskInformation m_objDiskInformation;
};

class PauseDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  PauseDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

class CancelDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  CancelDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

class StopDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  StopDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

class ResumeDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  ResumeDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

class ExitDownloadCommand : public BaseCommand {
 public:
  bool Process(const std::string& request, std::string& response, std::shared_ptr<ChannelBrokerBase>& channel);
  ExitDownloadCommand(const std::shared_ptr<TransactionManager>&);

 private:
  int Process(const DownloadGroupIdRequest& request, DownloadGroupIdResponse& response,
              std::shared_ptr<ChannelBrokerBase>&);
};

#endif  // COMMAND_H_
