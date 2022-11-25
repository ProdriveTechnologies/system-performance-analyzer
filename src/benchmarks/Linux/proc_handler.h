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
  void ParseMeminfo();

  Linux::FileSystem::ProcStatData GetStats() const { return procStat_; };
  Linux::FileSystem::MemInfoData GetMemInfo() const { return meminfo_; };

  Exports::MeasuredItem
  ParseProcField(const PlatformConfig::SDatafields &procInfo,
                 const std::string &fieldName);
  Exports::MeasuredItem
  ParseMemField(const PlatformConfig::SDatafields &procInfo);

private:
  Linux::FileSystem::ProcStatData procStat_;
  Linux::FileSystem::ProcStatData procStatCorrected_;
  Linux::FileSystem::MemInfoData meminfo_;
};
} // namespace Measurements