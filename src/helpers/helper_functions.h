#pragma once

#include <algorithm>
#include <sstream>
#include <string>
#include <type_traits> // std::underlying_type
#include <unordered_set>
#include <vector>

namespace Helpers
{
/** @brief ArgToString converts arguments to a string
 *
 * @param args constant arguments list which will be converted to a string
 * @return a string with all the arguments
 */
template <typename... Args> std::string ArgToString(const Args &... args)
{
  std::ostringstream ss;
  ((ss << args), ...);
  return ss.str();
}

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

std::vector<std::string> Split(const std::string &text, char delimiter);
std::vector<const char *> ToCString(const std::vector<std::string> &text);

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

template <typename Enum>
constexpr typename std::underlying_type<Enum>::type ToUnderlying(Enum e)
{
  return static_cast<typename std::underlying_type<Enum>::type>(e);
}

} // namespace Helpers