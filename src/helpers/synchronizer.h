#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

class Synchronizer
{
public:
  explicit Synchronizer(const size_t threadNr);

  void WaitForProcess();
  void setRunning(const bool runningState) { processRunning_ = runningState; }
  bool isRunning() const { return processRunning_; }
  void setThreadId(const int threadId) { threadId_ = threadId; }
  int getThreadId() const { return threadId_; }
  bool AllCompleted();

private:
  std::atomic_bool processRunning_;
  std::atomic<int> threadId_;
  const size_t threadNr_;
  std::atomic<int> waitForProcessId_;

  std::atomic<size_t> threadReadyCount_;
  std::mutex processReadyMtx_;
  std::condition_variable conditionVar_;
};