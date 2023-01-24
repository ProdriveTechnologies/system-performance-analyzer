#include "handler.h"

#include "src/helpers/logger.h"
#include "src/helpers/synchronizer.h"

#include <thread>
#include <unistd.h> // fork

CGstreamerHandler::CGstreamerHandler(Synchronizer* synchronizer,
                                     const Core::SProcess& userProcessInfo,
                                     const Core::SSettings& settings,
                                     const int processId)
: ProcessRunner::Base{ synchronizer, userProcessInfo }
, threadSync_{ synchronizer }
, processId_{ processId }
, running_{ false }
, gstPipeline_{ nullptr }
, gstBus_{ nullptr }
, gstMsg_{ nullptr }
, busWatchId_{ -1 }
, gstErrorMsg_{ nullptr }
, settings_{ settings }
, traceHandler_{ &pipe_ }
{
}
CGstreamerHandler::CGstreamerHandler(const CGstreamerHandler& gstreamer)
: ProcessRunner::Base{ gstreamer.threadSync_, gstreamer.userProcessInfo_ }
, threadSync_{ gstreamer.threadSync_ }
, processId_{ gstreamer.processId_ }
, running_{ false }
, gstPipeline_{ nullptr }
, gstBus_{ nullptr }
, gstMsg_{ nullptr }
, busWatchId_{ -1 }
, gstErrorMsg_{ nullptr }
, settings_{ gstreamer.settings_ }
, traceHandler_{ &pipe_ }
{
}

/**
 * @brief Error and message handler for GStreamer
 */
static gboolean bus_call([[maybe_unused]] GstBus* bus, GstMessage* msg, gpointer data)
{
  auto* userData = reinterpret_cast<CGstreamerHandler::LogStructure*>(data);
  switch (GST_MESSAGE_TYPE(msg))
  {
  case GST_MESSAGE_EOS:
    CLogger::Log(CLogger::Types::INFO, "GStreamer: End of stream occured");
    g_main_loop_quit(userData->loop);
    break;

  case GST_MESSAGE_ERROR:
  {
    gchar* debug;
    GError* error;

    gst_message_parse_error(msg, &error, &debug);
    g_free(debug);

    g_printerr("Error: %s\n", error->message);
    g_error_free(error);

    g_main_loop_quit(userData->loop);
    break;
  }
  case GST_MESSAGE_STREAM_STATUS:
    CLogger::Log(CLogger::Types::INFO, "GStreamer: Received GST message stream status");

    break;
  default:
    break;
  }

  return TRUE;
}

/**
 * @brief Destructor of the CGStreamerHandler class
 */
CGstreamerHandler::~CGstreamerHandler()
{
  FreeMemory();
  if (pipelineThread_.joinable())
    pipelineThread_.join();
}

/**
 * @brief Frees up the used memory of the class
 */
void CGstreamerHandler::FreeMemory()
{
  if (gstPipeline_ != nullptr && GST_OBJECT_REFCOUNT(gstPipeline_) > 0)
  {
    gst_element_set_state(gstPipeline_, GST_STATE_NULL);
    gst_object_unref(gstPipeline_);
  }
  if (gstMsg_ != nullptr)
    gst_message_unref(gstMsg_);
  if (gstErrorMsg_ != nullptr)
    g_clear_error(&gstErrorMsg_);
}

/**
 * @brief Starts the GStreamer threads and necessary processes
 *
 * @param command the GStreamer pipeline as a string
 */
void CGstreamerHandler::StartThread(const std::string& command)
{
  // First, clean up an old thread, if existent
  if (pipelineThread_.joinable())
    pipelineThread_.join();

  applicationPid_ = fork(); // Fork the process where GStreamer will run in
  if (applicationPid_ < 0)
  {
    std::string response{ strerror(errno) };
    response = "Fork failed! Reason: " + response;
    throw std::runtime_error(response);
  }
  else if (applicationPid_ == 0)
  {
    pipe_.SetChild();
    // Register a stop function
    signal(SIGINT, CGstreamerHandler::StopInterruptHandler);
    RunPipeline(command);
    exit(EXIT_SUCCESS);
  }
  else
  {
    // Initialize GST for the parent, such that it can also use the GStreamer
    // functions to parse the trace messages
    pipe_.SetParent();
    pipelineThread_ = std::thread(&CGstreamerHandler::ParentWaitProcess, this);
    traceHandler_.pid = processId_;
    // Parent process
    return;
  }
}

/**
 * @brief Stops a GStreamer process during execution
 */
void CGstreamerHandler::StopInterruptHandler([[maybe_unused]] int signal)
{
  g_main_loop_quit(loop_);
}

/**
 * @brief Thread that waits until the GStreamer process is finished.
 * This thread checks if the GStreamer process needs to be stopped in case of a SIGINT interrupt
 * Additionally, it manages the synchronization between the GStreamer process and the other processes
 */
void CGstreamerHandler::ParentWaitProcess()
{
  int waitCount = 0;
  const int maxWaitCount = 3; // Initial waits are twice

  while (waitCount != maxWaitCount)
  {
    if (pipe_.GetBytesAvailable() > 0)
    {
      std::string message = pipe_.ReadBetweenChars('$');

      if (message == waitMessage_)
      {
        CLogger::Log(CLogger::Types::INFO, "Starting synchronize ", waitCount + 1, " for child");
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
    else
    {
      // If the process is not running anymore (terminated for any reason), just stop the test
      if (kill(applicationPid_, 0) != 0)
      {
        processSync_->WaitForProcess();
        return;
      }
      std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }
}

/**
 * @brief Executes all the steps to run a GStreamer pipeline and to clean it up when it is done
 *
 * @param pipelineStr the GStreamer pipeline as a string
 */
void CGstreamerHandler::RunPipeline(const std::string& pipelineStr)
{
  loop_ = PipelineInitialization(pipelineStr);

  // start playing
  CLogger::Log(CLogger::Types::INFO, "Starting the pipeline for gstreamer");
  gst_element_set_state(gstPipeline_, GST_STATE_PLAYING);

  g_main_loop_run(loop_);
  ChildWaitProcess();
  // De-initialize
  FreeMemory();
  g_source_remove(busWatchId_);
  g_main_loop_unref(loop_);

  gst_deinit();
  running_ = false;
}

/**
 * @brief Prepares the GStreamer loop
 *
 * @param pipelineStr the GStreamer pipeline as a string
 * @return GMainLoop* a GStreamer object for the pipeline
 */
GMainLoop* CGstreamerHandler::PipelineInitialization(const std::string& pipelineStr)
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

  GMainLoop* loop = g_main_loop_new(NULL, FALSE);

  logUserData_.loop = loop;
  logUserData_.parentClass = this;
  tracerUserData_.parent = &traceHandler_;

  gst_debug_add_log_function(&GStreamer::TraceHandler::TraceCallbackFunction, &tracerUserData_, nullptr);

  gstPipeline_ = gst_parse_launch(pipelineStr.c_str(), &gstErrorMsg_);
  gstBus_ = gst_pipeline_get_bus(GST_PIPELINE(gstPipeline_));
  busWatchId_ = gst_bus_add_watch(gstBus_, bus_call, &logUserData_);
  gst_object_unref(gstBus_);

  CLogger::Log(CLogger::Types::INFO, "Starting synchronize 2 for gstreamer");
  ChildWaitProcess();
  std::this_thread::sleep_for(std::chrono::milliseconds(userProcessInfo_.startDelay));
  return loop;
}

/**
 * @brief Enables the necessary tracers for the system
 */
void CGstreamerHandler::SetTracingEnvironmentVars()
{
  setenv("GST_DEBUG", "GST_TRACER:7", true);
  // setenv("GST_TRACERS", "rusage;latency;framerate;proctime", true);
  std::string tracingVars = "framerate";
  if (settings_.enableProcTime)
  {
    tracingVars += ";proctime";
  }

  setenv("GST_TRACERS", tracingVars.c_str(), true);
}