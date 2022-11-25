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
                                     const int processId)
    : ProcessRunner::Base{synchronizer}, threadSync_{synchronizer},
      processId_{processId}, running_{false}, gstPipeline_{nullptr},
      gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{nullptr}, traceHandler_{
                                                                     &pipe_}
{
}
CGstreamerHandler::CGstreamerHandler(const CGstreamerHandler &gstreamer)
    : ProcessRunner::Base{gstreamer.threadSync_},
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
  if (gstBus_ != nullptr)
    gst_object_unref(gstBus_);
  if (gstMsg_ != nullptr)
    gst_message_unref(gstMsg_);
  if (gstErrorMsg_ != nullptr)
    g_clear_error(&gstErrorMsg_);
}

void CGstreamerHandler::StartThread(const std::string &command)
{
  // if (pipelineThread_.joinable())
  //   pipelineThread_.join();
  // pipelineThread_ = std::thread{&CGstreamerHandler::RunPipeline, this,
  // command};
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  applicationPid_ = fork();
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
    if (!gst_is_initialized())
      gst_init(nullptr, nullptr);
    pipe_.SetParent();
    pipelineThread_ = std::thread(&CGstreamerHandler::ParentWaitProcess, this);
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
#if 0
  /************************************************
   *  BEGIN EXAMPLE ONLINE
   *************************************************/
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;

  SetTracingEnvironmentVars();
  gst_debug_remove_log_function(gst_debug_log_default);
  gst_debug_add_log_function(&GStreamer::TraceHandler::TraceCallbackFunction,
                             nullptr, nullptr);
  /* Initialize GStreamer */
  gst_init(NULL, NULL);

  /* Create the elements */
  data.source = gst_element_factory_make("filesrc", "source");
  data.convert = gst_element_factory_make("queue", "convert");
  data.resample = gst_element_factory_make("rawvideoparse", "resample");
  data.sink = gst_element_factory_make("autovideosink", "sink");

  /* Create the empty pipeline */
  data.pipeline = gst_pipeline_new("test-pipeline");

  if (!data.pipeline || !data.source || !data.convert || !data.resample ||
      !data.sink)
  {
    g_printerr("Not all elements could be created.\n");
    return;
  }

  /* Build the pipeline. Note that we are NOT linking the source at this
   * point. We will do it later. */
  gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.convert,
                   data.resample, data.sink, NULL);
  if (!gst_element_link_many(data.source, data.convert, data.resample,
                             data.sink, NULL))
  {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(data.pipeline);
    return;
  }

  /* Set the URI to play */
  g_object_set(data.source, "location", "midlength.yuv", NULL);

  /* Connect to the pad-added signal */
  std::cout << "Adding pad signal, please execute it though" << std::endl;
  g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler),
                   &data);

  /* Start playing */
  ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr("Unable to set the pipeline to the playing state.\n");
    gst_object_unref(data.pipeline);
    return;
  }

  /* Listen to the bus */
  bus = gst_element_get_bus(data.pipeline);
  do
  {
    msg = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_STATE_CHANGED |
                                    GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Parse message */
    if (msg != NULL)
    {
      GError *err;
      gchar *debug_info;

      switch (GST_MESSAGE_TYPE(msg))
      {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received from element %s: %s\n",
                   GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n",
                   debug_info ? debug_info : "none");
        g_clear_error(&err);
        g_free(debug_info);
        terminate = TRUE;
        break;
      case GST_MESSAGE_EOS:
        g_print("End-Of-Stream reached.\n");
        terminate = TRUE;
        break;
      case GST_MESSAGE_STATE_CHANGED:
        /* We are only interested in state-changed messages from the pipeline */
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline))
        {
          GstState old_state, new_state, pending_state;
          gst_message_parse_state_changed(msg, &old_state, &new_state,
                                          &pending_state);
          g_print("Pipeline state changed from %s to %s:\n",
                  gst_element_state_get_name(old_state),
                  gst_element_state_get_name(new_state));
        }
        break;
      default:
        /* We should not reach here */
        g_printerr("Unexpected message received.\n");
        break;
      }
      gst_message_unref(msg);
    }
  } while (!terminate);

  /* Free resources */
  gst_object_unref(bus);
  gst_element_set_state(data.pipeline, GST_STATE_NULL);
  gst_object_unref(data.pipeline);
  std::cout << "DONE, RETURING" << std::endl;
  return;
#endif
  /*************************************************
   * END EXAMPLE ONLINE
   **************************************************/

  threadSync_->setThreadId(gettid());
  CLogger::Log(CLogger::Types::INFO, "Starting synchronize for gstreamer");
  ChildWaitProcess();
  //  threadSync_->WaitForProcess();
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
  GstBus *bussy = gst_pipeline_get_bus(GST_PIPELINE(gstPipeline_));
  int bus_watch_id = gst_bus_add_watch(bussy, bus_call, &logUserData_);
  gst_object_unref(bussy);

  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 2 for gstreamer");
  // threadSync_->WaitForProcess();
  ChildWaitProcess();
  CLogger::Log(CLogger::Types::INFO,
               "Starting the pipeline (finally) for gstreamer");
  // start playing
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);

  g_main_loop_run(loop);
  g_source_remove(bus_watch_id);
  g_main_loop_unref(loop);

  ChildWaitProcess();
  threadSync_->WaitForProcess();
  FreeMemory();
  gst_deinit();
  running_ = false;
}

/**
 * @brief Enables the necessary tracers for the system
 *
 */
void CGstreamerHandler::SetTracingEnvironmentVars()
{
  setenv("GST_DEBUG", "GST_TRACER:7", true);
  // setenv("GST_TRACERS", "rusage;latency;framerate", true);
  setenv("GST_TRACERS", "framerate", true);
}