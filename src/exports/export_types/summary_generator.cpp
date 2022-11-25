#include "summary_generator.h"

#include "src/globals.h"

#include "src/exports/summary_writer.h"
#include "src/exports/translations.h"
// #include <infoware/cpu.hpp>
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
  const auto sensors = allSensors.CreateMap();

  SummaryWriter::PrintSubSection("Thresholds:");
  for (const auto &e : sensors)
  {
    if (e.second->thresholdExceeded)
    {
      SummaryWriter::PrintRow("theshold exceeded");
      SummaryWriter::PrintRow(std::to_string(e.second->uniqueId));
      SummaryWriter::PrintRow("ID: " + e.second->userId);
      SummaryWriter::PrintRow(
          "Threshold for " + e.second->userId + " was exceeded for group " +
          e.second->userId +
          ". The values were: " + PrintValues(e.second->data));
    }
  }
}

bool CSummaryGenerator::Generate(
    const std::vector<Exports::ExportData> &measurementsData,
    const std::vector<PlatformConfig::SDatafields> &measurementsDef)
{
  SummaryWriter::PrintTitle(SummaryTranslations::headerName);
  PrintApplicationInfo();
  // PrintSystemInfo();
  // PrintCacheInfo();

  // Second, summarize system wide data
  PrintSystemSummary(measurementsData, measurementsDef);
  // Then, summarize each process the data
  // Finish with the data of each GStreamer pipeline (FPS, most delaying module,
  // CPU heavy module?)
  return true;
}

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
bool CSummaryGenerator::GenerateProcesses(
    const std::vector<Linux::CPerfMeasurements::ProcessesMeasure>
        &measuredProcesses)
{
  return true;
}

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

void CSummaryGenerator::PrintSystemSummary(
    const std::vector<Exports::ExportData> &measurementsData,
    const std::vector<PlatformConfig::SDatafields> &measurementsDef)
{
  SummaryWriter::PrintSection(SummaryTranslations::systemSummaryTitle);

  for (const auto &e : measurementsDef)
  {
    if (e.classType != PlatformConfig::Class::SYS_RESOURCE_USAGE)
      continue;
    auto averageUsage = GetAverage(measurementsData, e.id);
    std::string valName{SummaryTranslations::average};
    valName += e.name;
    SummaryWriter::PrintValue(valName, averageUsage);
  }

  // SummaryWriter::PrintValue(, measurementsData.at)
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