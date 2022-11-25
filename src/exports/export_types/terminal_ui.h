#pragma once

#include "../exports_base.h"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "src/helpers/helper_functions.h"

#include <array>
#include <unordered_map>

namespace Exports
{
class CTerminalUI : public CBase
{
public:
  CTerminalUI()
  : colorRange_{ 1, 15 }
  , currentColor_{ 1 }
  {
    hasLiveMode_ = true;
  }

  bool FullExport(const std::vector<SMeasurementItem>& config,
                  const FullMeasurement data,
                  const AllSensors& allSensors,
                  const std::vector<Measurements::CCorrelation::SResult>& correlations);

  void StartLiveMeasurements() {}
  void AddMeasurements(const Measurements::SMeasurementsData data);
  void FinishLiveMeasurements() {}

private:
  struct ColorRange
  {
    int min, max;
    ColorRange(int min_, int max_)
    : min{ min_ }
    , max{ max_ }
    {
    }
  };
  std::unordered_map<int, ftxui::Color> colorMap_;
  std::unordered_map<int, double> lastMeasurement_;
  const ColorRange colorRange_;
  int currentColor_;
  ftxui::Element document_;
  std::string position_;

  std::string GetFileName(const Measurements::EClassification c);
  ftxui::Element GetElement(const Measurements::SSensors& sensor, const double& measuredValue, const int processId);
  ftxui::Elements GetElements(const Measurements::SMeasurementsData data);
  ftxui::Elements GetElementsSensorGroups(const std::vector<Measurements::SAllSensors::SSensorGroups>& groups,
                                          const Measurements::SMeasurementsData data);
  double GetPercentage(const PlatformConfig::SDatafields& datafield, const double& measuredValue);
  ftxui::Color GetColor(const int id);
};

} // namespace Exports