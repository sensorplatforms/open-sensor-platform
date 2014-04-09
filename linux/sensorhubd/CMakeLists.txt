#
#
#
###############################################################################
cmake_minimum_required (VERSION 2.6)

project(sensorhubd)



#
# Include paths
##
#include_directories()

#
# Linker include pathso
##
#link_directories()

#
#
##
IF(CMAKE_BUILD_TYPE MATCHES Release)
message(STATUS "building in Release mode")
add_definitions(-DNDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=gnu++0x -fPIC")


#
# App
##
set(app_SOURCES
  main.cpp
  VirtualSensorDeviceManager.h
  VirtualSensorDeviceManager.cpp
  FreeMotion_RemoteProcedureCalls.h
  FreeMotion_RemoteProcedureCalls_Relay.cpp
  FmConfiguration.cpp
  UinputUtils.c
)

add_executable(sensorhubd ${app_SOURCES})

if(${CMAKE_CXX_COMPILER} MATCHES "arm")
  target_link_libraries(sensorhubd log android)
else()
  target_link_libraries(sensorhubd pthread)
endif()
