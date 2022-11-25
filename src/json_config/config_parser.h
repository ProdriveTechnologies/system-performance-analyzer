#pragma once

#include "config.h"
#include "src/helpers/json_helpers.h"

#include <nlohmann/json.hpp>
#include <string>

namespace Core
{
class ConfigParser
{
public:
  static SConfig Parse(const std::string& jsonFile);

private:
};

//*********** Conversions for the Core structs to JSON objects ***************//
inline void from_json(const nlohmann::json& j, SThreshold& p)
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

inline void from_json(const nlohmann::json& j, SProcess& p)
{
  std::string typeStr;
  j.at("type").get_to(typeStr);
  p.type = GetProcessType(typeStr);

  j.at("command").get_to(p.command);
  if (j.contains("start_delay"))
    j.at("start_delay").get_to(p.startDelay);
  j.at("id").get_to(p.processId);

  if (j.contains("use_steady_state"))
    j.at("use_steady_state").get_to(p.useSteadyState);
  // Parameter checks
  if (p.startDelay < 0)
  {
    throw std::runtime_error("Start delay may not be negative! Please change the start_delay for " + p.command);
  }
  if (p.processId < 0)
  {
    throw std::runtime_error("Process IDs may not be negative! Please change the process ID for " + p.command);
  }
}

inline void from_json(const nlohmann::json& j, SExports& p)
{
  std::string exportType;
  j.at("export_type").get_to(exportType);
  p.exportType = Exports::GetExportType(exportType);
  j.at("enabled").get_to(p.exportEnabled);
  if (p.exportType == Exports::ETypes::GRAPHS)
    j.at("foldername").get_to(p.filename);
  else
    j.at("filename").get_to(p.filename);
}

inline void from_json(const nlohmann::json& j, SSettings::SPipelineSensorConfig& p)
{
  j.at("minimum").get_to(p.minimumValue);
  j.at("maximum").get_to(p.maximumValue);

  std::string measureType;
  j.at("sensortype").get_to(measureType);
  p.sensorType = GStreamer::GetMeasureType(measureType);
}

inline void from_json(const nlohmann::json& j, SSettings& p)
{
  j.at("measure_loop_ms").get_to(p.measureLoopMs);
  if (j.contains("enable_proctime"))
    j.at("enable_proctime").get_to(p.enableProcTime);
  j.at("verbose").get_to(p.verbose);
  if (j.contains("enable_verbose_summary"))
    j.at("enable_verbose_summary").get_to(p.verboseSummary);

  if (j.contains("exports"))
    j.at("exports").get_to(p.exports);

  if (j.contains("enable_livemode"))
    j.at("enable_livemode").get_to(p.enableLiveMode);

  if (j.contains("enable_pretest_zeroes"))
    j.at("enable_pretest_zeroes").get_to(p.enablePretestZeroes);

  j.at("enable_logs").get_to(p.enableLogs);
  if (j.contains("pipeline_config"))
    j.at("pipeline_config").get_to(p.pipelineConfig);
}

inline void from_json(const nlohmann::json& j, SConfig& p)
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