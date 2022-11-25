#include "trace_parser.h"

#include "src/helpers/helper_functions.h"
#include <iostream>

namespace GStreamer
{
void TraceHandler::TraceCallbackFunction(
    [[maybe_unused]] GstDebugCategory *category, GstDebugLevel level,
    [[maybe_unused]] const gchar *file, [[maybe_unused]] const gchar *function,
    [[maybe_unused]] gint line, GObject *object, GstDebugMessage *message,
    gpointer user_data)
{
  if (level != GST_LEVEL_TRACE)
  {
    // Cannot parse this message, return
    return;
  }
  //   if (object == nullptr)
  //   {
  //     std::cout << "Thingy relates to nothingg" << std::endl;
  //   }
  //   else
  //   {
  //     std::cout << "THINGY relates to something: "
  //               << object->g_type_instance.g_class->g_type << std::endl;
  //   }
  //   std::cout << "File: " << file << "  line: " << line
  //             << " function: " << function
  //             << "  and type: " << gst_debug_message_get(message) <<
  //             std::endl;

  GstStructure *gstStructure =
      gst_structure_from_string(gst_debug_message_get(message), nullptr);

  if (gstStructure == nullptr)
  {
    // No proper structure included, returning as there is no data to parse
    return;
  }
  ParseTraceStructure(gstStructure);
}

void TraceHandler::ParseTraceStructure(const GstStructure *gstStructure)
{
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
    {
      static int lat = 0;
      lat++;
      std::cout << lat << std::endl;
    }
    break;
  case Helpers::hash("framerate"):
    std::cout << "Frame rate! : " << gst_structure_to_string(gstStructure)
              << std::endl;
    break;
  default:
    std::cout << "New: " << g_quark_to_string(gstStructure->name) << std::endl;
    std::cout << "Result: " << gst_structure_to_string(gstStructure)
              << std::endl;
  }
  auto value = gst_structure_get_value(gstStructure, "average-cpuload");
  if (value != NULL)
  {
    // std::cout << "Value: " << value->data->v_int << std::endl;
  }
}

} // namespace GStreamer