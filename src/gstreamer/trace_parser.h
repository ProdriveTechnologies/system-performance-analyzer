#pragma once

#include "measurement_types.h"
#include "src/helpers/atomic_queue.h"
#include "src/linux/pipe_comm.h"

#include <gst/gst.h>

class CGstreamerHandler; // Pre-defined value

namespace GStreamer
{
class TraceHandler
{
public:
  TraceHandler(Linux::PipeCommunicator* pipe)
  : pipe_{ pipe }
  {
  }

  static void TraceCallbackFunction([[maybe_unused]] GstDebugCategory* category,
                                    GstDebugLevel level,
                                    [[maybe_unused]] const gchar* file,
                                    [[maybe_unused]] const gchar* function,
                                    [[maybe_unused]] gint line,
                                    GObject* object,
                                    GstDebugMessage* message,
                                    gpointer user_data);
  void ParseTraceStructure(const std::string& gstStructureStr);
  void ParseTraceStructure(const GstStructure* gstStructure);

  struct TracerUserData
  {
    TraceHandler* parent;
  };

  size_t GetMeasurementsSize() const { return fifoMeasurements_.size(); }

  /**
   * @brief Gets the measurement object and directly removes it from the FIFO

   */
  EMeasurement GetMeasurement()
  {
    if (GetMeasurementsSize() != 0)
    {
      auto measurement = fifoMeasurements_.front();
      fifoMeasurements_.pop(); // Directly remove it from the fifo
      return measurement;
    }
    else
    {
      throw std::runtime_error("Could not retrieve measurement as the FIFO is empty!");
    }
  }
  int pid;

private:
  Helpers::AtomicQueue<EMeasurement> fifoMeasurements_;
  Linux::PipeCommunicator* pipe_;

  int TimeToInt(const std::string& time);
};
} // namespace GStreamer