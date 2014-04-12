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
 *  \brief Self contained illustrative example of a step counter using
 * simulated sensor data. 
 *
 * As you will see in the code within main(), the procedural flow of OSP
 * is rather simple.  The details of configuring the SensorDescriptor_t
 * where the complexity lies. Input sensor descriptors are packaged along
 * with the simulated data. Output sensor descriptors are in this file.
 *
 * While primarily a simplified example to demonstrate the basics of using
 * OSP, it can be used as rough benchmark as well.  The default 
 * ExamplePlatformImplementation is for a standard GCC toolchain against
 * simulated (canned) data. Reimplement ExamplePlatformInterface for
 * your device and compile with switch EXCLUDE_CANNED_DATA to get a 
 * better idea of resource  requirements for your platform.
 *
 * The code activates all resuls at startup. A good exercise for the reader
 * would be to modify this so the step counter is activated when there is a
 * significant motion event, outputs the step count as it updates,
 * then deactivates step counting when significant stillness is detected.
 *
 ****************************************************************************/
#include <stddef.h>

#include "osp-api.h"
#include "example_platform_interface.h"

//// Constants
#define NTP_OUTPUT_RATE_50HZ    TOFIX_EXTENDED(50.0f)

//// Function Prototypes
static void waitForNewDataToFeedOSP(void);
static int32_t stepCounterOutputCallback(OutputSensorHandle_t outputHandle,
                                          Android_StepCounterOutputData_t* pOutput);

//// Static data
static InputSensorHandle_t sXlHandle;
static InputSensorHandle_t sGyroHandle;
static InputSensorHandle_t sMagHandle;

static  OutputSensorHandle_t stepCounterHandle;

SensorDescriptor_t  stepCounterRequest= {SENSOR_STEP_COUNTER,
                                                DATA_CONVENTION_ANDROID,
                                                (OSP_OutputReadyCallback_t)stepCounterOutputCallback,
                                                 OSP_NO_NVM_WRITE_CALLBACK,
                                                 OSP_NO_SENSOR_CONTROL_CALLBACK,
                                                 OSP_NO_FLAGS,
                                                 OSP_NO_OPTIONAL_DATA};


//// Global data from Platform file
extern uint8_t gShutdown;
extern TriAxisSensorRawData_t gCurrentAccelData;
extern TriAxisSensorRawData_t gCurrentMagData;
extern TriAxisSensorRawData_t gCurrentGyroData;

//// Main
int main(int32_t argc, OSP_char_t** argv) {
  int status;
  const OSP_Library_Version_t* version;


  OSP_GetVersion(&version);
  PRINTF("OSP Example with OSP version: %s\n", version->VersionString);

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

  // request step counter output
  Platform_HandleErrorIf((OSP_SubscribeOutput(&stepCounterRequest, &stepCounterHandle) != OSP_STATUS_OK),
                         "OSP doesn't know how to provide SENSOR_STEP_COUNTER");

  // steady state behavior will have OSP computing based on input sensor data you feed it
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
  OSP_UnsubscribeOutput(stepCounterHandle);
}

static int32_t stepCounterOutputCallback(OutputSensorHandle_t OutputHandle,
                                          Android_StepCounterOutputData_t* pOutput) {

    PRINTF("{STC, %+03.2f, %d,0,}\r\n", TOFLT_TIME(pOutput->TimeStamp),
                                        pOutput->StepCount);

    return 0;
}


static void waitForNewDataToFeedOSP(void) {
  const uint32_t XL_DATA_MASK = 1<<SENSOR_ACCELEROMETER;
  const uint32_t MAG_DATA_MASK = 1<<SENSOR_MAGNETIC_FIELD;
  const uint32_t GYRO_DATA_MASK = 1<<SENSOR_GYROSCOPE;
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


