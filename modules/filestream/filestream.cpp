#include "filestream.h"

#include "src/helpers/helper_functions.h"
#include "src/linux/shared_memory.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace Module
{
FileStream::FileStream(Config &moduleConfig) : Base{moduleConfig} {}

void FileStream::Init() { CheckCustomVars(); }

bool FileStream::CheckCustomVars()
{
  // Verify filename
  if (!config_.taskConfig.valExists(JsonVars::FileName))
    return false;
  if (config_.taskConfig.getValWithName(JsonVars::FileName).empty())
    return false;
  // Verify Blocksize
  if (!config_.taskConfig.outIds.empty() &&
      !config_.taskConfig.valExists(JsonVars::BlockSize))
    return false;
  if (!config_.taskConfig.outIds.empty() &&
      Helpers::isNumber(config_.taskConfig.getValWithName(JsonVars::BlockSize)))
    return false;
  // Verify shared memory size
  if (!config_.taskConfig.outIds.empty() &&
      Helpers::isNumber(
          config_.taskConfig.getValWithName(JsonVars::SharedMemorySize)))
    return false;
}

void FileStream::Run()
{
  // This function must start the communication with the core and wait for the
  // start command
  std::string fileName = config_.taskConfig.getValWithName(JsonVars::FileName);
  std::ifstream fileObj{fileName, std::ifstream::in | std::ifstream::binary};
  // int fd = open(fileName.c_str(), O_RDWR, 0777);
  if (!fileObj.good())
  {
    // Logger::log()
    std::cerr << "File incorrect";
    return;
  }

  // Get the amount of blocks that can be used in the shared memory
  std::vector<int> maxBlockSizes;
  std::vector<int> currentBlockLoc;
  for (const auto &outStream : config_.outputStreams)
  {
    auto maxBlockSize = outStream.memorySize / outStream.blockSize;
    if (maxBlockSize == 0)
    {
      std::cerr << "Memory size is smaller then block size";
      return;
    }
    maxBlockSizes.push_back(maxBlockSize);
    currentBlockLoc.push_back(0);
  }

  // Wait for the start command over here
  running_ = true;
  int64_t bytesTransferred = 0;
  // Read the file and put it in shared memory
  while (running_)
  {
    // Writing to all output streams
    for (size_t i = 0; i < config_.outputStreams.size(); ++i)
    {
      const auto &outStream = config_.outputStreams.at(i);
      const auto currentBlock = (currentBlockLoc.at(i) * outStream.blockSize);
      fileObj.read(outStream.addressPtr + currentBlock, outStream.blockSize);
      // Send to the output module that block at block X can be read

      // Go to next block
      currentBlockLoc.at(i) = (currentBlockLoc.at(i) + 1) % maxBlockSizes.at(i);
    }
    if (fileObj.gcount() == 0)
    {
      running_ = false;
    }
    bytesTransferred += fileObj.gcount();
  }
  std::cout << "Stopping with the run, data transferred: " << bytesTransferred
            << std::endl;

  // while (true)
  // {
  //   // if(first
  //   // i++;
  //   // std::cout << "The value of i is" << i << std::endl;
  //   std::this_thread::sleep_for(std::chrono::seconds{8});
  // }
}

void FileStream::Loop()
{
  // while (running_)
  // {
  //   // Wait for message from the other module
  // }
}

} // namespace Module