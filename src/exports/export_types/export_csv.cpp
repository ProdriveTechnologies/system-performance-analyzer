#include "export_csv.h"

#include "src/helpers/helper_functions.h"

#include <iostream>
namespace Exports
{
double GetCpuUtilization(Linux::FileSystem::ProcStatData::Cpu data)
{
  long long totalJiffies = data.jiffiesIdle + data.jiffiesIoWait +
                           data.jiffiesIrq + data.jiffiesNice +
                           data.jiffiesSoftIrq + data.jiffiesSystem +
                           data.jiffiesUser;
  return ((totalJiffies - data.jiffiesIdle) * 100.0) / totalJiffies;
}
double GetUtilization(Linux::FileSystem::ProcStatData::Cpu data,
                      long percentageValue)
{
  long long totalJiffies = data.jiffiesIdle + data.jiffiesIoWait +
                           data.jiffiesIrq + data.jiffiesNice +
                           data.jiffiesSoftIrq + data.jiffiesSystem +
                           data.jiffiesUser;
  return ((percentageValue)*100.0) / totalJiffies;
}
std::string
CCsv::InitExport(const std::vector<PlatformConfig::SDatafields> &config)
{
  std::string row = "time"; // Dont have any data to initialize

  std::for_each(config.begin(), config.end(),
                [&](const PlatformConfig::SDatafields &dataField) {
                  row += DELIMITER + dataField.name;
                });
  return row;
}
std::string CCsv::ParseData(const ExportData &data)
{
  std::string row{data.time};
  for (const auto &e : data.measuredItems)
  {
    row += DELIMITER + std::to_string(e.measuredValue);
  }
  return row;
}
std::string CCsv::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

} // namespace Exports