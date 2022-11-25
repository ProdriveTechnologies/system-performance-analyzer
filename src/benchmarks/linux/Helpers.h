#pragma once
#include <fstream>

namespace Linux
{
int GetParameter(const std::string &location, const std::string &parameterName);
int ParseLine(const std::string &line);

} // namespace Linux