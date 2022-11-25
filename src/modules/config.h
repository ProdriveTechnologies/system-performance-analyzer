#pragma once

#include <string>
#include <vector>

#include "src/json_config/config.h"
#include "src/linux/pipe_comm.h"

namespace Module
{
struct Config
{
  struct Stream
  {
    int blockSize;
    int memorySize;
    caddr_t addressPtr = nullptr;
    int unitId;
  };
  Core::Task taskConfig;
  std::string moduleName;
  Linux::PipeCommunicator parentComm;
  std::vector<Stream> inputStreams;
  std::vector<Stream> outputStreams;
};

} // namespace Module