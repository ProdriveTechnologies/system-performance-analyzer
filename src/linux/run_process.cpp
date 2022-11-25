#include "run_process.h"

#include <iostream>
#include <string.h>
#include <sys/types.h> //gettid()
#include <sys/wait.h>  // waitpid()
#include <thread>
#include <unistd.h>

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include "src/helpers/synchronizer.h"

namespace Linux
{
RunProcess::RunProcess(Synchronizer *synchronizer,
                       const Core::SProcess &userProcessInfo)
    : ProcessRunner::Base{synchronizer, userProcessInfo}, running_{false}
{
}
RunProcess::RunProcess(const RunProcess &process)
    : ProcessRunner::Base{process.processSync_, process.userProcessInfo_},
      running_{false}

{
}

RunProcess::~RunProcess()
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
}

void RunProcess::StartThread(const std::string &command)
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  applicationPid_ = fork();
  if (applicationPid_ < 0)
  {
    std::string response{strerror(errno)};
    response = "Fork failed! Reason: " + response;
    throw std::runtime_error(response);
  }
  else if (applicationPid_ == 0)
  {
    pipe_.SetChild();
    ChildExecProcess(command);
  }
  else
  {
    pipe_.SetParent();
    pipelineThread_ = std::thread(&RunProcess::ParentWaitProcess, this);
    // Parent process
    return;
  }
}

void RunProcess::ChildExecProcess(const std::string &command)
{
  CLogger::Log(CLogger::Types::INFO, "Child writing into pipe");
  ChildWaitProcess();
  processName_ = command;
  CLogger::Log(CLogger::Types::INFO, "Child writing into pipe 2");
  ChildWaitProcess();
  std::this_thread::sleep_for(
      std::chrono::milliseconds(userProcessInfo_.startDelay));
  // auto execvArgs{parameters};
  CLogger::Log(CLogger::Types::INFO, "Executing linux command: ", command);
  std::vector<std::string> parameters = Helpers::Split(command, ' ');
  auto parametersCStr = Helpers::ToCString(parameters);
  parametersCStr.push_back(NULL);
  // Child process
  execv(parameters.front().c_str(),
        const_cast<char *const *>(parametersCStr.data()));
  // This code is never executed, also not when the process quits
  exit(EXIT_SUCCESS);
}

void RunProcess::ParentWaitProcess()
{
  int waitCount = 0;
  const int maxWaitCount = 2; // Initial waits are twice
  char readMsg[7];
  while (waitCount != maxWaitCount)
  {
    pipe_.Read(readMsg, waitMessage_.size());

    if (strcmp(readMsg, waitMessage_.data()) == 0)
    {
      CLogger::Log(CLogger::Types::INFO, "Starting synchronize ", waitCount + 1,
                   " for child");
      processSync_->WaitForProcess();
      waitCount++;
      strcpy(readMsg, "    ");
      pipe_.Write(waitDoneMsg_);
    }
  }
  // Waiting until the stress test is fully executes
  waitpid(applicationPid_, NULL, 0);
  processSync_->WaitForProcess();
}

} // namespace Linux