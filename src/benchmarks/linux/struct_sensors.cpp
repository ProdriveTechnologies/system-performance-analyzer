#include "struct_sensors.h"

namespace Measurements
{
std::string ToString(const EValueTypes t)
{
  switch (t)
  {
  case EValueTypes::MIN:
    return "min";
  case EValueTypes::MAX:
    return "max";
  case EValueTypes::AVERAGE:
    return "average";
  case EValueTypes::MEDIAN:
    return "median";
  default:
    throw std::runtime_error("ValueTypes not recognised in Measurements::EValueTypes!");
  }
}

size_t SensorIdHash::operator()(const Measurements::SensorIdentifier& k) const
{
  // computes the hash of a Measurements::SensorIdentifier using a variant
  // of the Fowler-Noll-Vo hash function
  // from: https://en.cppreference.com/w/cpp/utility/hash/operator()
  size_t result = 2166136261;

  for (size_t i = 0, ie = k.first.size(); i != ie; ++i)
  {
    result = (result * 16777619) ^ k.first[i];
  }
  return result ^ (k.second << 1);
}

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
std::vector<SSensors> SAllSensors::GetSensors(const EClassification c, const int processId) const
{
  auto group = data.find(c);
  if (group != data.end())
  {
    if (processId == -1 && group->second.size() == 1)
      return group->second.front().sensors;
    std::vector<SSensors> allPipelines;
    for (const auto& e : group->second)
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

EClassification SAllSensors::GetClassification(const int uniqueId) const
{
  for (const auto& classification : allClasses)
  {
    auto sensors = GetSensors(classification);
    for (const auto& sensor : sensors)
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
std::unordered_set<int> SAllSensors::GetProcesses() const
{
  std::unordered_set<int> result;
  for (const auto& [classifier, sensors] : data)
  {
    for (const auto& processSensors : sensors)
    {
      result.insert(processSensors.processId);
    }
  }
  return result;
}

void SAllSensors::AddSensors(const EClassification c, const std::vector<SSensors>& sensors, const int processId)
{
  SSensorGroups sensorGroup{ processId, sensors };
  AddSensors(c, { sensorGroup });
}

void SAllSensors::AddSensors(const EClassification c, const std::vector<SSensorGroups>& sensors)
{
  auto existingGroup = data.find(c);

  if (existingGroup != data.end())
  {
    // Group already exists, overwriting old data with new sensorgroups
    existingGroup->second = sensors;
    // And adding the values to the internal map as well
    for (auto& e : existingGroup->second)
      AddMapValues(&mapByProcessId, &e.sensors, e.processId);
  }
  else
  {
    // Group does not exist, thus create the group
    data.insert(std::make_pair(c, sensors));
    allClasses.push_back(c);
    for (auto& e : data.find(c)->second)
      AddMapValues(&mapByProcessId, &e.sensors, e.processId);
  }
}
std::vector<SAllSensors::SSensorGroups> SAllSensors::GetSensorGroups(const EClassification classification) const
{
  auto sensorGroup = data.find(classification);
  if (sensorGroup == data.end())
    return {};
  return sensorGroup->second;
}
SAllSensors::SensorMap SAllSensors::GetMap(const int processId) const
{
  auto mapResult = mapByProcessId.find(processId);
  if (mapResult == mapByProcessId.end())
  {
    throw std::runtime_error("Process ID not existent!");
  }
  return mapResult->second;
}
void SAllSensors::AddMapValues(SensorMapByProcess* mapObj, std::vector<SSensors>* sensors, const int processId) const
{
  auto mapResult = mapObj->find(processId);
  if (mapResult == mapObj->end())
  {
    mapObj->insert(std::make_pair(processId, SensorMap{}));
    mapResult = mapObj->find(processId);
  }
  for (auto& e : *sensors)
  {
    mapResult->second.insert(std::make_pair(e.userId, &e));
  }
}

} // namespace Measurements