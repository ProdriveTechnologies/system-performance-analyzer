cmake_minimum_required(VERSION 3.7.2)

set(PROJECT_NAME spa)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

# include(ExernalProject)
message("Starting compilation of project: " ${PROJECT_NAME})

# options
option(BUILD_TESTS "Build the tests" OFF)

# ### Getting Git hash for the application
# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest commit hash
execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# #### set the project name
project(${PROJECT_NAME} LANGUAGES CXX)

include(FetchContent) # To be able to download Git repos with the FetchContent_Declare function
include(ExternalProject) # To be able to download Git repos with the FetchContent_Declare function

# Add header files to the compilation
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the sources
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/linux/perf_measurements.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pipe_comm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/json_config/config_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/helper_functions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gstreamer/handler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gstreamer/trace_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/linux/xavier_sensors.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/synchronizer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/filesystem.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/process_runner/run_process_base.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/run_process.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/file_writer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/export_types/export_csv.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/export_types/export_graphs.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/json_config/sensor_config/config_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/linux/proc_handler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/summary_writer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/export_types/summary_generator.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/export_types/terminal_ui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/linux/performance_helpers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/json_helpers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/pipeline_measurements.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/system_measurements.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/exports/export_types/export_json.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/process_measurements.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/input_handler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/data_handler.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/linux/struct_sensors.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/filesystem/statm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/analysis/correlation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/spa.cpp
)
message(" - Enabling static code analyzer: clang-tidy")

# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.;-checks=llvm-,clang-analyzer-,modernize-")

# add the executable
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
  "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")

# Add compile options
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4200 /wd4250 /wd4355 /wd4505)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wunused-parameter)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

message(" - Downloading external projects")

# ################# Linux System Monitoring Library ##################
# message(" -- Downloading Linux System Monitoring Library")
# FetchContent_Declare(
# SystemMonitoring
# PREFIX ${EXTERNAL_INSTALL_LOCATION}
# BINARY_DIR ${EXTERNAL_INSTALL_LOCATION}
# INSTALL_DIR ${EXTERNAL_INSTALL_LOCATION}
# GIT_REPOSITORY "https://github.com/fuxey/Linux-System-Monitoring-Library"
# GIT_TAG "master"
# GIT_SUBMODULES "example"
# )
# FetchContent_MakeAvailable(SystemMonitoring)
# find_library(SYSMON_LINUX liblinuxmonitoring REQUIRED)
# message(${systemmonitoring_SOURCE_DIR})
# message(${SYSMON_LINUX_INCLUDE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE ${systemmonitoring_SOURCE_DIR} ${SYSMON_LINUX_INCLUDE_DIR})
# target_link_libraries(${PROJECT_NAME} ${SYSMON_LINUX_LIBRARIES}) # No libraries to link (yet)

# message(" -- Linux System Monitoring Library downloaded")

# ################# JSON Library ##################
message(" -- Downloading JSON library")
include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5)

FetchContent_GetProperties(json)

if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# ##################### Test ##############################
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

# ################# Hardware info Library ##################
# message(" -- Downloading hardware info library")
# include(FetchContent)

# FetchContent_Declare(infoware
# GIT_REPOSITORY https://github.com/ThePhD/infoware.git
# GIT_TAG v0.6.0)

# FetchContent_GetProperties(infoware)

# if(NOT infoware_POPULATED)
# FetchContent_Populate(infoware)
# add_subdirectory(${infoware_SOURCE_DIR} ${infoware_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

message("Going to download FTXUI")
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui

  # Important: Specify a GIT_TAG XXXXX here.
)

FetchContent_GetProperties(ftxui)

if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

message("Downloaded FTXUI")

# ################### GStreamer ########################
find_package(PkgConfig) # finding pkg-config is a helper tool

# using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER QUIET gstreamer-1.0)

if(NOT GSTREAMER_FOUND)
  message(FATAL_ERROR "Package missing! Please install: libstreamer1.0-dev")
endif()

# including GStreamer header files directory
include_directories(
  ${GLIB_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
)

# linking GStreamer library directory
link_directories(
  ${GLIB_LIBRARY_DIRS}
  ${GSTREAMER_LIBRARY_DIRS}
)

# Threading library
find_package(Threads)

find_library(LIBRT rt) # Note: LIBRT is not a Windows library. Therefore, for Windows this should be ignored

if(LIBRT)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRT} nlohmann_json::nlohmann_json ${GSTREAMER_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
message(" -- Libraries downloaded")

# Link libraries