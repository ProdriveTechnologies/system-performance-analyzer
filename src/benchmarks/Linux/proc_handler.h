#pragma once

#include "src/exports/export_struct.h"
#include "src/json_config/sensor_config/config.h"
#include "src/linux/filesystem.h"

namespace Measurements
{
class ProcHandler
{
public:
  ProcHandler() = default;

  void ParseProcStat();

  static std::vector<PlatformConfig::SDatafields>
  ParseProcField(const PlatformConfig::SDatafields &procInfo);

  Exports::MeasuredItem
  ParseProcField(const PlatformConfig::SDatafields &procInfo,
                 const std::string &fieldName);

private:
  Linux::FileSystem::ProcStatData procStat_;
  Linux::FileSystem::ProcStatData procStatCorrected_;
};
} // namespace Measurements