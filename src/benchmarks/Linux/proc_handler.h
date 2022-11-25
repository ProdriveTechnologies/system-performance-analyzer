#pragma once

#include "src/exports/export_struct.h"
#include "src/json_config/sensor_config/config.h"
#include "src/linux/filesystem.h"

namespace Measurements
{
class ProcHandler
{
public:
  ProcHandler();

  void ParseProcStat();

  std::vector<Exports::MeasuredItem>
  ParseProcField(const PlatformConfig::SDatafields &procInfo);

private:
  Linux::FileSystem::ProcStatData procStat_;

  Exports::MeasuredItem
  ParseProcField(const PlatformConfig::SDatafields &procInfo,
                 const std::string &fieldName);
};
} // namespace Measurements