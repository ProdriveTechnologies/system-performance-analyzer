#include "performance_helpers.h"

#include "summarize_data.h"
#include <algorithm>

namespace PerformanceHelpers
{
/**
 * @brief Returns an unique ID
 *
 * @return int
 * @note This function uses a static variable for the unique ID, the id is
 * therefore unique in all instances of this class (except when multiple threads
 * execute it concurrently)
 */
int GetUniqueId()
{
  static int uniqueId = 0;
  uniqueId++;
  return uniqueId;
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::SensorData
GetSummarizedData(const std::vector<Exports::ExportData> *data,
                  const int uniqueId)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.pipelineInfo)
    {
      for (const auto &e3 : e2.measuredItems)
      {
        if (e3.id == uniqueId)
          summarizedData.AddDataPoint(e3);
      }
    }
  }
  return summarizedData.GetSensorData();
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::Sensors
GetGstCategoriesSummary(const std::vector<Exports::ExportData> *data,
                        const std::unordered_set<int> uniqueIds,
                        const GStreamer::MeasureType type, const int pipelineId)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.pipelineInfo)
    {
      if (e2.pipelineId != pipelineId)
        continue;
      for (const auto &e3 : e2.measuredItems)
      {
        if (uniqueIds.find(e3.id) != uniqueIds.end())
          summarizedData.AddDataPoint(e3);
      }
    }
  }
  Measurements::Sensors result{GStreamer::GetMeasureType(type),
                               PerformanceHelpers::GetUniqueId(),
                               PlatformConfig::Class::PIPELINE_MEASUREMENTS};
  result.data = summarizedData.GetSensorData();
  result.suffix = GStreamer::GetMeasureType(type);
  return result;
}

/**
 * @brief Create a Map With Id object
 *
 * @param data
 * @return std::unordered_map<int, Measurements::Sensors>
 */
std::unordered_map<std::string, Measurements::Sensors>
CreateMapWithId(const std::vector<Measurements::Sensors> &data)
{
  std::unordered_map<std::string, Measurements::Sensors> mapData;
  for (const auto &e : data)
  {
    mapData.insert(std::make_pair(e.userId, e));
  }
  return mapData;
}

/**
 * @brief
 *
 * @param sensor
 * @param threshold
 * @return true threshold was exceeded
 * @return false threshold didnt get exceeded
 */
bool HandleThreshold(const Measurements::Sensors *sensor,
                     Core::SThreshold threshold)
{
  auto parseSign = [](const double lhs, const double rhs,
                      const Core::Sign sign) {
    switch (sign)
    {
    case Core::Sign::LE:
      return lhs <= rhs;
    case Core::Sign::LT:
      return lhs < rhs;
    case Core::Sign::GE:
      return lhs >= rhs;
    case Core::Sign::GT:
      return lhs > rhs;
    }
  };

  switch (threshold.type)
  {
  case Core::ThresholdType::MAX:
    return parseSign(threshold.value,
                     sensor->data.Get(Measurements::ValueTypes::MAX),
                     threshold.sign);
  case Core::ThresholdType::MIN:
    return parseSign(threshold.value,
                     sensor->data.Get(Measurements::ValueTypes::MIN),
                     threshold.sign);
  case Core::ThresholdType::AVERAGE:
    return parseSign(threshold.value,
                     sensor->data.Get(Measurements::ValueTypes::AVERAGE),
                     threshold.sign);
  case Core::ThresholdType::MEAN:
    return parseSign(threshold.value,
                     sensor->data.Get(Measurements::ValueTypes::MEAN),
                     threshold.sign);
  }
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::SensorData
GetSummarizedDataSensors(const std::vector<Exports::ExportData> *data,
                         const int uniqueId)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.measuredItems)
    {
      if (e2.id == uniqueId)
        summarizedData.AddDataPoint(e2);
    }
  }
  return summarizedData.GetSensorData();
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::Sensors
GetSummarizedDataSensors(const std::vector<Exports::ExportData> *data,
                         const std::unordered_set<int> uniqueIds,
                         const std::string &name,
                         const PlatformConfig::SDatafields &field)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.measuredItems)
    {
      if (uniqueIds.find(e2.id) != uniqueIds.end())
        summarizedData.AddDataPoint(e2);
    }
  }
  Measurements::Sensors result{name, PerformanceHelpers::GetUniqueId(),
                               PlatformConfig::Class::SYS_RESOURCE_USAGE};
  result.SetDataInfo(field.suffix, field.multiplier);
  result.data = summarizedData.GetSensorData();
  return result;
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::SensorData
GetSummarizedDataProcesses(const std::vector<Exports::ExportData> *data,
                           const int uniqueId)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.processInfo)
    {
      for (const auto &e3 : e2.measuredItems)
        if (e3.id == uniqueId)
          summarizedData.AddDataPoint(e3);
    }
  }
  return summarizedData.GetSensorData();
}

/**
 * @brief Get the Summarized Data object
 */
Measurements::Sensors
GetSummarizedDataProcesses(const std::vector<Exports::ExportData> *data,
                           const std::unordered_set<int> uniqueIds,
                           const std::string &name)
{
  Measurements::CSummarizeData summarizedData;
  // This is the loop for each measurement
  for (const auto &e : *data)
  {
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto &e2 : e.processInfo)
    {
      for (const auto &e3 : e2.measuredItems)
      {
        if (uniqueIds.find(e3.id) != uniqueIds.end())
          summarizedData.AddDataPoint(e3);
      }
    }
  }
  Measurements::Sensors result{name, PerformanceHelpers::GetUniqueId()};
  result.data = summarizedData.GetSensorData();
  return result;
}

} // namespace PerformanceHelpers