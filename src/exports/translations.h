#pragma once

#include <string>
namespace SummaryTranslations
{
constexpr std::string_view headerName{"SPA Tool Summary Report"};
constexpr std::string_view version{"Version"};
constexpr std::string_view created{"Created"};
constexpr std::string_view gitHash{"Git Hash"};
constexpr std::string_view executionTime{"Total execution time[ms]"};
constexpr std::string_view systemInfo{"Processor and System Information"};
constexpr std::string_view physicalCores{"Physical Cores"};
constexpr std::string_view logicalCores{"Logical Cores"};
constexpr std::string_view minCpuFrequency{"Minimum CPU frequency"};
constexpr std::string_view maxCpuFrequency{"Maximum CPU frequency"};
constexpr std::string_view modelName{"Model name"};
constexpr std::string_view architecture{"Architecture"};
constexpr std::string_view cacheTitle{"Caches"};
constexpr std::string_view size{"Size"};
constexpr std::string_view associativity{"Associativity"};
constexpr std::string_view lineSize{"Line size"};
constexpr std::string_view type{"Type"};

constexpr std::string_view systemSummaryTitle{"System Resource Usage"};
constexpr std::string_view processSummaryTitle{"Processes Resource Usage"};
constexpr std::string_view pipelineSummaryTitle{"Pipeline Measurements"};
constexpr std::string_view average{"Average "};
constexpr std::string_view minimum{"Minimum "};
constexpr std::string_view maximum{"Maximum "};

constexpr std::string_view thresholdTitle{"Thresholds"};
constexpr std::string_view thresholdExceeded{"No thresholds were exceeded!"};

constexpr std::string_view correlationTitle{"Correlations"};
constexpr std::string_view correlationNone{
    "No correlations found! This can be caused by having too little data "
    "measurements (<10)"};

} // namespace SummaryTranslations