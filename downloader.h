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
// Name:        downloader.h
// Description: Declares class to perform basic download feature

#ifndef DOWNLOADER_H_
#define DOWNLOADER_H_

#include <boost/signals2.hpp>
#include <memory>

#include "common_constants.h"
#include "download_api.h"
#include "download_event.h"
#include "download_protocol.h"

namespace AdDownloadStaticLib {
class DefaultDownloader : public BaseDownloader {
 public:
  DefaultDownloader(const std::string &url, const std::string &downloadPath, const int maxTime, void *logPtr);

  DefaultDownloader(const std::string &url, const std::string &downloadDir, const std::string &fileName,
                    const int maxTime, void *logPtr);

  void Notify(StatusEvent *e) override;

 protected:
  std::string m_download_group_id;
  std::string m_tracking_id;
};

typedef std::shared_ptr<DefaultDownloader> DefaultDownloaderPtr;

class TaskDownloader : public BaseDownloader {
 public:
  TaskDownloader(const DownloadGroupItem &item);
  virtual void Notify(StatusEvent *e) override;
  virtual int GetTotalBlock() const;
  virtual int GetDownloadedBlock() const;

 public:
  boost::signals2::signal<void(const DownloadGroupItem &item)> notify;
  boost::signals2::signal<void(const int state)> stateChange;

 protected:
  DownloadGroupItem m_downloadItem;
};

typedef std::shared_ptr<TaskDownloader> TaskDownloaderPtr;
}  // namespace AdDownloadStaticLib

#endif
