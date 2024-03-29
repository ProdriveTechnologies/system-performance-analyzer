#include "helper_functions.h"

#include <fstream>

namespace Helpers
{
bool FileExists(const std::string& fileName)
{
  std::ifstream fileObj(fileName);
  return fileObj.good();
}

/**
 * @note function originally from:
 * https://www.techiedelight.com/split-a-string-into-a-vector-in-cpp/
 */
std::vector<std::string> Split(const std::string& text, char delimiter)
{
  std::vector<std::string> result;
  std::stringstream textSs(text);

  while (textSs.good())
  {
    std::string substr;
    getline(textSs, substr, delimiter);
    if (substr.empty())
      continue;
    result.push_back(substr);
  }
  return result;
}

/**
 * @note function originally from:
 * https://www.techiedelight.com/split-a-string-into-a-vector-in-cpp/
 */
std::vector<const char*> ToCString(const std::vector<std::string>& text)
{
  std::vector<const char*> cStr;
  for (const auto& e : text)
  {
    cStr.push_back(e.c_str());
  }
  return cStr;
}

void replaceStr(std::string& data, const std::string toReplace, const std::string replacementStr)
{
  size_t pos = data.find(toReplace);

  while (pos != std::string::npos)
  {
    data.replace(pos, toReplace.size(), replacementStr);
    // Get the next occurrence from the current position
    pos = data.find(toReplace, pos + replacementStr.size());
  }
}

/**
 * @brief Converts decimals, like 1591 (standing for 1591 milliseconds) to an
 * int with only the largest numbers defined by "decimals"
 *
 * @NOTE: For proper working, data.size() >= decimals
 * @param data
 * @param decimals
 * @return int
 */
int DecimalsToInt(const std::string& data, const unsigned int decimals)
{
  if (data.size() == 0 || decimals == 0)
    return 0;

  size_t largestIndex = data.size() < decimals ? data.size() : decimals;

  return std::stoi(data.substr(0, largestIndex));
}

} // namespace Helpers