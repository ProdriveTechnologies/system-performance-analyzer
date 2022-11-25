#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "config.h"
#include "src/helpers/json_helpers.h"

namespace Core
{
class ConfigParser
{
public:
  static SConfig Parse(const std::string &jsonFile);

private:
};

//*********** Conversions for the Core structs to JSON objects ***************//
inline void from_json(const nlohmann::json &j, SThreshold &p)
{
  j.at("name").get_to(p.name);
  std::string typeBuffer;
  j.at("type").get_to(typeBuffer);
  p.type = GetThresholdType(typeBuffer);
  std::string signBuffer;
  j.at("sign").get_to(signBuffer);
  p.sign = GetSign(signBuffer);
  // When no process_id is present, it becomes -1
  p.processId = j.contains("process_id") ? j.at("process_id").get<int>() : -1;
  j.at("threshold").get_to(p.value);
}

inline void from_json(const nlohmann::json &j, SProcess &p)
{
  j.at("type").get_to(p.type);
  j.at("command").get_to(p.command);
  if (j.contains("start_delay"))
    j.at("start_delay").get_to(p.startDelay);
  j.at("id").get_to(p.processId);
  // Parameter checks
  if (p.startDelay < 0)
  {
    throw std::runtime_error(
        "Start delay may not be negative! Please change the start_delay for " +
        p.command);
  }
  if (p.processId < 0)
  {
    throw std::runtime_error(
        "Process IDs may not be negative! Please change the process ID for " +
        p.command);
  }
}

inline void from_json(const nlohmann::json &j, SSettings &p)
{
  j.at("measure_loop_ms").get_to(p.measureLoopMs);
  if (j.contains("enable_proctime"))
    j.at("enable_proctime").get_to(p.enableProcTime);
  j.at("verbose").get_to(p.verbose);
  if (j.contains("enable_verbose_summary"))
    j.at("enable_verbose_summary").get_to(p.verboseSummary);
  j.at("enable_logs").get_to(p.enableLogs);
}

inline void from_json(const nlohmann::json &j, SConfig &p)
{
  j.at("name").get_to(p.name);
  j.at("description").get_to(p.description);
  j.at("id").get_to(p.id);
  j.at("version").get_to(p.version);
  j.at("thresholds").get_to(p.thresholds);
  j.at("processes").get_to(p.processes);
  j.at("settings").get_to(p.settings);
}

} // namespace Core