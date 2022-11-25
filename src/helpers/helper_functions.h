#pragma once

#include <string>

namespace Helpers
{
inline bool isNumber(const std::string &str)
{
  for (const char &c : str)
  {
    if (std::isdigit(c) == 0)
      return false;
  }
  return true;
}
} // namespace Helpers