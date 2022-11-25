#pragma once

#include <gst/gst.h>

namespace GStreamer
{
class TraceHandler
{
public:
  static void TraceCallbackFunction([[maybe_unused]] GstDebugCategory *category,
                                    GstDebugLevel level,
                                    [[maybe_unused]] const gchar *file,
                                    [[maybe_unused]] const gchar *function,
                                    [[maybe_unused]] gint line, GObject *object,
                                    GstDebugMessage *message,
                                    gpointer user_data);
  static void ParseTraceStructure(const GstStructure *gstStructure);

private:
  // static std::unordered_map()
};
} // namespace GStreamer