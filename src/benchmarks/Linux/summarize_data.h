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
  }
  SensorData GetSensorData() const
  {
    SensorData sensorData;
    sensorData.summarizedValues.push_back(
        SensorData::MeasureValue{ValueTypes::MIN, minFound_});
    sensorData.summarizedValues.push_back(
        SensorData::MeasureValue{ValueTypes::MAX, maxFound_});
    sensorData.summarizedValues.push_back(
        SensorData::MeasureValue{ValueTypes::AVERAGE, average_.Get()});
    return sensorData;
  }

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
    double Get() const { return aggregatedData / datapoints; }
  };
  Average average_;

  void AddInitial(const Exports::MeasuredItem &item)
  {
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