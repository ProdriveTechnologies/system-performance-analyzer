#pragma once

#include "config.h"
#include "src/linux/shared_memory.h"
#include <unordered_map>

namespace Module
{
class Base
{
public:
  // Base() = default;
  Base(const Config &config);
  virtual ~Base(){};
  virtual void Run() = 0;
  void InitSharedMemory();

protected:
  const Config config_;
  bool running_;
  std::unordered_map<int, Linux::SharedMemory> sharedMemory_;
  std::vector<Linux::SharedMemory> sharedMemoryOut_;

private:
  // void InitSharedMemory(const std::vector<Module::Config::Stream> stream);
};

} // namespace Module