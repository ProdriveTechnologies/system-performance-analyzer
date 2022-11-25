cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME benchmarks)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

#include(ExernalProject)

message("Starting compilation of project: " ${PROJECT_NAME})
# set the project name
project(${PROJECT_NAME} LANGUAGES CXX)
include(FetchContent) # To be able to download Git repos with the FetchContent_Declare function
include(ExternalProject) # To be able to download Git repos with the FetchContent_Declare function

# Add header files to the compilation
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the sources
set(SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/Linux/Monitoring.cpp
    #        ${CMAKE_CURRENT_SOURCE_DIR}/src/Benchmarking/Linux/Monitoring.cpp
    #        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/pipe_comm.cpp
    #        ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/shared_memory.cpp
    #        ${CMAKE_CURRENT_SOURCE_DIR}/src/modules/base_class.cpp
    #        ${CMAKE_CURRENT_SOURCE_DIR}/modules/filestream/filestream.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/json_config/config_parser.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/helper_functions.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gstreamer/handler.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmarks/Linux/xavier_sensors.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/helpers/synchronizer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/linux/filesystem.cpp
        )

# add the executable
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

# Add compile options
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4200 /wd4250 /wd4355 /wd4505 )
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wunused-parameter )                                                                                                                                                                                                                                                                                 
endif()
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

message(" - Downloading external projects")

################## Linux System Monitoring Library ##################
# message(" -- Downloading Linux System Monitoring Library")
# FetchContent_Declare(
#         SystemMonitoring
#         PREFIX ${EXTERNAL_INSTALL_LOCATION}
#         BINARY_DIR ${EXTERNAL_INSTALL_LOCATION}
#         INSTALL_DIR ${EXTERNAL_INSTALL_LOCATION}
#         GIT_REPOSITORY "https://github.com/fuxey/Linux-System-Monitoring-Library"
#         GIT_TAG "master"
#         GIT_SUBMODULES "example"
# )
# FetchContent_MakeAvailable(SystemMonitoring)
# find_library(SYSMON_LINUX liblinuxmonitoring REQUIRED)
# message(${systemmonitoring_SOURCE_DIR})
# message(${SYSMON_LINUX_INCLUDE_DIR})
# target_include_directories(${PROJECT_NAME} PRIVATE ${systemmonitoring_SOURCE_DIR} ${SYSMON_LINUX_INCLUDE_DIR})
# target_link_libraries(${PROJECT_NAME} ${SYSMON_LINUX_LIBRARIES}) # No libraries to link (yet)

# message(" -- Linux System Monitoring Library downloaded")

################## JSON Library ##################
message(" -- Downloading JSON library")
include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


#################### GStreamer ########################
find_package(PkgConfig) #finding pkg-config is a helper tool

#using pkg-config to getting Gstreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

#including GStreamer header files directory
include_directories(
        ${GLIB_INCLUDE_DIRS}
        ${GSTREAMER_INCLUDE_DIRS}
)

#linking GStreamer library directory
link_directories(
        ${GLIB_LIBRARY_DIRS}
        ${GSTREAMER_LIBRARY_DIRS}
)

# Threading library
find_package(Threads)

find_library(LIBRT rt)     # Note: LIBRT is not a Windows library. Therefore, for Windows this should be ignored  
if(LIBRT)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRT} nlohmann_json::nlohmann_json ${GSTREAMER_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
message(" -- JSON library downloaded")


# Link libraries 