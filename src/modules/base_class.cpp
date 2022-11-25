#include "base_class.h"

namespace Module
{
Base::Base(const Config &config) : config_{config}, running_{false} {}

// void Base::InitSharedMemory()
// {
//   InitSharedMemory(config_.inputStreams);
//   InitSharedMemory(config_.outputStreams);
// }

// void Base::InitSharedMemory(const std::vector<Module::Config::Stream> stream)
// {
//   for (const auto &e : stream)
//   {
//     if (sharedMemory_.find(e.unitId) == sharedMemory_.end())
//     {
//       Linux::SharedMemory::Configuration config;
//       config.fileName = "/filename1";
//       config.memorySize = e.memorySize;
//       Linux::SharedMemory sharedMemory{e.addressPtr, config};
//       sharedMemory.Init();
//       sharedMemory_.insert(std::make_pair(e.unitId, sharedMemory));
//     }
//   }
// }

} // namespace Module