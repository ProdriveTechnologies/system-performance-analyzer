#pragma once

#include "src/linux/pipe_comm.h"
#include "src/process_runner/run_process_base.h"

#include <atomic>
#include <string>
#include <thread>

class Synchronizer;
namespace Linux
{
class RunProcess : public ProcessRunner::Base
{
public:
  RunProcess(Synchronizer* synchronizer, const Core::SProcess& userProcessInfo);
  RunProcess(const RunProcess& process);
  ~RunProcess();

  void StartThread(const std::string& command);

  int GetThreadPid() const { return applicationPid_; }
  bool IsRunning() const { return running_; }
  std::string GetProcessName() const { return processName_; }

private:
  pid_t applicationPid_;
  bool running_;
  std::string processName_;

  void ParentWaitProcess();
  void ChildExecProcess(const std::string& command);
};
} // namespace Linux