#include "handler.h"

#include <gst/gst.h>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <sys/types.h> //gettid()
#include <thread>
#include <unistd.h>
#include <vector>

#include "src/gstreamer/measurement_types.h"
#include "src/helpers/logger.h"
#include "src/helpers/synchronizer.h"
#include "trace_parser.h"

CGstreamerHandler::CGstreamerHandler(Synchronizer *synchronizer,
                                     const Core::SProcess &userProcessInfo,
                                     const int processId)
    : ProcessRunner::Base{synchronizer, userProcessInfo},
      threadSync_{synchronizer}, processId_{processId}, running_{false},
      gstPipeline_{nullptr}, gstBus_{nullptr}, gstMsg_{nullptr},
      gstErrorMsg_{nullptr}, traceHandler_{&pipe_}
{
}
CGstreamerHandler::CGstreamerHandler(const CGstreamerHandler &gstreamer)
    : ProcessRunner::Base{gstreamer.threadSync_, gstreamer.userProcessInfo_},
      threadSync_{gstreamer.threadSync_}, processId_{gstreamer.processId_},
      running_{false}, gstPipeline_{nullptr}, gstBus_{nullptr},
      gstMsg_{nullptr}, gstErrorMsg_{nullptr}, traceHandler_{&pipe_}
{
}

static gboolean bus_call([[maybe_unused]] GstBus *bus, GstMessage *msg,
                         gpointer data)
{
  auto *userData = reinterpret_cast<CGstreamerHandler::LogStructure *>(data);
  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
    CLogger::Log(CLogger::Types::INFO, "GStreamer: End of stream occured");
    g_main_loop_quit(userData->loop);
    break;

  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);

    g_main_loop_quit(userData->loop);
    break;
  }
  case GST_MESSAGE_STREAM_STATUS:
    CLogger::Log(CLogger::Types::INFO,
                 "GStreamer: Received GST message stream status");

    break;
  default:
    break;
  }

  return TRUE;
}

CGstreamerHandler::~CGstreamerHandler()
{
  FreeMemory();
  if (pipelineThread_.joinable())
    pipelineThread_.join();
}

void CGstreamerHandler::FreeMemory()
{
  if (gstPipeline_ != nullptr)
  {
    gst_element_set_state(gstPipeline_, GST_STATE_NULL);
    gst_object_unref(gstPipeline_);
  }
  if (gstMsg_ != nullptr)
    gst_message_unref(gstMsg_);
  if (gstErrorMsg_ != nullptr)
    g_clear_error(&gstErrorMsg_);
}

void CGstreamerHandler::StartThread(const std::string &command)
{
  // First, clean up an old thread, if existent
  if (pipelineThread_.joinable())
    pipelineThread_.join();

  applicationPid_ = fork(); // Fork the process where GStreamer will run in
  if (applicationPid_ < 0)
  {
    std::string response{strerror(errno)};
    response = "Fork failed! Reason: " + response;
    throw std::runtime_error(response);
  }
  else if (applicationPid_ == 0)
  {
    pipe_.SetChild();
    RunPipeline(command);
    exit(EXIT_SUCCESS);
  }
  else
  {
    // Initialize GST for the parent, such that it can also use the GStreamer
    // functions to parse the trace messages
    // if (!gst_is_initialized())
    //   gst_init(nullptr, nullptr);
    pipe_.SetParent();
    pipelineThread_ = std::thread(&CGstreamerHandler::ParentWaitProcess, this);
    traceHandler_.pid = processId_;
    // Parent process
    return;
  }
}
void CGstreamerHandler::ParentWaitProcess()
{
  int waitCount = 0;
  const int maxWaitCount = 3; // Initial waits are twice
  while (waitCount != maxWaitCount)
  {
    std::string message = pipe_.ReadBetweenChars('$');
    if (message == waitMessage_)
    {
      CLogger::Log(CLogger::Types::INFO, "Starting synchronize ", waitCount + 1,
                   " for child");
      processSync_->WaitForProcess();
      waitCount++;
      pipe_.Write(waitDoneMsg_);
      if (waitCount == 1)
      {
        // Initialize GST for the parent, must be separated from the GST
        // environments from the other processes
        if (!gst_is_initialized())
          gst_init(nullptr, nullptr);
      }
    }
    else
    {
      message.pop_back();
      message.erase(0, 1);
      traceHandler_.ParseTraceStructure(message);
    }
  }
  // Waiting until the stress test is fully executes
  // processSync_->WaitForProcess();
}

void CGstreamerHandler::RunPipelineThread(const std::string &pipelineStr)
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  pipelineThread_ =
      std::thread{&CGstreamerHandler::RunPipeline, this, pipelineStr};
}

void CGstreamerHandler::RunPipeline(const std::string &pipelineStr)
{
  GMainLoop *loop = PipelineInitialization(pipelineStr);

  // start playing
  CLogger::Log(CLogger::Types::INFO, "Starting the pipeline for gstreamer");
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);
  g_main_loop_run(loop);

  ChildWaitProcess();
  // De-initialize
  FreeMemory();
  g_source_remove(busWatchId_);
  g_main_loop_unref(loop);

  gst_deinit();
  running_ = false;
}

GMainLoop *
CGstreamerHandler::PipelineInitialization(const std::string &pipelineStr)
{
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize for gstreamer");
  ChildWaitProcess();

  running_ = true;
  pipelineStr_ = pipelineStr;
  FreeMemory();

  SetTracingEnvironmentVars();
  gst_debug_remove_log_function(gst_debug_log_default);
  // gstreamer initialization
  if (!gst_is_initialized())
    gst_init(nullptr, nullptr);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  logUserData_.loop = loop;
  logUserData_.parentClass = this;
  tracerUserData_.parent = &traceHandler_;

  gst_debug_add_log_function(&GStreamer::TraceHandler::TraceCallbackFunction,
                             &tracerUserData_, nullptr);

  gstPipeline_ = gst_parse_launch(pipelineStr.c_str(), &gstErrorMsg_);
  gstBus_ = gst_pipeline_get_bus(GST_PIPELINE(gstPipeline_));
  busWatchId_ = gst_bus_add_watch(gstBus_, bus_call, &logUserData_);
  gst_object_unref(gstBus_);

  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 2 for gstreamer");
  ChildWaitProcess();
  std::this_thread::sleep_for(
      std::chrono::milliseconds(userProcessInfo_.startDelay));
  return loop;
}

/**
 * @brief Enables the necessary tracers for the system
 *
 */
void CGstreamerHandler::SetTracingEnvironmentVars()
{
  setenv("GST_DEBUG", "GST_TRACER:7", true);
  // setenv("GST_TRACERS", "rusage;latency;framerate", true);
  setenv("GST_TRACERS", "framerate;proctime", true);
}