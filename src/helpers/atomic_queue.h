#pragma once

#include <mutex>
#include <queue>

namespace Helpers
{
template <typename T>
class AtomicQueue
{
public:
  void push(const T& value)
  {
    std::lock_guard<std::mutex> lock(mutexObj);
    queueObj.push(value);
  }

  void pop()
  {
    std::lock_guard<std::mutex> lock(mutexObj);
    queueObj.pop();
  }
  T front_pop()
  {
    std::lock_guard<std::mutex> lock(mutexObj);
    auto e = queueObj.front();
    queueObj.pop();
    return e;
  }
  T front() const { return queueObj.front(); }
  T back() const { return queueObj.back(); }
  size_t size() const { return queueObj.size(); }
  bool empty() const { return queueObj.empty(); }

private:
  std::queue<T> queueObj;
  mutable std::mutex mutexObj;
};

} // namespace Helpers