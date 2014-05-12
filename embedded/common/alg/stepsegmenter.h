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
#ifndef STEPSEGMENTER_H
#define STEPSEGMENTER_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-alg-types.h"

/*
 * This module is responsible for identifying a possible step segment associated
 * with walking by analyzing a stream of incoming filtered acceleration signals.
 *
 */

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define NUM_INITIAL_STORED_STEPS_2N     (2)
#define NUM_INITIAL_STORED_STEPS        (1 << NUM_INITIAL_STORED_STEPS_2N)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum EStepSegmenterState {
    startWalk = 0,
    midWalk,
    endWalk
} EStepSegmenterState;

typedef enum EExtremaType {
    positivePeak = 0,
    negativePeak,
    potentialPosPeak,
    potentialNegPeak
} EExtremaType;

//! Struct definition for step segmenter
typedef struct {
    StepSegment_t stepSegment;

    //constants related to step segmentation
    osp_float_t lastPosPeakMag;
    osp_float_t lastNegPeakMag;
    osp_float_t lastStepPeakMag;
    NTTIME lastPosPeakTime;
    NTTIME lastNegPeakTime;
    NTTIME expectedStridePeriod;

    //peak finding variables
    osp_float_t prevAccNorm;
    NTTIME prevTime;
    EExtremaType prevPeakType;

    //segmenter state
    EStepSegmenterState segmenterState;

    //flags
    uint8_t peakFlags;
    uint8_t stepFlags;

    //stored initial steps
    StepSegment_t storedSteps[NUM_INITIAL_STORED_STEPS];
    uint16_t numStoredSteps;

    //callback variables
    OSP_StepSegmentResultCallback_t resultReadyCallback;
    void * objPtr;

} StepSegmenter_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

// Constructor/destructor
void StepSegmenter_Init(StepSegmenter_t * pStruct, OSP_StepSegmentResultCallback_t pResultReadyCallback);
void StepSegmenter_CleanUp(StepSegmenter_t * pStruct);

// Reset functions
void StepSegmenter_Reset(StepSegmenter_t * pStruct);

//Update function
void StepSegmenter_UpdateAndCheckForSegment(StepSegmenter_t * pStruct, const osp_float_t accNorm, NTTIME tstamp);

// Get functions
void StepSegmenter_GetLatestSegment(StepSegmenter_t * pStruct, StepSegment_t * segment);

#ifdef __cplusplus
}
#endif

#endif //STEPSEGMENTER_H
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
