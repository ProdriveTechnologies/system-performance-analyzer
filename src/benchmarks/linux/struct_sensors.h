#pragma once

#include <unordered_map>

#include <stdexcept>
#include <string>
#include <vector>

#include "src/helpers/helper_functions.h"
#include "src/json_config/sensor_config/config.h"

namespace Measurements
{
enum class ValueTypes
{
  MIN,
  MAX,
  AVERAGE,
  MEAN
};
inline std::string ToString(const ValueTypes t)
{
  switch (t)
  {
  case ValueTypes::MIN:
    return "min";
  case ValueTypes::MAX:
    return "max";
  case ValueTypes::AVERAGE:
    return "average";
  case ValueTypes::MEAN:
    return "mean";
  default:
    throw std::runtime_error(
        "ValueTypes not recognised in Measurements::ValueTypes!");
  }
}
struct SensorData
{
  struct MeasureValue
  {
    ValueTypes type;
    double value;
  };
  std::vector<MeasureValue> summarizedValues;

  double Get(const ValueTypes valueType) const
  {
    for (const auto &e : summarizedValues)
    {
      if (e.type == valueType)
        return e.value;
    }
    throw std::runtime_error("Didn't find valuetype!");
  }
  // int *rawData; // TODO: Change pointer type;
};
struct Sensors
{
  bool thresholdExceeded = false;
  std::string userId; // UserId is the ID that's given by the user in the
                      // "xavier_config.json" file. If it is in an array, this
                      // userId will have a number of the array behind it
  int uniqueId;
  PlatformConfig::Class classType;
  std::string suffix;
  double multiplier = 1.0;

  bool performanceIndicator = false;
  bool measuredRaw = false;

  // The summarized data, such as average, minimum found, maximum found
  SensorData data;

  // Constructor with the name and unique id
  Sensors(const std::string &userId_, const int uniqueId_,
          PlatformConfig::Class sensorClass_ = PlatformConfig::Class::NONE)
      : userId{userId_}, uniqueId{uniqueId_}, classType{sensorClass_}
  {
  }
  // Constructor with an SDataFields object
  Sensors(const PlatformConfig::SDatafields &fields)
      : userId{fields.name}, uniqueId{fields.id}, classType{fields.classType},
        suffix{fields.suffix}, multiplier{fields.multiplier}, measuredRaw{true}
  {
  }
  Sensors() = default;
  void SetDataInfo(const std::string &suffix_, const double multiplier_ = 1.0)
  {
    suffix = suffix_;
    multiplier = multiplier_;
  }
  // Something of a pointer to the necessary data
};

enum class Classification
{
  PIPELINE,
  PROCESSES,
  SYSTEM
};

using SensorName = std::string;
using SensorProcessId = int;
using SensorIdentifier = std::pair<SensorName, SensorProcessId>;
// Hash function for the Measurements::SensorIdentifier field
struct SensorIdHash
{
  inline size_t operator()(const Measurements::SensorIdentifier &k) const
  {
    // computes the hash of a GStreamer::Identifier using a variant
    // of the Fowler-Noll-Vo hash function
    // from: https://en.cppreference.com/w/cpp/utility/hash/operator()
    size_t result = 2166136261;

    for (size_t i = 0, ie = k.first.size(); i != ie; ++i)
    {
      result = (result * 16777619) ^ k.first[i];
    }
    return result ^ (k.second << 1);
  }
};

/**
 * @brief This struct can contain all the sensors of the different types of
 * measurements. It contains functions to make it easy to access the data
 */
struct AllSensors
{
  std::vector<Classification> allClasses;
  struct SensorGroups
  {
    int processId; // Only used for Classification::PIPELINE and
    // Classification::PROCESSES
    std::vector<Sensors> sensors;
    int processDelay = -1;
    SensorGroups(const int processId_, const std::vector<Sensors> &sensors_)
        : processId{processId_}, sensors{sensors_}
    {
    }
    SensorGroups() = default;
  };

  // All the sensors classified by the different groups (Pipeline data, process
  // data, system data)
  std::unordered_map<Classification, std::vector<SensorGroups>> data;

  using SensorMap = std::unordered_map<std::string, Measurements::Sensors *>;
  using ProcessId = int;
  using SensorMapByProcess = std::unordered_map<ProcessId, SensorMap>;
  SensorMapByProcess mapByProcessId;

  /**
   * @brief Returns the sensors based on a group classification and
   * optionally a process ID
   *
   * @param c
   * @param processId the id of the process in the json configuration. If
   * empty, it will return either the only pipeline that exists, or add the
   * sensors of the pipelines together and return the combined sensors
   * @return std::vector<Sensors> the sensors of the wanted pipeline/group
   * or all pipelines combined
   */
  std::vector<Sensors> GetSensors(const Classification c,
                                  const int processId = -1) const
  {
    auto group = data.find(c);
    if (group != data.end())
    {
      if (processId == -1 && group->second.size() == 1)
        return group->second.front().sensors;
      std::vector<Sensors> allPipelines;
      for (const auto &e : group->second)
      {
        if (processId == -1)
          allPipelines = Helpers::CombineVectors(allPipelines, e.sensors);
        else if (processId == e.processId)
          return e.sensors;
      }
      if (!allPipelines.empty())
        return allPipelines;
    }
    return {};
  }

  Classification GetClassification(const int uniqueId) const
  {
    for (const auto &classification : allClasses)
    {
      auto sensors = GetSensors(classification);
      for (const auto &sensor : sensors)
      {
        if (uniqueId == sensor.uniqueId)
          return classification;
      }
    }
    throw std::runtime_error("Unknown unique ID inserted!");
  }

  /**
   * @brief Returns all process IDs
   */
  std::unordered_set<int> GetProcesses() const
  {
    std::unordered_set<int> result;
    for (const auto &[classifier, sensors] : data)
    {
      for (const auto &processSensors : sensors)
      {
        result.insert(processSensors.processId);
      }
    }
    return result;
  }

  void AddSensors(const Classification c, const std::vector<Sensors> &sensors,
                  const int processId = -1)
  {
    SensorGroups sensorGroup{processId, sensors};

    auto existingGroup = data.find(c);
    if (existingGroup != data.end())
    {
      existingGroup->second.push_back(sensorGroup);
      AddMapValues(&mapByProcessId, &existingGroup->second.back().sensors,
                   processId);
    }
    else
    {
      std::vector<SensorGroups> groups{sensorGroup};
      data.insert(std::make_pair(c, groups));
      allClasses.push_back(c);
      AddMapValues(&mapByProcessId, &data.find(c)->second.back().sensors,
                   processId);
    }
  }
  void AddSensors(const Classification c,
                  const std::vector<SensorGroups> &sensors)
  {
    auto existingGroup = data.find(c);

    if (existingGroup != data.end())
    {
      existingGroup->second = sensors;
      for (auto &e : existingGroup->second)
        AddMapValues(&mapByProcessId, &e.sensors, e.processId);
    }
    else
    {
      data.insert(std::make_pair(c, sensors));
      for (auto &e : data.find(c)->second)
        AddMapValues(&mapByProcessId, &e.sensors, e.processId);
    }
  }
  std::vector<SensorGroups>
  GetSensorGroups(const Classification classification) const
  {
    auto sensorGroup = data.find(classification);
    if (sensorGroup == data.end())
      return {};
    return sensorGroup->second;
  }
  SensorMap GetMap(const int processId) const
  {
    auto mapResult = mapByProcessId.find(processId);
    if (mapResult == mapByProcessId.end())
    {
      throw std::runtime_error("Process ID not existent!");
    }
    return mapResult->second;
  }

private:
  void AddMapValues(SensorMapByProcess *mapObj, std::vector<Sensors> *sensors,
                    const int processId) const
  {
    auto mapResult = mapObj->find(processId);
    if (mapResult == mapObj->end())
    {
      mapObj->insert(std::make_pair(processId, SensorMap{}));
      mapResult = mapObj->find(processId);
    }
    for (auto &e : *sensors)
    {
      mapResult->second.insert(std::make_pair(e.userId, &e));
    }
  }
};

} // namespace Measurements
