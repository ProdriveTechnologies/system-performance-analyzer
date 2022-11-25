#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace Helpers
{
template <typename DataType> std::string GetString(const DataType &data)
{
  std::ostringstream ss;
  ss << data;
  return ss.str();
}

inline bool isNumber(const std::string &str)
{
  for (const char &c : str)
  {
    if (std::isdigit(c) == 0)
      return false;
  }
  return true;
}

bool FileExists(const std::string &fileName);

/**
 * @note function originally from:
 * https://stackoverflow.com/questions/39379411/c-remove-elements-that-are-in-one-vector-from-another
 */
template <typename T>
void RemoveIntersection(std::vector<T> &a, const std::vector<T> &b)
{
  std::unordered_multiset<T> st;
  st.insert(b.begin(), b.end());
  auto predicate = [&st](const T &k) { return st.count(k) >= 1; };
  a.erase(std::remove_if(a.begin(), a.end(), predicate), a.end());
}
} // namespace Helpers