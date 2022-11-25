#include "export_json.h"

#include "src/helpers/helper_functions.h"
#include <unordered_map>

#include "src/exports/file_writer.h"
#include <nlohmann/json.hpp>

namespace Exports
{
bool CJson::FullExport(
    const std::vector<SMeasurementItem> &config,
    [[maybe_unused]] const FullMeasurement data,
    [[maybe_unused]] const AllSensors &allSensors,
    const std::vector<Measurements::CCorrelation::SResult> &correlations)
{
  std::string labels;
  nlohmann::json jsonObject;

  for (size_t i = 0; i < config.size(); ++i)
  {
    const auto &item = config.at(i);
    jsonObject[item.name] = ParseLabel(item);
  }

  for (const auto &e : correlations)
  {
    nlohmann::json jsonCorr;
    jsonCorr["sensor1"] = e.sensor1.userId;
    jsonCorr["sensor2"] = e.sensor2.userId;
    jsonCorr["value"] = e.correlation;
    jsonObject["correlations"].push_back(jsonCorr);
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
nlohmann::json CJson::ParseLabel(const SMeasurementItem &item,
                                 const nlohmann::json parent)
{
  nlohmann::json result = parent;
  // How do i know the difference between level deeper or same level? I dont
  // name with an array should be level deeper with values, if the values
  // contain something, it is again level
  std::visit(Overload{[&](const auto &e) {
                        result[item.name] = e;
                      }, // for string, double, and int
                      [&](const std::vector<SMeasurementItem> &items) {
                        if (item.type == EType::ARRAY)
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