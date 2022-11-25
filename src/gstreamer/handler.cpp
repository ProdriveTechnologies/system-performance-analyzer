#include "handler.h"

#include "src/helpers/synchronizer.h"
#include <gst/gst.h>
#include <iostream>
#include <sys/types.h> //gettid()
#include <thread>
#include <unistd.h>

CGstreamerHandler::CGstreamerHandler(Synchronizer *synchronizer)
    : threadSync_{synchronizer}, running_{false}, gstPipeline_{nullptr},
      gstBus_{nullptr}, gstMsg_{nullptr}, gstErrorMsg_{nullptr}
{
}

CGstreamerHandler::~CGstreamerHandler() { freeMemory(); }

void CGstreamerHandler::freeMemory()
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

void CGstreamerHandler::runPipelineThread(const std::string &pipelineStr)
{
  if (pipelineThread_.joinable())
    pipelineThread_.join();
  pipelineThread_ =
      std::thread{&CGstreamerHandler::runPipeline, this, pipelineStr};
}

void CGstreamerHandler::runPipeline(const std::string &pipelineStr)
{
  std::cerr << " and tid: " << gettid();
  threadSync_->setThreadId(gettid());
  threadSync_->waitForProcess();
  running_ = true;
  freeMemory();

  // gstreamer initialization
  gst_init(nullptr, nullptr);

  // building pipeline
  std::cout << "Starting pipeline" << std::endl;
  gstPipeline_ = gst_parse_launch(pipelineStr.c_str(), &gstErrorMsg_);

  if (gstPipeline_ == nullptr)
  {
    std::cout << "Error occured, returning!" << gstErrorMsg_->message
              << std::endl;
  }
  else
  {
    std::cout << "No error occured, what the" << std::endl;
  }
  threadSync_->waitForProcess();

  // start playing
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);

  // wait until error or EOS ( End Of Stream )
  gstBus_ = gst_element_get_bus(gstPipeline_);
  gstMsg_ = gst_bus_timed_pop_filtered(
      gstBus_, GST_CLOCK_TIME_NONE,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
  running_ = false;
}