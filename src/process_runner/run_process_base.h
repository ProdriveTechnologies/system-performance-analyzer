#pragma once

#include <string>
#include <thread>
#include <vector>
// #include "src/helpers/synchronizer.h"
class Synchronizer;

namespace ProcessRunner
{
class Base
{
public:
  Base(Synchronizer *processSync);
  ~Base();
  Base(const Base &base);
  virtual void StartThread(const std::string &command) = 0;

protected:
  Synchronizer *processSync_;
  std::thread pipelineThread_;
}; // namespace ProcessRunner
} // namespace ProcessRunner