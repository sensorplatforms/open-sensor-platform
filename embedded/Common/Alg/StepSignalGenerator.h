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
#ifndef STEPSIGNALGENERATOR_H_
#define STEPSIGNALGENERATOR_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-alg-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define ACC_BUF_SIZE_2N (5)
#define ACC_BUF_SIZE (1 << ACC_BUF_SIZE_2N)

#define SetDebugMatrixFloat(...)
#define SetDebugMatrixTime(...)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct {
    // data counter
    int16_t count;

    // raw accel buffers
    float acc[3][ACC_BUF_SIZE];

    Bool isInitialized;
} StepSignalGenerator_t;


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
//Init/reset/destroy methods
Bool StepSignalGenerator_Init(StepSignalGenerator_t * pStepSigGen);
void StepSignalGenerator_Reset(StepSignalGenerator_t * pStepSigGen);
void StepSignalGenerator_CleanUp(StepSignalGenerator_t * pStepSigGen);

// update filters (returns true when inertial acc buffers have been updated)
Bool StepSignalGenerator_UpdateFilters(StepSignalGenerator_t * pStepSigGen, const float * acc, NTTIME * tstamp, float *accZ);

#ifdef __cplusplus
}
#endif

#endif //STEPSIGNALGENERATOR_H_
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
