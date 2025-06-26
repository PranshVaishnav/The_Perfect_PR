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
// Name:        error_codes.h
// Description: A respository of error codes

#ifndef DOWNLOADER_ERROR_CODES_H_
#define DOWNLOADER_ERROR_CODES_H_

const int kNoError = 0;
const int kErrorInvalidRequest = -1;
const int kErrorRejectRequest = -2;
const int kErrorInvalidCommand = -3;
const int kErrorInvalidFilename = -4;
const int kErrorInternetConnectionDenied = -5;

#endif