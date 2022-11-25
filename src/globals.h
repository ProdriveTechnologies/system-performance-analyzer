#pragma once

#include <string>

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif

namespace Globals
{
constexpr std::string_view compilationDate = __DATE__;
constexpr std::string_view compilationTime = __TIME__;
constexpr std::string_view version = "0.10";

constexpr double strongCorrelation = 0.8;
} // namespace Globals