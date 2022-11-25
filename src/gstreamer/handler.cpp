#include "handler.h"

#include "src/helpers/synchronizer.h"
#include <gst/gst.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h> //gettid()
#include <thread>
#include <unistd.h>

#include "src/gstreamer/measurement_types.h"
#include "src/helpers/logger.h"
#include "trace_parser.h"
#include <memory>
#include <vector>

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData
{
  GstElement *pipeline;
  GstElement *source;
  GstElement *convert;
  GstElement *resample;
  GstElement *sink;
} CustomData;

/* This function will be called by the pad-added signal */
/**
 * @brief This function is not used anymore and can therefore be
 * removed (including the unnecessary data)
 */
static void pad_added_handler(GstElement *src, GstPad *new_pad,
                              CustomData *data)
{
  std::cout << "daymy" << std::endl;
  GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;

  g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad),
          GST_ELEMENT_NAME(src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked(sink_pad))
  {
    g_print("We are already linked. Ignoring.\n");
    return;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps(new_pad);
  new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
  new_pad_type = gst_structure_get_name(new_pad_struct);
  if (!g_str_has_prefix(new_pad_type, "audio/x-raw"))
  {
    g_print("It has type '%s' which is not raw audio. Ignoring.\n",
            new_pad_type);
    return;
  }

  /* Attempt the link */
  ret = gst_pad_link(new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED(ret))
  {
    g_print("Type is '%s' but link failed.\n", new_pad_type);
  }
  else
  {
    g_print("Link succeeded (type '%s').\n", new_pad_type);
  }
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref(new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref(sink_pad);
}

CGstreamerHandler::CGstreamerHandler(Synchronizer *synchronizer,
                                     const int processId)
    : ProcessRunner::Base{synchronizer}, threadSync_{synchronizer},
      processId_{processId}, running_{false}, gstPipeline_{nullptr},
      gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{nullptr}
{
}
CGstreamerHandler::CGstreamerHandler(const CGstreamerHandler &gstreamer)
    : ProcessRunner::Base{gstreamer.threadSync_},
      threadSync_{gstreamer.threadSync_},
      processId_{gstreamer.processId_}, running_{false}, gstPipeline_{nullptr},
      gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{nullptr}
{
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
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
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  pipelineThread_ = std::thread{&CGstreamerHandler::RunPipeline, this, command};
}

void CGstreamerHandler::RunPipelineThread(const std::string &pipelineStr)
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  pipelineThread_ =
      std::thread{&CGstreamerHandler::RunPipeline, this, pipelineStr};
}

/**
 * @note This function is NOT used! Anymore RIP!
 * @todo remove this function, as it is not in use anymore...
 */
void CGstreamerHandler::logFunction(GstDebugCategory *category,
                                    GstDebugLevel level, const gchar *file,
                                    const gchar *function, gint line,
                                    GObject *object, GstDebugMessage *message,
                                    gpointer userData)
{
  // std::cout << "category: " << category->name << " and "
  //          << category->description << std::endl;
  if (strcmp(category->name, "GST_TRACER"))
  {
    std::cout << "Not a GST_TRACER subsystem!" << std::endl;
    return;
  }
  if (object == nullptr)
  {
    std::cout << "Thingy relates to nothingg" << std::endl;
  }
  else
  {
    std::cout << "THINGY relates to something: "
              << object->g_type_instance.g_class->g_type << std::endl;
  }
  LogStructure *logData = reinterpret_cast<LogStructure *>(userData);
  q++;
  std::cout << "File: " << file << "  line: " << line
            << " function: " << function
            << "  and type: " << gst_debug_message_get(message)
            << "\n and Q: " << q << std::endl;

  GstStructure *gstStructure =
      gst_structure_from_string(gst_debug_message_get(message), nullptr);
  std::cout << "Came here already" << std::endl;
  if (gstStructure == NULL)
  {
    std::cout << "Couldn't parse it" << std::endl;
  }
  else
  {
    auto value = gst_structure_get_value(gstStructure, "average-cpuload");
    if (value != NULL)
    {
      std::cout << "Value: " << value->data->v_int << std::endl;
    }
    std::cout << "GstStructure" << gstStructure->name
              << " types: " << gstStructure->type << " values" << std::endl;
  }
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
  threadSync_->WaitForProcess();
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
  threadSync_->WaitForProcess();
  CLogger::Log(CLogger::Types::INFO,
               "Starting the pipeline (finally) for gstreamer");
  // start playing
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);

  g_main_loop_run(loop);
  g_source_remove(bus_watch_id);
  g_main_loop_unref(loop);
  // wait until error or EOS ( End Of Stream )
  // gstBus_ = gst_element_get_bus(gstPipeline_);
  // do
  // {
  //   if (gstMsg_ != nullptr)
  //     g_clear_error(&gstErrorMsg_);

  //   gstMsg_ = gst_bus_timed_pop_filtered(
  //       gstBus_, GST_CLOCK_TIME_NONE,
  //       static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS |
  //                                   GST_MESSAGE_LATENCY | GST_MESSAGE_ANY));

  //   //   std::cout << "Received a message" << std::endl;
  //   //   std::cout << "Message type: " << gstMsg_->type << std::endl;

  // } while (gstMsg_->type != GST_MESSAGE_EOS);

  threadSync_->WaitForProcess();
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
  // setenv("GST_TRACE_CHANNEL", "");
}