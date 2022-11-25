#include <utility>

namespace Helpers
{
/**
 * @brief In C++23, can be replaced by std::to_underlying
 */
template <class Enum>
constexpr std::underlying_type_t<Enum> ToUnderlying(const Enum value)
{
  return static_cast<std::underlying_type_t<Enum>>(value);
}

} // namespace Helpers