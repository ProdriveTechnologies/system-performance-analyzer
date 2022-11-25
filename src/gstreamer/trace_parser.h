#pragma once

#include <gst/gst.h>

#include "measurement_types.h"
#include "src/helpers/atomic_queue.h"

class CGstreamerHandler; // Pre-defined value

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
  void ParseTraceStructure(const GstStructure *gstStructure);

  struct TracerUserData
  {
    TraceHandler *parent;
  };

  size_t GetMeasurementsSize() const { return fifoMeasurements.size(); }

  /**
   * @brief Gets the measurement object and directly removes it from the FIFO

   */
  Measurement GetMeasurement()
  {
    if (GetMeasurementsSize() != 0)
    {
      auto measurement = fifoMeasurements.front();
      fifoMeasurements.pop(); // Directly remove it from the fifo
      return measurement;
    }
    else
    {
      throw std::runtime_error(
          "Could not retrieve measurement as the FIFO is empty!");
    }
  }

private:
  Helpers::AtomicQueue<Measurement> fifoMeasurements;
  // static std::unordered_map()
};
} // namespace GStreamer