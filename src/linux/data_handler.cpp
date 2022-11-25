#include "data_handler.h"

#include "src/benchmarks/linux/performance_helpers.h"
namespace Linux
{
/**
 * @brief creates a map with the format necessary for initializing the
 * CDataHandler class
 */
std::unordered_map<PlatformConfig::Types, Linux::CDataHandler::Config>
GetDatahandlerMap(const std::vector<Linux::SDataHandlers> &dataHandlers,
                  const std::string &replacementTag)
{
  std::unordered_map<PlatformConfig::Types, Linux::CDataHandler::Config> result;
  for (auto &e : dataHandlers)
  {
    Linux::CDataHandler::Config config;

    // Get the pointer to the correct object in the variant
    std::visit(
        Overload{
            [&config](auto &handler) { config.parserObj = handler.get(); },
        },
        e.datahandler);
    config.replacementTag = replacementTag;
    result.insert(std::make_pair(e.type, config));
  }
  return result;
}

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

bool CDataHandler::ParseMeasurements(const std::string &replacement,
                                     const int masterId)
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
  for (auto e : datafields_)
  {
    auto parser = parsers_.find(e.type);
    if (parser == parsers_.end())
      throw std::runtime_error("LinuxDataHandler: Parser not found!");

    std::string correctedPath = e.path;
    if (!parser->second.replacementTag.empty())
      Helpers::replaceStr(correctedPath, parser->second.replacementTag,
                          replacement);

    auto uniqueId = masterId > 0 ? CreateUniqueId(masterId, e.id) : e.id;
    e.id = uniqueId;
    bool isSuccesful = parser->second.parserObj->ParseMeasurement(
        e, correctedPath, replacement);
    if (!isSuccesful)
      return false;
    lastMeasurements_.push_back(parser->second.parserObj->GetMeasurement());
  }
  return true;
}

/**
 * @brief Adds the unique ID from the map if it doesnt exist, else it just
 * returns the existing unique id
 *
 * @param masterId
 * @param datafieldId
 * @return int
 */
int CDataHandler::CreateUniqueId(const int masterId, const int datafieldId)
{
  auto uniqueIdMap = uniqueIds_.find(SIdentifier{masterId, datafieldId});
  if (uniqueIdMap != uniqueIds_.end())
  {
    return uniqueIdMap->second;
  }
  else
  {
    int uniqueId = PerformanceHelpers::GetUniqueId();
    uniqueIds_.insert(
        std::make_pair(SIdentifier{masterId, datafieldId}, uniqueId));
    return uniqueId;
  }
}

int CDataHandler::GetUniqueId(const int masterId, const int datafieldId) const
{
  auto uniqueIdMap = uniqueIds_.find(SIdentifier{masterId, datafieldId});
  if (uniqueIdMap != uniqueIds_.end())
  {
    return uniqueIdMap->second;
  }
  else
    throw std::runtime_error("Could not find the unique id!");
}

} // namespace Linux