#include "summary_generator.h"

#include "src/exports/summary_writer.h"
#include "src/exports/translations.h"
#include "src/globals.h"
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
bool CSummaryGenerator::FullExport([[maybe_unused]] const std::vector<SMeasurementItem>& config,
                                   const FullMeasurement data,
                                   const AllSensors& allSensors,
                                   const std::vector<Measurements::CCorrelation::SResult>& correlationResults)
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

void CSummaryGenerator::PrintCorrelations(const std::vector<Measurements::CCorrelation::SResult>& correlationResults)
{
  SummaryWriter::PrintSection(SummaryTranslations::correlationTitle);
  bool correlation = false;
  for (const auto& e : correlationResults)
  {
    if (std::abs(e.correlation) >= Globals::strongCorrelation)
    {
      correlation = true;
      SummaryWriter::PrintRow("Pearsons correlation of " + std::to_string(e.correlation) + " for the sensors " +
                              e.sensor1.userId + " and " + e.sensor2.userId);
    }
  }
  if (!correlation)
    SummaryWriter::PrintSection(SummaryTranslations::correlationNone);
}

void CSummaryGenerator::PrintThresholds(const AllSensors& allSensors)
{
  auto processIds = allSensors.GetProcesses();

  SummaryWriter::PrintSection(SummaryTranslations::thresholdTitle);
  bool exceeded = false;
  for (const auto& processId : processIds)
  {
    auto sensors = allSensors.GetMap(processId);
    for (const auto& sensor : sensors)
    {
      if (sensor.second->thresholdExceeded)
      {
        // Create the string to show the threshold to the user
        std::string thresholdStr = "Threshold for " + sensor.second->userId + " was exceeded";
        if (processId >= 0)
          thresholdStr += " for process id: " + std::to_string(processId);
        thresholdStr += ". The values were: " + PrintValues(sensor.second->data);

        // Print the string to the user
        SummaryWriter::PrintRow(thresholdStr);
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

void CSummaryGenerator::PrintApplicationInfo(const std::string& totalExecTime)
{
  SummaryWriter::PrintValue(SummaryTranslations::version, Globals::version);
  std::string created{ Globals::compilationDate };
  created += " ";
  created += Globals::compilationTime;
  SummaryWriter::PrintValue(SummaryTranslations::created, created);
  SummaryWriter::PrintValue(SummaryTranslations::gitHash, std::string(GIT_COMMIT_HASH));
  SummaryWriter::PrintValue(SummaryTranslations::executionTime, totalExecTime);
  SummaryWriter::PrintRow();
}

/**
 * @brief
 */
void CSummaryGenerator::PrintSystemSummary(const AllSensors& allSensors)
{
  SummaryWriter::PrintSection(SummaryTranslations::systemSummaryTitle);
  auto systemRsc = allSensors.data.find(Measurements::EClassification::SYSTEM);
  if (systemRsc != allSensors.data.end())
  {
    if (systemRsc->second.size() > 1)
      CLogger::Log(CLogger::Types::WARNING, "System size > 1");
    for (const auto& systemRscSensors : systemRsc->second)
    {
      for (const auto& sensor : systemRscSensors.sensors)
      {
        if (sensor.classType != PlatformConfig::EClass::SYS_RESOURCE_USAGE && !settings_.settings.verboseSummary)
          continue;
        PrintValue(SummaryTranslations::average, sensor, Measurements::EValueTypes::AVERAGE);
      }
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::processSummaryTitle);
  auto processRsc = allSensors.data.find(Measurements::EClassification::PROCESSES);
  if (processRsc != allSensors.data.end())
  {
    for (const auto& processGroup : processRsc->second)
    {
      SummaryWriter::PrintSubSection("Process: " + std::to_string(processGroup.processId));
      for (const auto& sensor : processGroup.sensors)
      {
        if (sensor.classType != PlatformConfig::EClass::SYS_RESOURCE_USAGE && !settings_.settings.verboseSummary)
          continue;
        PrintValue(SummaryTranslations::average, sensor, Measurements::EValueTypes::AVERAGE);
      }
      SummaryWriter::PrintRow();
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::pipelineSummaryTitle);
  auto pipelineRsc = allSensors.data.find(Measurements::EClassification::PIPELINE);
  if (pipelineRsc != allSensors.data.end())
  {
    for (const auto& pipelineGroup : pipelineRsc->second)
    {
      SummaryWriter::PrintSubSection("Pipeline: " + std::to_string(pipelineGroup.processId));
      for (const auto& sensor : pipelineGroup.sensors)
      {
        if (settings_.settings.verboseSummary || sensor.measuredRaw == false)
        {
          PrintValue(SummaryTranslations::average, sensor, Measurements::EValueTypes::AVERAGE);
        }
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
                                   const Measurements::SSensors& sensor,
                                   const Measurements::EValueTypes valueType)
{
  std::string valName{ translation };
  valName += sensor.userId;
  try
  {
    auto correctedValue = sensor.data.Get(valueType) * sensor.multiplier;
    SummaryWriter::PrintValue(valName, correctedValue, sensor.suffix);
  }
  catch (const std::exception& e)
  {
    SummaryWriter::PrintValue(valName, std::string_view("No measurement found"));
  }
}

} // namespace Exports