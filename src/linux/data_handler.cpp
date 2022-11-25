#include "data_handler.h"

namespace Linux
{
void CDataHandler::Initialize(
    std::unordered_map<PlatformConfig::Types, Config> parsers,
    const std::vector<PlatformConfig::SDatafields> &datafields)
{
  parsers_ = parsers;
  datafields_ = datafields;
  for (const auto &e : parsers_)
  {
    e.second.parserObj->Initialize();
  }
}

bool CDataHandler::ParseMeasurements(const std::string &replacement)
{
  lastMeasurements_ = std::vector<Exports::MeasuredItem>{};

  // Initializes the parameters that need to be loaded once during every
  // measurement, such as retrieving data from the /proc/stat file
  for (const auto &parser : parsers_)
  {
    auto isSuccesful = parser.second.parserObj->InitializeRuntime(replacement);
    if (!isSuccesful)
      return false; // Initializing failed, return error
  }

  // Loop through all measurement fields
  for (const auto &e : datafields_)
  {
    auto parser = parsers_.find(e.type);
    if (parser == parsers_.end())
      throw std::runtime_error("LinuxDataHandler: Parser not found!");

    std::string correctedPath = e.path;
    if (!parser->second.replacementTag.empty())
      Helpers::replaceStr(correctedPath, parser->second.replacementTag,
                          replacement);

    bool isSuccesful = parser->second.parserObj->ParseMeasurement(
        e, correctedPath, replacement);
    if (!isSuccesful)
      return false;
    lastMeasurements_.push_back(parser->second.parserObj->GetMeasurement());
  }
  return true;
}

} // namespace Linux