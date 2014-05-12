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
#include "SignificantMotionDetector.h"
#include "SignalGenerator.h"
#include "osp-alg-types.h"
#include <math.h>
#include <string.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* moving window size for mean and noise - assumed input rate between 6-8Hz */
#define MOVING_WINDOW_MEAN_BUF_SIZE_2N (3)
#define MOVING_WINDOW_MEAN_BUF_SIZE (1 << MOVING_WINDOW_MEAN_BUF_SIZE_2N)
#define MOVING_WINDOW_MEAN_BUF_MASK (MOVING_WINDOW_MEAN_BUF_SIZE - 1)

// thresholds for triggering significant motion
#define ENERGY_THRESHOLD_FOR_SIG_MOTION   (0.5f)
#define ENERGY_FLOOR                      (0.0221f)

//seconds for triggering significant motion/stillness
#define TIME_FOR_SIGNIFICANT_DECISION (4.0f)

//expected sample period
#define SIGNIFICANT_MOTION_EXPECTED_SAMPLE_PERIOD (SIGNAL_GENERATOR_OUTPUT_SAMPLE_PERIOD)

//comes out in counts
#define NUM_COUNTS_MOTION_FOR_SIGNIFICANT_DECISION ((uint8_t)(TIME_FOR_SIGNIFICANT_DECISION/TOFLT_TIME(SIGNIFICANT_MOTION_EXPECTED_SAMPLE_PERIOD)))

//filter delays for significant motion
#define SIGNIFICANT_MOTION_DETECTOR_DELAY (((MOVING_WINDOW_MEAN_BUF_SIZE-1) * SIGNIFICANT_MOTION_EXPECTED_SAMPLE_PERIOD) >> 1)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
// struct for containing significant motion detector data
typedef struct {
    OSP_EventResultCallback_t sigMotCallback;

    osp_float_t meanbuf[NUM_ACCEL_AXES][MOVING_WINDOW_MEAN_BUF_SIZE];
    osp_float_t meanaccumulator[NUM_ACCEL_AXES];

    osp_float_t energybuf[MOVING_WINDOW_MEAN_BUF_SIZE];
    osp_float_t energyaccumulator;

    osp_bool_t isSignificantMotion;

    uint8_t motionCounter;
    uint8_t signalCounter;
} SigMotionDetector_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static SigMotionDetector_t _sigMotData;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
static osp_float_t UpdateSignals(const osp_float_t sigIn[NUM_ACCEL_AXES]);
static void SignificantMotionStateMachine(const NTTIME time, const osp_float_t energy);


/****************************************************************************************************
 * @fn      SignificantMotDetector_Init
 *          Initializes callback function variable and all underlying private variables
 *
 ***************************************************************************************************/
void SignificantMotDetector_Init(OSP_EventResultCallback_t pSigMotionCallback){
    _sigMotData.sigMotCallback = pSigMotionCallback;    

    //Reset buffers
    SignificantMotDetector_Reset();
}

/****************************************************************************************************
 * @fn      SignificantMotDetector_CleanUp
 *          Clears callback function variable
 *
 ***************************************************************************************************/
void SignificantMotDetector_CleanUp(void){
    _sigMotData.sigMotCallback = NULL;
}

/****************************************************************************************************
 * @fn      SignificantMotDetector_Reset
 *          Resets filter variables
 *
 ***************************************************************************************************/
void SignificantMotDetector_Reset(void){
    OSP_EventResultCallback_t callback = _sigMotData.sigMotCallback;
    memset(&_sigMotData,0,sizeof(_sigMotData));
    _sigMotData.sigMotCallback = callback;
}

/****************************************************************************************************
 * @fn      SignificantMotDetector_SetFilteredAccelerometerMeasurement
 *          Main worker of significant motion detector. Results are only produced when this is called
 *
 ***************************************************************************************************/
void SignificantMotDetector_SetFilteredAccelerometerMeasurement(const NTTIME tstamp, const osp_float_t acc[NUM_ACCEL_AXES]){
    osp_float_t totalEnergy;

    totalEnergy = UpdateSignals(acc);

    //Run state machine once buffers have been filled
    if(_sigMotData.signalCounter >= MOVING_WINDOW_MEAN_BUF_SIZE){
        SignificantMotionStateMachine(tstamp, totalEnergy);
    }
}

/****************************************************************************************************
 * @fn      UpdateSignals
 *          Computes energy signal used for significant motion detection
 *
 ***************************************************************************************************/
osp_float_t UpdateSignals(const osp_float_t accIn[NUM_ACCEL_AXES]) {
    const uint16_t movingWindowIdx = _sigMotData.signalCounter & (uint16_t)MOVING_WINDOW_MEAN_BUF_MASK;
    uint8_t i;    
    osp_float_t absAccMinusMean[NUM_ACCEL_AXES];
    osp_float_t mean[NUM_ACCEL_AXES];
    osp_float_t temp;
    osp_float_t totalEnergy = 0.f;

    //compute mean of filtered accel signal and energy surrogate
    for (i = 0; i < NUM_ACCEL_AXES; i++) {

        // mean of filtered accel over window
        mean[i] = SignalGenerator_UpdateMovingWindowMean(_sigMotData.meanbuf[i],
                                                         &_sigMotData.meanaccumulator[i],
                                                         accIn[i],
                                                         movingWindowIdx,
                                                         MOVING_WINDOW_MEAN_BUF_SIZE_2N);

        absAccMinusMean[i] = ABS(accIn[i] - mean[i]);

        // accumulate total energy surrogate
        totalEnergy += absAccMinusMean[i];
    }

    //filter energy signal
    temp = SignalGenerator_UpdateMovingWindowMean(_sigMotData.energybuf,
                                                  &_sigMotData.energyaccumulator,
                                                  totalEnergy,
                                                  movingWindowIdx,
                                                  MOVING_WINDOW_MEAN_BUF_SIZE_2N);
    totalEnergy = temp;
    
    if (totalEnergy < ENERGY_FLOOR) {
        totalEnergy = ENERGY_FLOOR;
    }

    _sigMotData.signalCounter++;

    return totalEnergy;
}

/****************************************************************************************************
 * @fn      SignificantMotionStateMachine
 *          Detects significant motion based on threshold crossing and callbacks to subscribers
 *
 ***************************************************************************************************/
void SignificantMotionStateMachine(const NTTIME time, const osp_float_t energy) {

    osp_bool_t isSignificantMotion = FALSE;
    NTTIME eventTime = time - SIGNIFICANT_MOTION_DETECTOR_DELAY;

    //get max signal
    if (energy >= ENERGY_THRESHOLD_FOR_SIG_MOTION) {
        _sigMotData.motionCounter++;
    }
    else {
        _sigMotData.motionCounter = 0;
    }

    //check threshold crossing for significant motion
    if (_sigMotData.motionCounter > NUM_COUNTS_MOTION_FOR_SIGNIFICANT_DECISION) {
        isSignificantMotion = TRUE;
        _sigMotData.motionCounter = NUM_COUNTS_MOTION_FOR_SIGNIFICANT_DECISION;
    }

    //callback only if significant motion was just triggered
    if(isSignificantMotion && !_sigMotData.isSignificantMotion && _sigMotData.sigMotCallback){
        _sigMotData.sigMotCallback(&eventTime);
    }

    _sigMotData.isSignificantMotion = isSignificantMotion;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
