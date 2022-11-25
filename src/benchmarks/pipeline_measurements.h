#pragma once

#include "gstreamer_identifier.h"
#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/gstreamer/measurement_types.h"
#include "src/helpers/helper_functions.h"
#include "src/json_config/config.h"

#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class CGstreamerHandler; // Forward declaration for compilation improvement

namespace GStreamer
{
/**
 * @brief This class manages the measurements from GStreamer. It manages the
 * following tasks:
 * 1. Read the measurements from the GStreamer thread
 * 2. Parse the measurements and store it internally
 * 3. Provide structures to store it efficiently
 * 4. Support exports and analysis
 * 5. Detect performance decreases
 */
class CPipelineMeasurements
{
public:
  CPipelineMeasurements();

  void AddPipelines(std::vector<CGstreamerHandler*> gstreamerStream);

  void Initialize(std::vector<Measurements::SMeasurementsData>* allData);
  void ConcludeMeasurement();

  std::vector<Measurements::SMeasurementGroup> ProcessGstreamer();
  void SetConfig(const Core::SConfig& config) { config_ = config; }
  std::vector<Exports::PipelineConfig> GetPipelineConfig() const;
  Exports::SMeasurementItem GetPipelineConfig2() const;
  std::vector<Exports::SMeasurementItem> GetPipelineConfig(const int pipelineNr) const;
  std::vector<Exports::SMeasurementItem> GetMeasurementLabels(const int pipelineNr) const;
  static std::vector<Measurements::SMeasurementGroup> SortData(
    const std::vector<Measurements::SMeasurementGroup>& data);
  std::vector<Measurements::SAllSensors::SSensorGroups> GetSensors(const bool summarizeData = true) const;
  void setProctime(const bool proctime)
  {
    enableProctime_ = proctime;
    if (enableProctime_)
      predefinedSensors.push_back(procTime_);
  }

private:
  using PipelineNr = int;
  std::vector<Measurements::SMeasurementsData>* allData_;
  std::unordered_map<PipelineNr, CGstreamerHandler*> streams_;
  std::unordered_map<PipelineNr, std::unordered_map<SIdentifier, int>> uniqueIds_;
  Core::SConfig config_;
  bool enableProctime_;

  int GetUniqueId(const int pipelineId, const SIdentifier& id);
  std::unordered_map<int, SIdentifier> GetPluginNames(const int pipelineId) const;
  int GetProcessDelay(const int pipelineId) const;

  std::string CreateSensorName(const std::string& moduleName, EMeasureType type, const int pipelineNr = -1) const;

  bool GetPerformanceIndicator(const EMeasureType type) const
  {
    for (const auto& [predefinedType, perfIndicator] : predefinedSensors)
    {
      if (type == predefinedType)
        return perfIndicator;
    }
    return false;
  }
  inline std::unordered_set<int> GetUniqueIdsByType(const EMeasureType type, const int pipelineNr) const
  {
    std::unordered_set<int> result;
    for (const auto& e : uniqueIds_)
    {
      if (e.first != pipelineNr)
        continue;
      for (const auto& e2 : e.second)
      {
        if (e2.first.type == type)
          result.insert(e2.second);
      }
    }
    return result;
  }
  using PerformanceIndicator = bool;
  static inline std::vector<std::pair<EMeasureType, PerformanceIndicator>>
    predefinedSensors = { { EMeasureType::FPS, true }
                          /* MeasureType::LATENCY */ };
  static inline const std::pair<EMeasureType, PerformanceIndicator> procTime_ =
    std::make_pair(EMeasureType::PROCESSING_TIME, false);
};
} // namespace GStreamer
