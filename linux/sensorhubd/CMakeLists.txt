#
# Open Sensor Platform Project
# https://github.com/sensorplatforms/open-sensor-platform
#
# Copyright (C) 2013 Sensor Platforms Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
#
# To Build
#  - cmake <path to this project file>
#  - make
#
###############################################################################
cmake_minimum_required (VERSION 2.6)

project(sensorhubd)



#
# Include paths
##
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

#
# Linker include paths
##
#link_directories()

#
#
##
IF(CMAKE_BUILD_TYPE MATCHES Release)
message(STATUS "building in Release mode")
add_definitions(-DNDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -fPIC -Wall -Werror")

#
# App
##
set(app_SOURCES
  main.cpp
  virtualsensordevicemanager.h
  virtualsensordevicemanager.cpp
  osp_remoteprocedurecalls.h
  osp_remoteprocedurecalls_relay.cpp
  osp_configuration.cpp
  uinpututils.c
)

add_executable(sensorhubd ${app_SOURCES})

if(${CMAKE_CXX_COMPILER} MATCHES "arm")
  target_link_libraries(sensorhubd log android)
else()
  target_link_libraries(sensorhubd pthread)
endif()
