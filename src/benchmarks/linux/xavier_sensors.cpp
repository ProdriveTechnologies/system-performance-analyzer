#include "xavier_sensors.h"
#include "perf_measurements.h"

/**
 * @brief Returns the numeric value of a file
 *
 * @param datafield the data field
 * @return Measurements::SMeasuredItem the result
 *    [error]: SMeasuredItem{} with ID being -1 (or -2 when datafield.id is -1),
 * thus ID != datafield.id
 */
Measurements::SMeasuredItem
CXavierSensors::ParseDirect(const PlatformConfig::SMeasureField &datafield)
{
  Measurements::SMeasuredItem item;
  item.id = datafield.id;
  auto value = ReadLocation(datafield.path);
  if (value.empty())
  {
    // Error case, ID != datafield.id
    item.id = datafield.id == -1 ? -2 : -1;
    return item;
  }
  item.measuredValue = std::stoi(value);
  return item;
}

/**
 * @brief ReadLocation reads the data at a certain path location on Linux
 *
 * @param path the path of the file
 * @return std::string the content of the file
 */
std::string CXavierSensors::ReadLocation(const std::string &path)
{
  std::string dataBuffer;
  std::ifstream fileObj(path);
  if (!fileObj.is_open() || !fileObj.good())
  {
    return {};
  }
  fileObj >> dataBuffer;
  fileObj.close();

  return dataBuffer;
}