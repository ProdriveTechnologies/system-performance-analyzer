#pragma once

#include <string>

namespace Globals
{
constexpr std::string_view compilationDate = __DATE__;
constexpr std::string_view compilationTime = __TIME__;
constexpr std::string_view version = "0.01";
} // namespace Globals