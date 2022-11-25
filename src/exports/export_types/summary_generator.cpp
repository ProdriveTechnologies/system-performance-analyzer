#include "summary_generator.h"

#include "src/globals.h"

#include "src/exports/summary_writer.h"
#include "src/exports/translations.h"
// #include <infoware/cpu.hpp>
#include "src/helpers/logger.h"
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
bool CSummaryGenerator::FullExport(const std::vector<MeasurementItem> &config,
                                   const FullMeasurement data,
                                   const AllSensors &allSensors)
{
  // Generic info, not specific to the measurements
  SummaryWriter::PrintTitle(SummaryTranslations::headerName);
  PrintApplicationInfo();

  for (const auto &e : config)
  {
  }

  PrintSystemSummary(allSensors);
  PrintThresholds(allSensors);

  return true;
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
}

// bool CSummaryGenerator::Generate(
//     const std::vector<Exports::ExportData> &measurementsData,
//     const std::vector<PlatformConfig::SDatafields> &measurementsDef)
// {
//   SummaryWriter::PrintTitle(SummaryTranslations::headerName);
//   PrintApplicationInfo();
//   // PrintSystemInfo();
//   // PrintCacheInfo();

//   // Second, summarize system wide data
//   // PrintSystemSummary(measurementsData, measurementsDef);
//   // Then, summarize each process the data
//   // Finish with the data of each GStreamer pipeline (FPS, most delaying
//   module,
//   // CPU heavy module?)
//   return true;
// }

void CSummaryGenerator::PrintApplicationInfo()
{
  SummaryWriter::PrintValue(SummaryTranslations::version, Globals::version);
  std::string created{Globals::compilationDate};
  created += " ";
  created += Globals::compilationTime;
  SummaryWriter::PrintValue(SummaryTranslations::created, created);
  SummaryWriter::PrintValue(SummaryTranslations::gitHash,
                            std::string(GIT_COMMIT_HASH));
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
  auto systemRsc = allSensors.data.find(Measurements::Classification::SYSTEM);
  if (systemRsc != allSensors.data.end())
  {
    if (systemRsc->second.size() > 1)
      CLogger::Log(CLogger::Types::WARNING, "System size > 1");
    for (const auto &systemRscSensors : systemRsc->second)
    {
      for (const auto &sensor : systemRscSensors.sensors)
      {
        if (sensor.classType != PlatformConfig::Class::SYS_RESOURCE_USAGE)
          continue;
        PrintValue(SummaryTranslations::average, sensor,
                   Measurements::ValueTypes::AVERAGE);
      }
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::processSummaryTitle);
  auto processRsc =
      allSensors.data.find(Measurements::Classification::PROCESSES);
  if (processRsc != allSensors.data.end())
  {
    for (const auto &processGroup : processRsc->second)
    {
      SummaryWriter::PrintSubSection("Process: " +
                                     std::to_string(processGroup.processId));
      for (const auto &sensor : processGroup.sensors)
      {
        // if (sensor.classType != PlatformConfig::Class::)
        //   continue;
        PrintValue(SummaryTranslations::average, sensor,
                   Measurements::ValueTypes::AVERAGE);
      }
    }
  }
  SummaryWriter::PrintRow();
  SummaryWriter::PrintSection(SummaryTranslations::pipelineSummaryTitle);
  auto pipelineRsc =
      allSensors.data.find(Measurements::Classification::PIPELINE);
  if (pipelineRsc != allSensors.data.end())
  {
    for (const auto &pipelineGroup : pipelineRsc->second)
    {
      SummaryWriter::PrintSubSection("Pipeline: " +
                                     std::to_string(pipelineGroup.processId));
      for (const auto &sensor : pipelineGroup.sensors)
      {
        // if (sensor.classType != PlatformConfig::Class::)
        //   continue;
        PrintValue(SummaryTranslations::average, sensor,
                   Measurements::ValueTypes::AVERAGE);
      }
    }
  }
  SummaryWriter::PrintRow();
}

double CSummaryGenerator::GetAverage(
    const std::vector<Exports::ExportData> &measurementsData, const int id)
{
  auto getFieldById = [](const std::vector<Exports::MeasuredItem> &items,
                         const int id) {
    for (const auto &item : items)
    {
      if (item.id == id)
        return item;
    }
    return Exports::MeasuredItem{0, 0};
  };
  double total;

  for (const auto &e : measurementsData)
  {
    total += getFieldById(e.measuredItems, id).measuredValue;
  }
  const size_t dataSize = measurementsData.size();
  return dataSize > 0 ? (total / dataSize) : 0.0;
}

/**
 * @brief Prints a value from a sensor to the summary
 *
 * @param translation
 * @param sensor
 * @param valueType
 */
void CSummaryGenerator::PrintValue(const std::string_view translation,
                                   const Measurements::Sensors &sensor,
                                   const Measurements::ValueTypes valueType)
{
  std::string valName{translation};
  valName += sensor.userId;
  SummaryWriter::PrintValue(valName, sensor.data.Get(valueType));
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