#pragma once

#include <string>
#include <variant>

#include "src/gstreamer/handler.h"
#include "src/json_config/config.h"
#include "src/linux/run_process.h"

// Struct to store the info for the processes. The variant is used to store
// the object type in
struct ProcessInfo
{
  Core::ProcessType type;
  std::string command;
  std::variant<CGstreamerHandler, Linux::RunProcess> processes; // int
  ProcessInfo(const Core::SProcess &process, CGstreamerHandler streamHandler)
      : type{process.type}, command{process.command}, processes{streamHandler}
  {
  }
  ProcessInfo(const Core::SProcess &process, Linux::RunProcess processHandler)
      : type{process.type}, command{process.command}, processes{processHandler}
  {
  }
  ~ProcessInfo() = default;
};