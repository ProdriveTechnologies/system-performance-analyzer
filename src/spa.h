#pragma once

#include "src/benchmarks/linux/perf_measurements.h"
#include "src/helpers/synchronizer.h"
#include "src/json_config/config_parser.h"

#include <string>

/**
 * @brief The main class to execute the System Performance Analyzer
 * functionalities
 *
 */
class CSystemPerformanceAnalyzer
{
public:
  CSystemPerformanceAnalyzer(const std::string& configFile, const std::string& sensorFile);

  void StartExecution();

private:
  Core::SConfig configFile_;
  Synchronizer synchronizer_;

  Linux::CPerfMeasurements measurements_;

  std::vector<ProcessInfo> processes_;

  void CreateProcesses();
  void ExecuteTest();
};