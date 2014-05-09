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
#include "StepSegmenter.h"


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
//Define peak property flags
#define PEAKPROP_PEAK_PERIOD_SHORT_ENOUGH           (1<<0)
#define PEAKPROP_STEP_PERIOD_SHORT_ENOUGH           (1<<1)
#define PEAKPROP_MAG_LARGE_ENOUGH                   (1<<2)
#define PEAKPROP_STRIDE_PERIOD_NEAR_EXPECTED        (1<<3)
#define PEAKPROP_CONSECUTIVE_SMALL_PEAKS            (1<<4)
#define PEAKPROP_CONSECUTIVE_LARGE_ENOUGH_PEAKS     (1<<5)
#define PEAKPROP_VALID_NEG_PEAK_SINCE_LAST_STEP     (1<<6)

//Define step property flags
#define STEPPROP_MAG_VALID                          (1<<0)
#define STEPPROP_INVALID_NEG_PEAK                   (1<<4)

//local constants
#define DEFAULT_EXPECTED_STRIDE_PERIOD TOFIX_TIME   (1.8f)
#define MAX_STRIDE_TIME TOFIX_TIME                  (2.5f)
#define MAX_STEP_TIME TOFIX_TIME                    (1.3f)

//initial step counter variables
#define NUM_STEPS_BEFORE_REPORTING                  (3)
#define STORED_STEPS_MASK                           (NUM_INITIAL_STORED_STEPS - 1)


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
// Variables that effect segmentation
static float MIN_PEAK_ACC = 1.1; //m/s^2
static float MIN_PEAK_ACC_INITIAL = 0.5; //m/s^2

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
 * @fn      UpdateAndLookForPeak
 *          Takes new accel norm data and checks for a peak
 *
 ***************************************************************************************************/
static osp_bool_t UpdateAndLookForPeak(StepSegmenter_t * pStruct, float accNorm, EExtremaType * peakType){
    osp_bool_t peakFound = FALSE;

    if(accNorm >= pStruct->prevAccNorm){
        if(pStruct->prevPeakType == potentialNegPeak){
            peakFound = TRUE;
            *peakType = negativePeak;
        }
        pStruct->prevPeakType = potentialPosPeak;
    } else if(accNorm < pStruct->prevAccNorm){
        if(pStruct->prevPeakType == potentialPosPeak){
            peakFound = TRUE;
            *peakType = positivePeak;
        }
        pStruct->prevPeakType = potentialNegPeak;
    }

    return peakFound;
}


/****************************************************************************************************
 * @fn      IsValidNegativePeak
 *          <brief>
 *
 ***************************************************************************************************/
static osp_bool_t IsValidNegativePeak(StepSegmenter_t * pStruct){

    //if in the middle of walking, use peak if:
    //(1) peak magnitude is sufficient,
    //(2) it shows up near where expected, or
    //(3) it has been longer than expected since we had a negative peak
    if((pStruct->segmenterState == midWalk)){
        NTTIME timeSinceLastPeak = pStruct->prevTime - pStruct->lastNegPeakTime;
        NTTIME expectedTimeSinceLastPeak = (pStruct->expectedStridePeriod >> 1);
        NTPRECISE peakMag = pStruct->lastStepPeakMag - pStruct->prevAccNorm;
        if((timeSinceLastPeak > expectedTimeSinceLastPeak) ||
           (peakMag > MIN_PEAK_ACC_INITIAL) ||
           (expectedTimeSinceLastPeak - timeSinceLastPeak < (pStruct->expectedStridePeriod >> 2))){
               return TRUE;
        } else {
            return FALSE;
        }
    }

    return TRUE;
}


/****************************************************************************************************
 * @fn      ComputeStepProperties
 *          <brief>
 *
 ***************************************************************************************************/
static void ComputeStepProperties(StepSegmenter_t * pStruct){
    pStruct->stepFlags = 0;

    if(pStruct->peakFlags & PEAKPROP_MAG_LARGE_ENOUGH){
        pStruct->stepFlags |=  STEPPROP_MAG_VALID;
    }
    if(pStruct->lastNegPeakTime > pStruct->lastPosPeakTime){
        pStruct->stepFlags |= STEPPROP_INVALID_NEG_PEAK;
    }
}


/****************************************************************************************************
 * @fn      ComputePeakProperties
 *          <brief>
 *
 ***************************************************************************************************/
static void ComputePeakProperties(StepSegmenter_t * pStruct){
    NTTIME peakPeriod = pStruct->prevTime - pStruct->lastPosPeakTime;
    NTTIME stepPeriod = pStruct->prevTime - pStruct->stepSegment.stopTime;
    float peakMag = pStruct->prevAccNorm - pStruct->lastNegPeakMag;

    //clear existing flags
    pStruct->peakFlags = 0;

    //compute properties:
    if(peakPeriod <= MAX_STRIDE_TIME){
        pStruct->peakFlags |= PEAKPROP_PEAK_PERIOD_SHORT_ENOUGH;
    }
    if(stepPeriod <= MAX_STEP_TIME){
        pStruct->peakFlags |= PEAKPROP_STEP_PERIOD_SHORT_ENOUGH;
    }
    if(pStruct->lastNegPeakTime < pStruct->stepSegment.stopTime){
        pStruct->peakFlags |= PEAKPROP_VALID_NEG_PEAK_SINCE_LAST_STEP;
    }
    if(peakMag > MIN_PEAK_ACC){
        pStruct->peakFlags |= PEAKPROP_MAG_LARGE_ENOUGH;
    } else {
        if(pStruct->segmenterState == midWalk){
            if((pStruct->stepFlags & STEPPROP_MAG_VALID)){
                pStruct->peakFlags |= PEAKPROP_CONSECUTIVE_LARGE_ENOUGH_PEAKS;
            }
        }
        //if this is a potential first step, use lower peak mag requirement
        if(pStruct->segmenterState == endWalk && peakMag > MIN_PEAK_ACC_INITIAL){
            pStruct->peakFlags |= PEAKPROP_MAG_LARGE_ENOUGH;
        }
    }

    if(pStruct->segmenterState == midWalk){
        NTTIME stridePeriod = pStruct->prevTime - pStruct->stepSegment.startTime;
        if(ABS(stridePeriod-pStruct->expectedStridePeriod) <= (pStruct->expectedStridePeriod >> 2)){
            pStruct->peakFlags |= PEAKPROP_STRIDE_PERIOD_NEAR_EXPECTED;
        }
    }
}


/****************************************************************************************************
 * @fn      PeakMeetsStepCriteria
 *          <brief>
 *
 ***************************************************************************************************/
static osp_bool_t PeakMeetsStepCriteria(uint16_t peakFlags){

    //must have small enough period if not in swingingMode (where we may have missed a step)
    if(!(peakFlags & PEAKPROP_PEAK_PERIOD_SHORT_ENOUGH)){
        return FALSE;
    }

    //check peak magnitudes
    if((peakFlags & PEAKPROP_MAG_LARGE_ENOUGH)){
        return TRUE;
    }

    //check for valid negative peak
    if(!(peakFlags & PEAKPROP_VALID_NEG_PEAK_SINCE_LAST_STEP)){
        return FALSE;
    }

    //if midwalk consecutive peaks are okay magnitude-wise and the peak is where we expect it, use it
    if((peakFlags & PEAKPROP_CONSECUTIVE_LARGE_ENOUGH_PEAKS) &&
       (peakFlags & PEAKPROP_STRIDE_PERIOD_NEAR_EXPECTED)){
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************************************
 * @fn      AddStep
 *          <brief>
 *
 ***************************************************************************************************/
static void AddStep(StepSegmenter_t * pStruct, StepSegment_t segment){
    EStepSegmenterState initState = pStruct->segmenterState;
    NTTIME stridePeriod = segment.stopTime - pStruct->stepSegment.startTime;
    uint8_t stepIdx = (pStruct->numStoredSteps-1) & (uint16_t)STORED_STEPS_MASK;
    osp_bool_t validStep = TRUE;

    //Compute step props
    ComputeStepProperties(pStruct);

    //set segment
    pStruct->stepSegment = segment;

    //check for initial state changes
    if(pStruct->segmenterState != endWalk && pStruct->stepSegment.type == lastStep){
        pStruct->segmenterState = endWalk;
        pStruct->expectedStridePeriod = DEFAULT_EXPECTED_STRIDE_PERIOD;
    } else if(pStruct->segmenterState == endWalk) {
        pStruct->segmenterState = startWalk;
        pStruct->numStoredSteps = 0;
    }


    // STATE MACHINE
    switch(pStruct->segmenterState){
    case endWalk:
        //if we just ended walking, send out final step
        if(initState == midWalk && pStruct->resultReadyCallback){
            pStruct->storedSteps[stepIdx].type = lastStep;
            pStruct->resultReadyCallback(&pStruct->stepSegment);
        }
        break;


    case startWalk:
        if(pStruct->numStoredSteps < NUM_STEPS_BEFORE_REPORTING){
            //peak height delta = peakLeft - peakRight
            //                  = (posPeakMag - negPeakMag) - (prevZ - negPeakMag)
            //                  = (posPeakMag - prevZ)
            float peakHeightDelta = ABS(pStruct->lastPosPeakMag - pStruct->prevAccNorm);
            float peakHeightRight = pStruct->prevAccNorm - pStruct->lastNegPeakMag;
            float peakComp = 0.75*peakHeightRight;
            if(((pStruct->numStoredSteps == 0) &&
                (peakHeightDelta > peakComp))){
                validStep = FALSE;
                stepIdx = 0;
                pStruct->numStoredSteps = 0;
            }
            if(pStruct->numStoredSteps == 0){
                pStruct->stepSegment.type = firstStep;
            }
        } else {
            //we've reached the number of stored steps - send
            if(pStruct->resultReadyCallback){
                uint8_t i;
                for(i = 0; i < NUM_STEPS_BEFORE_REPORTING; i++){
                    pStruct->resultReadyCallback(&pStruct->storedSteps[i]);
                }
            }
            pStruct->resultReadyCallback(&pStruct->stepSegment);
            pStruct->segmenterState = midWalk;
        }
        break;


    case midWalk:
        if(pStruct->resultReadyCallback){
            pStruct->resultReadyCallback(&pStruct->stepSegment);
        }
        break;
    }

    // Update expected stride period if mid-walk
    if(pStruct->segmenterState == midWalk){
        pStruct->expectedStridePeriod = stridePeriod;
    }

    // If valid step, add step to stored steps
    if(pStruct->segmenterState == startWalk){
        if(validStep){
            stepIdx = (pStruct->numStoredSteps) & (uint16_t)STORED_STEPS_MASK;
            pStruct->numStoredSteps++;
        }
        pStruct->storedSteps[stepIdx] = pStruct->stepSegment;
    }
}


/****************************************************************************************************
 * @fn      CheckForPotentialNewStep
 *          <brief>
 *
 ***************************************************************************************************/
static osp_bool_t CheckForPotentialNewStep(StepSegmenter_t *pStruct, EExtremaType peakType, StepSegment_t * potSegment){
    osp_bool_t endWalkConditions = FALSE;

    //set potential segment params
    potSegment->startTime = pStruct->stepSegment.stopTime;
    potSegment->stopTime = pStruct->prevTime;
    potSegment->type = midStep;
    pStruct->lastStepPeakMag = pStruct->prevAccNorm;

    //Check for end-walk conditions
    if((!(pStruct->peakFlags & PEAKPROP_STEP_PERIOD_SHORT_ENOUGH)) ||
       (pStruct->peakFlags & PEAKPROP_CONSECUTIVE_SMALL_PEAKS)){
        endWalkConditions = TRUE;
        if(pStruct->segmenterState != endWalk){
            potSegment->type = lastStep;
            if(pStruct->stepSegment.stopTime > pStruct->lastPosPeakTime){
                potSegment->stopTime = pStruct->lastPosPeakTime;
            }
        }
    }

    //Check if this is a potential segment
    if(PeakMeetsStepCriteria(pStruct->peakFlags)){
        if(pStruct->segmenterState == endWalk){
            potSegment->startTime = pStruct->lastPosPeakTime;
            potSegment->type = firstStep;
        }
        return TRUE;
    } else {
        if((pStruct->segmenterState != endWalk) && endWalkConditions){
            return TRUE;
        }
    }

    return FALSE;
}

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      StepSegmenter_Init
 *          Initialize segmenter struct initialization variables
 *
 ***************************************************************************************************/
void StepSegmenter_Init(StepSegmenter_t * pStruct, OSP_StepSegmentResultCallback_t pResultReadyCallback){

    //Set up callback
    pStruct->resultReadyCallback= pResultReadyCallback;

    //reset remaining parameters
    StepSegmenter_Reset(pStruct);
}


/****************************************************************************************************
 * @fn      StepSegmenter_Reset
 *          Reset all parameters in step segmenter struct
 *
 ***************************************************************************************************/
void StepSegmenter_Reset(StepSegmenter_t * pStruct){
    pStruct->stepSegment.startTime = 0;
    pStruct->stepSegment.stopTime = 0;
    pStruct->stepSegment.type = lastStep;

    pStruct->prevAccNorm = 0;
    pStruct->prevTime = 0;
    pStruct->prevPeakType = negativePeak;

    pStruct->lastPosPeakMag = 0;
    pStruct->lastNegPeakMag = 0;
    pStruct->lastPosPeakTime = 0;
    pStruct->lastNegPeakTime = 0;
    pStruct->expectedStridePeriod = DEFAULT_EXPECTED_STRIDE_PERIOD;

    pStruct->segmenterState = endWalk;
    pStruct->peakFlags = 0;
    pStruct->stepFlags = 0;
}


/****************************************************************************************************
 * @fn      StepSegmenter_CleanUp
 *          <brief>
 *
 ***************************************************************************************************/
void StepSegmenter_CleanUp(StepSegmenter_t * pStruct){
    StepSegmenter_Reset(pStruct);
    pStruct->resultReadyCallback = NULL;
}


/****************************************************************************************************
 * @fn      StepSegmenter_UpdateAndCheckForSegment
 *          <brief>
 *
 ***************************************************************************************************/
void StepSegmenter_UpdateAndCheckForSegment(StepSegmenter_t * pStruct, const float accNorm, NTTIME tstamp){
    EExtremaType peakType;
    StepSegment_t potSegment;

    // Check for peak
    if(UpdateAndLookForPeak(pStruct, accNorm, &peakType)){

        //If negative peak, check validity and then update info if valid
        if(peakType == negativePeak){
            if(IsValidNegativePeak(pStruct)){
                pStruct->lastNegPeakTime = pStruct->prevTime;
                pStruct->lastNegPeakMag = pStruct->prevAccNorm;
            }
        }

        //ELSE this is a positive peak - check for potential new step
        else {
            //Compute peak properties
            ComputePeakProperties(pStruct);

            //If potential new step, add step to state machine
            if(CheckForPotentialNewStep(pStruct, peakType, &potSegment)){
                AddStep(pStruct, potSegment);
            }

            //Update peak mag and time
            pStruct->lastPosPeakMag = pStruct->prevAccNorm;
            pStruct->lastPosPeakTime = pStruct->prevTime;
        }
    }

    //Store previous accel norm and time
    pStruct->prevAccNorm = accNorm;
    pStruct->prevTime = tstamp;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
