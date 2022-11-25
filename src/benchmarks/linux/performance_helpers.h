#pragma once

#include <unordered_set>

#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/json_config/config.h"

namespace PerformanceHelpers
{
int GetUniqueId();

std::unordered_map<std::string, Measurements::Sensors>
CreateMapWithId(const std::vector<Measurements::Sensors> &data);
bool HandleThreshold(const Measurements::Sensors *sensor,
                     Core::SThreshold threshold);

Measurements::SensorData
GetSummarizedData(const Measurements::Classification classification,
                  const std::vector<Measurements::SMeasurementsData> *data,
                  const int uniqueId, const double multiplier,
                  const bool useSteadyState = false);

Measurements::Sensors
GetSummarizedData(const Measurements::Classification classification,
                  const std::vector<Measurements::SMeasurementsData> *data,
                  const std::unordered_set<int> uniqueId,
                  const Measurements::Sensors &sensorTemplate,
                  const bool useSteadyState = false);

} // namespace PerformanceHelpers