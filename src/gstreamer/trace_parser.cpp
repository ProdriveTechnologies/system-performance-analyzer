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

  std::unique_ptr<GstStructure, decltype(&gst_structure_free)> gstStructure{
      gst_structure_from_string(gst_debug_message_get(message), nullptr),
      gst_structure_free};

  if (!gstStructure)
  {
    // No proper structure included, returning as there is no data to parse
    return;
  }
  // if (object == nullptr || object == NULL)
  //   std::cout << "Ofcourse! Object pointer is a null pointer, how related..."
  //             << std::endl;
  // std::cout << "File: " << file << " and functor" << function << std::endl;
  // std::cout << gst_debug_message_get(message) << std::endl;
  // if (gstStructure->)
  logData->parent->ParseTraceStructure(gstStructure.get());
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
    std::cout << "FPS: " << trace.valueInt << std::endl;
    std::cout << "Frame rate! : " << gst_structure_to_string(gstStructure)
              << std::endl;
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