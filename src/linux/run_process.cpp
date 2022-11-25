#include "run_process.h"

#include <iostream>
#include <string.h>
#include <sys/types.h> //gettid()
#include <sys/wait.h>  // waitpid()
#include <thread>
#include <unistd.h>

#include "src/helpers/helper_functions.h"
#include "src/helpers/synchronizer.h"

namespace Linux
{
RunProcess::RunProcess(Synchronizer *synchronizer)
    : ProcessRunner::Base{synchronizer}, running_{false}
{
}
RunProcess::RunProcess(const RunProcess &gstreamer)
    : ProcessRunner::Base{gstreamer.processSync_}, running_{false}

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
  ChildWaitProcess();
  ChildWaitProcess();
  // auto execvArgs{parameters};
  std::vector<std::string> parameters = Helpers::Split(command, ' ');
  auto parametersCStr = Helpers::ToCString(parameters);
  parametersCStr.push_back(NULL);
  // Child process
  execv(parameters.front().c_str(),
        const_cast<char *const *>(parametersCStr.data()));
  // Stop the child when it returns (and close pipes accordingly)
  // close(moduleConfig.readPipeParent);
  // close(moduleConfig.writePipeParent);
  //   ChildWaitProcess(); This code is all never executed... Which is a design
  //   flaw
  exit(EXIT_SUCCESS);
}

void RunProcess::ChildWaitProcess()
{
  pipe_.Write("WAIT");
  char readMsg[5];
  while (true)
  {
    pipe_.Read(readMsg, 4);
    if (strcmp(readMsg, "DONE") == 0)
      return;
  }
}

void RunProcess::ParentWaitProcess()
{
  int waitCount = 0;
  const int maxWaitCount = 2; // Initial waits are twice
  char readMsg[5];
  while (waitCount != maxWaitCount)
  {
    pipe_.Read(readMsg, 4);

    if (strcmp(readMsg, "WAIT") == 0)
    {
      processSync_->WaitForProcess();
      waitCount++;
      strcpy(readMsg, "    ");
      pipe_.Write("DONE");
    }
  }
  // Waiting until the stress test is fully executes
  waitpid(applicationPid_, NULL, 0);
  processSync_->WaitForProcess();
}

} // namespace Linux