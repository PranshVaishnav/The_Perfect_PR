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
// Name:        transaction_validator.h
// Description: Declares class to validate the incoming message

#ifndef TRANSACTION_VALIDATER_H_
#define TRANSACTION_VALIDATER_H_

#include "transaction.h"

#include <map>

class TransactionValidator {
 public:
  int ValidateStartRequest(const std::map<std::string, TransactionPtr>&, const TransactionPtr&) const;
  int ValidateStopRequest(const std::map<std::string, TransactionPtr>&, const TransactionPtr&) const;
  int ValidatePauseRequest(const std::map<std::string, TransactionPtr>&, const TransactionPtr&) const;
  int ValidateResumeRequest(const std::map<std::string, TransactionPtr>&, const TransactionPtr&) const;
  int ValidateCancelRequest(const std::map<std::string, TransactionPtr>&, const TransactionPtr&) const;
};

#endif
