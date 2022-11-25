#include "export_json.h"

#include "src/helpers/helper_functions.h"
#include <unordered_map>

#include "src/exports/file_writer.h"
#include <nlohmann/json.hpp>

namespace Exports
{
std::string
CJson::InitExport(const std::vector<PlatformConfig::SDatafields> &config)
{
  return "";
}
std::string CJson::ParseData(const ExportData &data) { return ""; }

std::string CJson::ParseData(const std::string &time,
                             const std::vector<PipelineInfo> &data)
{
  std::string row{time};
  for (const auto &e : data)
  {
    row += ParseDataPipeline(e.measuredItems);
  }
  return row;
}

std::string CJson::ParseDataPipeline(const std::vector<MeasuredItem> &items)
{
  return "";
}
std::string CJson::ParseData(const std::string &timeStr,
                             const std::vector<MeasuredItem> &items)
{
  return "";
}

std::string CJson::FinishExport()
{
  return ""; // Don't have anything to write when finishing
}

bool CJson::FullExport(
    const std::vector<MeasurementItem> &config, const FullMeasurement data,
    const AllSensors &allSensors,
    const std::vector<Measurements::CCorrelation::SResult> &correlations)
{
  std::string labels;
  nlohmann::json jsonObject;

  for (size_t i = 0; i < config.size(); ++i)
  {
    const auto &item = config.at(i);
    jsonObject[item.name] = ParseLabel(item);
    // FileWriter systemFile{item.name + EXTENSION};
    // labels = "time" + ParseLabel(item);
    // systemFile.AddRow(labels);

    // for (const auto &e : *data)
    // {
    //   if (i == 0)
    //   {
    //     systemFile.AddRow(ParseData(e), false);
    //   }
    //   else if (i == 1)
    //   {
    //     systemFile.AddRow(ParseData(e.time, e.pipelineInfo), false);
    //   }
    // }
  }
  // std::string filename = filename_;
  // filename += EXTENSION;
  FileWriter fileWriter{filename_ + EXTENSION};
  fileWriter.AddRow(jsonObject.dump());

  return true;
}
/**
 * @brief Loops through the items and retrieves the labels out of them
 *
 * @param item
 * @return std::string
 */
nlohmann::json CJson::ParseLabel(const MeasurementItem &item,
                                 const nlohmann::json parent)
{
  nlohmann::json result = parent;
  // How do i know the difference between level deeper or same level? I dont
  // name with an array should be level deeper with values, if the values
  // contain something, it is again level
  std::visit(Overload{[&](const auto &e) {
                        result[item.name] = e;
                      }, // for string, double, and int
                      [&](const std::vector<MeasurementItem> &items) {
                        if (item.type == Type::ARRAY)
                        {
                          nlohmann::json newJson = nlohmann::json::array();
                          for (const auto &e2 : items)
                          {
                            newJson.push_back(ParseLabel(e2));
                          }
                          result[item.name] = newJson;
                        }
                        else
                        {
                          nlohmann::json newJson;
                          for (const auto &e2 : items)
                          {
                            newJson = ParseLabel(e2, newJson);
                          }
                          result[item.name] = newJson;
                        }
                      }},
             item.value);
  return result;
}

} // namespace Exports