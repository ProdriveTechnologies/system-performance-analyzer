#include "Helpers.h"
#include <fstream>

namespace Linux
{

/**
 * @brief getParameter gets a parameter from a configuration file
 *
 * @param location the file location that needs to be parsed
 * @param parameterName the name of the parameter of which the value is returned
 * @return int the found value of the parameter
 */
int getParameter(const std::string &location, const std::string &parameterName)
{
  std::fstream file(location, std::fstream::in);

  std::string line;
  size_t size = parameterName.size();

  while (getline(file, line))
  {
    if (line.size() > size && line.substr(0, size) == parameterName)
    {
      break;
    }
  }
  return parseLine(line);
}

/**
 * @brief parses a line in a file from linux
 *
 * @param line the line that needs to be parsed
 * @return int the number that is read on the parsed line
 */
int parseLine(const std::string &line)
{
  std::string result;
  for (auto e : line)
  {
    if (e >= '0' && e <= '9')
    {
      result += e;
    }
  }
  if (result.size() > 0)
    return std::stoi(result);
  else
    throw std::runtime_error("Incorrect string to parse");
}

} // namespace Linux