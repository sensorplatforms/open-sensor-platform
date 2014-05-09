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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp_embeddedalgcalls.h"
#include "stepdetector.h"
#include "osp-alg-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static OSP_StepResultCallback_t _fpStepResultCallback = NULL;
static OSP_StepSegmentResultCallback_t _fpStepSegmentResultCallback = NULL;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      OnStepUpdate
 *          Call back for step update
 *
 ***************************************************************************************************/
static void OnStepUpdate(StepDataOSP_t * step){
    if(_fpStepResultCallback){
        _fpStepResultCallback(step);
    }
}


/****************************************************************************************************
 * @fn      OnStepSegmentUpdate
 *          Call back from algorithm for step segment update
 *
 ***************************************************************************************************/
static void OnStepSegmentUpdate(StepSegment_t * segment){
    if(_fpStepSegmentResultCallback){
        _fpStepSegmentResultCallback(segment);
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   A P I   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      OSP_InitializeAlgorithms
 *          Call to initialize the algorithms implementation.
 *
 ***************************************************************************************************/
void OSP_InitializeAlgorithms(void){
    StepDetector_Init(OnStepUpdate, OnStepSegmentUpdate);
}


/****************************************************************************************************
 * @fn      OSP_ResetAlgorithms
 *          Call this to reset the algorithms to initial startup state
 *
 ***************************************************************************************************/
void OSP_ResetAlgorithms(void){
    StepDetector_Reset();
}


/****************************************************************************************************
 * @fn      OSP_DestroyAlgorithms
 *          Call this function before exit to shutdown the algorithms properly
 *
 ***************************************************************************************************/
void OSP_DestroyAlgorithms(void){
    StepDetector_CleanUp();
}


/****************************************************************************************************
 * @fn      OSP_SetAccelerometerMeasurement
 *          API to feed accelerometer data into the algorithms
 *
 ***************************************************************************************************/
void OSP_SetAccelerometerMeasurement(NTTIME timeInSeconds, NTPRECISE measurementInMetersPerSecondSquare[3]){
    //update step detector
    StepDetector_SetAccelerometerMeasurement(measurementInMetersPerSecondSquare, timeInSeconds);
}


/****************************************************************************************************
 * @fn      OSP_RegisterStepSegmentCallback
 *          Register step segment call back with the algorithms
 *
 ***************************************************************************************************/
void OSP_RegisterStepSegmentCallback(OSP_StepSegmentResultCallback_t fpCallback){
    _fpStepSegmentResultCallback = fpCallback;
}


/****************************************************************************************************
 * @fn      OSP_RegisterStepCallback
 *          Register step result call back with the algorithms
 *
 ***************************************************************************************************/
void OSP_RegisterStepCallback(OSP_StepResultCallback_t fpCallback){
    _fpStepResultCallback = fpCallback;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
