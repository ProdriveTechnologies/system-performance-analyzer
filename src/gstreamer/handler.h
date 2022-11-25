#pragma once

#include <atomic>
#include <gst/gst.h>
#include <string>
#include <thread>

#include "src/process_runner/run_process_base.h"

class Synchronizer;

class CGstreamerHandler : public ProcessRunner::Base
{
public:
  CGstreamerHandler(Synchronizer *synchronizer);
  CGstreamerHandler(const CGstreamerHandler &gstreamer);
  ~CGstreamerHandler();

  void StartThread(const std::string &command);
  void RunPipelineThread(const std::string &pipelineStr);

  void RunPipeline(const std::string &pipelineStr);

  int GetThreadPid() const { return threadPid_; }
  bool IsRunning() const { return running_; }

private:
  Synchronizer *threadSync_;
  std::atomic<int> threadPid_;
  bool running_;

  GstElement *gstPipeline_;
  GstBus *gstBus_;
  GstMessage *gstMsg_;
  GError *gstErrorMsg_;
  // std::thread pipelineThread_;

  void FreeMemory();
};