#pragma once

#include "src/exports/export_struct.h"
#include "struct_sensors.h"

#include <algorithm>
#include <cmath>

namespace Measurements
{
class CSummarizeData
{
public:
  CSummarizeData()
  : useSteadyState_{ false }
  , multiplier_{ 1.0 }
  {
  }
  CSummarizeData(const bool useSteadyState)
  : useSteadyState_{ useSteadyState }
  , multiplier_{ 1.0 }
  {
  }
  void SetMultiplier(const double multiplier) { multiplier_ = multiplier; }
  void AddDataPoint(const Measurements::SMeasuredItem item)
  {
    average_.Add(item);
    allMeasurements_.push_back(item.measuredValue);
  }
  SSensorData GetSensorData() const
  {
    SSensorData sensorData;
    // There must be a datapoint to add sensible data
    if (average_.datapoints != 0)
    {
      sensorData.summarizedValues.push_back(SSensorData::SMeasureValue{ EValueTypes::MIN, GetMin() * multiplier_ });
      sensorData.summarizedValues.push_back(SSensorData::SMeasureValue{ EValueTypes::MAX, GetMax() * multiplier_ });
      sensorData.summarizedValues.push_back(
        SSensorData::SMeasureValue{ EValueTypes::AVERAGE, average_.Get() * multiplier_ });
      sensorData.summarizedValues.push_back(
        SSensorData::SMeasureValue{ EValueTypes::MEDIAN, GetMedian() * multiplier_ });
    }
    return sensorData;
  }
  int GetSize() const { return average_.datapoints; }

private:
  SSensorData sensorData_;
  bool useSteadyState_;
  double multiplier_;
  struct Average
  {
    double aggregatedData = 0.0;
    int64_t datapoints = 0;
    void Add(const Measurements::SMeasuredItem item)
    {
      aggregatedData += item.measuredValue;
      datapoints += 1;
    }
    double Get() const { return datapoints != 0 ? aggregatedData / datapoints : 0; }
  };
  Average average_;
  std::vector<double> allMeasurements_;

  double GetMin() const
  {
    auto localVector = allMeasurements_;
    if (useSteadyState_ && localVector.size() > 10)
    {
      localVector.erase(localVector.end() - 2, localVector.end());
      localVector.erase(localVector.begin(), localVector.begin() + 2);
    }
    return *std::min_element(localVector.begin(), localVector.end());
  }
  double GetMax() const
  {
    auto localVector = allMeasurements_;
    if (useSteadyState_ && localVector.size() > 10)
    {
      localVector.erase(localVector.end() - 2, localVector.end());
      localVector.erase(localVector.begin(), localVector.begin() + 2);
    }
    return *std::max_element(localVector.begin(), localVector.end());
  }
  inline bool IsEven(int value) const { return (value % 2) == 0; }
  double GetMedian() const
  {
    auto sortFunction = [](const double& lhs, const double& rhs) { return lhs < rhs; };
    if (!allMeasurements_.empty())
    {
      std::vector<double> sorted = allMeasurements_;
      std::sort(sorted.begin(), sorted.end(), sortFunction);

      const size_t loc = std::floor(sorted.size() / 2.0);
      // If it's average, calculate the arithmetic mean of the two middle points
      if (IsEven(sorted.size()))
        return (sorted.at(loc) + sorted.at(loc - 1)) / 2.0;
      return sorted.at(loc);
    }
    return 0.0;
  }
};
} // namespace Measurements