#pragma once

#include <unordered_map>
#include <vector>

#include <functional>
#include <iostream>
#include <string>

#include "gstreamer_identifier.h"
#include "src/benchmarks/Linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/gstreamer/measurement_types.h"
#include "src/helpers/helper_functions.h"

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
class CProcessMeasurements
{
public:
  //   CProcessMeasurements(std::vector<CGstreamerHandler *> gstreamerStream);
  CProcessMeasurements() = default;

  void AddPipelines(std::vector<CGstreamerHandler *> gstreamerStream)
  {
    streams_ = gstreamerStream;
  }

  void Initialize(std::vector<Exports::ExportData> *allData);
  void ConcludeMeasurement();

  std::vector<Exports::PipelineInfo> ProcessGstreamer();
  std::vector<Exports::PipelineConfig> GetPipelineConfig() const;
  Exports::MeasurementItem GetPipelineConfig2() const;
  std::vector<Exports::MeasurementItem>
  GetPipelineConfig(const size_t pipelineNr) const;
  std::vector<Exports::MeasurementItem>
  GetMeasurementLabels(const size_t pipelineNr) const;
  std::vector<Exports::MeasurementItem>
  GetPipelineMeasurements(const size_t pipelineNr) const;
  std::vector<Exports::PipelineInfo>
  SortData(const std::vector<Exports::PipelineInfo> &data);
  std::vector<Measurements::Sensors> GetSensors() const;

private:
  std::vector<Exports::ExportData> *allData_;
  std::vector<CGstreamerHandler *> streams_;
  std::vector<std::unordered_map<Identifier, int>> uniqueIds_;

  int GetUniqueId(const size_t pipelineId, const Identifier &id);
  std::unordered_map<int, Identifier>
  GetPluginNames(const size_t pipelineId) const;

  std::string CreateSensorName(const int pipelineNr,
                               const std::string moduleName,
                               MeasureType type) const;
  inline std::unordered_set<int>
  GetUniqueIdsByType(const MeasureType type) const
  {
    std::unordered_set<int> result;
    for (const auto &e : uniqueIds_)
    {
      for (const auto &e2 : e)
      {
        if (e2.first.type == type)
          result.insert(e2.second);
      }
    }
    return result;
  }

  static inline std::vector<MeasureType> predefinedSensors = {MeasureType::FPS};
};
} // namespace GStreamer
