#pragma once

#include "src/linux/filesystem.h"
#include <chrono>
#include <thread>

namespace Linux
{
namespace FileSystem
{
/**
 * @brief CAggregatedLatest is used to get the latest value from aggregated data
 * (which is aggregated during its livetime)
 *
 */
template <typename TimeType = std::milli> class CLiveData
{
public:
  CLiveData(const std::chrono::duration<uint64_t, TimeType> timerLength,
            const int cores)
      : timer_{timerLength}, XAVIER_CORES{cores}
  {
  }
  void Init()
  {
    timer_.start(false);
    aggregatedLatest_ = Linux::FileSystem::GetProcStat();
    while (!timer_.elapsed())
    {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(timer_.timeTillElapsed()));
    }
  }

  Linux::FileSystem::ProcStatData getLastData()
  {
    if (timer_.elapsed())
    {
      timer_.restart();
      auto previousAggregated = aggregatedLatest_;
      aggregatedLatest_ = Linux::FileSystem::GetProcStat();
      lastResult_ = aggregatedLatest_ - previousAggregated;
    }
    return lastResult_;
  }

private:
  Linux::FileSystem::ProcStatData aggregatedLatest_;
  Linux::FileSystem::ProcStatData lastResult_;
  Timer<> timer_;
  const int XAVIER_CORES;
};

} // namespace FileSystem

} // namespace Linux