#include <iostream>

#include <chrono>
#include <sys/wait.h>
#include <thread>
#include <variant>

#include "src/processes_struct.h"
// #include "modules/filestream/filestream.h"
#include "src/benchmarks/Linux/perf_measurements.h"
#include "src/benchmarks/Linux/xavier_sensors.h"
// #include "src/gstreamer/handler.h"
#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include "src/helpers/stopwatch.h"
#include "src/helpers/synchronizer.h"
#include "src/input_handler.h"
#include "src/json_config/config_parser.h"
#include "src/linux/filesystem.h"

int VerifyArguments(const bool incorrectArgs, const int argc,
                    const CInputHandler::SUserArgs &userArgs);

void print_info()
{
  std::cout
      << "Execute this application using ./benchmarks -c [config file]\n\n"
      << "Usage: ./benchmarks [OPTION [ARG]] ...\n"
      << " -h, --help\t\t show this help statement\n"
      << " -c, --config [config file]\t\t use the [config file] for the "
         "measurements (must be in the JSON format, read documentation for the "
         "exact format or use the example)\n"
      << " -s --sensors [sensorconfig]\t\t use the [sensorconfig file] for the "
         "path and sensor information (must be in the JSON format, read "
         "documentation for the exact format or use the example, by default "
         "\"sensor_config.json\")\n\n"
      << "NOTE: OPTION --config (or -c) is required for the correct JSON "
         "configuration!\n"
      << std::endl;
}
int main(int argc, char *argv[])
{
  // Input checks
  CInputHandler inputHandler;
  bool incorrectArgs = inputHandler.Parse(argc, argv);
  auto userArgs = inputHandler.GetUserArguments();
  if (VerifyArguments(incorrectArgs, argc, userArgs) == -1)
    return -1; // Terminate application, arguments are incorrect

  // Enabling/disabling logger components
  CLogger::Enable(false, true);
  // Parsing user configuration
  auto config = Core::ConfigParser::Parse(userArgs.configFile);
  CLogger::Log(CLogger::Types::INFO, "Started application");
  Synchronizer synchronizer{config.processes.size() +
                            1}; // + 1 because of monitoring thread
  // CGstreamerHandler gstreamer{&synchronizer}; //{config.gstreamerPipeline};
  Linux::CPerfMeasurements measurements{&synchronizer, userArgs.sensorInfoFile,
                                        config.thresholds};
  // std::vector<std::variant<CGstreamerHandler>> processes;

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

  measurements.Start(config, &processes);
}

/**
 * @brief Verifies the arguments
 *
 * @return int(-1): incorrect arguments
 *         int(0): correct arguments
 */
int VerifyArguments(const bool incorrectArgs, const int argc,
                    const CInputHandler::SUserArgs &userArgs)
{
  if (incorrectArgs || argc < 2 || userArgs.configFile.empty())
  {
    print_info();
    return -1;
  }

  if (!Helpers::FileExists(userArgs.sensorInfoFile))
  {
    std::cout << "Sensor config file not found: " << userArgs.sensorInfoFile
              << std::endl;
    print_info();
    return -1;
  }
  else if (!Helpers::FileExists(userArgs.configFile))
  {
    std::cout << "Config file not found: " << userArgs.configFile << std::endl;
    print_info();
    return -1;
  }
  return 0; // Arguments are correct
}