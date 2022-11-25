#pragma once

#include <chrono>

/**
 * @brief A simple stopwatch
 *
 * The stopwatch can be started, stopped and restarted. The elapsed time of the
 * stopwatch can be requested.
 *
 */
class Stopwatch
{
public:
  Stopwatch();
  ~Stopwatch() = default;

  void Start();
  void Stop();
  void Restart() { Start(); };
  template <typename TimeType = std::milli>
  uint64_t GetTime() const;

private:
  std::chrono::system_clock::time_point startTime_;
  std::chrono::system_clock::time_point stopTime_;
  bool stopped_;
};
inline Stopwatch::Stopwatch()
: stopped_{ false }
{
}
/**
 * @brief starts the stopwatch
 */
inline void Stopwatch::Start()
{
  startTime_ = std::chrono::system_clock::now();
  stopped_ = false;
}

/**
 * @brief stops the stopwatch
 */
inline void Stopwatch::Stop()
{
  stopTime_ = std::chrono::system_clock::now();
  stopped_ = true;
}

/**
 * @brief Returns if the timer has already been elapsed
 *
 * @return true the timer has elapsed
 * @return false the timer has not been elapsed yet
 */
template <typename TimeType>
uint64_t Stopwatch::GetTime() const
{
  auto endTime = stopped_ ? stopTime_ : std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<uint64_t, TimeType>>(endTime - startTime_).count();
}
