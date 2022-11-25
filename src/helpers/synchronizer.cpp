#include "synchronizer.h"

#include <thread>

Synchronizer::Synchronizer(const size_t threadNr)
: processRunning_{ false }
, threadNr_{ threadNr }
, waitForProcessId_{ 0 }
, threadReadyCount_{ 0 }
{
}

void Synchronizer::WaitForProcess()
{
  std::unique_lock<std::mutex> lk{ processReadyMtx_ };
  threadReadyCount_++;                   // Not synchronized yet
  int waitProcessId = waitForProcessId_; // Make a copy of the ID to wait for
  if (threadReadyCount_ == threadNr_)
  {
    // Synchronized, may start
    threadReadyCount_ = 0;
    waitForProcessId_ += 1;
    lk.unlock();
    conditionVar_.notify_all();
    return;
  }
  conditionVar_.wait(lk, [this, waitProcessId] { return waitProcessId == (waitForProcessId_ - 1); });
}

bool Synchronizer::AllCompleted()
{
  const size_t allThreadsExcludingThis = threadNr_ - 1;
  bool res = threadReadyCount_ == allThreadsExcludingThis;
  if (res)
    return true;
  else
    return false;
}