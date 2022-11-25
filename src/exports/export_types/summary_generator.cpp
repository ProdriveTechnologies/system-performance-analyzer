#include "summary_generator.h"

#include "src/globals.h"

#include "src/exports/summary_writer.h"
#include "src/exports/translations.h"
// #include <infoware/cpu.hpp>
#include "src/helpers/logger.h"
#include <cmath>
#include <iostream>

namespace Exports
{
/**
 * @brief Generates the full summary in the console
 *
 * @param config
 * @param data
 * @return true
 * @return false
 */
bool CSummaryGenerator::FullExport(
    [[maybe_unused]] const std::vector<SMeasurementItem> &config,
    const FullMeasurement data, const AllSensors &allSensors,
    const std::vector<Measurements::CCorrelation::SResult> &correlationResults)
{
  // Generic info, not specific to the measurements
  SummaryWriter::PrintTitle(SummaryTranslations::headerName);
  std::string totalTime = GetTotalTime(data);
  PrintApplicationInfo(totalTime);

  PrintSystemSummary(allSensors);
  PrintThresholds(allSensors);
  PrintCorrelations(correlationResults);
  return true;
}

void CSummaryGenerator::PrintCorrelations(
    const std::vector<Measurements::CCorrelation::SResult> &correlationResults)
{
  SummaryWriter::PrintSection(SummaryTranslations::correlationTitle);
  bool correlation = false;
  for (const auto &e : correlationResults)
  {
    if (std::abs(e.correlation) >= Globals::strongCorrelation)
    {
      correlation = true;
      SummaryWriter::PrintRow(
          "Pearsons correlation of " + std::to_string(e.correlation) +
          " for the sensors " + e.sensor1.userId + " and " + e.sensor2.userId);
    }
  }
  if (!correlation)
    SummaryWriter::PrintSection(SummaryTranslations::correlationNone);
}

void CSummaryGenerator::PrintThresholds(const AllSensors &allSensors)
{
  auto processIds = allSensors.GetProcesses();

  SummaryWriter::PrintSection(SummaryTranslations::thresholdTitle);
  bool exceeded = false;
  for (const auto &processId : processIds)
  {
    auto sensors = allSensors.GetMap(processId);
    for (const auto &sensor : sensors)
    {
      if (sensor.second->thresholdExceeded)
      {
        SummaryWriter::PrintRow(
            "Threshold for " + sensor.second->userId +
            " was exceeded for process id: " + std::to_string(processId) +
            ". The values were: " + PrintValues(sensor.second->data));
        exceeded = true;
      }
    }
  }
  if (!exceeded)
  {
    SummaryWriter::PrintRow(SummaryTranslations::thresholdExceeded);
  }
  SummaryWriter::PrintRow();
}

void CSummaryGenerator::PrintApplicationInfo(const std::string &totalExecTime)
{
  SummaryWriter::PrintValue(SummaryTranslations::version, Globals::version);
  std::string created{Globals::compilationDate};
  created += " ";
  created += Globals::compilationTime;
  SummaryWriter::PrintValue(SummaryTranslations::created, created);
  SummaryWriter::PrintValue(SummaryTranslations::gitHash,
                            std::string(GIT_COMMIT_HASH));
  SummaryWriter::PrintValue(SummaryTranslations::executionTime, totalExecTime);
  SummaryWriter::PrintRow();
}
void CSummaryGenerator::PrintSystemInfo()
{
  // const auto quantities = iware::cpu::quantities();
  SummaryWriter::PrintSection(SummaryTranslations::systemInfo);
  // SummaryWriter::PrintValue(SummaryTranslations::logicalCores,
  //                           quantities.logical);
  // SummaryWriter::PrintValue(SummaryTranslations::physicalCores,
  //                           quantities.physical);
  // SummaryWriter::PrintValue(SummaryTranslations::modelName,
  //                           iware::cpu::model_name());
  // SummaryWriter::PrintValue(SummaryTranslations::architecture,
  //                           GetArchitecture(iware::cpu::architecture()));
  SummaryWriter::PrintRow();
}
// bool CSummaryGenerator::GenerateProcesses(
//     const std::vector<Linux::CPerfMeasurements::ProcessesMeasure>
//         &measuredProcesses)
// {
//   return true;
// }

void CSummaryGenerator::PrintCacheInfo()
{
#if 0
  SummaryWriter::PrintSection(SummaryTranslations::cacheTitle);
  for (unsigned i = lowestCacheNr; i <= highestCacheNr; ++i)
  {
    const auto cache = iware::cpu::cache(i);
    if (cache.size == 0)
    {
      if (i == 0)
        continue;
      else
        break;
    }
    SummaryWriter::PrintSubSection("L" + std::to_string(i));
    SummaryWriter::PrintValue(SummaryTranslations::size, cache.size);
    SummaryWriter::PrintValue(SummaryTranslations::lineSize, cache.line_size);
    // SummaryWriter::PrintValue(SummaryTranslations::associativity,
    //                           cache.associativity);
    SummaryWriter::PrintValue(SummaryTranslations::type,
                              GetCacheType(cache.type));
    SummaryWriter::PrintRow();
  }
#endif
}

/**
 * @brief
 */
void CSummaryGenerator::PrintSystemSummary(const AllSensors &allSensors)
{
  SummaryWriter::PrintSection(SummaryTranslations::systemSummaryTitle);
  auto systemRsc = allSensors.data.find(Measurements::EClassification::SYSTEM);
  if (systemRsc != allSensors.data.end())
  {
    if (systemRsc->second.size() > 1)
      CLogger::Log(CLogger::Types::WARNING, "System size > 1");
    for (const auto &systemRscSensors : systemRsc->second)
    {
      for (const auto &sensor : systemRscSensors.sensors)
      {
        if (sensor.classType != PlatformConfig::EClass::SYS_RESOURCE_USAGE &&
            !settings_.settings.verboseSummary)
          continue;
        PrintValue(SummaryTranslations::average, sensor,
                   Measurements::EValueTypes::AVERAGE);
      }
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::processSummaryTitle);
  auto processRsc =
      allSensors.data.find(Measurements::EClassification::PROCESSES);
  if (processRsc != allSensors.data.end())
  {
    for (const auto &processGroup : processRsc->second)
    {
      SummaryWriter::PrintSubSection("Process: " +
                                     std::to_string(processGroup.processId));
      for (const auto &sensor : processGroup.sensors)
      {
        if (sensor.classType != PlatformConfig::EClass::SYS_RESOURCE_USAGE &&
            !settings_.settings.verboseSummary)
          continue;
        PrintValue(SummaryTranslations::average, sensor,
                   Measurements::EValueTypes::AVERAGE);
      }
      SummaryWriter::PrintRow();
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::pipelineSummaryTitle);
  auto pipelineRsc =
      allSensors.data.find(Measurements::EClassification::PIPELINE);
  if (pipelineRsc != allSensors.data.end())
  {
    for (const auto &pipelineGroup : pipelineRsc->second)
    {
      SummaryWriter::PrintSubSection("Pipeline: " +
                                     std::to_string(pipelineGroup.processId));
      for (const auto &sensor : pipelineGroup.sensors)
      {
        if (settings_.settings.verboseSummary || sensor.measuredRaw == false)
        {
          PrintValue(SummaryTranslations::average, sensor,
                     Measurements::EValueTypes::AVERAGE);
        }
        // if (sensor.classType != PlatformConfig::Class::)
        //   continue;
      }
      SummaryWriter::PrintRow();
    }
  }
  SummaryWriter::PrintRow();
}

/**
 * @brief Prints a value from a sensor to the summary
 *
 * @param translation
 * @param sensor
 * @param valueType
 */
void CSummaryGenerator::PrintValue(const std::string_view translation,
                                   const Measurements::SSensors &sensor,
                                   const Measurements::EValueTypes valueType)
{
  std::string valName{translation};
  valName += sensor.userId;
  try
  {
    auto correctedValue = sensor.data.Get(valueType) * sensor.multiplier;
    SummaryWriter::PrintValue(valName, correctedValue, sensor.suffix);
  }
  catch (const std::exception &e)
  {
    SummaryWriter::PrintValue(valName,
                              std::string_view("No measurement found"));
  }
}

#if 0
std::string
CSummaryGenerator::GetCacheType(const iware::cpu::cache_type_t cache)
{
  switch (cache)
  {
  case iware::cpu::cache_type_t::unified:
    return "Unified";
  case iware::cpu::cache_type_t::instruction:
    return "Instruction";
  case iware::cpu::cache_type_t::data:
    return "Data";
  case iware::cpu::cache_type_t::trace:
    return "Trace";
  default:
    return "Unknown";
  }
}

std::string CSummaryGenerator::GetArchitecture(
    const iware::cpu::architecture_t architecture)
{
  switch (architecture)
  {
  case iware::cpu::architecture_t::x64:
    return "x64";
  case iware::cpu::architecture_t::arm:
    return "ARM";
  case iware::cpu::architecture_t::itanium:
    return "Itanium";
  case iware::cpu::architecture_t::x86:
    return "x86";
  default:
    return "Unknown";
  }
}
#endif

} // namespace Exports