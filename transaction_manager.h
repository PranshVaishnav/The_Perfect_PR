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
// Name:        tansaction_manager.h
// Description: Manages the lifecycle of a Transaction

#ifndef TRANSACTION_MANAGER_H_
#define TRANSACTION_MANAGER_H_

#include <boost/signals2.hpp>
#include <map>
#include <mutex>

#include "task_cache.h"
#include "task_progress_listener.h"
#include "task_scheduler.h"
#include "task_state_listener.h"
#include "transaction.h"
#include "transaction_validator.h"
#include "disk_information.h"

class ChannelBrokerBase;

class TransactionManager : public TaskStateListener, public std::enable_shared_from_this<TransactionManager> {
 public:
  TransactionManager(const TaskSchedulerPtr&, const std::shared_ptr<TaskCache>&);
  TransactionManager(const TaskSchedulerPtr&);
  int StartTransaction(TransactionPtr&);
  int StopTransaction(TransactionPtr&);
  int CancelTransaction(TransactionPtr&);
  int PauseTransaction(TransactionPtr&);
  int ResumeTransaction(TransactionPtr&);
  void StopAllTransactions();
  int Exit();
  boost::signals2::signal<void()> break_connection;

 protected:
  void TaskStateChanged(const Task& task) override;

 private:
  void RegisterParent(const TransactionPtr&, const TaskPtr&);
  void DeregisterParent(const TransactionPtr&, const Task&);
  void DeregisterActiveParents(const Task&);
  bool ShouldExecute(const TaskPtr&, const TransactionPtr&);
  const std::set<TransactionPtr>& GetParents(const Task&);
  void Start(TransactionPtr&);
  void DeleteTransaction(const TransactionPtr&);
  void RemoveTaskIfOrphan(const Task&);
  void StopTransactionsIfError(const Task& task);

 private:
  typedef std::map<std::string, std::set<TransactionPtr>> TaskParentMap;
  std::map<std::string, TransactionPtr> m_transactions;
  std::mutex m_transactionMutex;
  TaskParentMap m_taskParentMap;
  TaskSchedulerPtr m_taskScheduler;
  std::shared_ptr<TaskCache> m_taskCache;
  TransactionValidator m_transactionValidator;
  AdResourceMonitor::DiskInformation m_diskInformation;
  std::atomic<bool> m_stopAllTransactions{false};
};

#endif
