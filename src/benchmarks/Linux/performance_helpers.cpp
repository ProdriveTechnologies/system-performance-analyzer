#include "performance_helpers.h"

namespace PerformanceHelpers
{
/**
 * @brief Returns an unique ID
 *
 * @return int
 * @note This function uses a static variable for the unique ID, the id is
 * therefore unique in all instances of this class (except when multiple threads
 * execute it concurrently)
 */
int GetUniqueId()
{
  static int uniqueId = 0;
  uniqueId++;
  return uniqueId;
}

} // namespace PerformanceHelpers