#include "handler.h"

#include "src/helpers/synchronizer.h"
#include <gst/gst.h>
#include <iostream>
#include <sys/types.h> //gettid()
#include <thread>
#include <unistd.h>

CGstreamerHandler::CGstreamerHandler(Synchronizer *synchronizer)
    : ProcessRunner::Base{synchronizer},
      threadSync_{synchronizer}, running_{false}, gstPipeline_{nullptr},
      gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{nullptr}
{
}
CGstreamerHandler::CGstreamerHandler(const CGstreamerHandler &gstreamer)
    : ProcessRunner::Base{gstreamer.threadSync_},
      threadSync_{gstreamer.threadSync_}, running_{false},
      gstPipeline_{nullptr}, gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{
                                                                     nullptr}
{
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *)data;
  std::cout << "Received bus call message" << std::endl;
  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
    g_print("End of stream\n");
    g_main_loop_quit(loop);
    break;

  case GST_MESSAGE_ERROR:
  {
    gchar *debug;
    GError *error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);

    g_main_loop_quit(loop);
    break;
  }
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

void CGstreamerHandler::RunPipeline(const std::string &pipelineStr)
{
  threadSync_->setThreadId(gettid());
  threadSync_->WaitForProcess();
  running_ = true;
  FreeMemory();

  // gstreamer initialization
  gst_init(nullptr, nullptr);
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  gstPipeline_ = gst_parse_launch(pipelineStr.c_str(), &gstErrorMsg_);
  GstBus *bussy = gst_pipeline_get_bus(GST_PIPELINE(gstPipeline_));
  int bus_watch_id = gst_bus_add_watch(bussy, bus_call, loop);
  gst_object_unref(bussy);

  // building pipeline
  std::cout << "Starting pipeline" << std::endl;

  if (gstPipeline_ == nullptr)
  {
    std::cout << "Error occured, returning!" << gstErrorMsg_->message
              << std::endl;
  }
  else
  {
    std::cout << "No error occured, what the" << std::endl;
  }
  threadSync_->WaitForProcess();

  // start playing
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);

  g_main_loop_run(loop);
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

  //   std::cout << "Received a message" << std::endl;
  //   std::cout << "Message type: " << gstMsg_->type << std::endl;

  // } while (gstMsg_->type != GST_MESSAGE_EOS);

  threadSync_->WaitForProcess();
  running_ = false;
}
