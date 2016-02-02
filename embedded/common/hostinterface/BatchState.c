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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "common.h"
#include "BatchState.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* State command table which holds command supported for particular state */
static const osp_bool_t StateCommandTable[NUM_BATCH_STATES] [CMD_NUM_MAX] = 
{
    /* Standby state */
    [BATCH_STANDBY][CMD_ENABLE]                = FALSE,
    [BATCH_STANDBY][CMD_BATCH]                 = FALSE,
    [BATCH_STANDBY][CMD_FLUSH]                 = FALSE,
    [BATCH_STANDBY][CMD_RANGE_RESOLUTION]      = TRUE,
    [BATCH_STANDBY][CMD_POWER]                 = TRUE,
    [BATCH_STANDBY][CMD_MIN_MAX_DELAY]         = TRUE,
    [BATCH_STANDBY][CMD_EVENT_MAX_COUNT]       = TRUE,
    [BATCH_STANDBY][CMD_AXIS_MAPPING]          = TRUE,
    [BATCH_STANDBY][CMD_CONVERSION_OFFSET]     = TRUE,
    [BATCH_STANDBY][CMD_CONVERSION_SCALE]      = TRUE,
    [BATCH_STANDBY][CMD_SENSOR_NOISE]          = TRUE,
    [BATCH_STANDBY][CMD_TEMPRATURE_OFFSET]     = TRUE,
    [BATCH_STANDBY][CMD_ON_WAKE_TIME]          = TRUE,
    [BATCH_STANDBY][CMD_HPF_LPF_CUTOFF]        = TRUE,
    [BATCH_STANDBY][CMD_SENSOR_NAME]           = TRUE,
    [BATCH_STANDBY][CMD_XYZ_OFFSET]            = TRUE,
    [BATCH_STANDBY][CMD_FACTORY_CAL]           = TRUE,
    [BATCH_STANDBY][CMD_FACTORY_CAL_OFFSET]    = TRUE,
    [BATCH_STANDBY][CMD_NON_LINEAR]            = TRUE,
    [BATCH_STANDBY][CMD_BIAS_STABILTY]         = TRUE,
    [BATCH_STANDBY][CMD_REPEATABILTY]          = TRUE,
    [BATCH_STANDBY][CMD_TEMPRATURE_COFF]       = TRUE,
    [BATCH_STANDBY][CMD_SHAKE_SUSCEPTIBILITY]  = TRUE,
    [BATCH_STANDBY][CMD_EXPECTED_NORM]         = TRUE,
    [BATCH_STANDBY][CMD_VERSION]               = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_SCALE]     = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_SKEW]      = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_OFFSET]    = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_ROTATION]  = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_QUALITY]   = TRUE,
    [BATCH_STANDBY][CMD_DYNAMIC_CAL_SOURCE]    = TRUE,
    [BATCH_STANDBY][CMD_CONFIG_DONE]           = TRUE,
    [BATCH_STANDBY][CMD_SH_TIME_SET]           = TRUE,
    [BATCH_STANDBY][CMD_TIME_SYNC_START]       = TRUE,
    [BATCH_STANDBY][CMD_TIME_SYNC_FOLLOW_UP]   = TRUE,
    [BATCH_STANDBY][CMD_TIME_SYNC_END]         = TRUE,

    /* Config state */
    [BATCH_CONFIG][CMD_ENABLE]                = FALSE,
    [BATCH_CONFIG][CMD_BATCH]                 = FALSE,
    [BATCH_CONFIG][CMD_FLUSH]                 = FALSE,
    [BATCH_CONFIG][CMD_RANGE_RESOLUTION]      = TRUE,
    [BATCH_CONFIG][CMD_POWER]                 = TRUE,
    [BATCH_CONFIG][CMD_MIN_MAX_DELAY]         = TRUE,
    [BATCH_CONFIG][CMD_EVENT_MAX_COUNT]       = TRUE,
    [BATCH_CONFIG][CMD_AXIS_MAPPING]          = TRUE,
    [BATCH_CONFIG][CMD_CONVERSION_OFFSET]     = TRUE,
    [BATCH_CONFIG][CMD_CONVERSION_SCALE]      = TRUE,
    [BATCH_CONFIG][CMD_SENSOR_NOISE]          = TRUE,
    [BATCH_CONFIG][CMD_TEMPRATURE_OFFSET]     = TRUE,
    [BATCH_CONFIG][CMD_ON_WAKE_TIME]          = TRUE,
    [BATCH_CONFIG][CMD_HPF_LPF_CUTOFF]        = TRUE,
    [BATCH_CONFIG][CMD_SENSOR_NAME]           = TRUE,
    [BATCH_CONFIG][CMD_XYZ_OFFSET]            = TRUE,
    [BATCH_CONFIG][CMD_FACTORY_CAL]           = TRUE,
    [BATCH_CONFIG][CMD_FACTORY_CAL_OFFSET]    = TRUE,
    [BATCH_CONFIG][CMD_NON_LINEAR]            = TRUE,
    [BATCH_CONFIG][CMD_BIAS_STABILTY]         = TRUE,
    [BATCH_CONFIG][CMD_REPEATABILTY]          = TRUE,
    [BATCH_CONFIG][CMD_TEMPRATURE_COFF]       = TRUE,
    [BATCH_CONFIG][CMD_SHAKE_SUSCEPTIBILITY]  = TRUE,
    [BATCH_CONFIG][CMD_EXPECTED_NORM]         = TRUE,
    [BATCH_CONFIG][CMD_VERSION]               = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_SCALE]     = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_SKEW]      = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_OFFSET]    = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_ROTATION]  = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_QUALITY]   = TRUE,
    [BATCH_CONFIG][CMD_DYNAMIC_CAL_SOURCE]    = TRUE,
    [BATCH_CONFIG][CMD_CONFIG_DONE]           = TRUE,
    [BATCH_CONFIG][CMD_SH_TIME_SET]           = TRUE,
    [BATCH_CONFIG][CMD_TIME_SYNC_START]       = TRUE,
    [BATCH_CONFIG][CMD_TIME_SYNC_FOLLOW_UP]   = TRUE,
    [BATCH_CONFIG][CMD_TIME_SYNC_END]         = TRUE,

    /* Idle state */
    [BATCH_IDLE][CMD_ENABLE]                = TRUE,
    [BATCH_IDLE][CMD_BATCH]                 = TRUE,
    [BATCH_IDLE][CMD_FLUSH]                 = TRUE,
    [BATCH_IDLE][CMD_RANGE_RESOLUTION]      = FALSE,
    [BATCH_IDLE][CMD_POWER]                 = FALSE,
    [BATCH_IDLE][CMD_MIN_MAX_DELAY]         = FALSE,
    [BATCH_IDLE][CMD_EVENT_MAX_COUNT]       = FALSE,
    [BATCH_IDLE][CMD_AXIS_MAPPING]          = FALSE,
    [BATCH_IDLE][CMD_CONVERSION_OFFSET]     = FALSE,
    [BATCH_IDLE][CMD_CONVERSION_SCALE]      = FALSE,
    [BATCH_IDLE][CMD_SENSOR_NOISE]          = FALSE,
    [BATCH_IDLE][CMD_TEMPRATURE_OFFSET]     = FALSE,
    [BATCH_IDLE][CMD_ON_WAKE_TIME]          = FALSE,
    [BATCH_IDLE][CMD_HPF_LPF_CUTOFF]        = FALSE,
    [BATCH_IDLE][CMD_SENSOR_NAME]           = TRUE,
    [BATCH_IDLE][CMD_XYZ_OFFSET]            = FALSE,
    [BATCH_IDLE][CMD_FACTORY_CAL]           = FALSE,
    [BATCH_IDLE][CMD_FACTORY_CAL_OFFSET]    = FALSE,
    [BATCH_IDLE][CMD_NON_LINEAR]            = FALSE,
    [BATCH_IDLE][CMD_BIAS_STABILTY]         = FALSE,
    [BATCH_IDLE][CMD_REPEATABILTY]          = FALSE,
    [BATCH_IDLE][CMD_TEMPRATURE_COFF]       = FALSE,
    [BATCH_IDLE][CMD_SHAKE_SUSCEPTIBILITY]  = FALSE,
    [BATCH_IDLE][CMD_EXPECTED_NORM]         = FALSE,
    [BATCH_IDLE][CMD_VERSION]               = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_SCALE]     = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_SKEW]      = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_OFFSET]    = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_ROTATION]  = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_QUALITY]   = TRUE,
    [BATCH_IDLE][CMD_DYNAMIC_CAL_SOURCE]    = TRUE,
    [BATCH_IDLE][CMD_CONFIG_DONE]           = TRUE,
    [BATCH_IDLE][CMD_SH_TIME_SET]           = TRUE,
    [BATCH_IDLE][CMD_TIME_SYNC_START]       = TRUE,
    [BATCH_IDLE][CMD_TIME_SYNC_FOLLOW_UP]   = TRUE,
    [BATCH_IDLE][CMD_TIME_SYNC_END]         = TRUE,

    /* Active State */
    [BATCH_ACTIVE][CMD_ENABLE]                = TRUE,
    [BATCH_ACTIVE][CMD_BATCH]                 = TRUE,
    [BATCH_ACTIVE][CMD_FLUSH]                 = TRUE,
    [BATCH_ACTIVE][CMD_RANGE_RESOLUTION]      = FALSE,
    [BATCH_ACTIVE][CMD_POWER]                 = FALSE,
    [BATCH_ACTIVE][CMD_MIN_MAX_DELAY]         = FALSE,
    [BATCH_ACTIVE][CMD_EVENT_MAX_COUNT]       = FALSE,
    [BATCH_ACTIVE][CMD_AXIS_MAPPING]          = FALSE,
    [BATCH_ACTIVE][CMD_CONVERSION_OFFSET]     = FALSE,
    [BATCH_ACTIVE][CMD_CONVERSION_SCALE]      = FALSE,
    [BATCH_ACTIVE][CMD_SENSOR_NOISE]          = FALSE,
    [BATCH_ACTIVE][CMD_TEMPRATURE_OFFSET]     = FALSE,
    [BATCH_ACTIVE][CMD_ON_WAKE_TIME]          = FALSE,
    [BATCH_ACTIVE][CMD_HPF_LPF_CUTOFF]        = FALSE,
    [BATCH_ACTIVE][CMD_SENSOR_NAME]           = FALSE,
    [BATCH_ACTIVE][CMD_XYZ_OFFSET]            = FALSE,
    [BATCH_ACTIVE][CMD_FACTORY_CAL]           = FALSE,
    [BATCH_ACTIVE][CMD_FACTORY_CAL_OFFSET]    = FALSE,
    [BATCH_ACTIVE][CMD_NON_LINEAR]            = FALSE,
    [BATCH_ACTIVE][CMD_BIAS_STABILTY]         = FALSE,
    [BATCH_ACTIVE][CMD_REPEATABILTY]          = FALSE,
    [BATCH_ACTIVE][CMD_TEMPRATURE_COFF]       = FALSE,
    [BATCH_ACTIVE][CMD_SHAKE_SUSCEPTIBILITY]  = FALSE,
    [BATCH_ACTIVE][CMD_EXPECTED_NORM]         = FALSE,
    [BATCH_ACTIVE][CMD_VERSION]               = FALSE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_SCALE]     = TRUE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_SKEW]      = TRUE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_OFFSET]    = TRUE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_ROTATION]  = TRUE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_QUALITY]   = TRUE,
    [BATCH_ACTIVE][CMD_DYNAMIC_CAL_SOURCE]    = TRUE,
    [BATCH_ACTIVE][CMD_CONFIG_DONE]           = FALSE,
    [BATCH_ACTIVE][CMD_SH_TIME_SET]           = TRUE,
    [BATCH_ACTIVE][CMD_TIME_SYNC_START]       = TRUE,
    [BATCH_ACTIVE][CMD_TIME_SYNC_FOLLOW_UP]   = TRUE,
    [BATCH_ACTIVE][CMD_TIME_SYNC_END]         = TRUE,
    
    /* Batch with Host Suspend State */
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_ENABLE]                = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_BATCH]                 = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_FLUSH]                 = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_RANGE_RESOLUTION]      = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_POWER]                 = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_MIN_MAX_DELAY]         = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_EVENT_MAX_COUNT]       = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_AXIS_MAPPING]          = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_CONVERSION_OFFSET]     = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_CONVERSION_SCALE]      = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_SENSOR_NOISE]          = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_TEMPRATURE_OFFSET]     = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_ON_WAKE_TIME]          = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_HPF_LPF_CUTOFF]        = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_SENSOR_NAME]           = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_XYZ_OFFSET]            = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_FACTORY_CAL]           = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_FACTORY_CAL_OFFSET]    = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_NON_LINEAR]            = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_BIAS_STABILTY]         = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_REPEATABILTY]          = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_TEMPRATURE_COFF]       = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_SHAKE_SUSCEPTIBILITY]  = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_EXPECTED_NORM]         = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_VERSION]               = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_SCALE]     = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_SKEW]      = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_OFFSET]    = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_ROTATION]  = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_QUALITY]   = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_DYNAMIC_CAL_SOURCE]    = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_CONFIG_DONE]           = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_SH_TIME_SET]           = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_TIME_SYNC_START]       = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_TIME_SYNC_FOLLOW_UP]   = FALSE,
    [BATCH_ACTIVE_HOST_SUSPEND][CMD_TIME_SYNC_END]         = FALSE,
};

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static BatchState_t BatchStateMachine= {
    .CurState = BATCH_STANDBY,
    .PreState = BATCH_STANDBY,
    .IsInitialized = FALSE,
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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      BatchStateInitialize
 *          Initialize the Batch state machine.
 *
 ** @param   void
 *
 * @return  OSP_STATUS_OK or error code 
 *
 ***************************************************************************************************/
int16_t BatchStateInitialize( void )
{
    /* Check Batch state machine is initialized */
    if (!BatchStateMachine.IsInitialized)
    {
        /* Set default state of state machine */
        BatchStateMachine.CurState = BATCH_STANDBY;
        BatchStateMachine.PreState = BATCH_STANDBY;

        BatchStateMachine.IsInitialized = TRUE;
    }

    return OSP_STATUS_OK;

}


/****************************************************************************************************
 * @fn      BatchStateSet
 *          Change the state of the Batch state machine
 *
 ** @param  BatchStateType_t - stateType : new state type
 *
 * @return  OSP_STATUS_OK or error code 
 *
 ***************************************************************************************************/
int16_t BatchStateSet( BatchStateType_t stateType )
{
    int16_t errCode = OSP_STATUS_INVALID_PARAMETER;

    /* Check state type is valid */
    if (stateType < NUM_BATCH_STATES)
    {
        /* Check state machine is initialized */
        if (!BatchStateMachine.IsInitialized)
        {
            return OSP_STATUS_UNINITIALIZED;
        }
        /* Update state to new state */
        BatchStateMachine.PreState = BatchStateMachine.CurState;
        BatchStateMachine.CurState = stateType;

        DPRINTF("BatchState:  changing state from State (%d) to State (%d).\r\n", 
                BatchStateMachine.PreState, 
                BatchStateMachine.CurState );
        
        errCode = OSP_STATUS_OK;
    }

    return errCode;
}


/****************************************************************************************************
 * @fn      BatchStateGet
 *          Get current state of the Batch state machine
 *
 * @param  BatchStateType_t  *pCurStat 
 *
 * @return  OSP_STATUS_OK or error code 
 *
 ***************************************************************************************************/
int16_t BatchStateGet( BatchStateType_t  *pCurState)
{
    int16_t errCode = OSP_STATUS_OK;
    
    /* Check state machine is initialized */
    if (!BatchStateMachine.IsInitialized)
    {
        return ( OSP_STATUS_UNINITIALIZED );
    }
    /* Read current state and Update */
    *pCurState = BatchStateMachine.CurState;

    return errCode;
}


/****************************************************************************************************
 * @fn      BatchStateCommandValid
 *          Check command is valid for current state
 *
 ** @param  BatchCmdList_t cmdType : value of command to be check
 *
 * @return  OSP_STATUS_OK or error code 
 *
 ***************************************************************************************************/
int16_t BatchStateCommandValidate( BatchCmdList_t cmdType )
{
    int16_t errCode ;
    BatchStateType_t  currState;

    /* Check command type is valid */
    if ( cmdType < CMD_NUM_MAX)
    {
        /* Check state machine is initialized */
        if (!BatchStateMachine.IsInitialized)
        {
            return ( OSP_STATUS_UNINITIALIZED );
        }
        else
        {
            /* Read current state */ 
            currState = BatchStateMachine.CurState;

            /* Check lookup table for command is valid for state */
            if ( !StateCommandTable[currState][cmdType] )
            {
                return ( OSP_STATUS_INVALID_PARAMETER );
            }
            errCode = OSP_STATUS_OK;
        }
    }

    return errCode;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
