#include "export_csv.h"

#include "src/helpers/helper_functions.h"
namespace Exports
{
double GetCpuUtilization(Linux::FileSystem::ProcStatData::Cpu data)
{
  long long totalJiffies = data.jiffiesIdle + data.jiffiesIoWait +
                           data.jiffiesIrq + data.jiffiesNice +
                           data.jiffiesSoftIrq + data.jiffiesSystem +
                           data.jiffiesUser;
  return ((totalJiffies - data.jiffiesIdle) / totalJiffies) * 100;
}
double GetUtilization(Linux::FileSystem::ProcStatData::Cpu data,
                      long percentageValue)
{
  long long totalJiffies = data.jiffiesIdle + data.jiffiesIoWait +
                           data.jiffiesIrq + data.jiffiesNice +
                           data.jiffiesSoftIrq + data.jiffiesSystem +
                           data.jiffiesUser;
  return ((percentageValue) / totalJiffies) * 100;
}
std::string CCsv::InitExport()
{
  return ""; // Dont have any data to initialize
}
std::string CCsv::ParseData(const ExportData &data)
{
  std::string row{data.time};
  row += Helpers::ArgToString(DELIMITER,
                              GetCpuUtilization(data.cpuUtilization.totalCpu));

  for (const auto &e : data.cpuUtilization.cpus)
  {
    row += Helpers::ArgToString(DELIMITER, GetCpuUtilization(e), DELIMITER,
                                GetUtilization(e, e.jiffiesIrq), DELIMITER,
                                GetUtilization(e, e.jiffiesSoftIrq));
  }
  for (const auto &e : data.cpuInfo)
  {
    row += Helpers::ArgToString(DELIMITER, e.temperature);
  }
  return row;
}
std::string CCsv::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

} // namespace Exports