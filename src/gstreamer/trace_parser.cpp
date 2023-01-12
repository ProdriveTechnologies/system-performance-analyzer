#include "trace_parser.h"

#include "handler.h"
#include "measurement_types.h"
#include "src/helpers/helper_functions.h"

#include <iostream>
#include <memory> // unique_ptr

namespace GStreamer
{
void TraceHandler::TraceCallbackFunction([[maybe_unused]] GstDebugCategory* category,
                                         GstDebugLevel level,
                                         [[maybe_unused]] const gchar* file,
                                         [[maybe_unused]] const gchar* function,
                                         [[maybe_unused]] gint line,
                                         [[maybe_unused]] GObject* object,
                                         GstDebugMessage* message,
                                         gpointer userData)
{
  if (level != GST_LEVEL_TRACE)
  {
    // Cannot parse this message, return
    return;
  }
  TracerUserData* logData = reinterpret_cast<TracerUserData*>(userData);

  const char* debugMessage = gst_debug_message_get(message);
  std::string debugMessageStr = debugMessage;
  logData->parent->pipe_->Write("$" + debugMessageStr + "$");
}

void TraceHandler::ParseTraceStructure(const std::string& gstStructureStr)
{
  std::unique_ptr<GstStructure, decltype(&gst_structure_free)> gstStructure{
    gst_structure_from_string(gstStructureStr.c_str(), nullptr),
    gst_structure_free
  };
  if (!gstStructure)
  {
    // No proper structure included, returning as there is no data to parse
    return;
  }
  ParseTraceStructure(gstStructure.get());
}

void TraceHandler::ParseTraceStructure(const GstStructure* gstStructure)
{
  GStreamer::EMeasurement trace;
  switch (Helpers::hash(g_quark_to_string(gstStructure->name)))
  {
  case Helpers::hash("thread-rusage"):
    break;
  case Helpers::hash("proc-rusage"):
    break;
  case Helpers::hash("proctime"):
  {
    trace.type = EMeasureType::PROCESSING_TIME;
    trace.pluginName = gst_structure_get_string(gstStructure, "element");
    std::string time = gst_structure_get_string(gstStructure, "time");
    trace.valueInt = TimeToInt(time);
  }
  break;
  case Helpers::hash("latency"):
  {
    trace.type = EMeasureType::LATENCY;
    trace.pluginName = gst_structure_get_string(gstStructure, "src-element");
    // gst_structure_get_uint(gstStructure, "fps", &trace.valueInt);
    // Latency should be enabled in gstreamer/handler.cpp if necessary
    // And the latency should be correctly retrieved from the gst message
  }
  break;
  case Helpers::hash("framerate"):
  {
    trace.type = EMeasureType::FPS;
    trace.pluginName = gst_structure_get_string(gstStructure, "pad");
    gst_structure_get_uint(gstStructure, "fps", &trace.valueInt);
  }
  break;
  default:
    std::cout << "New: " << g_quark_to_string(gstStructure->name) << std::endl;
    std::cout << "Result: " << gst_structure_to_string(gstStructure) << std::endl;
  }
  if (trace.type != EMeasureType::NONE)
  {
    // Send the "trace" measurement towards the Measurements class
    fifoMeasurements_.push(trace);
  }
}

/**
 * @brief Converts time to a milliseconds,
 *
 * @param time format: 0:00:00.000000000
 * @return int
 */
int TraceHandler::TimeToInt(const std::string& time)
{
  std::string timeCpy = time;
  Helpers::replaceStr(timeCpy, ".", ":");
  auto separateElements = Helpers::Split(timeCpy, ':');
  if (separateElements.size() != 4)
    throw std::runtime_error("Incorrect format! Should have 4 elements!");
  int result = 0;
  result += Helpers::DecimalsToInt(separateElements.at(3), 4); // Millis
  result += std::stoi(separateElements.at(2)) * 1000;
  result += std::stoi(separateElements.at(1)) * 1000 * 60;
  result += std::stoi(separateElements.at(0)) * 1000 * 60 * 60;
  return result;
}

} // namespace GStreamer