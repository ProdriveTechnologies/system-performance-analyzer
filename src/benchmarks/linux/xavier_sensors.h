#pragma once

#include <string>

#include "src/exports/export_struct.h"
#include "src/json_config/sensor_config/config.h"

class CXavierSensors
{
public:
  static Measurements::SMeasuredItem
  ParseDirect(const PlatformConfig::SMeasureField &datafield);
  static std::string ReadLocation(const std::string &path);
};