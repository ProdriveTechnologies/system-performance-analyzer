#include "pipeline_measurements.h"

#include <algorithm>
#include <map>
#include <stdexcept>

#include "src/benchmarks/linux/performance_helpers.h"
#include "src/gstreamer/handler.h"
#include "src/json_config/sensor_config/config.h"

namespace GStreamer
{
CPipelineMeasurements::CPipelineMeasurements() : enableProctime_{false} {}
void CPipelineMeasurements::Initialize(
    std::vector<Measurements::SMeasurementsData> *allData)
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
std::vector<Measurements::SMeasurementGroup> CPipelineMeasurements::SortData(
    const std::vector<Measurements::SMeasurementGroup> &data)
{
  std::vector<Measurements::SMeasurementGroup> sorted;
  auto sortFunction = [](const Measurements::SMeasuredItem &lhs,
                         const Measurements::SMeasuredItem &rhs) {
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
 * @return std::vector<Measurements::SMeasurementGroup>
 */
std::vector<Measurements::SMeasurementGroup>
CPipelineMeasurements::ProcessGstreamer()
{
  std::vector<Measurements::SMeasurementGroup> result;
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

    // Convert the measured data to an Measurements::SMeasuredItem type
    Measurements::SMeasurementGroup gsData;
    gsData.pipelineId = e.second->GetProcessId();
    for (const auto &m : summarizeMap)
    {
      Measurements::SMeasuredItem field{GetUniqueId(e.first, m.first),
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
  pipelineConfig.name = "PipelineMeasurements";
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
 * @note For the live mode, we need to assign one "fps" measurement to
 * visualize. Because it is shown during runtime, we don't have access to the
 * averages. Therefore, the "fps" of the first component will be marked for the
 * live view (For each MeasureType, one component will be marked)
 */
std::vector<Measurements::AllSensors::SensorGroups>
CPipelineMeasurements::GetSensors(const bool summarizeData) const
{
  std::vector<Measurements::AllSensors::SensorGroups> result;

  for (const auto &pipeline : uniqueIds_)
  {
    std::cout << "Unique ID execution: " << pipeline.first << std::endl;
    std::unordered_set<GStreamer::MeasureType> livemodeTypes;
    Measurements::AllSensors::SensorGroups sensorGroup;
    sensorGroup.processId = pipeline.first;
    sensorGroup.processDelay = GetProcessDelay(pipeline.first);
    const bool useSteadyState =
        config_.GetProcess(pipeline.first).useSteadyState;

    for (const auto &e : pipeline.second)
    {
      std::vector<Measurements::Sensors> sensors;
      std::string sensorName =
          CreateSensorName(e.first.moduleName, e.first.type);
      Measurements::Sensors sensor{sensorName, e.second};
      sensor.measuredRaw = true;

      sensor.performanceIndicator = GetPerformanceIndicator(e.first.type);
      sensor.SetDataInfo(GetMeasureType(e.first.type));
      // TODO: remove magic numbers, ideally, make it configurable by the user
      sensor.userData.minimumValue = 0;
      sensor.userData.maximumValue = 250;
      if (livemodeTypes.find(e.first.type) == livemodeTypes.end())
      {
        livemodeTypes.insert(e.first.type);
        sensor.userData.showInLive = true;
      }

      if (summarizeData)
      {
        sensor.data = PerformanceHelpers::GetSummarizedData(
            Measurements::Classification::PIPELINE, allData_, e.second,
            sensor.multiplier, useSteadyState);
        // TODO: remove
        std::cout << std::to_string(sensorGroup.processId) + ": "
                  << sensor.userId << ":" << sensor.data.Printable()
                  << std::endl;
      }
      sensorGroup.sensors.push_back(sensor);
    }

    if (summarizeData)
    {
      for (const auto &[type, _] : predefinedSensors)
      {
        auto uniqueIdsSet = GetUniqueIdsByType(type, pipeline.first);
        Measurements::Sensors sensorTemplate{
            GStreamer::GetMeasureType(type), PerformanceHelpers::GetUniqueId(),
            PlatformConfig::EClass::PIPELINE_MEASUREMENTS};
        sensorTemplate.suffix = GStreamer::GetMeasureType(type);

        sensorGroup.sensors.push_back(PerformanceHelpers::GetSummarizedData(
            Measurements::Classification::PIPELINE, allData_, uniqueIdsSet,
            sensorTemplate, useSteadyState));
      }
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

/**
 * @brief Returns the delay of a pipeline
 *
 * @param pipelineId
 * @return int
 */
int CPipelineMeasurements::GetProcessDelay(const int pipelineId) const
{
  for (const auto &e : streams_)
  {
    if (e.second->GetUserProcessId() == pipelineId)
    {
      return e.second->GetProcessDelay();
    }
  }
  throw std::runtime_error("Could not find process with pipelineId");
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