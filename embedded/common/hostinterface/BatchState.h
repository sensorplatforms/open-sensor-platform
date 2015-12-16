/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2015 Audience Inc.
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
#if !defined (BATCH_STATE_H)
#define   BATCH_STATE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "SensorPackets.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Enum for different state type within Batch */
typedef enum _BatchStateType
{
    BATCH_STANDBY              = 0,
    BATCH_CONFIG               = 1,
    BATCH_IDLE                 = 2,
    BATCH_ACTIVE               = 3,
    BATCH_ACTIVE_HOST_SUSPEND  = 4,
    NUM_BATCH_STATES
} BatchStateType_t;

/* Enum for list of command supported by Batch */
typedef enum _BatchCmdList
{
    CMD_ERROR_DATA              = PARAM_ID_ERROR_CODE_IN_DATA,
    CMD_ENABLE                  = PARAM_ID_ENABLE,
    CMD_BATCH                   = PARAM_ID_BATCH,
    CMD_FLUSH                   = PARAM_ID_FLUSH,
    CMD_RANGE_RESOLUTION        = PARAM_ID_RANGE_RESOLUTION,
    CMD_POWER                   = PARAM_ID_POWER,
    CMD_MIN_MAX_DELAY           = PARAM_ID_MINMAX_DELAY,
    CMD_EVENT_MAX_COUNT         = PARAM_ID_FIFO_EVT_CNT,
    CMD_AXIS_MAPPING            = PARAM_ID_AXIS_MAPPING,
    CMD_CONVERSION_OFFSET       = PARAM_ID_CONVERSION_OFFSET,
    CMD_CONVERSION_SCALE        = PARAM_ID_CONVERSION_SCALE,
    CMD_SENSOR_NOISE            = PARAM_ID_SENSOR_NOISE,
    CMD_TEMPRATURE_OFFSET       = PARAM_ID_TIMESTAMP_OFFSET,
    CMD_ON_WAKE_TIME            = PARAM_ID_ONTIME_WAKETIME,
    CMD_HPF_LPF_CUTOFF          = PARAM_ID_HPF_LPF_CUTOFF,
    CMD_SENSOR_NAME             = PARAM_ID_SENSOR_NAME,
    CMD_XYZ_OFFSET              = PARAM_ID_XYZ_OFFSET,
    CMD_FACTORY_CAL             = PARAM_ID_F_SKOR_MATRIX,
    CMD_FACTORY_CAL_OFFSET      = PARAM_ID_F_CAL_OFFSET,
    CMD_NON_LINEAR              = PARAM_ID_F_NONLINEAR_EFFECTS,
    CMD_BIAS_STABILTY           = PARAM_ID_BIAS_STABILITY,
    CMD_REPEATABILTY            = PARAM_ID_REPEATABILITY,
    CMD_TEMPRATURE_COFF         = PARAM_ID_TEMP_COEFF,
    CMD_SHAKE_SUSCEPTIBILITY    = PARAM_ID_SHAKE_SUSCEPTIBILITY,
    CMD_EXPECTED_NORM           = PARAM_ID_EXPECTED_NORM,
    CMD_VERSION                 = PARAM_ID_VERSION,
    CMD_DYNAMIC_CAL_SCALE       = PARAM_ID_DYNAMIC_CAL_SCALE,
    CMD_DYNAMIC_CAL_SKEW        = PARAM_ID_DYNAMIC_CAL_SKEW,
    CMD_DYNAMIC_CAL_OFFSET      = PARAM_ID_DYNAMIC_CAL_OFFSET,
    CMD_DYNAMIC_CAL_ROTATION    = PARAM_ID_DYNAMIC_CAL_ROTATION,
    CMD_DYNAMIC_CAL_QUALITY     = PARAM_ID_DYNAMIC_CAL_QUALITY,
    CMD_DYNAMIC_CAL_SOURCE      = PARAM_ID_DYNAMIC_CAL_SOURCE,
    CMD_CONFIG_DONE             = PARAM_ID_CONFIG_DONE,
    CMD_NUM_MAX                 = N_PARAM_ID,
} BatchCmdList_t;

/* Structure to hold state of Batch state machine */
typedef struct _BatchState
{
    BatchStateType_t   CurState;
    BatchStateType_t   PreState;
    osp_bool_t IsInitialized;
} BatchState_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
int16_t BatchStateInitialize( void );                      /* Initialized state machine */
int16_t BatchStateSet( BatchStateType_t stateType );          /* Set new state in Batch state machine */
int16_t BatchStateGet( BatchStateType_t  *pCurState);         /* Get current state of state machine */
int16_t BatchStateCommandValidate( BatchCmdList_t cmdType );  /* Check command is valid for current state */

#endif /* BATCH_STATE_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
