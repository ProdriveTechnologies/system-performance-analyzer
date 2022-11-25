#pragma once

#include "src/helpers/helper_functions.h"

class CLogger
{
public:
  enum class Types
  {
    DEBUG,
    INFO,
    WARNING,
    ERROR
  };
  template <typename... Args>
  static void Log(const Types logType, const Args &... args);

  static void Enable(const bool debug, const bool info)
  {
    enableDebugLogging_ = debug;
    enableInfoLogging_ = info;
  }

private:
  static inline bool enableDebugLogging_ = false;
  static inline bool enableInfoLogging_ = false;
};

template <typename... Args>
void CLogger::Log(const Types logType, const Args &... args)
{
  if (!enableDebugLogging_ && logType == Types::DEBUG)
    return;
  if (!enableInfoLogging_ && logType == Types::INFO)
    return;
  std::string keyword;
  switch (logType)
  {
  case Types::DEBUG:
    keyword = "--DEBUG--: ";
    break;
  case Types::INFO:
    keyword = "--INFO--: ";
    break;
  case Types::WARNING:
    keyword = "--WARNING--: ";
    break;
  case Types::ERROR:
    keyword = "--ERROR--: ";
    break;
  }
  switch (logType)
  {
  case Types::DEBUG:
  case Types::INFO:
    std::cout << keyword << Helpers::ArgToString(args...) << std::endl;
    break;
  case Types::WARNING:
  case Types::ERROR:
    std::cerr << keyword << Helpers::ArgToString(args...) << "\n";
    break;
  }
}