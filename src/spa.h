#pragma once

#include "src/benchmarks/linux/perf_measurements.h"
#include "src/helpers/synchronizer.h"
#include "src/json_config/config_parser.h"
#include "src/process_runner/run_process_base.h"

#include <signal.h>
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
  void ForceStopExecution();

private:
  Core::SConfig configFile_;
  Synchronizer synchronizer_;

  Linux::CPerfMeasurements measurements_;

  inline static std::vector<ProcessInfo> processes_;
  inline static std::vector<ProcessRunner::Base*> allProcessIds_;
  inline static std::atomic_bool executionInterrupted_ = false;
  inline static std::mutex mutexObj_;

  void CreateProcesses();
  void ExecuteTest();
  void SetSigIntHandler();
  static void SigIntHandler(int s);
};