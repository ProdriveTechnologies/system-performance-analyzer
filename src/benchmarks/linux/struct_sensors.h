#pragma once

#include "src/helpers/helper_functions.h"
#include "src/json_config/sensor_config/config.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Measurements
{
enum class EValueTypes
{
  MIN,
  MAX,
  AVERAGE,
  MEDIAN
};

std::string ToString(const EValueTypes t);

struct SSensorData
{
  struct SMeasureValue
  {
    EValueTypes type;
    double value;
  };
  std::vector<SMeasureValue> summarizedValues;

  double Get(const EValueTypes valueType) const
  {
    for (const auto& e : summarizedValues)
    {
      if (e.type == valueType)
        return e.value;
    }
    throw std::runtime_error("Didn't find valuetype: " + ToString(valueType) + "!");
  }
  std::string Printable() const
  {
    std::string result;
    for (const auto& e : summarizedValues)
      result += ToString(e.type) + ": " + std::to_string(e.value) + " ; ";

    return result;
  }
};

struct SSensors
{
  bool thresholdExceeded = false;
  std::string userId; // UserId is the ID that's given by the user in the
                      // "xavier_config.json" file. If it is in an array, this
                      // userId will have a number of the array behind it
  int uniqueId;
  PlatformConfig::EClass classType;
  std::string suffix;
  double multiplier = 1.0;

  bool performanceIndicator = false;
  bool measuredRaw = false;

  PlatformConfig::SDatafields userData;

  // The summarized data, such as average, minimum found, maximum found
  SSensorData data;

  // Constructor with the name and unique id
  SSensors(const std::string& userId_,
           const int uniqueId_,
           PlatformConfig::EClass sensorClass_ = PlatformConfig::EClass::NONE)
  : userId{ userId_ }
  , uniqueId{ uniqueId_ }
  , classType{ sensorClass_ }
  {
  }
  // Constructor with an SDataFields object
  explicit SSensors(const PlatformConfig::SDatafields& fields)
  : userId{ fields.name }
  , uniqueId{ fields.id }
  , classType{ fields.classType }
  , suffix{ fields.suffix }
  , multiplier{ fields.multiplier }
  , measuredRaw{ true }
  , userData{ fields }
  {
  }
  SSensors() = default;
  void SetDataInfo(const std::string& suffix_, const double multiplier_ = 1.0)
  {
    suffix = suffix_;
    multiplier = multiplier_;
  }
  // Something of a pointer to the necessary data
};

enum class EClassification
{
  PIPELINE,
  PROCESSES,
  SYSTEM
};

inline std::unordered_map<EClassification, std::string> classificationToString = {
  { EClassification::PIPELINE, "PipelineMeasurements" },
  { EClassification::PROCESSES, "ProcessMeasurements" },
  { EClassification::SYSTEM, "SystemResources" }
};

inline std::string GetClassificationStr(const EClassification c)
{
  auto res = classificationToString.find(c);
  if (res != classificationToString.end())
    return res->second;
  throw std::runtime_error("Could not find translation for Classification!");
}

inline void SetClassificationStr(const EClassification c, const std::string& newText)
{
  auto res = classificationToString.find(c);
  if (res != classificationToString.end())
    res->second = newText;
  else
    classificationToString.insert(std::make_pair(c, newText));
}

using SensorName = std::string;
using SensorProcessId = int;
using SensorIdentifier = std::pair<SensorName, SensorProcessId>;

// Hash function for the Measurements::SensorIdentifier field
struct SensorIdHash
{
  size_t operator()(const Measurements::SensorIdentifier& k) const;
};

/**
 * @brief This struct can contain all the sensors of the different types of
 * measurements. It contains functions to make it easy to access the data
 */
struct SAllSensors
{
  std::vector<EClassification> allClasses;
  struct SSensorGroups
  {
    int processId; // Only used for EClassification::PIPELINE and
    // EClassification::PROCESSES
    std::vector<SSensors> sensors;
    int processDelay = -1;
    SSensorGroups(const int processId_, const std::vector<SSensors>& sensors_)
    : processId{ processId_ }
    , sensors{ sensors_ }
    {
    }
    SSensorGroups() = default;
  };

  // All the sensors classified by the different groups (Pipeline data, process data, system data)
  std::unordered_map<EClassification, std::vector<SSensorGroups>> data;

  using SensorMap = std::unordered_map<std::string, Measurements::SSensors*>;
  using ProcessId = int;
  using SensorMapByProcess = std::unordered_map<ProcessId, SensorMap>;
  SensorMapByProcess mapByProcessId;

  SensorMap GetMap(const int processId) const;

  std::vector<SSensors> GetSensors(const EClassification c, const int processId = -1) const;

  EClassification GetClassification(const int uniqueId) const;

  /**
   * @brief Returns all process IDs
   */
  std::unordered_set<int> GetProcesses() const;

  void AddSensors(const EClassification c, const std::vector<SSensors>& sensors, const int processId = -1);
  void AddSensors(const EClassification c, const std::vector<SSensorGroups>& sensors);
  std::vector<SSensorGroups> GetSensorGroups(const EClassification classification) const;

private:
  void AddMapValues(SensorMapByProcess* mapObj, std::vector<SSensors>* sensors, const int processId) const;
};

} // namespace Measurements
