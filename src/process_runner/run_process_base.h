#pragma once

#include "src/json_config/config.h"
#include "src/linux/pipe_comm.h"

#include <string>
#include <thread>

class Synchronizer;

namespace ProcessRunner
{
class Base
{
public:
  Base(Synchronizer* processSync, const Core::SProcess& userProcessInfo);
  ~Base();
  Base(const Base& base);
  virtual void StartThread(const std::string& command) = 0;

  void ChildWaitProcess();
  int GetUserProcessId() const { return userProcessInfo_.processId; }
  int GetProcessDelay() const { return userProcessInfo_.startDelay; }
  Core::SProcess GetUserProcessInfo() const { return userProcessInfo_; }
  pid_t GetApplicationPid() const { return applicationPid_; }

protected:
  Synchronizer* processSync_;
  std::thread pipelineThread_;
  Linux::PipeCommunicator pipe_;
  const Core::SProcess userProcessInfo_;
  pid_t applicationPid_;

  inline static const std::string waitMessage_ = "$WAIT$";
  inline static const std::string waitDoneMsg_ = "$DONE$";
}; // namespace ProcessRunner
} // namespace ProcessRunner