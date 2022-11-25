#pragma once

#include "src/helpers/helper_functions.h"
#include "src/helpers/logger.h"
#include <stdexcept>
#include <string>
#include <vector>

namespace PlatformConfig
{
enum class ETypes
{
  PROC_STAT,
  DIRECT,
  ARRAY,
  PROC_MEM,
  PID_STAT,
  PID_STATM,
  DIRECT_PID
};
enum class EClass : int
{
  NONE = 0,
  SYS_RESOURCE_USAGE = 1,
  PIPELINE_MEASUREMENTS = 3,
};

inline ETypes GetType(const std::string &typeStr)
{
  switch (Helpers::hash(typeStr))
  {
  case Helpers::hash("PROC"):
    return ETypes::PROC_STAT;
  case Helpers::hash("DIRECT"):
    return ETypes::DIRECT;
  case Helpers::hash("ARRAY"):
    return ETypes::ARRAY;
  case Helpers::hash("PROCMEM"):
    return ETypes::PROC_MEM;
  case Helpers::hash("PIDSTAT"):
    return ETypes::PID_STAT;
  case Helpers::hash("PIDSTATM"):
    return ETypes::PID_STATM;
  case Helpers::hash("DIRECT_PID"):
    return ETypes::DIRECT_PID;
  default:
    throw std::runtime_error("Platform Config: Type unknown! Type: " + typeStr);
  }
}
inline EClass GetClass(const int classNr)
{
  switch (classNr)
  {
  case Helpers::ToUnderlying(EClass::SYS_RESOURCE_USAGE):
    return EClass::SYS_RESOURCE_USAGE;
  default:
    CLogger::Log(CLogger::Types::WARNING,
                 "Class type not found! Number: " + std::to_string(classNr));
  }
  return EClass::NONE;
}

struct SDatafields
{
  std::string nameClass;
  std::string name;
  int id = 0;
  std::string userId;
  ETypes type;
  EClass classType = EClass::NONE;
  std::string suffix;
  double multiplier = 1.0;
  std::string enabledPath;
  std::string calculation;
  bool showInLive = false;

  double maximumValue = -1.0;
  double minimumValue = 0;

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
  ETypes type;
};
struct SConfig
{
  std::string name;
  std::vector<SDatafields> sensors;
};

} // namespace PlatformConfig