#pragma once

#include <chrono>

/**
 * @brief A simple timer
 *
 * The timer can be started and restarted. The length of the timer can be
 * adjusted. And it can be checked if the timer has already been elapsed.
 *
 */
template <typename Rep = int64_t, typename TimeType = std::milli>
class Timer
{
public:
  Timer(const std::chrono::duration<Rep, TimeType> timerLength);
  ~Timer() = default;

  void start(const bool alreadyElapsed);
  void changeTime(const std::chrono::duration<Rep, TimeType> timerLength) { _timerLength = timerLength; }
  void restart();
  bool elapsed() const;
  template <typename LocalTimeType = std::milli>
  uint64_t timeTillElapsed() const
  {
    const auto timeNow = std::chrono::system_clock::now();
    auto timePassed = std::chrono::duration_cast<std::chrono::duration<uint64_t, TimeType>>(_startingTime - timeNow);
    auto timeTillElapsed =
      std::chrono::duration_cast<std::chrono::duration<uint64_t, TimeType>>(_timerLength - timePassed);
    return timeTillElapsed.count();
  }

private:
  std::chrono::time_point<std::chrono::system_clock> _startingTime;
  std::chrono::duration<Rep, TimeType> _timerLength;
};

/**
 * @brief Construct a new Timer object
 *
 * @param timerLength a std::chrono time containing the length of the timer.
 *
 * For example, this class can be constructed with a timer of 1 second like
 * this: Timer timer(std::chrono::milliseconds{1000});
 * @note it is important to use the "{}" around the "std::chrono::milliseconds"
 * in the constructor else, it could give errors (most vexing parse)
 */
template <typename Rep, typename TimeType>
Timer<Rep, TimeType>::Timer(const std::chrono::duration<Rep, TimeType> timerLength)
: _timerLength{ timerLength }
{
}

/**
 * @brief starts the timer
 *
 * @param alreadyElapsed defines if the timer should already be elapsed when
 * starting (so the timer can be reset with the restart() function)
 */
template <typename Rep, typename TimeType>
void Timer<Rep, TimeType>::start(const bool alreadyElapsed)
{
  _startingTime = alreadyElapsed ? std::chrono::system_clock::now() - _timerLength : std::chrono::system_clock::now();
}

/**
 * @brief Restarts the timer
 *
 */
template <typename Rep, typename TimeType>
void Timer<Rep, TimeType>::restart()
{
  _startingTime = std::chrono::system_clock::now();
}

/**
 * @brief Returns if the timer has already been elapsed
 *
 * @return true the timer has elapsed
 * @return false the timer has not been elapsed yet
 */
template <typename Rep, typename TimeType>
bool Timer<Rep, TimeType>::elapsed() const
{
  auto currentTime = std::chrono::system_clock::now();
  auto timeDifference = std::chrono::duration_cast<std::chrono::duration<Rep, TimeType>>(currentTime - _startingTime);

  return (timeDifference > _timerLength);
}
