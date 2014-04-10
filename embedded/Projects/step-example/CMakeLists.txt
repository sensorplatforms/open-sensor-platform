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
################################################################################
cmake_minimum_required (VERSION 2.6)

project(step-example)

#
# Include Paths
##
set (PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
include_directories(${PUBLIC_INCLUDE_DIRS})

#
# Linker Paths
##
link_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../lib )

# 
#  Compile Time Defines
##
#add_definitions( -D BOARD_XXX)

#
# Step Counter Example
##
set(StepExample_SOURCES
  main.c
  simulated_platform.c)
add_executable(step-example ${StepExample_SOURCES})
target_link_libraries(step-example osp)
