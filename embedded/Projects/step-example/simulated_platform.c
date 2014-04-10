/*****************************************************************************
 *                                                                           *
 *                       Sensor Platforms Inc.                               *
 *                   2860 Zanker Road, Suite 210                             *
 *                        San Jose, CA 95134                                 *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *               Copyright (c) 2012 Sensor Platforms Inc.                    *
 *                       All Rights Reserved                                 *
 *                                                                           *
 *                   Proprietary and Confidential                            *
 *             Use only under license described in EULA.txt                  *
 *                                                                           *
 ****************************************************************************/
/*! \file                                                                    *
 *                                                                           *
 *  @author Sensor Platforms Inc: http://www.sensorplatforms.com             *
 *  @author        Support Email: support@sensorplatforms.com                *
 *                                                                           *
 *  \ingroup embedded
 *  \brief reads data from a compiled in dataset as if it came from sensors
 *
 * use this template for
 *   - Benchmarking
 *   - First step in platform integration
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "FM_Types.h"
#include "ExamplePlatformImplementation.h"

#define STEP_DATA
#ifdef STEP_DATA
# include "stepsAccel.dat"
# include "stepsMag.dat"
# include "stepsGyro.dat"
#else
# include "stillAccel.dat"
# include "stillMag.dat"
# include "stillGyro.dat"
#endif

//// Globals
fm_char_t gShutdown= 0;

uint32_t gCurrentTimestamp =0;
TriAxisSensorRawData_t gCurrentAccelData = {0};
TriAxisSensorRawData_t gCurrentMagData = {0};
TriAxisSensorRawData_t gCurrentGyroData = {0};


void emptyFunction()
{

}

static const SystemDescriptor_t _systemDescriptor=
{
    TOFIX_TIMECOEFFICIENT(0.000001f),        // timestamp conversion factor = 1us / count
    (FM_CriticalSectionCallback_t) emptyFunction,
    (FM_CriticalSectionCallback_t) emptyFunction,
};

// nothing needed for this implementation
void Platform_Initialize(void) {

}

// this simulation can just quit on errors
void Platform_HandleErrorIf(fm_char_t isError, const fm_char_t* msg) {

  if(isError) {
    PRINTF("**ERROR: %s\r\n", msg);
    exit(-1);
  }
}

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
    result|= 1<<FM_SENSOR_TYPE_ACCELEROMETER;

    accelSampleIndex= ++accelSampleIndex % SIMULATED_ACCEL_NUM_SAMPLES;
  }

  if ((callCount % SIMULATED_MAG_DECIMATION_VS_100Hz) == 0) {
    gCurrentMagData.Data[0]= SIMULATED_MAG_DATA[magSampleIndex][0];
    gCurrentMagData.Data[1]= SIMULATED_MAG_DATA[magSampleIndex][1];
    gCurrentMagData.Data[2]= SIMULATED_MAG_DATA[magSampleIndex][2];
    gCurrentMagData.TimeStamp= gCurrentTimestamp;
    result|= 1<<FM_SENSOR_TYPE_MAGNETIC_FIELD;
    
    magSampleIndex= ++magSampleIndex % SIMULATED_MAG_NUM_SAMPLES;                                       
  }

  if ((callCount % SIMULATED_GYRO_DECIMATION_VS_100Hz) == 0) {
    gCurrentGyroData.Data[0]= SIMULATED_GYRO_DATA[gyroSampleIndex][0];
    gCurrentGyroData.Data[1]= SIMULATED_GYRO_DATA[gyroSampleIndex][1];
    gCurrentGyroData.Data[2]= SIMULATED_GYRO_DATA[gyroSampleIndex][2];   
    gCurrentGyroData.TimeStamp= gCurrentTimestamp;
    result|= 1<<FM_SENSOR_TYPE_GYROSCOPE;

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
