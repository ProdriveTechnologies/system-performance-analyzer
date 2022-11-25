#pragma once

#include <string>

namespace Measurements
{
// struct
// {
//   std::vector<Sensors> sensors;
// };

struct Sensors
{
  bool thresholdExceeded = false;
  std::string userId;  // UserId is the ID that's given by the user in the
                       // "xavier_config.json" file. If it is in an array, this
                       // userId will have a number of the array behind it
  std::string groupId; // Only applicable for arrays, this is the groupname and
  // therefore does not contain the array number. This is
  // the "original" user id of the sensor

  Sensors(const std::string &userId);
  // Something of a pointer to the necessary data
};
// THIS FUNCTION MUST BE MOVED< ONLY AN EXAMPLE
// void AddGstreamerSensors()
// {
//   std::vector<Sensors> sensors = {Sensors{"fps"}, Sensors{"latency"}};

} // namespace Measurements