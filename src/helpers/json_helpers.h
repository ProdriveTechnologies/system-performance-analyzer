#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace JsonHelpers
{
template <typename VariantType, typename Variant>
nlohmann::json GetSingleVariantValue(const Variant value)
{
  if (const auto* val = std::get_if<VariantType>(&value))
    return nlohmann::json{ *val };
  return {};
}

template <typename... VariantTypes, typename VariantType>
nlohmann::json GetVariantValue(const VariantType variantType)
{
  std::vector<nlohmann::json> jsonRes = { nlohmann::json{}, (GetSingleVariantValue<VariantTypes>(variantType))... };

  for (const auto& e : jsonRes)
  {
    if (e != nlohmann::json{})
      return e; // If it has data, return
  }
  return {}; // not found
}

nlohmann::json GetJsonObj(const std::string& jsonFile);
std::string ReadFileIntoString(const std::string& path);
} // namespace JsonHelpers