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
set(SOURCES # ${CMAKE_CURRENT_SOURCE_DIR}/src/Benchmarking/Monitoring.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/Benchmarking/Linux/Monitoring.cpp
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

message("Downloading external projects")

message("Downloading Linux System Monitoring Library")
FetchContent_Declare(
        SystemMonitoring
        PREFIX ${EXTERNAL_INSTALL_LOCATION}
        BINARY_DIR ${EXTERNAL_INSTALL_LOCATION}
        INSTALL_DIR ${EXTERNAL_INSTALL_LOCATION}
        GIT_REPOSITORY "https://github.com/fuxey/Linux-System-Monitoring-Library"
        GIT_TAG "master"
        GIT_SUBMODULES "example"
)
FetchContent_MakeAvailable(SystemMonitoring)
message(${systemmonitoring_SOURCE_DIR} " or " ${systemmonitoring_BINARY_DIR} " or " ${systemmonitoring_POPULATED})

find_library(SYSMON_LINUX liblinuxmonitoring REQUIRED)
message(${SYSMON_LINUX} "  orr   " ${SYSMON_LINUX_INCLUDE_DIR})
message("include directories")
target_include_directories(${projectname} ${systemmonitoring_SOURCE_DIR} ${SYSMON_LINUX_INCLUDE_DIR})

# Link libraries 
target_link_libraries(${projectname} ${SYSMON_LINUX_LIBRARIES}) # No libraries to link (yet)