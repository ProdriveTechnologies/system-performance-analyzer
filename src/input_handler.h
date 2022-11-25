#pragma once

#include <getopt.h>
#include <string>

class CInputHandler
{
public:
  enum class Arguments : char
  {
    HELP = 'h',
    CONFIG = 'c',
    SENSORS = 's'
  };
  struct SUserArgs
  {
    std::string configFile;
    // Default file name:
    std::string sensorInfoFile = "sensor_config.json";
  };

  bool Parse(int argc, char *argv[]);
  SUserArgs GetUserArguments() const { return userArguments_; }

private:
  static constexpr char configName[] = "config";
  static constexpr char sensorsName[] = "sensors";
  static constexpr char helpName[] = "help";
  SUserArgs userArguments_;
  static inline option longopts_[] = {{configName, required_argument, NULL, 0},
                                      {sensorsName, required_argument, NULL, 0},
                                      {helpName, no_argument, NULL, 0},
                                      {NULL, 0, NULL, 3}};

  bool HandleOption(const int getoptRetVal, const int getoptOptIndex);
  void ParseArguments(const Arguments arg, SUserArgs *userArgs,
                      char *extraArg = nullptr);
};