#include <iostream>

#include <chrono>
#include <sys/wait.h>
#include <thread>
#include <variant>

// #include "modules/filestream/filestream.h"
#include "src/benchmarks/Linux/perf_measurements.h"
#include "src/benchmarks/Linux/xavier_sensors.h"
#include "src/gstreamer/handler.h"
#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/helpers/synchronizer.h"
#include "src/json_config/config_parser.h"
#include "src/linux/filesystem.h"
#include "src/linux/run_process.h"
#include "src/linux/shared_memory.h"

// #include "src/modules/unit_handler.h"

template <typename... Ts> // (7)
struct Overload : Ts...
{
  using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>; // (2)

void print_func() { std::cout << "Printing the world" << std::endl; }

void print_info()
{
  std::cout
      << "Execute this application using ./benchmarks\n"
      << "NOTE: Make sure that file \"json_example.json\" is included in the "
         "same directory as the application\n"
      << "\"json_example.json\" contains the configuration for the benchmarks"
      << std::endl;
}
int main()
{
  if (!Helpers::FileExists("json_example.json"))
  {
    print_info();
    return -1;
  }

  auto config = Core::ConfigParser::Parse("json_example.json");
  CLogger::Enable(false, false);
  CLogger::Log(CLogger::Types::INFO, "Started application");
  Synchronizer synchronizer{config.processes.size() +
                            1}; // + 1 because of monitoring thread
  // CGstreamerHandler gstreamer{&synchronizer}; //{config.gstreamerPipeline};
  Linux::CPerfMeasurements measurements{&synchronizer};
  // std::vector<std::variant<CGstreamerHandler>> processes;

  // Struct to store the info for the processes. The variant is used to store
  // the object type in
  struct ProcessInfo
  {
    std::string type;
    std::string command;
    std::variant<CGstreamerHandler, Linux::RunProcess> processes; // int
    ProcessInfo(const Core::SProcess &process, CGstreamerHandler streamHandler)
        : type{process.type}, command{process.command}, processes{streamHandler}
    {
    }
    ProcessInfo(const Core::SProcess &process, Linux::RunProcess processHandler)
        : type{process.type}, command{process.command}, processes{
                                                            processHandler}
    {
    }
    ~ProcessInfo() = default;
  };
  std::vector<ProcessInfo> processes;

  for (const auto &e : config.processes)
  {
    if (e.type == "linux_command")
    {
      processes.push_back(ProcessInfo{e, Linux::RunProcess{&synchronizer}});
      CLogger::Log(CLogger::Types::INFO, "Added linux process: ", e.command);
    }
    else if (e.type == "gstreamer")
    {
      processes.push_back(ProcessInfo{e, CGstreamerHandler{&synchronizer}});
      CLogger::Log(CLogger::Types::INFO,
                   "Added GStreamer pipeline: ", e.command);
    }
  }
  // Start the threads
  for (auto &e : processes)
  {
    std::visit(
        Overload{
            [&e](auto &handler) { handler.StartThread(e.command); },
        },
        e.processes);
  }

  // gstreamer.RunPipelineThread(config.gstreamerPipeline);
  measurements.Start(config);

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
