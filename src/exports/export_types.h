#pragma once

#include "src/exports/translations.h"

#include <unordered_map>

namespace Exports
{
enum class ETypes
{
  JSON,
  GRAPHS,
  CSV,
  TERMINAL_SUMMARY,
  TERMINAL_UI
};

const inline std::unordered_map<std::string, ETypes> stringToEtypes = { { "json", ETypes::JSON },
                                                                        { "graphs", ETypes::GRAPHS },
                                                                        { "csv", ETypes::CSV },
                                                                        { "terminal", ETypes::TERMINAL_SUMMARY } };

inline std::string GetSupportedTypes()
{
  std::string result;
  for (const auto& e : stringToEtypes)
  {
    result += "\"" + e.first + "\", ";
  }
  return result;
}

inline ETypes GetExportType(const std::string& exportName)
{
  auto result = stringToEtypes.find(exportName);
  if (result != stringToEtypes.end())
    return result->second;

  throw std::runtime_error(TranslationErrors::errorExportType + " " + GetSupportedTypes());
}

} // namespace Exports