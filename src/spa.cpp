#include "spa.h"

/**
 * @brief Construct a new System Performance Analyzer object
 *
 * @param configFile
 * @param sensorFile
 *
 * @note this constructor can throw when a wrong JSON file is inserted
 */
CSystemPerformanceAnalyzer::CSystemPerformanceAnalyzer(const std::string& configFile, const std::string& sensorFile)
: configFile_{ Core::ConfigParser::Parse(configFile) }
, synchronizer_{ configFile_.processes.size() + 1 } // + 1 because of monitoring thread
, measurements_{ &synchronizer_, sensorFile, configFile_.thresholds }
{
}

/**
 * @brief Starts the configured processes and the measurements
 *
 */
void CSystemPerformanceAnalyzer::StartExecution()
{
  CreateProcesses();
  ExecuteTest();
}

/**
 * @brief Creates the processes that should be executed during a test
 */
void CSystemPerformanceAnalyzer::CreateProcesses()
{
  for (const auto& e : configFile_.processes)
  {
    if (e.type == Core::ProcessType::LINUX_PROCESS)
    {
      processes_.push_back(ProcessInfo{ e, Linux::RunProcess{ &synchronizer_, e } });
      CLogger::Log(CLogger::Types::INFO, "Added linux process: ", e.command);
    }
    else if (e.type == Core::ProcessType::GSTREAMER)
    {
      processes_.push_back(ProcessInfo{ e, CGstreamerHandler{ &synchronizer_, e, configFile_.settings, e.processId } });
      CLogger::Log(CLogger::Types::INFO, "Added GStreamer pipeline: ", e.command);
    }
  }
}

/**
 * @brief Starts each process in a thread and starts the measurements
 */
void CSystemPerformanceAnalyzer::ExecuteTest()
{
  for (auto& e : processes_)
  {
    std::visit(
      Overload{
        [&e](auto& handler) { handler.StartThread(e.command); },
      },
      e.processes);
  }
  // Start the measurements thread
  measurements_.Start(configFile_, &processes_);
}