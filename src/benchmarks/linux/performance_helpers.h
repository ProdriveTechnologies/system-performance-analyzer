#pragma once

#include "src/benchmarks/linux/struct_measurements.h"
#include "src/benchmarks/linux/struct_sensors.h"
#include "src/exports/export_struct.h"
#include "src/json_config/config.h"

#include <unordered_set>

namespace PerformanceHelpers
{
int GetUniqueId();

bool HandleThreshold(const Measurements::SSensors* sensor, Core::SThreshold threshold);

Measurements::SSensorData GetSummarizedData(const Measurements::EClassification classification,
                                            const std::vector<Measurements::SMeasurementsData>* data,
                                            const int uniqueId,
                                            const double multiplier,
                                            const bool useSteadyState = false);

Measurements::SSensors GetSummarizedData(const Measurements::EClassification classification,
                                         const std::vector<Measurements::SMeasurementsData>* data,
                                         const std::unordered_set<int>& uniqueId,
                                         const Measurements::SSensors& sensorTemplate,
                                         const bool useSteadyState = false);

} // namespace PerformanceHelpers