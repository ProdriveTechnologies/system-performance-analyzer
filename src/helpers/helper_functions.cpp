#include "helper_functions.h"

#include <fstream>
namespace Helpers
{
bool FileExists(const std::string &fileName)
{
  std::ifstream fileObj(fileName);
  return fileObj.good();
}
} // namespace Helpers