#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

class Synchronizer
{
public:
  Synchronizer();

  void waitForProcess();
  void setRunning(const bool runningState) { processRunning_ = runningState; }
  bool isRunning() const { return processRunning_; }
  void setThreadId(const int threadId) { threadId_ = threadId; }
  int getThreadId() const { return threadId_; }

private:
  std::atomic_bool processRunning_;
  std::atomic<int> threadId_;

  std::atomic_bool processReady_;
  std::mutex processReadyMtx_;
  std::condition_variable conditionVar_;
};