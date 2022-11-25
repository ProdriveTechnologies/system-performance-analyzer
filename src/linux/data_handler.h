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
  PlatformConfig::Types type;
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

  void Initialize(std::unordered_map<PlatformConfig::Types, Config> parsers,
                  const std::vector<PlatformConfig::SDatafields> &datafields);

  /**
   * @brief Reads the measurements
   *
   * @param replacement
   * @return true getting the measurements was succesful
   * @return false getting the measurements failed!
   */
  bool ParseMeasurements(const std::string &replacement = "");

  std::vector<Exports::MeasuredItem> GetMeasurements() const
  {
    return lastMeasurements_;
  }

private:
  std::unordered_map<PlatformConfig::Types, Config> parsers_;
  std::vector<Config> configuration_;
  std::vector<PlatformConfig::SDatafields> datafields_;

  std::vector<Exports::MeasuredItem> lastMeasurements_;
};
} // namespace Linux