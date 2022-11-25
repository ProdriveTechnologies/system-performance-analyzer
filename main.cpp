#include <iostream>

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include "src/input_handler.h"
#include "src/spa.h"

int VerifyArguments(const bool incorrectArgs, const int argc,
                    const CInputHandler::SUserArgs &userArgs);

/**
 * @brief Prints info when a wrong command is inserted or with parameter -h or
 * --help
 *
 */
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
         "\"sensor_config.json\")\n"
      << " -l --log [loglevel]\t\t enables logging for the tool, [loglevel]: "
         "0: only error and warnings, 1: info logging, 2: debug logging, 3: "
         "all loggings\n\n"
      << "NOTE: OPTION --config (or -c) is required for the correct JSON "
         "configuration!\n"
      << std::endl;
}

/**
 * @brief The main function of the application
 */
int main(int argc, char *argv[])
{
  // Input checks
  CInputHandler inputHandler;
  bool incorrectArgs = inputHandler.Parse(argc, argv);
  auto userArgs = inputHandler.GetUserArguments();
  if (VerifyArguments(incorrectArgs, argc, userArgs) == -1)
    return -1; // Terminate application, arguments are incorrect

  // Enabling/disabling logger components (based on the input)
  CLogger::Enable(userArgs.enableInfoLog, userArgs.enableDebugLog);
  // Parsing user configuration
  auto config = Core::ConfigParser::Parse(userArgs.configFile);

  CLogger::Log(CLogger::Types::INFO, "Started application");
  CSystemPerformanceAnalyzer spa{userArgs.configFile, userArgs.sensorInfoFile};
  // Start the actual execution of the configured tasks and measurements
  spa.StartExecution();
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