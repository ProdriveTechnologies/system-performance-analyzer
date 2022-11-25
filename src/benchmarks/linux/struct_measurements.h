#pragma once

#include "struct_sensors.h"

namespace Measurements
{
struct SMeasuredItem
{
  int id = -1;
  double measuredValue = -1.0;
};
struct SMeasurementGroup
{
  int pipelineId = -1;
  std::vector<SMeasuredItem> measuredItems;
};
struct SMeasurementsData
{
  std::string time;
  std::vector<Classification> allClasses;
  std::unordered_map<Classification, std::vector<SMeasurementGroup>> data;

  void AddMeasurements(const Classification c,
                       const std::vector<SMeasurementGroup> &measurements)
  {
    auto existingGroup = data.find(c);
    if (existingGroup != data.end())
    {
      // Group already exists, overwriting old data with new sensorgroups
      existingGroup->second = measurements;
    }
    else
    {
      // Group does not exist, thus create the group
      data.insert(std::make_pair(c, measurements));
      allClasses.push_back(c);
    }
  }
  void AddMeasurements(const Classification c,
                       const std::vector<SMeasuredItem> &measurement,
                       const int pipelineId = -1)
  {
    auto measurementGroup = SMeasurementGroup{pipelineId, measurement};
    AddMeasurements(c, {measurementGroup});
  }
  std::vector<SMeasuredItem> GetItems(const Classification c) const
  {
    std::vector<SMeasuredItem> items;

    auto classData = data.find(c);
    if (classData == data.end())
      return {};

    for (const auto &e : classData->second)
      items = Helpers::CombineVectors(items, e.measuredItems);
    return items;
  }
};
} // namespace Measurements