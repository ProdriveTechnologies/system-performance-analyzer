#pragma once

#include <unordered_map>

#include <stdexcept>
#include <string>
#include <vector>

#include "src/helpers/helper_functions.h"

namespace Measurements
{
enum class ValueTypes
{
  MIN,
  MAX,
  AVERAGE
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
// struct
// {
//   std::vector<Sensors> sensors;
// };

struct Sensors
{
  bool thresholdExceeded = false;
  std::string userId; // UserId is the ID that's given by the user in the
                      // "xavier_config.json" file. If it is in an array, this
                      // userId will have a number of the array behind it
  int uniqueId;
  std::string groupId; // Only applicable for arrays, this is the groupname and
  // therefore does not contain the array number. This is
  // the "original" user id of the sensor

  SensorData data;
  Sensors(const std::string &userId_, const int uniqueId_)
      : userId{userId_}, uniqueId{uniqueId_}
  {
  }
  // Something of a pointer to the necessary data
};

enum class Classification
{
  PIPELINE,
  PROCESSES,
  SYSTEM
};

/**
 * @brief This struct can contain all the sensors of the different types of
 * measurements. It contains functions to make it easy to access the data
 */
struct AllSensors
{
  struct SensorGroups
  {
    int processId; // Only used for Classification::PIPELINE and
                   // Classification::PROCESSES
    std::vector<Sensors> sensors;
  };

  std::unordered_map<Classification, std::vector<SensorGroups>> data;
  std::unordered_map<std::string, Measurements::Sensors *> mapByNameId;
  // std::unordered_map<std::string, Measurements::Sensors &> mapByNameId2;
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
    throw std::runtime_error("Couldn't find sensor in map!");
  }

  void AddSensors(const Classification c, const std::vector<Sensors> &sensors,
                  const int processId = -1)
  {
    SensorGroups sensorGroup{processId, sensors};

    auto existingGroup = data.find(c);
    if (existingGroup != data.end())
    {
      existingGroup->second.push_back(sensorGroup);
      AddMapValues(&existingGroup->second.back().sensors);
    }
    else
    {
      std::vector<SensorGroups> groups{sensorGroup};
      data.insert(std::make_pair(c, groups));
      AddMapValues(&data.find(c)->second.back().sensors);
      // AddMapValues2(data.find(c)->second.back().sensors);
    }
  }
  std::unordered_map<std::string, Measurements::Sensors *> CreateMap() const
  {
    return mapByNameId;
  }
  // std::unordered_map<std::string, Measurements::Sensors &> GetMap() const
  // {
  //   return mapByNameId2;
  // }

private:
  void AddMapValues(const std::vector<Sensors> *sensors)
  {
    for (auto e : *sensors)
    {
      mapByNameId.insert(std::make_pair(e.userId, &e));
    }
  }
  // void AddMapValues2(const std::vector<Sensors> &sensors)
  // {
  //   for (auto e : sensors)
  //   {
  //     mapByNameId2.insert(std::make_pair(e.userId, e));
  //   }
  // }
};

// std::unordered_map<std::string, Sensors> sensors_;

// THIS FUNCTION MUST BE MOVED< ONLY AN EXAMPLE
// void AddGstreamerSensors()
// {
//   std::vector<Sensors> sensors = {Sensors{"fps"}, Sensors{"latency"}};

} // namespace Measurements