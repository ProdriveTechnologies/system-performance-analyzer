#pragma once
#include <nlohmann/json.hpp>
#include <variant>

namespace JsonHelpers
{
template <typename VariantType, typename Variant>
nlohmann::json getSingleVariantValue(const Variant value)
{
  if (const auto *val = std::get_if<VariantType>(&value))
    return nlohmann::json{*val};
  return {};
}

template <typename... VariantTypes, typename VariantType>
nlohmann::json getVariantValue(const VariantType variantType)
{
  std::vector<nlohmann::json> jsonRes = {
      nlohmann::json{}, (getSingleVariantValue<VariantTypes>(variantType))...};

  for (const auto &e : jsonRes)
  {
    if (e != nlohmann::json{})
      return e; // If it has data, return
  }
  return {}; // not found
}
} // namespace JsonHelpers