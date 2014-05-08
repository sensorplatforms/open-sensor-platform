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
#include "StepSignalGenerator.h"
//#include "FM_Platform.h"
#include <string.h>
#include <math.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define ACC_BUF_MASK                    (ACC_BUF_SIZE - 1)
#define DECIMATION_COUNT_2N             (3)
#define DECIMATION_COUNT                ((1 << DECIMATION_COUNT_2N) - 1)
// filter kernel parameters
#define NTAPS_WALK_BANDPASS             (13)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
const static NTTIME DESIGN_SAMPLE_PERIOD = TOFIX_TIME(1.0/50.f);
const static float _lpKernel[NTAPS_WALK_BANDPASS+1] = {
    (0.103165619075298f),
    (0.100275494158268f),
    (0.0920005366206169f),
    (0.0794529989361763f),
    (0.0642554983496666f),
    (0.0482467822730541f),
    (0.0331619493663311f),
    (0.0203533004969358f),
    (0.0106068234890699f),
    (0.00408613402396441f),
    (0.00040603827801533f),
    (-0.0011907909065485f),
    (-0.00160198356024921f),
    (-0.00163559184875339f)
};

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
 * @fn      AddDataAndRunFilter
 *          Helper routine for ???
 *
 ***************************************************************************************************/
static void AddDataAndRunFilter(StepSignalGenerator_t * pStepSigGen, float *filteredacc, const float *rawacc){
    uint16_t i,j,k;
    float next1 = 0, next2 = 0;

    //Add raw data to buffers
    for (j = 0; j < 3; j++){
        pStepSigGen->acc[j][pStepSigGen->count & ACC_BUF_MASK] = rawacc[j];
    }
    pStepSigGen->count++;

    //run filter if enough data points
    if(pStepSigGen->count < 2*NTAPS_WALK_BANDPASS+1){
        memcpy(filteredacc,rawacc,3*sizeof(float));
        return;
    }

    for(j = 0; j < 3; j++){
        filteredacc[j] = 0;
        for(i = 0; i < NTAPS_WALK_BANDPASS; i++){
            //Get right point
            k = (pStepSigGen->count - i -1) + ACC_BUF_SIZE;
            next1 = pStepSigGen->acc[j][k & ACC_BUF_MASK];

            //Get left point
            k = (pStepSigGen->count - (2*NTAPS_WALK_BANDPASS - i) -1) + ACC_BUF_SIZE;
            next2 = pStepSigGen->acc[j][k & ACC_BUF_MASK];

            filteredacc[j] += _lpKernel[NTAPS_WALK_BANDPASS - i] * (next1 + next2);
        }
        //add in center tap
        k = (pStepSigGen->count - NTAPS_WALK_BANDPASS -1) + ACC_BUF_SIZE;
        next1 = pStepSigGen->acc[j][k & ACC_BUF_MASK];

        filteredacc[j] += _lpKernel[0] * next1;
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      StepSignalGenerator_Init
 *          <brief>
 *
 ***************************************************************************************************/
Bool StepSignalGenerator_Init(StepSignalGenerator_t * pStepSigGen){
    pStepSigGen->count = 0;

    // Initialize buffer memory to zero
    memset(&pStepSigGen->acc[0][0], 0, 3*ACC_BUF_SIZE*sizeof(float));

    SetDebugMatrixFloat("stepSignal_lpKernel",&_lpKernel[0],NTAPS_WALK_BANDPASS+1,1);
    pStepSigGen->isInitialized = TRUE;

    return pStepSigGen->isInitialized;
}


/****************************************************************************************************
 * @fn      StepSignalGenerator_Reset
 *          <brief>
 *
 ***************************************************************************************************/
void StepSignalGenerator_Reset(StepSignalGenerator_t * pStepSigGen){
    pStepSigGen->count = 0;
}


/****************************************************************************************************
 * @fn      StepSignalGenerator_CleanUp
 *          <brief>
 *
 ***************************************************************************************************/
void StepSignalGenerator_CleanUp(StepSignalGenerator_t * pStepSigGen){
    pStepSigGen->isInitialized = FALSE;
}


/****************************************************************************************************
 * @fn      StepSignalGenerator_UpdateFilters
 *          update filters (returns true when inertial acc buffers have been updated)
 *
 ***************************************************************************************************/
Bool StepSignalGenerator_UpdateFilters(StepSignalGenerator_t * pStepSigGen, const float * acc, NTTIME * tstamp, float * accZ){
    float lpf[3];

    SetDebugMatrixFloat("stepSignal_rawacc",acc,3,1);
    SetDebugMatrixTime("stepSignal_rawacctime",tstamp,1,1);

    //check that the filters have been initialized
    if(!pStepSigGen->isInitialized){
        if(!StepSignalGenerator_Init(pStepSigGen)){
            return FALSE;
        }
    }

    //run full filter updates
    AddDataAndRunFilter(pStepSigGen, lpf, acc);
    *tstamp -= (NTAPS_WALK_BANDPASS)*DESIGN_SAMPLE_PERIOD;

    //log output
    SetDebugMatrixFloat("stepSignal_filteredacc",lpf,3,1);
    SetDebugMatrixTime("stepSignal_filteredacctime",tstamp,1,1);


    //compute norm z acceleration
    *accZ = sqrtf(lpf[0]*lpf[0] + lpf[1]*lpf[1] + lpf[2]*lpf[2]);

    SetDebugMatrixFloat("stepSignal_accnorm",accZ,1,1);

    if(pStepSigGen->count > 2*NTAPS_WALK_BANDPASS+1 &&
       (pStepSigGen->count & DECIMATION_COUNT) == DECIMATION_COUNT){
        SetDebugMatrixTime("stepSignal_dectime",tstamp,1,1);
        SetDebugMatrixFloat("stepSignal_decaccnorm",accZ,1,1);
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
