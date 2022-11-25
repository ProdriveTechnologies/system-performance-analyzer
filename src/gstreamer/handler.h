#pragma once

#include <atomic>
#include <gst/gst.h>
#include <string>
#include <thread>

class Synchronizer;

class CGstreamerHandler
{
public:
  CGstreamerHandler(Synchronizer *synchronizer);
  ~CGstreamerHandler();

  void runPipelineThread(const std::string &pipelineStr);

  void runPipeline(const std::string &pipelineStr);

  int getThreadPid() const { return threadPid_; }
  bool isRunning() const { return running_; }
  bool *getRunningPtr() { return &running_; }

private:
  Synchronizer *threadSync_;
  std::atomic<int> threadPid_;
  bool running_;

  GstElement *gstPipeline_;
  GstBus *gstBus_;
  GstMessage *gstMsg_;
  GError *gstErrorMsg_;
  std::thread pipelineThread_;

  void freeMemory();
};