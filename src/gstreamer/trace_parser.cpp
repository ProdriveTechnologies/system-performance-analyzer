#include "trace_parser.h"

#include "src/helpers/helper_functions.h"
#include <iostream>
#include <memory>

#include "handler.h"
#include "measurement_types.h"

namespace GStreamer
{
void TraceHandler::TraceCallbackFunction(
    [[maybe_unused]] GstDebugCategory *category, GstDebugLevel level,
    [[maybe_unused]] const gchar *file, [[maybe_unused]] const gchar *function,
    [[maybe_unused]] gint line, GObject *object, GstDebugMessage *message,
    gpointer userData)
{
  if (level != GST_LEVEL_TRACE)
  {
    // Cannot parse this message, return
    return;
  }
  TracerUserData *logData = reinterpret_cast<TracerUserData *>(userData);

  const char *debugMessage = gst_debug_message_get(message);
  std::cout << "Gst structure string1: " << debugMessage << std::endl;
  std::string debugMessageStr = debugMessage;
  logData->parent->pipe_->Write("$" + debugMessageStr + "$");
}

void TraceHandler::ParseTraceStructure(const std::string &gstStructureStr)
{
  std::unique_ptr<GstStructure, decltype(&gst_structure_free)> gstStructure{
      gst_structure_from_string(gstStructureStr.c_str(), nullptr),
      gst_structure_free};
  if (!gstStructure)
  {
    // No proper structure included, returning as there is no data to parse
    return;
  }
  ParseTraceStructure(gstStructure.get());
}

void TraceHandler::ParseTraceStructure(const GstStructure *gstStructure)
{
  GStreamer::Measurement trace;
  switch (Helpers::hash(g_quark_to_string(gstStructure->name)))
  {
  case Helpers::hash("thread-rusage"):
    // std::cout << "T";
    break;
  case Helpers::hash("proc-rusage"):
    // std::cout << "P";
    break;
  case Helpers::hash("latency"):
    // std::cout << "L";
    break;
  case Helpers::hash("framerate"):
  {
    trace.type = MeasureType::FPS;
    trace.pluginName = gst_structure_get_string(gstStructure, "pad");
    gst_structure_get_uint(gstStructure, "fps", &trace.valueInt);
  }
  break;
  default:
    std::cout << "New: " << g_quark_to_string(gstStructure->name) << std::endl;
    std::cout << "Result: " << gst_structure_to_string(gstStructure)
              << std::endl;
  }
  if (trace.type != MeasureType::NONE)
  {
    // Send the "trace" measurement towards the Measurements class
    fifoMeasurements.push(trace);
  }
}

} // namespace GStreamer