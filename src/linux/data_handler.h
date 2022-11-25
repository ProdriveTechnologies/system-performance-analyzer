#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "path_parser_base.h"
#include "src/exports/export_struct.h"

#include "src/linux/datahandlers/direct_handler.h"
#include "src/linux/datahandlers/pidstat_handler.h"
#include "src/linux/datahandlers/pidstatm_handler.h"
#include "src/linux/datahandlers/procmeminfo_handler.h"
#include "src/linux/datahandlers/procstat_handler.h"

namespace Linux
{
// Pre-declaration, still needs to be included when used
class CPidStatHandler;
class CDirectHandler;
class CPidStatmHandler;
class CProcStatHandler;
class CProcMeminfoHandler;

struct SDataHandlers
{
  PlatformConfig::ETypes type;
  std::variant<std::unique_ptr<Linux::CPidStatHandler>,
               std::unique_ptr<Linux::CDirectHandler>,
               std::unique_ptr<Linux::CPidStatmHandler>,
               std::unique_ptr<Linux::CProcStatHandler>,
               std::unique_ptr<Linux::CProcMeminfoHandler>>
      datahandler;
};
class CDataHandler
{
public:
  struct Config
  {
    std::string replacementTag; // optional, to replace during runtime
    // The class that can parse the path, note: its a pointer and should be
    // stored outside the class!
    CPathParserBase *parserObj;
  };
  CDataHandler() = default;

  void Initialize(std::unordered_map<PlatformConfig::ETypes, Config> parsers,
                  const std::vector<PlatformConfig::SDatafields> &datafields);

  /**
   * @brief Reads the measurements
   *
   * @param replacement optional replacement for the replacementTag that is
   * configured (during Initialization)
   * @return true getting the measurements was succesful
   * @return false getting the measurements failed!
   */
  bool ParseMeasurements(const std::string &replacement = "",
                         const int masterId = -1);

  std::vector<Measurements::SMeasuredItem> GetMeasurements() const
  {
    return lastMeasurements_;
  }

  int GetUniqueId(const int masterId, const int datafieldId) const;

private:
  struct SIdentifier
  {
    int masterId;
    int datafieldId;
    inline size_t operator==(const SIdentifier &k) const
    {
      return k.masterId == masterId && k.datafieldId == datafieldId;
    }
    /**
     * @brief hash function for SIdentifier (uses a Cantor Pairing function)
     *
     * @param k the parameters should be >= 0
     * @return size_t
     */
    inline size_t operator()(const SIdentifier &k) const
    {
      // The function for Cantor Pairing according to
      // https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
      return (k.masterId + k.datafieldId) * (k.masterId + k.datafieldId + 1) /
                 2 +
             k.masterId;
    }
  };

  std::unordered_map<PlatformConfig::ETypes, Config> parsers_;
  std::vector<Config> configuration_;
  std::vector<PlatformConfig::SDatafields> datafields_;
  std::unordered_map<SIdentifier, int, SIdentifier> uniqueIds_;

  std::vector<Measurements::SMeasuredItem> lastMeasurements_;
  int CreateUniqueId(const int masterId, const int datafieldId);
};

std::unordered_map<PlatformConfig::ETypes, Linux::CDataHandler::Config>
GetDatahandlerMap(const std::vector<Linux::SDataHandlers> &dataHandlers,
                  const std::string &replacementTag = "");
} // namespace Linux