#include "export_csv.h"

#include "src/helpers/helper_functions.h"
#include <unordered_map>

#include "src/exports/file_writer.h"

namespace Exports
{
std::string CCsv::ParseData(const std::string &timeStr,
                            const std::vector<Measurements::SSensors> &sensors,
                            const std::vector<SMeasuredItem> &items)
{
  std::string row{timeStr};
  for (const auto &e : sensors)
  {
    if (e.measuredRaw)
    {
      row += DELIMITER;
      auto value = FindMeasuredItem(items, e.uniqueId);
      if (value.id == e.uniqueId)
        row += std::to_string(value.measuredValue * e.multiplier);
    }
  }
  return row;
}

bool CCsv::FullExport(
    [[maybe_unused]] const std::vector<SMeasurementItem> &config,
    const FullMeasurement data, const AllSensors &allSensors,
    [[maybe_unused]] const std::vector<Measurements::CCorrelation::SResult>
        &correlations)
{
  std::string labels;

  for (const auto &classification : allSensors.allClasses)
  {
    const auto name = Measurements::GetClassificationStr(classification);
    FileWriter systemFile{name + ".csv"};

    labels = "time" + ParseLabel(allSensors.GetSensors(classification));
    systemFile.AddRow(labels);

    for (const auto &e : *data)
    {
      systemFile.AddRow(ParseData(e.time, allSensors.GetSensors(classification),
                                  e.GetItems(classification)),
                        false);
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
std::string CCsv::ParseLabel(const std::vector<Measurements::SSensors> &sensors)
{
  std::string result;

  for (const auto &e : sensors)
  {
    if (e.measuredRaw)
      result += DELIMITER + e.userId;
  }
  return result;
}

} // namespace Exports