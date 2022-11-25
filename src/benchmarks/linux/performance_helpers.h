#pragma once

#include <unordered_set>

#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/json_config/config.h"

namespace PerformanceHelpers
{
int GetUniqueId();

Measurements::SensorData
GetSummarizedData(const std::vector<Exports::ExportData> *data,
                  const int uniqueId, const bool useSteadyState);
std::unordered_map<std::string, Measurements::Sensors>
CreateMapWithId(const std::vector<Measurements::Sensors> &data);
bool HandleThreshold(const Measurements::Sensors *sensor,
                     Core::SThreshold threshold);

Measurements::Sensors
GetGstCategoriesSummary(const std::vector<Exports::ExportData> *data,
                        const std::unordered_set<int> uniqueIds,
                        const GStreamer::MeasureType type, const int pipelineId,
                        const bool useSteadyState);

Measurements::SensorData
GetSummarizedDataSensors(const std::vector<Exports::ExportData> *data,
                         const int uniqueId);

Measurements::Sensors
GetSummarizedDataSensors(const std::vector<Exports::ExportData> *data,
                         const std::unordered_set<int> uniqueId,
                         const std::string &name,
                         const PlatformConfig::SDatafields &field);

Measurements::SensorData
GetSummarizedDataProcesses(const std::vector<Exports::ExportData> *data,
                           const int uniqueId);

Measurements::Sensors
GetSummarizedDataProcesses(const std::vector<Exports::ExportData> *data,
                           const std::unordered_set<int> uniqueId,
                           const std::string &name);

Measurements::SensorData
GetSummarizedData(const Measurements::Classification classification,
                  const std::vector<Measurements::SMeasurementsData> *data,
                  const int uniqueId);

Measurements::Sensors
GetSummarizedData(const Measurements::Classification classification,
                  const std::vector<Measurements::SMeasurementsData> *data,
                  const std::unordered_set<int> uniqueId,
                  const std::string &name);

} // namespace PerformanceHelpers