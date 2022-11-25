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
 * @brief Create a Map With Id object
 *
 * @param data
 * @return std::unordered_map<int, Measurements::SSensors>
 */
std::unordered_map<std::string, Measurements::SSensors> CreateMapWithId(const std::vector<Measurements::SSensors>& data)
{
  std::unordered_map<std::string, Measurements::SSensors> mapData;
  for (const auto& e : data)
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
bool HandleThreshold(const Measurements::SSensors* sensor, Core::SThreshold threshold)
{
  auto parseSign = [](const double lhs, const double rhs, const Core::ESign sign) {
    switch (sign)
    {
    case Core::ESign::LE:
      return lhs <= rhs;
    case Core::ESign::LT:
      return lhs < rhs;
    case Core::ESign::GE:
      return lhs >= rhs;
    case Core::ESign::GT:
      return lhs > rhs;
    }
    throw std::runtime_error("Cannot parse sign! Illegal sign!");
  };

  switch (threshold.type)
  {
  case Core::EThresholdType::MAX:
    return parseSign(threshold.value, sensor->data.Get(Measurements::EValueTypes::MAX), threshold.sign);
  case Core::EThresholdType::MIN:
    return parseSign(threshold.value, sensor->data.Get(Measurements::EValueTypes::MIN), threshold.sign);
  case Core::EThresholdType::AVERAGE:
    return parseSign(threshold.value, sensor->data.Get(Measurements::EValueTypes::AVERAGE), threshold.sign);
  case Core::EThresholdType::MEDIAN:
    return parseSign(threshold.value, sensor->data.Get(Measurements::EValueTypes::MEDIAN), threshold.sign);
  default:
    throw std::runtime_error("Cannot parse threshold type! Illegal type!");
  }
}

Measurements::SSensorData GetSummarizedData(const Measurements::EClassification classification,
                                            const std::vector<Measurements::SMeasurementsData>* data,
                                            const int uniqueId,
                                            const double multiplication,
                                            const bool useSteadyState)
{
  Measurements::CSummarizeData summarizedData{ useSteadyState };
  // This is the loop for each measurement
  for (const auto& e : *data)
  {
    auto datapoints = e.GetItems(classification);
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto& e2 : datapoints)
    {
      if (e2.id == uniqueId)
      {
        summarizedData.AddDataPoint(e2);
        //  break; // May leave the (inner) loop for the datapoints, go to next
        // datapoint
      }
    }
  }
  summarizedData.SetMultiplier(multiplication);
  return summarizedData.GetSensorData();
}

Measurements::SSensors GetSummarizedData(const Measurements::EClassification classification,
                                         const std::vector<Measurements::SMeasurementsData>* data,
                                         const std::unordered_set<int> uniqueIds,
                                         const Measurements::SSensors& sensorTemplate,
                                         const bool useSteadyState)
{
  Measurements::CSummarizeData summarizedData{ useSteadyState };
  // This is the loop for each measurement
  for (const auto& e : *data)
  {
    auto datapoints = e.GetItems(classification);
    // This is the loop for the datapoints, only the uniqueId datapoint is used
    for (const auto& e2 : datapoints)
    {
      if (uniqueIds.find(e2.id) != uniqueIds.end())
        summarizedData.AddDataPoint(e2);
    }
  }
  Measurements::SSensors result = sensorTemplate;
  result.uniqueId = PerformanceHelpers::GetUniqueId();
  summarizedData.SetMultiplier(sensorTemplate.multiplier);
  result.data = summarizedData.GetSensorData();
  return result;
}

} // namespace PerformanceHelpers