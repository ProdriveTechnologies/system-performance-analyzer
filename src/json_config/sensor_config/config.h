#pragma once

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace PlatformConfig
{
enum class Types
{
  PROC_STAT,
  DIRECT,
  ARRAY,
  PROC_MEM,
  PID_STAT,
  PID_STATM,
  DIRECT_PID
};
enum class Class : int
{
  NONE = 0,
  SYS_RESOURCE_USAGE = 1,
  PIPELINE_MEASUREMENTS = 3,
};

inline Types GetType(const std::string &typeStr)
{
  switch (Helpers::hash(typeStr))
  {
  case Helpers::hash("PROC"):
    return Types::PROC_STAT;
  case Helpers::hash("DIRECT"):
    return Types::DIRECT;
  case Helpers::hash("ARRAY"):
    return Types::ARRAY;
  case Helpers::hash("PROCMEM"):
    return Types::PROC_MEM;
  case Helpers::hash("PIDSTAT"):
    return Types::PID_STAT;
  case Helpers::hash("PIDSTATM"):
    return Types::PID_STATM;
  case Helpers::hash("DIRECT_PID"):
    return Types::DIRECT_PID;
  default:
    throw std::runtime_error("Platform Config: Type unknown! Type: " + typeStr);
  }
}
inline Class GetClass(const int classNr)
{
  switch (classNr)
  {
  case Helpers::ToUnderlying(Class::SYS_RESOURCE_USAGE):
    return Class::SYS_RESOURCE_USAGE;
  default:
    CLogger::Log(CLogger::Types::WARNING,
                 "Class type not found! Number: " + std::to_string(classNr));
  }
  return Class::NONE;
}

struct SDatafields
{
  std::string nameClass;
  std::string name;
  int id = 0;
  std::string userId;
  Types type;
  Class classType = Class::NONE;
  std::string suffix;
  double multiplier = 1.0;
  std::string enabledPath;
  std::string calculation;

  double maximumValue = -1.0;
  double minimumValue = -1.0;

  // Required for type: PROC or DIRECT
  std::string path;

  // Required for type: PROC and PROCMEM
  std::string value;
  std::string comparedTo;

  // Required for type: ARRAY
  size_t size = 0;
  std::vector<SDatafields> datafields;
};
struct SMeasureField
{
  int id = 0;
  std::string path;
  Types type;
};
struct SConfig
{
  std::string name;
  std::vector<SDatafields> sensors;
};

} // namespace PlatformConfig