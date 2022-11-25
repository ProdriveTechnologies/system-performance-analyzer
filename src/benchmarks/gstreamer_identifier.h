#pragma once

//#include <functional>
#include <string>

#include "src/gstreamer/measurement_types.h"
#include "src/helpers/helper_functions.h"

/**
 * @brief Placed in its own file because the hash function is necessary for an
 * unordered_map
 *
 */
namespace GStreamer
{
struct Identifier
{
  GStreamer::MeasureType type;
  std::string moduleName;
  bool operator==(const Identifier &r) const
  {
    return ((type == r.type) && (moduleName == r.moduleName));
  }
};
} // namespace GStreamer

namespace std
{
template <> struct hash<GStreamer::Identifier>
{
  inline size_t operator()(const GStreamer::Identifier &k) const
  {
    // computes the hash of a GStreamer::Identifier using a variant
    // of the Fowler-Noll-Vo hash function
    // from: https://en.cppreference.com/w/cpp/utility/hash/operator()
    size_t result = 2166136261;

    for (size_t i = 0, ie = k.moduleName.size(); i != ie; ++i)
    {
      result = (result * 16777619) ^ k.moduleName[i];
    }

    return result ^ (Helpers::ToUnderlying(k.type) << 1);
  }
};

} // namespace std