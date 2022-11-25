#include "run_process_base.h"

#include "src/helpers/synchronizer.h"

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
} // namespace ProcessRunner