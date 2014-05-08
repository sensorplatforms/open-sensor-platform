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
#if !defined (_OSPEMBEDDEDALGCALLS_H_)
#define   _OSPEMBEDDEDALGCALLS_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-alg-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is the embedded API for OSP algorithms.
 * Note that all data passed in/out is in fixed point formats.
 *
 * On start-up, call:
 * -) OSP_InitializeAlgorithms();
 *
 * When registering for a result, call:
 * -) OSP_RegisterXXCallback(); (for all XX results desired)
 * -) OSP_ResetAlgorithms();
 *
 * On new accel data, call:
 * -) OSP_SetAccelerometerMeasurement(time, acc);
 *
 * On shut-down, call:
 * -) OSP_DestroyAlgorithms();
 *
 */

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
//! Initialize call for algorithm code
void OSP_InitializeAlgorithms(void);

//! Call to reset algorithm internal state
void OSP_ResetAlgorithms(void);

//! Tears down algorithm setup
void OSP_DestroyAlgorithms(void);

//! Sends sensor data into the underlying algorithms for processing
/*!
*  Passes raw accelerometer sensor data into the algorithm code for processing.
*  When a result is triggered, the appropriate callback will be called.
*
*  \param timeInSeconds IN timestamp of the corresponding sensor measurement.
*         Expected data format NTTIME is fixed point format 64 bit, Q24.
*  \param measurementInMetersPerSecondSquare IN raw 3-axis accelerometer sensor
*         measurement in units of (meters per second)^2.
*         Expected data format NTPRECISE is fixed point format 32 bit, Q24.
*
*/
void OSP_SetAccelerometerMeasurement(NTTIME timeInSeconds, NTPRECISE measurementInMetersPerSecondSquare[3]);

//! Registers a callback for step detection results
/*!
* Sets the callback for step detection results, where a step detection result
* comes in the form of a StepDataOSP_t object (defined in OSP_Types.h)
*
*  \param fpCallback IN function pointer for step result callback.
*/
void OSP_RegisterStepCallback(OSP_StepResultCallback_t fpCallback);


//! Registers a callback for step segment results
/*!
* Sets the callback for step segment results, where a step segment result
* comes in the form of a StepSegment_t object (defined in OSP_Types.h)
*
*  \param fpCallback IN function pointer for step segment callback.
*/
void OSP_RegisterStepSegmentCallback(OSP_StepSegmentResultCallback_t fpCallback);

#ifdef __cplusplus
}
#endif

#endif //_OSPEMBEDDEDALGCALLS_H_
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
