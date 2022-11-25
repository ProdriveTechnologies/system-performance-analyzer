#pragma once

#include <atomic>
#include <gst/gst.h>
#include <string>
#include <thread>

#include "src/process_runner/run_process_base.h"
#include "trace_parser.h"

class Synchronizer;

class CGstreamerHandler : public ProcessRunner::Base
{
public:
  CGstreamerHandler(Synchronizer *synchronizer, const int processId);
  CGstreamerHandler(const CGstreamerHandler &gstreamer);
  ~CGstreamerHandler();

  void StartThread(const std::string &command);
  void RunPipelineThread(const std::string &pipelineStr);

  void RunPipeline(const std::string &pipelineStr);

  int GetThreadPid() const { return threadPid_; }
  std::string GetPipeline() const { return pipelineStr_; }
  bool IsRunning() const { return running_; }

  size_t GetMeasurementsSize() { return traceHandler_.GetMeasurementsSize(); }
  int GetProcessId() const { return processId_; }

  GStreamer::Measurement GetMeasurement()
  {
    return traceHandler_.GetMeasurement();
  }
  struct LogStructure
  {
    GMainLoop *loop;
    CGstreamerHandler *parentClass;
  };

private:
  std::string pipelineStr_;
  Synchronizer *threadSync_;
  std::atomic<int> threadPid_;
  const int processId_;
  bool running_;
  GstElement *gstPipeline_;
  GstBus *gstBus_;
  GstMessage *gstMsg_;
  GError *gstErrorMsg_;
  LogStructure logUserData_;
  GStreamer::TraceHandler::TracerUserData tracerUserData_;
  GStreamer::TraceHandler traceHandler_;
  // std::thread pipelineThread_;

  void FreeMemory();
  void SetTracingEnvironmentVars();
  static void logFunction(GstDebugCategory *category, GstDebugLevel level,
                          const gchar *file, const gchar *function, gint line,
                          GObject *object, GstDebugMessage *message,
                          gpointer userData) G_GNUC_NO_INSTRUMENT;
  inline static int q = 0;
};