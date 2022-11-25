#include "export_csv.h"

#include "src/helpers/helper_functions.h"
namespace Exports
{
std::string CCsv::InitExport()
{
  return ""; // Dont have any data to initialize
}
std::string CCsv::ParseData(const ExportData &data)
{
  return Helpers::ArgToString(data.time, DELIMITER,
                              data.cpuInfo.at(0).temperature, DELIMITER,
                              data.cpuInfo.at(1).temperature);
}
std::string CCsv::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

} // namespace Exports