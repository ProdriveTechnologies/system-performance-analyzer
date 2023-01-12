#include "run_process_base.h"

#include "src/helpers/synchronizer.h"

#include <string.h> // strcmp

namespace ProcessRunner
{
Base::Base(Synchronizer* processSync, const Core::SProcess& userProcessInfo)
: processSync_{ processSync }
, pipelineThread_{}
, userProcessInfo_{ userProcessInfo }
{
}
Base::~Base()
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
}
Base::Base(const Base& base)
: processSync_{ base.processSync_ }
{
}

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