#include "pipeline_measurements.h"

#include <algorithm>
#include <map>
#include <stdexcept>

#include "src/benchmarks/linux/performance_helpers.h"
#include "src/gstreamer/handler.h"
#include "src/json_config/sensor_config/config.h"

namespace GStreamer
{
void CPipelineMeasurements::Initialize(
    std::vector<Exports::ExportData> *allData)
{
  allData_ = allData;
  // Initialize the uniqueIds_ vector
  for (const auto &e : streams_)
  {
    uniqueIds_.insert(
        std::make_pair(e.first, std::unordered_map<Identifier, int>{}));
  }
}

void CPipelineMeasurements::AddPipelines(
    std::vector<CGstreamerHandler *> gstreamerStream)
{
  for (auto &e : gstreamerStream)
  {
    auto pipelineId = e->GetProcessId();
    streams_.insert(std::make_pair(pipelineId, e));
  }
}

/**
 * @brief Sorts the measuredItems for each pipeline
 * The sorting is based on the ID field
 */
std::vector<Exports::PipelineInfo>
CPipelineMeasurements::SortData(const std::vector<Exports::PipelineInfo> &data)
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
std::vector<Exports::PipelineInfo> CPipelineMeasurements::ProcessGstreamer()
{
  std::vector<Exports::PipelineInfo> result;
  // Check each individual gstreamer pipeline
  for (auto &e : streams_)
  {
    size_t queueSize = e.second->GetMeasurementsSize();

    // Read each measurement of the GStreamer pipeline and put it in a map
    std::vector<PlatformConfig::SMeasureField> measureField_;
    std::unordered_map<Identifier, GStreamer::Measurement> summarizeMap;

    for (size_t qI = 0; qI < queueSize; qI++)
    {
      auto measurement = e.second->GetMeasurement();
      const auto id = Identifier{measurement.type, measurement.pluginName};
      if (summarizeMap.find(id) == summarizeMap.end())
        summarizeMap.insert(std::make_pair(id, measurement));
    }

    // Convert the measured data to an Exports::MeasuredItem type
    Exports::PipelineInfo gsData;
    gsData.pipelineId = e.second->GetProcessId();
    for (const auto &m : summarizeMap)
    {
      Exports::MeasuredItem field{GetUniqueId(e.first, m.first),
                                  static_cast<double>(m.second.valueInt)};
      gsData.measuredItems.push_back(field);
    }
    result.push_back(gsData);
  }
  return result;
}

std::vector<Exports::PipelineConfig>
CPipelineMeasurements::GetPipelineConfig() const
{
  std::vector<Exports::PipelineConfig> result;
  for (const auto &e : streams_)
  {
    Exports::PipelineConfig stream;
    stream.pipelineCommand = e.second->GetPipeline();
    stream.pipelineId = e.first;
    stream.pluginNames = GetPluginNames(e.first);
  }
  return result;
}
Exports::MeasurementItem CPipelineMeasurements::GetPipelineConfig2() const
{
  Exports::MeasurementItem pipelineConfig;
  pipelineConfig.name = "Pipeline config";
  pipelineConfig.type = Exports::Type::INFO;
  // What info should this return? The pipeline command, the pipeline id, and
  // the pipeline measurement values
  std::vector<Exports::MeasurementItem> pipelineItems;
  for (const auto &e : streams_)
  {
    Exports::MeasurementItem item;
    item.name = std::to_string(e.first);
    item.type = Exports::Type::INFO;
    item.value = GetPipelineConfig(e.first);
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
std::vector<Measurements::AllSensors::SensorGroups>
CPipelineMeasurements::GetSensors() const
{
  std::vector<Measurements::AllSensors::SensorGroups> result;

  for (const auto &pipeline : uniqueIds_)
  {
    Measurements::AllSensors::SensorGroups sensorGroup;
    sensorGroup.processId = pipeline.first;

    for (const auto &e : pipeline.second)
    {
      std::vector<Measurements::Sensors> sensors;
      std::string sensorName =
          CreateSensorName(e.first.moduleName, e.first.type);
      Measurements::Sensors sensor{sensorName, e.second};

      sensor.performanceIndicator = GetPerformanceIndicator(e.first.type);
      sensor.SetDataInfo(GetMeasureType(e.first.type));
      sensor.data = PerformanceHelpers::GetSummarizedData(allData_, e.second);
      sensorGroup.sensors.push_back(sensor);
    }
    for (const auto &[type, _] : predefinedSensors)
    {
      auto uniqueIdsSet = GetUniqueIdsByType(type);
      sensorGroup.sensors.push_back(PerformanceHelpers::GetGstCategoriesSummary(
          allData_, uniqueIdsSet, type));
    }
    result.push_back(sensorGroup);
  }
  return result;
}

std::vector<Exports::MeasurementItem>
CPipelineMeasurements::GetPipelineConfig(const int pipelineNr) const
{
  std::vector<Exports::MeasurementItem> result;
  auto stream = streams_.find(pipelineNr);
  if (stream == streams_.end())
    throw std::runtime_error(
        "GStreamer measurements: GetPipelineConfig received "
        "incorrect pipeline number!");
  result.push_back(Exports::MeasurementItem{"Command", Exports::Type::INFO,
                                            stream->second->GetPipeline()});
  result.push_back(Exports::MeasurementItem{"Labels", Exports::Type::ARRAY,
                                            GetMeasurementLabels(pipelineNr)});
  return result;
}

std::vector<Exports::MeasurementItem>
CPipelineMeasurements::GetMeasurementLabels(const int pipelineNr) const
{
  auto ids = uniqueIds_.find(pipelineNr);

  if (ids == uniqueIds_.end())
    throw std::runtime_error(
        "GStreamer measurements: GetMeasurementLabels received "
        "incorrect pipeline number!");
  // Using an ordered map so it automatically orders on the int, which is the
  // unique id of the field
  std::map<int, Exports::MeasurementItem> mapResult;
  for (const auto &e : ids->second)
  {
    Exports::MeasurementItem item;
    item.name = "Label";
    item.type = Exports::Type::LABEL;
    item.value = CreateSensorName(e.first.moduleName, e.first.type, pipelineNr);
    mapResult.insert(std::make_pair(e.second, item));
  }

  std::vector<Exports::MeasurementItem> result;
  std::transform(mapResult.begin(), mapResult.end(), std::back_inserter(result),
                 [](const auto p) { return p.second; });
  return result; // the vector ordered by unique IDs of the measurements
}

/**
 * @brief Get the Plugin Names object
 *
 * @return std::unordered_map<int, Identifier>
 */
std::unordered_map<int, Identifier>
CPipelineMeasurements::GetPluginNames(const int pipelineId) const
{
  auto ids = uniqueIds_.find(pipelineId);

  std::unordered_map<int, Identifier> result;
  for (const auto &e : ids->second)
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
int CPipelineMeasurements::GetUniqueId(const int pipelineId,
                                       const Identifier &id)
{
  auto &idsMap = uniqueIds_.find(pipelineId)->second;
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

/**
 * @brief Create a Sensor Name object
 *
 * @param pipelineNr should be the number from the pipeline, indexed from 0
 */
std::string CPipelineMeasurements::CreateSensorName(
    const std::string moduleName, MeasureType type, const int pipelineNr) const
{
  if (pipelineNr == -1)
    return moduleName + "." + GetMeasureType(type);
  else
    return std::to_string(pipelineNr) + "." + moduleName + "." +
           GetMeasureType(type);
}
} // namespace GStreamer