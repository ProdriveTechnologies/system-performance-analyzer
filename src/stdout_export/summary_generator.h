#pragma once

#include <infoware/cpu.hpp>
#include <string>

#include "src/exports/export_struct.h"
#include "src/json_config/sensor_config/config.h"
#include "src/linux/filesystem.h"

namespace Exports
{
class CSummaryGenerator
{
public:
  bool
  Generate(const std::vector<Exports::ExportData> &measurementsData,
           const std::vector<PlatformConfig::SDatafields> &measurementsDef);

private:
  // Static info based on the host system
  void PrintApplicationInfo();
  void PrintSystemInfo();
  void PrintCacheInfo();
  void PrintPcieInfo();

  double GetAverage(const std::vector<Exports::ExportData> &measurementsData,
                    const int id);

  // Dynamic info based on the test which was executed
  void PrintSystemSummary(
      const std::vector<Exports::ExportData> &measurementsData,
      const std::vector<PlatformConfig::SDatafields> &measurementsDef);

  static std::string GetCacheType(const iware::cpu::cache_type_t cache);
  static std::string
  GetArchitecture(const iware::cpu::architecture_t architecture);
  static constexpr int lowestCacheNr = 0;
  static constexpr int highestCacheNr = 10;
};

} // namespace Exports