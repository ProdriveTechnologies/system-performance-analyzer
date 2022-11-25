#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "src/linux/pipe_comm.h"
#include "src/process_runner/run_process_base.h"

class Synchronizer;
namespace Linux
{
class RunProcess : public ProcessRunner::Base
{
public:
  RunProcess(Synchronizer *synchronizer);
  RunProcess(const RunProcess &gstreamer);
  ~RunProcess();

  void StartThread(const std::string &command);

  int GetThreadPid() const { return applicationPid_; }
  bool IsRunning() const { return running_; }

private:
  pid_t applicationPid_;
  bool running_;

  Linux::PipeCommunicator pipe_;

  void ChildWaitProcess();
  void ParentWaitProcess();
  void ChildExecProcess(const std::string &command);
};
} // namespace Linux