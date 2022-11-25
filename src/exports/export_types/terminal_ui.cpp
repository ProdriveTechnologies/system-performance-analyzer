#include "terminal_ui.h"

#include <cmath>
#include <iostream>
#include <src/globals.h>
#include <stdio.h>
#include <stdlib.h>

namespace Exports
{
bool CTerminalUI::FullExport(
    [[maybe_unused]] const std::vector<MeasurementItem> &config,
    [[maybe_unused]] const FullMeasurement data,
    [[maybe_unused]] const AllSensors &allSensors,
    [[maybe_unused]] const std::vector<Measurements::CCorrelation::SResult>
        &correlations)
{
  return true;
}
void CTerminalUI::StartLiveMeasurements()
{
  //   using namespace ftxui;

  //   auto summary = [&] {
  //     auto content = ftxui::vbox({
  //         hbox({text(L"- done:   "), text(L"3") | bold}) |
  //         color(Color::Green), hbox({text(L"- active: "), text(L"2") | bold})
  //         | color(Color::RedLight), hbox({text(L"- queue:  "), text(L"9") |
  //         bold}) | color(Color::Red),
  //     });
  //     return window(text(L" Summary "), content);
  //   };

  //   document_ = //
  //       ftxui::vbox({
  //           ftxui::hbox({
  //               text("fps") | border,
  //               gauge(0.5) | border | flex,
  //           }),
  //       });

  //   // Limit the size of the document to 80 char.
  //   document_ = document_ | size(WIDTH, LESS_THAN, 80);

  //   auto screen = Screen::Create(Dimension::Full(),
  //   Dimension::Fit(document_)); Render(screen, document_); screen.Print();
}

void CTerminalUI::AddMeasurements(const Measurements::SMeasurementsData data)
{
  auto elements = GetElements(data);
  document_ = //
      ftxui::vbox(elements);

  // Limit the size of the document to 80 char.
  document_ = document_ | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 120);

  auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),
                                      ftxui::Dimension::Fit(document_));
  Render(screen, document_);
  std::cout << position_;
  screen.Print();
  position_ = screen.ResetPosition();
}

ftxui::Elements
CTerminalUI::GetElements(const Measurements::SMeasurementsData data)
{
  ftxui::Elements elements;
  for (const auto &c : liveSensors_->allClasses)
  {
    const auto sensorGroups = liveSensors_->GetSensorGroups(c);
    elements = Helpers::CombineVectors(
        elements, GetElementsSensorGroups(sensorGroups, data));
  }
  return elements;
}

ftxui::Elements CTerminalUI::GetElementsSensorGroups(
    const std::vector<Measurements::AllSensors::SensorGroups> &sensorGroups,
    const Measurements::SMeasurementsData data)
{
  ftxui::Elements elements;
  for (const auto &group : sensorGroups)
  {
    for (const auto &e : group.sensors)
    {
      if (e.userData.showInLive)
      {
        try
        {
          auto measurement = data.GetWithId(e.uniqueId);
          lastMeasurement_[e.uniqueId] = measurement.measuredValue;
          elements.push_back(
              GetElement(e, measurement.measuredValue, group.processId));
        }
        catch (const std::exception &err)
        {
          auto res = lastMeasurement_.find(e.uniqueId);
          if (res != lastMeasurement_.end())
            elements.push_back(GetElement(e, res->second, group.processId));
          else
            elements.push_back(
                GetElement(e, e.userData.minimumValue, group.processId));
        }
      }
    }
  }
  return elements;
}

ftxui::Element CTerminalUI::GetElement(const Measurements::Sensors &sensor,
                                       const double &item, const int processId)
{
  double value = GetPercentage(sensor.userData, item);
  std::string name;
  if (processId == -1)
    name = sensor.userId;
  else
    name = std::to_string(processId) + "." + sensor.userId;

  return ftxui::hbox(
      {ftxui::text(name) | ftxui::border,
       ftxui::color(GetColor(sensor.uniqueId), ftxui::gauge(value)) |
           ftxui::border | ftxui::flex,
       ftxui::text(std::to_string(item * sensor.multiplier) + sensor.suffix) |
           ftxui::border});
}
void CTerminalUI::FinishLiveMeasurements() {}

/**
 * @brief Get percentage between 0 and 1 (where 1 = 100% and 0 = 0%)
 */
double CTerminalUI::GetPercentage(const PlatformConfig::SDatafields &datafield,
                                  const double &measuredValue)
{
  auto corrected = measuredValue * datafield.multiplier;
  auto range = datafield.maximumValue - datafield.minimumValue;
  return (corrected - datafield.minimumValue) / (range);
}
ftxui::Color CTerminalUI::GetColor(const int id)
{
  auto res = colorMap_.find(id);

  if (res != colorMap_.end())
    return res->second;

  int colorId = currentColor_++;
  if (currentColor_ > colorRange_.max)
    currentColor_ = colorRange_.min;
  auto color = ftxui::Color{static_cast<ftxui::Color::Palette16>(colorId)};
  colorMap_.insert({id, color});
  return color;
}

} // namespace Exports