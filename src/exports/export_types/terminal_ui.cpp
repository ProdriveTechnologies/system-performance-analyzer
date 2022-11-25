#include "terminal_ui.h"

#include <cmath>
#include <iostream>
#include <src/globals.h>
#include <stdio.h>
#include <stdlib.h>

namespace Exports
{
/**
 * @brief FullExport not implemented for the terminal UI, as this export only
 * implements a live view
 */
bool CTerminalUI::FullExport([[maybe_unused]] const std::vector<SMeasurementItem>& config,
                             [[maybe_unused]] const FullMeasurement data,
                             [[maybe_unused]] const AllSensors& allSensors,
                             [[maybe_unused]] const std::vector<Measurements::CCorrelation::SResult>& correlations)
{
  return true;
}

/**
 * @brief Shows the most recent measurements to the user in a live view
 */
void CTerminalUI::AddMeasurements(const Measurements::SMeasurementsData data)
{
  auto elements = GetElements(data);
  document_ = ftxui::vbox(elements);

  // Limit the size of the document to 80 char.
  document_ = document_ | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 120);

  auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(document_));
  Render(screen, document_);
  std::cout << position_;
  screen.Print();
  position_ = screen.ResetPosition();
}

/**
 * @brief Gets all ftxui::Elements containing the most recent measurements for a live view
 */
ftxui::Elements CTerminalUI::GetElements(const Measurements::SMeasurementsData data)
{
  ftxui::Elements elements;
  for (const auto& c : liveSensors_->allClasses)
  {
    const auto sensorGroups = liveSensors_->GetSensorGroups(c);
    elements = Helpers::CombineVectors(elements, GetElementsSensorGroups(sensorGroups, data));
  }
  return elements;
}

/**
 * @brief Gets the elements of each group
 */
ftxui::Elements CTerminalUI::GetElementsSensorGroups(
  const std::vector<Measurements::SAllSensors::SSensorGroups>& sensorGroups,
  const Measurements::SMeasurementsData data)
{
  ftxui::Elements elements;
  for (const auto& group : sensorGroups)
  {
    for (const auto& e : group.sensors)
    {
      if (e.userData.showInLive)
      {
        try
        {
          auto measurement = data.GetWithId(e.uniqueId);
          lastMeasurement_[e.uniqueId] = measurement.measuredValue;
          elements.push_back(GetElement(e, measurement.measuredValue, group.processId));
        }
        catch (const std::exception& err)
        {
          auto res = lastMeasurement_.find(e.uniqueId);
          if (res != lastMeasurement_.end())
            elements.push_back(GetElement(e, res->second, group.processId));
          else
            elements.push_back(GetElement(e, e.userData.minimumValue, group.processId));
        }
      }
    }
  }
  return elements;
}

/**
 * @brief Returns a ftxui::Element based on a sensor, measured value, and
 * process id. Thus, creates the ftxui::Element
 */
ftxui::Element CTerminalUI::GetElement(const Measurements::SSensors& sensor,
                                       const double& measuredValue,
                                       const int processId)
{
  double value = GetPercentage(sensor.userData, measuredValue);
  std::string name = processId == -1 ? sensor.userId : std::to_string(processId) + "." + sensor.userId;

  std::string fullValue = std::to_string(measuredValue * sensor.multiplier) + " " + sensor.suffix;

  return ftxui::hbox({ ftxui::text(name) | ftxui::border,
                       ftxui::color(GetColor(sensor.uniqueId), ftxui::gauge(value)) | ftxui::border | ftxui::flex,
                       ftxui::text(fullValue) | ftxui::border });
}

/**
 * @brief Get percentage between 0 and 1 (where 1 = 100% and 0 = 0%)
 */
double CTerminalUI::GetPercentage(const PlatformConfig::SDatafields& datafield, const double& measuredValue)
{
  auto corrected = measuredValue * datafield.multiplier;
  auto range = datafield.maximumValue - datafield.minimumValue;
  return (corrected - datafield.minimumValue) / (range);
}

/**
 * @brief Get the color of a bar based on the ID
 *
 * @param id the unique ID of a measurement
 * @return ftxui::Color the color of the bar
 */
ftxui::Color CTerminalUI::GetColor(const int id)
{
  auto res = colorMap_.find(id);

  if (res != colorMap_.end())
    return res->second;

  int colorId = currentColor_++;
  if (currentColor_ > colorRange_.max)
    currentColor_ = colorRange_.min;
  auto color = ftxui::Color{ static_cast<ftxui::Color::Palette16>(colorId) };
  colorMap_.insert({ id, color });
  return color;
}

} // namespace Exports