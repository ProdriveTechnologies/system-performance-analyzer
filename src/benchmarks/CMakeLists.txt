# Header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files and library creation
set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Linux/Monitoring.cpp)

# Find mosquittopp
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Mosquitto REQUIRED)

# Create library
add_library(MQTT ${SOURCES})

# Link library
target_link_libraries(MQTT PRIVATE ${MOSQUITTO_LIBRARIES})

# Add include directories
target_include_directories(MQTT INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(MQTT INTERFACE ${MOSQUITTO_INCLUDE_DIR})
