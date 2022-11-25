#include "helper_functions.h"

#include <fstream>
namespace Helpers
{
bool FileExists(const std::string &fileName)
{
  std::ifstream fileObj(fileName);
  return fileObj.good();
}

/**
 * @note function originally from:
 * https://www.techiedelight.com/split-a-string-into-a-vector-in-cpp/
 */
std::vector<std::string> Split(const std::string &text, char delimiter)
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
std::vector<const char *> ToCString(const std::vector<std::string> &text)
{
  std::vector<const char *> cStr;
  for (const auto &e : text)
  {
    cStr.push_back(e.c_str());
  }
  return cStr;
}
} // namespace Helpers