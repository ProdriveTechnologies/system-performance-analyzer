#pragma once

#include "src/exports/export_struct.h"
#include "struct_sensors.h"

namespace Measurements
{
class CSummarizeData
{
public:
  CSummarizeData() : firstDataPoint_{true} {}
  void AddDataPoint(const Exports::MeasuredItem item)
  {
    if (firstDataPoint_)
      AddInitial(item);
    else
      CheckMinMax(item);

    average_.Add(item);
    allMeasurements_.push_back(item.measuredValue);
  }
  SensorData GetSensorData() const
  {
    SensorData sensorData;
    // There must be a datapoint to add sensible data
    if (average_.datapoints != 0)
    {
      sensorData.summarizedValues.push_back(
          SensorData::MeasureValue{ValueTypes::MIN, minFound_});
      sensorData.summarizedValues.push_back(
          SensorData::MeasureValue{ValueTypes::MAX, maxFound_});
      sensorData.summarizedValues.push_back(
          SensorData::MeasureValue{ValueTypes::AVERAGE, average_.Get()});
      sensorData.summarizedValues.push_back(
          SensorData::MeasureValue{ValueTypes::MEAN, GetMean()});
    }
    return sensorData;
  }
  int GetSize() const { return average_.datapoints; }

private:
  SensorData sensorData_;
  bool firstDataPoint_;
  double minFound_;
  double maxFound_;
  struct Average
  {
    double aggregatedData = 0.0;
    int64_t datapoints = 0;
    void Add(const Exports::MeasuredItem item)
    {
      aggregatedData += item.measuredValue;
      datapoints += 1;
    }
    double Get() const
    {
      return datapoints != 0 ? aggregatedData / datapoints : 0;
    }
  };
  Average average_;
  std::vector<double> allMeasurements_;

  double GetMean() const
  {
    if (!allMeasurements_.empty())
    {
      const size_t loc = allMeasurements_.size() / 2;
      return allMeasurements_.at(loc);
    }
    return 0.0;
  }

  void AddInitial(const Exports::MeasuredItem &item)
  {
    firstDataPoint_ = false;
    minFound_ = item.measuredValue;
    maxFound_ = item.measuredValue;
  }
  void CheckMinMax(const Exports::MeasuredItem &item)
  {
    if (item.measuredValue < minFound_)
      minFound_ = item.measuredValue;
    if (item.measuredValue > maxFound_)
      maxFound_ = item.measuredValue;
  }
};
} // namespace Measurements