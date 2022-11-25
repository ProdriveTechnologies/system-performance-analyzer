cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME videostream)

#include(ExernalProject)

message("Starting compilation of project: " ${PROJECT_NAME})
# set the project name
project(${PROJECT_NAME} LANGUAGES CXX)

# Add header files to the compilation
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Set the sources
set(SOURCES # ${CMAKE_CURRENT_SOURCE_DIR}/src/Benchmarking/Monitoring.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/linux/shared_memory.cpp
        )

# add the executable
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

# Add compile options
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4100 /wd4200 /wd4250 /wd4355 /wd4505)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wunused-parameter) 
  find_library(LIBRT rt)                                                                                                                                                                                                                                                                                        
  if(LIBRT)
    target_link_libraries(${PROJECT_NAME} ${LIBRT})
  endif()
endif()
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)


#message("include directories")
#target_include_directories(${projectname})

# Link libraries 
# target_link_libraries(${projectname} ) # No libraries to link (yet)