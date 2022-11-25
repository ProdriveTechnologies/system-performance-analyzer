#include "synchronizer.h"

#include <thread>

Synchronizer::Synchronizer() : processRunning_{false}, processReady_{false} {}

void Synchronizer::waitForProcess()
{
  std::unique_lock<std::mutex> lk{processReadyMtx_};
  if (processReady_)
  {
    // Synchronized, may start (monitoring)
    processReady_ = false;
    lk.unlock();
    conditionVar_.notify_all();
    return;
  }
  processReady_ = true; // Not synchronized yet (gstreamer)
  conditionVar_.wait(lk, [this] { return !processReady_; });
}