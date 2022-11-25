#include "export_json.h"

#include "src/exports/file_writer.h"
#include "src/helpers/helper_functions.h"

#include <nlohmann/json.hpp>
#include <unordered_map>

namespace Exports
{
bool CJson::FullExport(const std::vector<SMeasurementItem>& config,
                       [[maybe_unused]] const FullMeasurement data,
                       [[maybe_unused]] const AllSensors& allSensors,
                       const std::vector<Measurements::CCorrelation::SResult>& correlations)
{
  nlohmann::json jsonObject;

  for (size_t i = 0; i < config.size(); ++i)
  {
    const auto& item = config.at(i);
    jsonObject[item.name] = ParseLabel(item);
  }

  for (const auto& e : correlations)
  {
    nlohmann::json jsonCorr;
    jsonCorr["sensor1"] = e.sensor1.userId;
    jsonCorr["sensor2"] = e.sensor2.userId;
    jsonCorr["value"] = e.correlation;
    jsonObject["correlations"].push_back(jsonCorr);
  }
  FileWriter fileWriter{ filename_ + EXTENSION };
  fileWriter.AddRow(jsonObject.dump());

  return true;
}
/**
 * @brief Loops through the items and retrieves the labels out of them
 */
nlohmann::json CJson::ParseLabel(const SMeasurementItem& item, const nlohmann::json parent)
{
  nlohmann::json result = parent;
  // This visit checks every field and places them in the JSON
  std::visit(Overload{ [&](const auto& e) { result[item.name] = e; }, // for string, double, and int
                       [&](const std::vector<SMeasurementItem>& items) {
                         if (item.type == EType::ARRAY)
                         {
                           nlohmann::json newJson = nlohmann::json::array();

                           std::transform(items.begin(),
                                          items.end(),
                                          std::back_inserter(newJson),
                                          [this](const auto& e2) { return ParseLabel(e2); });
                           result[item.name] = newJson;
                         }
                         else
                         {
                           nlohmann::json newJson =
                             std::accumulate(items.begin(),
                                             items.end(),
                                             nlohmann::json{},
                                             [this](const auto& sum, const auto& e2) { return ParseLabel(e2, sum); });

                           result[item.name] = newJson;
                         }
                       } },
             item.value);
  return result;
}

} // namespace Exports