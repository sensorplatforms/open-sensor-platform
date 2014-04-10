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
/*! \file
 * Self contained illustrative example of a step counter using
 * simulated sensor data. 
 *
 * As you will see in the code within main(), the procedural flow of OSP
 * is rather simple.  The details of configuring the SensorDescriptor_t and 
 * ResultRequestDescriptor_t are where the complexity lies, and they are 
 * implemented in the Platform module.
 *
 * While primarily a simplified example to demonstrate the basics of using
 * OSP, it can be used as rough benchmark as well.  The default 
 * ExamplePlatformImplementation is for a standard GCC toolchain against
 * simulated (canned) data. Reimplement ExamplePlatform for
 * your device and compile with switch EXCLUDE_CANNED_DATA to get a 
 * better idea of resource  requirements for your platform.
 *
 * The code activates all resuls at startup. A good exercise for the reader
 * would be to modify this so the step counter is activated when there is a
 * significant amount of movement, outputs the step count as it updates,
 * then deactivates step counting when the device is sufficiently still again.
 *
 ****************************************************************************/
#include <stddef.h>

#include "osp_api.h"
#include "example_platform_interface.h"

//// Constants
#define NTP_OUTPUT_RATE_50HZ_IN_SECONDS    TOFIX_PRECISE(0.2f)

//// Function Prototypes
static void waitForNewDataToFeedOSP(void);
static int32_t significantMotionResultCallback(FusionResultHandle_t resultHandle, 
                                          Android_SignificantMotionResultData_t* pResult);
static int32_t significantStillnessResultCallback(FusionResultHandle_t resultHandle, 
                                          Android_SignificantMotionResultData_t* pResult);
static int32_t stepCounterResultCallback(FusionResultHandle_t resultHandle,
                                          Android_StepCounterResultData_t* pResult);
static int32_t sensorResultCallback(FusionResultHandle_t resultHandle,
                                                     Android_CalibratedAccelResultData_t* pResult);

//// Static data
static SensorHandle_t sXlHandle;
static SensorHandle_t sGyroHandle;
static SensorHandle_t sMagHandle;

static  FusionResultHandle_t significantMotionHandle;
static  FusionResultHandle_t significantStillnessHandle;
static  FusionResultHandle_t stepDetectorHandle;
static  FusionResultHandle_t stepCounterHandle;

  ResultRequestDescriptor_t  significantMotionRequest= {RESULT_SIGNIFICANT_MOTION, 
                                                   (OSP_ResultReadyCallback_t)significantMotionResultCallback,
                                                   RESULT_FORMAT_ANDROID,
                                                   OSP_NOTIFY_DEFAULT,    // use OSP_OVERRIDE_ONESHOT if you want this to act like most other results
                                                   TOFIX_PRECISE(0.70f), // threshold for motion
                                                   CONST_PRECISE(5.0f),  // number of events needed to signify motion
                                                   OSP_NO_FLAGS, OSP_NO_OPTIONAL_DATA};

  ResultRequestDescriptor_t  significantStillnessRequest= {RESULT_SIGNIFICANT_STILLNESS, 
                                                   (OSP_ResultReadyCallback_t)significantStillnessResultCallback,
                                                   RESULT_FORMAT_ANDROID,
                                                   OSP_OVERRIDE_ONESHOT,    // use OSP_OVERRIDE_ONESHOT if you want this to act like most other results
                                                   CONST_PRECISE(.7f),   // threshold for stillness 
                                                   CONST_PRECISE(25.0f), // number of events needed to enter stillness
                                                   OSP_NO_FLAGS, OSP_NO_OPTIONAL_DATA};

  ResultRequestDescriptor_t  stepCounterRequest= {RESULT_STEP_COUNTER,
                                                   (OSP_ResultReadyCallback_t)stepCounterResultCallback,
                                                   RESULT_FORMAT_ANDROID,
                                                   OSP_NOTIFY_DEFAULT,
                                                   5, OSP_NOTIFY_DEFAULT,
                                                   OSP_NO_FLAGS, OSP_NO_OPTIONAL_DATA};


//// Global data from Platform file
extern uint8_t gShutdown;
extern TriAxisSensorRawData_t gCurrentAccelData;
extern TriAxisSensorRawData_t gCurrentMagData;
extern TriAxisSensorRawData_t gCurrentGyroData;

//// Main
int main(int32_t argc, OSP_char_t** argv) {
  int status;
  const OSP_Version_t* version;


  OSP_GetVersion(&version);
  PRINTF("OSP Example with OSP version: %s\n", 
         version->VersionString);

  // initialize your hardware 
  Platform_Initialize();
  
  // initalize with system wide settings such as output formats, Critical Section protection functions, etc   
  const SystemDescriptor_t* pSystemDesc= Platform_GetSystemDescriptor(); 
  OSP_Initialize(pSystemDesc);

  // tell OSP what kind of sensor data you'll be feeding it
  status= OSP_RegisterSensor(Platform_GetSensorDescriptorByName("sim-xl"), &sXlHandle);
  status|= OSP_RegisterSensor(Platform_GetSensorDescriptorByName("sim-mag"), &sMagHandle);
  status|= OSP_RegisterSensor(Platform_GetSensorDescriptorByName("sim-gyro"), &sGyroHandle);
  Platform_HandleErrorIf( status != OSP_STATUS_OK, "could not initialize required sensors");

  // request rotationVector output at 50Hz                                            
  Platform_HandleErrorIf((OSP_SubscribeResult(&significantMotionRequest, &significantMotionHandle) != OSP_STATUS_OK),
                         "OSP doesn't know how to provide RESULT_SIGNIFICANT_MOTION!");
  Platform_HandleErrorIf((OSP_SubscribeResult(&significantStillnessRequest, &significantStillnessHandle) != OSP_STATUS_OK),
                         "OSP doesn't know how to provide RESULT_SIGNIFICANT_STILLNESS!");
  Platform_HandleErrorIf((OSP_SubscribeResult(&stepCounterRequest, &stepCounterHandle) != OSP_STATUS_OK),
                         "OSP doesn't know how to provide RESULT_STEP_DETECTOR");

  // steady state behavior will have OSP computing orientation based on inputs you feed it
  while(!gShutdown) {
 
    // new data is usually fed straight from driver ISRs in an actual sensor hub 
    waitForNewDataToFeedOSP();

    // foreground and background processing are usually kicked off from timers in an actual sensor hub 
    while (OSP_DoForegroundProcessing() != OSP_STATUS_IDLE)
      ; //keep doing foreground computation until its finished
    while(OSP_DoBackgroundProcessing() != OSP_STATUS_IDLE)
      ; //similarly with background compute. Note that it's safe to call background processing more often than needed    
  }

  //Unsubscribe is important to see the last remaining data in the step counter
  OSP_UnsubscribeResult(stepCounterHandle);
}

static int32_t stepCounterResultCallback(FusionResultHandle_t resultHandle,
                                          Android_StepCounterResultData_t* pResult) {

    PRINTF("{STC, %+03.2f, %d,0,}\r\n", TOFLT_TIME(pResult->TimeStamp),
                                        pResult->StepCount);

    return 0;
}


static int32_t significantMotionResultCallback(FusionResultHandle_t resultHandle, Android_SignificantMotionResultData_t* pResult) {

  PRINTF("{SM, %+03.2f, 1.0,0,}\r\n", TOFLT_TIME(pResult->TimeStamp));

  return 0;
}

static int32_t significantStillnessResultCallback(FusionResultHandle_t resultHandle, Android_SignificantMotionResultData_t* pResult) {

  PRINTF("{SS, %+03.2f, 1.0,0,}\r\n", TOFLT_TIME(pResult->TimeStamp));

  return 0;
}

static int32_t sensorResultCallback(FusionResultHandle_t resultHandle, Android_CalibratedAccelResultData_t* pResult) {

  return 0;
}

static void waitForNewDataToFeedOSP(void) {
  const uint32_t XL_DATA_MASK = 1<<OSP_SENSOR_TYPE_ACCELEROMETER;
  const uint32_t MAG_DATA_MASK = 1<<OSP_SENSOR_TYPE_MAGNETIC_FIELD;
  const uint32_t GYRO_DATA_MASK = 1<<OSP_SENSOR_TYPE_GYROSCOPE;
  int status = OSP_STATUS_OK;

  int32_t dataAvailable = Platform_BlockOnNewSensorReadings();

  if(dataAvailable & XL_DATA_MASK) {
    status|= OSP_SetData(sXlHandle, &gCurrentAccelData);
  }
  if(dataAvailable & MAG_DATA_MASK) {
    status|= OSP_SetData(sMagHandle, &gCurrentMagData);
  }
  if(dataAvailable & GYRO_DATA_MASK) {
    status|= OSP_SetData(sGyroHandle, &gCurrentGyroData);
  }

  Platform_HandleErrorIf( status != OSP_STATUS_OK, "could not set sensor data\n");

}


