#pragma once

#include <unordered_map>
#include <vector>

#include <functional>
#include <iostream>
#include <string>

#include "gstreamer_identifier.h"
#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/gstreamer/measurement_types.h"
#include "src/helpers/helper_functions.h"
#include "src/json_config/config.h"

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
  //   CProcessMeasurements(std::vector<CGstreamerHandler *> gstreamerStream);
  CPipelineMeasurements();

  void AddPipelines(std::vector<CGstreamerHandler *> gstreamerStream);

  void Initialize(std::vector<Measurements::SMeasurementsData> *allData);
  void ConcludeMeasurement();

  std::vector<Measurements::SMeasurementGroup> ProcessGstreamer();
  void SetConfig(const Core::SConfig &config) { config_ = config; }
  std::vector<Exports::PipelineConfig> GetPipelineConfig() const;
  Exports::MeasurementItem GetPipelineConfig2() const;
  std::vector<Exports::MeasurementItem>
  GetPipelineConfig(const int pipelineNr) const;
  std::vector<Exports::MeasurementItem>
  GetMeasurementLabels(const int pipelineNr) const;
  static std::vector<Measurements::SMeasurementGroup>
  SortData(const std::vector<Measurements::SMeasurementGroup> &data);
  std::vector<Measurements::AllSensors::SensorGroups>
  GetSensors(const bool summarizeData = true) const;
  void setProctime(const bool proctime)
  {
    enableProctime_ = proctime;
    if (enableProctime_)
      predefinedSensors.push_back(procTime_);
  }

private:
  using PipelineNr = int;
  std::vector<Measurements::SMeasurementsData> *allData_;
  std::unordered_map<PipelineNr, CGstreamerHandler *> streams_;
  std::unordered_map<PipelineNr, std::unordered_map<Identifier, int>>
      uniqueIds_;
  Core::SConfig config_;
  bool enableProctime_;

  int GetUniqueId(const int pipelineId, const Identifier &id);
  std::unordered_map<int, Identifier>
  GetPluginNames(const int pipelineId) const;
  int GetProcessDelay(const int pipelineId) const;

  std::string CreateSensorName(const std::string moduleName, MeasureType type,
                               const int pipelineNr = -1) const;

  bool GetPerformanceIndicator(const MeasureType type) const
  {
    for (const auto &[predefinedType, perfIndicator] : predefinedSensors)
    {
      if (type == predefinedType)
        return perfIndicator;
    }
    return false;
  }
  inline std::unordered_set<int>
  GetUniqueIdsByType(const MeasureType type) const
  {
    std::unordered_set<int> result;
    for (const auto &e : uniqueIds_)
    {
      for (const auto &e2 : e.second)
      {
        if (e2.first.type == type)
          result.insert(e2.second);
      }
    }
    return result;
  }
  using PerformanceIndicator = bool;
  static inline std::vector<std::pair<MeasureType, PerformanceIndicator>>
      predefinedSensors = {{MeasureType::FPS, true}
                           /* MeasureType::LATENCY */};
  static inline const std::pair<MeasureType, PerformanceIndicator> procTime_ =
      std::make_pair(MeasureType::PROCESSING_TIME, false);
};
} // namespace GStreamer
