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
  SetSigIntHandler();
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
      auto process = std::get<Linux::RunProcess>(processes_.back().processes);
      CLogger::Log(CLogger::Types::INFO, "Added linux process: ", e.command);
    }
    else if (e.type == Core::ProcessType::GSTREAMER)
    {
      auto process2 = CGstreamerHandler{ &synchronizer_, e, configFile_.settings, e.processId };
      processes_.push_back(ProcessInfo{ e, process2 });
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

/**
 * @brief Sets the SIGINT interrupt handler
 */
void CSystemPerformanceAnalyzer::SetSigIntHandler()
{
  signal(SIGINT, CSystemPerformanceAnalyzer::SigIntHandler);
}

/**
 * @brief Catches a signal, stops the test and starts the analysis
 *
 * @param s
 */
void CSystemPerformanceAnalyzer::SigIntHandler([[maybe_unused]] int s)
{
  // Only one instance may execute this code
  std::lock_guard<std::mutex> guard(mutexObj_);
  if (executionInterrupted_)
    return;
  executionInterrupted_ = true;

  // The tests are separate applications, thus these need to be killed
  // Necessities to work: All PIDs to kill (that's most likely it)
  for (const auto& e : processes_)
  {
    if (const Linux::RunProcess* pval = std::get_if<Linux::RunProcess>(&e.processes))
    {
      auto pid = pval->GetApplicationPid();
      if (pid == 0)
        std::cout << "Process ID is 0, thus has stopped" << std::endl;
      else
        kill(pid, SIGTERM);
    }
    else
      continue; // Continue to next if its a GStreamerHandler object, as this one receives the signal from within the
                // process
  }
  return;
}