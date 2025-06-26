#include "transaction_validator.h"
#include "downloader_error_codes.h"

typedef std::map<std::string, TransactionPtr> TransactionPtrMap;
typedef std::vector<TransactionalState> TransactionStateVector;

const TransactionStateVector kValidPausedStates = { TransactionalState::TRANSACTION_ACTIVATED };
const TransactionStateVector kValidResumeStates = { TransactionalState::TRANSACTION_PAUSED };
const TransactionStateVector kValidStopStates { TransactionalState::TRANSACTION_ACTIVATED, TransactionalState::TRANSACTION_NEW,                         TransactionalState::TRANSACTION_PAUSED };
const TransactionStateVector kValidCancelStates = { TransactionalState::TRANSACTION_ACTIVATED, TransactionalState::TRANSACTION_NEW,
  TransactionalState::TRANSACTION_PAUSED };


inline bool NotInRange(TransactionalState state, const std::vector<TransactionalState>& stateMap) {
  return stateMap.end() == std::find(stateMap.begin(), stateMap.end(), state);
}

int ValidateRequest(const TransactionPtrMap& map, const TransactionPtr& ptr,
  const std::vector<TransactionalState>& validRange) {
  auto found = map.find(ptr->GetId());
  if (found == map.end()) {
    LOG_DEBUG("Transaction map does not have Transaction ", ptr->GetId());
    return kErrorRejectRequest;
  }

  if (NotInRange(found->second->GetState(), validRange)) {
    LOG_DEBUG("Transaction state change is not allowed for Transaction ", ptr->GetId());
    return kErrorRejectRequest;
  }
  return kNoError;
}

int TransactionValidator::ValidateStartRequest(const TransactionPtrMap& map, const TransactionPtr& ptr) const {
  if (map.find(ptr->GetId()) != map.end()) {
    LOG_DEBUG("Transaction map does not have Transaction ", ptr->GetId());
    return kErrorRejectRequest;
  }
  return kNoError;
}

int TransactionValidator::ValidateStopRequest(const TransactionPtrMap& map, const TransactionPtr& ptr) const {
  return ValidateRequest(map, ptr, kValidStopStates);
}

int TransactionValidator::ValidatePauseRequest(const TransactionPtrMap& map, const TransactionPtr& ptr) const {
  return ValidateRequest(map, ptr, kValidPausedStates);
}

int TransactionValidator::ValidateResumeRequest(const TransactionPtrMap& map, const TransactionPtr& ptr) const {
  return ValidateRequest(map, ptr, kValidResumeStates);
}

int TransactionValidator::ValidateCancelRequest(const TransactionPtrMap& map, const TransactionPtr& ptr) const {
  return ValidateRequest(map, ptr, kValidCancelStates);
}
