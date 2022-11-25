#pragma once
#include <fstream>

namespace Linux
{

int getParameter(const std::string &location, const std::string &parameterName);
int parseLine(const std::string &line);

} // namespace Linux