#include "input_handler.h"

#include "src/helpers/helper_functions.h"
#include <iostream>
#include <string.h>

/**
 * @brief Parses the input arguments
 *
 * @return true Error occured, incorrect arguments
 * @return false Input arguments handled succesfully
 */
bool CInputHandler::Parse(int argc, char *argv[])
{
  SUserArgs userArgs;

  int optionIndex{0};
  int c;
  while ((c = getopt_long(argc, argv, "hc:s:", longopts_, &optionIndex)) != -1)
  {
    auto incorrectArgs = HandleOption(c, optionIndex);
    if (incorrectArgs || argc < 2)
      return true;
  }
  return false;
}

/**
 * @brief Handles an option from getopts
 *
 * @param getoptRetVal the return value of getopt
 * @param getoptOptIndex the index given by getopt
 * @return true Error occured, incorrect arguments
 * @return false Option handled correctly
 */
bool CInputHandler::HandleOption(const int getoptRetVal,
                                 const int getoptOptIndex)
{
  bool incorrectArgs = false;
  switch (getoptRetVal)
  {
  case 0:
    if (optarg)
    {
      if (strcmp(optarg, helpName) == 0)
      {
        incorrectArgs = true;
      }
      else if (strcmp(longopts_[getoptOptIndex].name, configName) == 0)
      {
        ParseArguments(Arguments::CONFIG, &userArguments_, optarg);
      }
      else if (strcmp(longopts_[getoptOptIndex].name, sensorsName) == 0)
      {
        ParseArguments(Arguments::SENSORS, &userArguments_, optarg);
      }
    }
    break;
  case Helpers::ToUnderlying(Arguments::HELP):
    incorrectArgs = true;
    break;
  case Helpers::ToUnderlying(Arguments::CONFIG):
    if (optarg)
      ParseArguments(Arguments::CONFIG, &userArguments_, optarg);
    break;
  case Helpers::ToUnderlying(Arguments::SENSORS):
    if (optarg)
      ParseArguments(Arguments::SENSORS, &userArguments_, optarg);
    break;
  case '?':
    std::cout << "Unknown option: " << optopt << std::endl;
    incorrectArgs = true;
    break;
  case ':':
    std::cout << "Missing option for: " << optopt << std::endl;
    incorrectArgs = true;
    break;
  default:
    incorrectArgs = true;
  }
  return incorrectArgs;
}

/**
 * @brief parses the arguments and stores it accordingly in the user arguments
 */
void CInputHandler::ParseArguments(const Arguments arg, SUserArgs *userArgs,
                                   char *extraArg)
{
  switch (arg)
  {
  case Arguments::SENSORS:
    userArgs->sensorInfoFile = extraArg;
    break;
  case Arguments::CONFIG:
    userArgs->configFile = extraArg;
    break;
  default:; // Do nothing
  }
}