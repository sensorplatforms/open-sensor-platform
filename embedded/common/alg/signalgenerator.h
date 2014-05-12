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
#ifndef _SIGNALGENERATOR_H_
#define _SIGNALGENERATOR_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-alg-types.h"
#include "osp-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
// Moving average filter length
#define AVERAGING_FILTER_BUF_SIZE_2N (4)
#define AVERAGING_FILTER_BUF_SIZE (1 << AVERAGING_FILTER_BUF_SIZE_2N)

// Expected sample period in seconds
#define SIGNAL_GENERATOR_EXPECTED_SAMPLE_PERIOD TOFIX_TIME(0.02f)

// This is the total decimation of the signal generator with respect to incoming data rate
#define SIGNAL_GENERATOR_TOTAL_DECIMATION_2N (3)

// This is the expected output sample period of the signal generator
#define SIGNAL_GENERATOR_OUTPUT_SAMPLE_PERIOD (SIGNAL_GENERATOR_EXPECTED_SAMPLE_PERIOD << SIGNAL_GENERATOR_TOTAL_DECIMATION_2N)

// This is the expected time delay of the signal generator output
#define SIGNAL_GENERATOR_DELAY (((AVERAGING_FILTER_BUF_SIZE-1) * SIGNAL_GENERATOR_EXPECTED_SAMPLE_PERIOD) >> 1)

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

#ifdef __cplusplus
extern "C" {
#endif

// Constructor
void SignalGenerator_Init(void);

// Returns true if filtered signal is updated
osp_bool_t SignalGenerator_SetAccelerometerData(const float accInMetersPerSecondSquare[NUM_ACCEL_AXES], float* accFilteredOut);

// Moving average function
float SignalGenerator_UpdateMovingWindowMean(float * buffer, float * pMeanAccumulator,
                                             float newmeas, uint16_t idx, uint16_t buflen2N);

#ifdef __cplusplus
}
#endif


#endif //_SIGNALGENERATOR_H_
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
