#include "export_csv.h"

#include "src/helpers/helper_functions.h"
#include <unordered_map>

#include "src/exports/file_writer.h"

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
std::string InitExport(const std::vector<PipelineConfig> &config)
{
  std::string row = "time"; // Dont have any data to initialize

  std::for_each(config.begin(), config.end(), [&](const PipelineConfig &item) {
    // InitPipelineConfig(item.pipelineId, item.pluginNames);
  });
  return row;
}
// std::string
// InitPipelineConfig(const size_t pipelineId,
//                    const std::unordered_map<int, GStreamer::Identifier>
//                    &items)
// {
// }
std::string CCsv::ParseData(const ExportData &data)
{
  std::string row{data.time};
  for (const auto &e : data.measuredItems)
  {
    row += DELIMITER + std::to_string(e.measuredValue);
  }
  return row;
}

std::string CCsv::ParseData(const std::string &time,
                            const std::vector<PipelineInfo> &data)
{
  std::string row{time};
  for (const auto &e : data)
  {
    row += ParseDataPipeline(e.measuredItems);
  }
  return row;
}

std::string CCsv::ParseDataPipeline(const std::vector<MeasuredItem> &items)
{
  std::string row;
  for (const auto &e : items)
  {
    row += DELIMITER + std::to_string(e.measuredValue);
  }
  return row;
}
std::string CCsv::ParseData(const std::string &timeStr,
                            const std::vector<MeasuredItem> &items)
{
  std::string row{timeStr};
  for (const auto &e : items)
  {
    row += DELIMITER + std::to_string(e.measuredValue);
  }
  return row;
}

std::string CCsv::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

bool CCsv::FullExport(
    const std::vector<MeasurementItem> &config, const FullMeasurement data,
    const AllSensors &allSensors,
    const std::vector<Measurements::CCorrelation::SResult> &correlations)
{
  std::string labels;

  for (size_t i = 0; i < config.size(); ++i)
  {
    const auto &item = config.at(i);
    FileWriter systemFile{item.name + ".csv"};
    labels = "time" + ParseLabel(item);
    systemFile.AddRow(labels);

    for (const auto &e : *data)
    {
      if (i == 0)
      {
        systemFile.AddRow(ParseData(e), false);
      }
      else if (i == 1)
      {
        systemFile.AddRow(ParseData(e.time, e.pipelineInfo), false);
      }
      else if (i == 2)
      {
        systemFile.AddRow(ParseData(e.time, e.processInfo), false);
      }
    }
  }

  return true;
}

/**
 * @brief Loops through the items and retrieves the labels out of them
 *
 * @param item
 * @return std::string
 */
std::string CCsv::ParseLabel(const MeasurementItem &item)
{
  std::string result;
  std::visit(
      Overload{[&](const std::string &e) {
                 if (item.type == Type::LABEL)
                 {
                   result = "," + e;
                 }
               },
               [&](const auto &) { ; }, // Ignore double and ints for labels
               [&](const std::vector<MeasurementItem> &items) {
                 for (const auto &e2 : items)
                 {
                   result += ParseLabel(e2);
                 }
               }},
      item.value);
  return result;
}

} // namespace Exports