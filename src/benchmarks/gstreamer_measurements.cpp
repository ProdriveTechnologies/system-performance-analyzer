#include "gstreamer_measurements.h"

#include <algorithm>
#include <map>

#include "src/benchmarks/Linux/performance_helpers.h"
#include "src/gstreamer/handler.h"
#include "src/json_config/sensor_config/config.h"

namespace GStreamer
{
void CProcessMeasurements::Initialize()
{
  // Initialize the uniqueIds_ vector
  for (size_t i = 0; i < streams_.size(); ++i)
  {
    uniqueIds_.push_back(std::unordered_map<Identifier, int>{});
  }
}

/**
 * @brief Sorts the measuredItems for each pipeline
 * The sorting is based on the ID field
 */
std::vector<Exports::PipelineInfo>
CProcessMeasurements::SortData(const std::vector<Exports::PipelineInfo> &data)
{
  std::vector<Exports::PipelineInfo> sorted;
  auto sortFunction = [](const Exports::MeasuredItem &lhs,
                         const Exports::MeasuredItem &rhs) {
    return lhs.id < rhs.id;
  };
  for (auto e : data)
  {
    std::sort(e.measuredItems.begin(), e.measuredItems.end(), sortFunction);
    sorted.push_back(e);
  }
  return sorted;
}

/**
 * @brief Processes the gstreamer live data (which is retrieved directly from
 * the streams)
 *
 * @return std::vector<Exports::PipelineInfo>
 */
std::vector<Exports::PipelineInfo> CProcessMeasurements::ProcessGstreamer()
{
  std::vector<Exports::PipelineInfo> result;
  // Check each individual gstreamer pipeline
  for (size_t i = 0; i < streams_.size(); ++i)
  {
    auto &e = streams_.at(i);
    size_t queueSize = e->GetMeasurementsSize();
    // Read each measurement of the GStreamer pipeline and put it in a map
    std::vector<PlatformConfig::SMeasureField> measureField_;
    std::unordered_map<Identifier, GStreamer::Measurement> summarizeMap;
    for (size_t qI = 0; qI < queueSize; qI++)
    {
      auto measurement = e->GetMeasurement();
      const auto id = Identifier{measurement.type, measurement.pluginName};
      if (summarizeMap.find(id) == summarizeMap.end())
        summarizeMap.insert(std::make_pair(id, measurement));
    }
    // Convert the measured data to an Exports::MeasuredItem type
    Exports::PipelineInfo gsData;
    gsData.pipelineId = e->GetThreadPid();
    for (const auto &m : summarizeMap)
    {
      Exports::MeasuredItem field{GetUniqueId(i, m.first),
                                  static_cast<double>(m.second.valueInt)};
      gsData.measuredItems.push_back(field);
    }
    result.push_back(gsData);
  }
  return result;
}

std::vector<Exports::PipelineConfig>
CProcessMeasurements::GetPipelineConfig() const
{
  std::vector<Exports::PipelineConfig> result;
  for (size_t i = 0; i < streams_.size(); ++i)
  {
    const auto &e = streams_.at(i);
    Exports::PipelineConfig stream;
    stream.pipelineCommand = e->GetPipeline();
    stream.pipelineId = i;
    stream.pluginNames = GetPluginNames(i);
  }
  return result;
}
Exports::MeasurementItem CProcessMeasurements::GetPipelineConfig2() const
{
  Exports::MeasurementItem pipelineConfig;
  pipelineConfig.name = "Pipeline config";
  pipelineConfig.type = Exports::Type::INFO;
  // What info should this return? The pipeline command, the pipeline id, and
  // the pipeline measurement values
  std::vector<Exports::MeasurementItem> pipelineItems;
  for (size_t i = 0; i < streams_.size(); ++i)
  {
    Exports::MeasurementItem item;
    item.name = std::to_string(i);
    item.type = Exports::Type::INFO;
    item.value = GetPipelineConfig(i);
    pipelineItems.push_back(item);
  }

  pipelineConfig.value = pipelineItems;
  return pipelineConfig;
}

/**
 * @brief Returns the sensors supported and measured by the pipeline
 *
 * @return Measurements::Sensors
 */
// Measurements::Sensors GetSensors() const {}

std::vector<Exports::MeasurementItem>
CProcessMeasurements::GetPipelineConfig(const size_t pipelineNr) const
{
  std::vector<Exports::MeasurementItem> result;
  result.push_back(Exports::MeasurementItem{
      "Command", Exports::Type::INFO, streams_.at(pipelineNr)->GetPipeline()});
  result.push_back(Exports::MeasurementItem{"Labels", Exports::Type::ARRAY,
                                            GetMeasurementLabels(pipelineNr)});
  return result;
}

std::vector<Exports::MeasurementItem>
CProcessMeasurements::GetMeasurementLabels(const size_t pipelineNr) const
{
  auto ids = uniqueIds_.at(pipelineNr);
  // Using an ordered map so it automatically orders on the int, which is the
  // unique id of the field
  std::map<int, Exports::MeasurementItem> mapResult;
  for (const auto &e : ids)
  {
    Exports::MeasurementItem item;
    item.name = "Label";
    item.type = Exports::Type::LABEL;
    item.value = std::to_string(pipelineNr) + "." + e.first.moduleName + "." +
                 GetMeasureType(e.first.type);
    mapResult.insert(std::make_pair(e.second, item));
  }

  std::vector<Exports::MeasurementItem> result;
  std::transform(mapResult.begin(), mapResult.end(), std::back_inserter(result),
                 [](const auto p) { return p.second; });
  return result; // the vector ordered by unique IDs of the measurements
}

// std::vector<Exports::MeasurementItem>
// CProcessMeasurements::GetPipelineMeasurements()
// {
//   std::vector<Exports::MeasurementItem> result;
//   for (const auto &e : streams_)
//   {
//     Exports::MeasurementItem item;
//     // item.name =
//   }
// }

/**
 * @brief Get the Plugin Names object
 *
 * @return std::unordered_map<int, Identifier>
 */
std::unordered_map<int, Identifier>
CProcessMeasurements::GetPluginNames(const size_t pipelineId) const
{
  auto ids = uniqueIds_.at(pipelineId);
  std::unordered_map<int, Identifier> result;
  for (const auto &e : ids)
  {
    result.insert(std::make_pair(e.second, e.first));
  }
  return result;
}

/**
 * @brief Retrieves the unique id for a measurement
 * The unique id is based on the module name and field type
 * Because the module names are only known during runtime, the map containing
 * the IDs is also filled during runtime
 */
int CProcessMeasurements::GetUniqueId(const size_t pipelineId,
                                      const Identifier &id)
{
  auto &idsMap = uniqueIds_.at(pipelineId);
  auto result = idsMap.find(id);
  if (result != idsMap.end())
  {
    return result->second;
  }
  else
  {
    int assignedId = PerformanceHelpers::GetUniqueId();
    idsMap.insert(std::make_pair(id, assignedId));
    return assignedId;
  }
}
} // namespace GStreamer