#include <iostream>

#include "modules/filestream/filestream.h"
#include "src/helpers/stopwatch.h"
#include "src/json_config/config_parser.h"
#include "src/linux/shared_memory.h"
#include "src/modules/unit_handler.h"
#include <chrono>
#include <sys/wait.h>
#include <thread>

#include "src/benchmarks/Linux/xavier_sensors.h"

void print_func() { std::cout << "Printing the world" << std::endl; }

struct CommPipes
{
  int pipes[2];
  Module::Config childInfo;
};

int main()
{
  CXavierSensors xavierSensors;
  auto result = xavierSensors.GetCoreInfo(1);
  std::cout << "Enabled: " << result.enabled << std::endl;
  std::cout << "Frequency: " << result.frequency << std::endl;
  std::cout << "Temperature: " << result.temperature << std::endl;
  std::cout << "Min frequency: " << result.minFrequency << std::endl;
  std::cout << "Max frequency: " << result.maxFrequency << std::endl;
  /*
   auto config = Core::ConfigParser::Parse("./json_example.json");
   std::cout << "Done parsing" << std::endl;
   std::cout << "Parsed: " << config.getStr() << std::endl;
   Core::UnitHandler unitHandler;
   pid_t childPid;
   std::vector<CommPipes> childPipes;
   std::vector<Linux::SharedMemory> memoryBlocks;
   for (const auto &e : config.tasks)
   {
     Module::Config moduleConfig;
     moduleConfig.moduleName = e.type;
     moduleConfig.taskConfig = e;
     CommPipes childInfo;
     childInfo.childInfo = moduleConfig;
     pipe(childInfo.pipes); // Get the pipes for the communication
     moduleConfig.readPipeParent = childInfo.pipes[0];
     moduleConfig.writePipeParent = childInfo.pipes[1];
     if (e.outIds.size() == 1)
     {
       Linux::SharedMemory::Configuration config;
       config.fileName = "/filename1";
       config.memorySize = 100000;
       config.mode = Linux::SharedMemory::Mode::READ_WRITE;
       Linux::SharedMemory memory(config);
       memoryBlocks.push_back(memory);
       memoryBlocks.back().Allocate();
       Module::Config::Stream outputStream;
       outputStream.memorySize = 100000000;
       outputStream.blockSize = 10000000;
       outputStream.addressPtr = memoryBlocks.back().GetAddress();
       outputStream.unitId = e.outIds.at(0);
       printf("Address of parent is %p\n", (void *)outputStream.addressPtr);
       moduleConfig.outputStreams.push_back(outputStream);
     }
     // Loop through the tasks and start them
     if (e.type == "filestream")
       childPid = unitHandler.AddUnit<Module::FileStream>(moduleConfig);
   }
   Stopwatch stopwatch;
   stopwatch.start();

   std::cout << "Child process ID: " << childPid << std::endl;
   int statusRes;
   waitpid(childPid, &statusRes, 0);

   std::cout << WIFEXITED(statusRes)
             << "Child is done after: " << stopwatch.getTime<std::micro>()
             << " microseconds" << std::endl; */
}
