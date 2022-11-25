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
std::string CCsv::InitExport(const PlatformConfig::SConfig &config)
{
  std::string row = "time"; // Dont have any data to initialize

  for (const auto &e : config.sensors)
  {
    switch (Helpers::hash(e.type))
    {
    case Helpers::hash("ARRAY"):
    {
      for (size_t i = 0; i < e.size; i++)
      {
        std::for_each(e.datafields.begin(), e.datafields.end(),
                      [&](const PlatformConfig::SDatafields &dataField) {
                        row += DELIMITER + dataField.name + std::to_string(i);
                      });
      }
    }
    break;
    default:
      row += "," + e.name;
    }
  }

  // for (int i = 0; i < 8; i++)
  // {
  //   row += ",cpu" + std::to_string(i) + ",cpu" + std::to_string(i) +
  //   "irq,cpu" +
  //          std::to_string(i) + "softIrq";
  // }
  // for (int i = 0; i < 8; i++)
  // {
  //   row += ",temperature" + std::to_string(i);
  // }
  return row;
}
std::string CCsv::ParseData(const ExportData &data)
{
  std::string row{data.time};
  for (const auto &e : data.measuredItems)
  {
    row += DELIMITER + std::to_string(e.measuredValue);
  }
  // row += Helpers::ArgToString(DELIMITER,
  //                             GetCpuUtilization(data.cpuUtilization.totalCpu));

  // for (const auto &e : data.cpuUtilization.cpus)
  // {
  //   row += Helpers::ArgToString(DELIMITER, GetCpuUtilization(e), DELIMITER,
  //                               GetUtilization(e, e.jiffiesIrq), DELIMITER,
  //                               GetUtilization(e, e.jiffiesSoftIrq));
  // }
  // for (const auto &e : data.cpuInfo)
  // {
  //   row += Helpers::ArgToString(DELIMITER, e.temperature);
  // }
  return row;
}
std::string CCsv::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

} // namespace Exports