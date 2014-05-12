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
#include "SignalGenerator.h"
#include <string.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
// moving window mean for incoming raw data
#define AVERAGING_FILTER_BUF_MASK (AVERAGING_FILTER_BUF_SIZE - 1)

// decimation count inside prefiltering
#define DECIMATION_COUNT_2N (3)
#define DECIMATION_MASK ((1 << DECIMATION_COUNT_2N)-1)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct {
    uint16_t callcounter;

    float accbuf[NUM_ACCEL_AXES][AVERAGING_FILTER_BUF_SIZE];
    float accAccumulator[NUM_ACCEL_AXES];

} SignalGenerator_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static SignalGenerator_t _signalGenerator;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static osp_bool_t PerformFiltering(const float accInMetersPerSecondSquare[NUM_ACCEL_AXES], float* accFilteredOut);

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      SignalGenerator_Init
 *          Initializes memory for signal generator structure
 *
 ***************************************************************************************************/
void SignalGenerator_Init(void) {
    memset(&_signalGenerator,0,sizeof(_signalGenerator));
}

/****************************************************************************************************
 * @fn      SignalGenerator_SetAccelerometerData
 *          Main caller function for signal generation. Since this module decimates the 
 *          incoming signal after filtering, this function returns a boolean which indicates 
 *          when the accFilteredOut variable has been updated.
 *
 ***************************************************************************************************/
osp_bool_t SignalGenerator_SetAccelerometerData(const float accInMetersPerSecondSquare[NUM_ACCEL_AXES], float* accFilteredOut){
    return PerformFiltering(accInMetersPerSecondSquare, accFilteredOut);
}

/****************************************************************************************************
 * @fn      SignalGenerator_UpdateMovingWindowMean
 *          Generic function for updating a moving window mean for a buffer of size 2^buflen2N
 *
 ***************************************************************************************************/
float SignalGenerator_UpdateMovingWindowMean(float * buffer, float * pMeanAccumulator,
                                             float newmeas, uint16_t idx, uint16_t buflen2N) {
    *pMeanAccumulator += newmeas;
    *pMeanAccumulator -= buffer[idx];
    buffer[idx] = newmeas;

    return (float) ((*pMeanAccumulator)/((float)(1 << buflen2N)));
}

/****************************************************************************************************
 * @fn      PerformFiltering
 *          Filters acceleration data with moving window average.
 *          Returns true if filtered data was updated.
 *
 ***************************************************************************************************/
osp_bool_t PerformFiltering(const float accInMetersPerSecondSquare[NUM_ACCEL_AXES], float *accFilteredOut) {
    uint8_t iAxis;
    osp_bool_t success = FALSE;

    const uint16_t movingWindowIdx = _signalGenerator.callcounter & (uint16_t)AVERAGING_FILTER_BUF_MASK;

    // Compute moving average of input acceleration data
    for (iAxis = 0; iAxis < NUM_ACCEL_AXES; iAxis++) {
        accFilteredOut[iAxis] = SignalGenerator_UpdateMovingWindowMean(_signalGenerator.accbuf[iAxis],
                                                                    &_signalGenerator.accAccumulator[iAxis],
                                                                    accInMetersPerSecondSquare[iAxis],
                                                                    movingWindowIdx,
                                                                    AVERAGING_FILTER_BUF_SIZE_2N);
    }

    /// Decimate
    if ((_signalGenerator.callcounter > AVERAGING_FILTER_BUF_SIZE-1) &&
        ((_signalGenerator.callcounter & DECIMATION_MASK) == DECIMATION_MASK)) {
        success = TRUE;
    }

    _signalGenerator.callcounter++;

    return success;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
