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
#if !defined (OSP_ALG_TYPES)
#define   OSP_ALG_TYPES

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <stddef.h>
#include "osp-types.h"
#include "osp-fixedpoint-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define NUM_ACCEL_AXES                  (3)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
//! Enumeration typedef for segment type
typedef enum EStepSegmentType {
    firstStep,
    midStep,
    lastStep
} EStepSegmentType;

//! Struct definition for a step segment
typedef struct {
    //store start/stop times of the last step
    NTTIME startTime;
    NTTIME stopTime;
    EStepSegmentType type;
} StepSegment_t;

//struct for defining a step
typedef struct StepDataOSP_t{
    NTTIME startTime;
    NTTIME stopTime;
    osp_float_t stepFrequency;
    uint32_t numStepsTotal;
    uint32_t numStepsSinceWalking;
} StepDataOSP_t;

typedef void (*OSP_StepSegmentResultCallback_t)(StepSegment_t * segment);
typedef void (*OSP_StepResultCallback_t)(StepDataOSP_t* stepData);
typedef void (*OSP_EventResultCallback_t)(NTTIME * eventTime);

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* OSP_ALG_TYPES */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
