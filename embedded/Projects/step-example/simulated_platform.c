/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2013 Sensor Platforms Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*! \file                                                                    *
 *                                                                           *
 *  \brief provides data from a compiled in dataset as if it came from sensors
 *
 * use this template for
 *   - Benchmarking
 *   - First step in platform integration
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "osp-api.h"
#include "example_platform_interface.h"

#include "steps_accel.dat"
#include "steps_mag.dat"
#include "steps_gyro.dat"

//// Globals
char gShutdown= 0;

uint32_t gCurrentTimestamp =0;
TriAxisSensorRawData_t gCurrentAccelData = {0};
TriAxisSensorRawData_t gCurrentMagData = {0};
TriAxisSensorRawData_t gCurrentGyroData = {0};


//// Private

static void emptyFunction()
{

}

static const SystemDescriptor_t _systemDescriptor=
{
    TOFIX_TIMECOEFFICIENT(0.000001f),        // timestamp conversion factor = 1us / count
    (OSP_CriticalSectionCallback_t) emptyFunction,
    (OSP_CriticalSectionCallback_t) emptyFunction,
};

//// Implementations

// nothing special needed for this implementation
void Platform_Initialize(void) {

}

// this simulation can just quit on errors
void Platform_HandleErrorIf(char isError, const char* msg) {

  if(isError) {
    PRINTF("**ERROR: %s\r\n", msg);
    exit(-1);
  }
}

// returns the sensor descriptors that go with the simulated data
SensorDescriptor_t* Platform_GetSensorDescriptorByName(const char* sensorType) {
  if (strcmp(sensorType, "sim-xl")          == 0){
    return &SIMULATED_ACCEL_DESCRIPTOR;
  } 
  else if (strcmp(sensorType, "sim-mag")    == 0) {
    return &SIMULATED_MAG_DESCRIPTOR;

  } 
  else if (strcmp(sensorType, "sim-gyro")   == 0) {
    return &SIMULATED_GYRO_DESCRIPTOR;
    
  } else {
    return NULL;
  }    
}

// returns the system descriptor configured for this example
const SystemDescriptor_t* Platform_GetSystemDescriptor() {
 
   return &_systemDescriptor;

}

// make the current sample available in a global location, setting the quit flag at the end of them
uint32_t Platform_BlockOnNewSensorReadings() {
  uint32_t result =0;
  static uint32_t callCount =0;
  static uint32_t accelSampleIndex =0;
  static uint32_t magSampleIndex =0;
  static uint32_t gyroSampleIndex =0;

  #define TEN_MILLISECS_IN_MICROSECS 10000

  if ((callCount % SIMULATED_ACCEL_DECIMATION_VS_100Hz) == 0) {
    gCurrentAccelData.Data[0]= SIMULATED_ACCEL_DATA[accelSampleIndex][0];
    gCurrentAccelData.Data[1]= SIMULATED_ACCEL_DATA[accelSampleIndex][1];
    gCurrentAccelData.Data[2]= SIMULATED_ACCEL_DATA[accelSampleIndex][2];
    gCurrentAccelData.TimeStamp= gCurrentTimestamp;
    result|= 1<<OSP_SENSOR_TYPE_ACCELEROMETER;

    accelSampleIndex= ++accelSampleIndex % SIMULATED_ACCEL_NUM_SAMPLES;
  }

  if ((callCount % SIMULATED_MAG_DECIMATION_VS_100Hz) == 0) {
    gCurrentMagData.Data[0]= SIMULATED_MAG_DATA[magSampleIndex][0];
    gCurrentMagData.Data[1]= SIMULATED_MAG_DATA[magSampleIndex][1];
    gCurrentMagData.Data[2]= SIMULATED_MAG_DATA[magSampleIndex][2];
    gCurrentMagData.TimeStamp= gCurrentTimestamp;
    result|= 1<<OSP_SENSOR_TYPE_MAGNETIC_FIELD;
    
    magSampleIndex= ++magSampleIndex % SIMULATED_MAG_NUM_SAMPLES;                                       
  }

  if ((callCount % SIMULATED_GYRO_DECIMATION_VS_100Hz) == 0) {
    gCurrentGyroData.Data[0]= SIMULATED_GYRO_DATA[gyroSampleIndex][0];
    gCurrentGyroData.Data[1]= SIMULATED_GYRO_DATA[gyroSampleIndex][1];
    gCurrentGyroData.Data[2]= SIMULATED_GYRO_DATA[gyroSampleIndex][2];   
    gCurrentGyroData.TimeStamp= gCurrentTimestamp;
    result|= 1<<OSP_SENSOR_TYPE_GYROSCOPE;

    ++gyroSampleIndex;
    if (gyroSampleIndex == SIMULATED_GYRO_NUM_SAMPLES) {
      gShutdown= TRUE; 
    }
    gyroSampleIndex= gyroSampleIndex % SIMULATED_GYRO_NUM_SAMPLES;    
  }

  // Kick the other indexes along
  gCurrentTimestamp+= TEN_MILLISECS_IN_MICROSECS;
  callCount++;

  return result;
}
