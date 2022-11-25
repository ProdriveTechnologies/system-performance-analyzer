#include "run_process_base.h"

#include "src/helpers/synchronizer.h"
#include <string.h> // strcpy

namespace ProcessRunner
{
Base::Base(Synchronizer *processSync)
    : processSync_{processSync}, pipelineThread_{}
{
}
Base::~Base()
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
}
Base::Base(const Base &base) : processSync_{base.processSync_} {}

void Base::ChildWaitProcess()
{
  pipe_.Write(waitMessage_);
  char readMsg[7];
  while (true)
  {
    pipe_.Read(readMsg, waitDoneMsg_.size());

    if (strcmp(readMsg, waitDoneMsg_.data()) == 0)
      return;
  }
}

} // namespace ProcessRunner