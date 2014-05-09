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
#include "Common.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern uint16_t os_time;               //RTX internal

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
RtcClock_t gRtcClockData;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static uint32_t sInitialTime;

static AsfTimer sRtcUpdateTimer = NULL_TIMER;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      UpdateRTC
 *          Updates RTC clock based on the os_time value. os_time is the free running system tick
 *          counter
 *
 ***************************************************************************************************/
static void UpdateRTC( void )
{
    uint32_t currTime;

    currTime = os_time;
    /* Check for rollover */
    if (currTime < sInitialTime)
    {
        currTime += 65536; //os_time is 16bit
    }

    if ((currTime - sInitialTime) >= TICS_PER_SEC)
    {
        gRtcClockData.seconds += (currTime-sInitialTime)/TICS_PER_SEC;
        gRtcClockData.msec = (currTime-sInitialTime)%TICS_PER_SEC * MSEC_PER_TICK;
        sInitialTime = os_time; //reset for next iteration

        if (gRtcClockData.seconds >= 60)
        {
            gRtcClockData.minutes += gRtcClockData.seconds/60;
            gRtcClockData.seconds = (gRtcClockData.seconds%60);
        }

        if (gRtcClockData.minutes >= 60)
        {
            gRtcClockData.hours += gRtcClockData.minutes/60;
            gRtcClockData.minutes = (gRtcClockData.minutes%60);
        }
    }

    D2_printf("\r\nTIME: %d:%02d:%02d.%d\r\n", gRtcClockData.hours, gRtcClockData.minutes,
        gRtcClockData.seconds, gRtcClockData.msec);
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      InstrManagerUserInit
 *          This handler is called from the instrumentation task to do application specific
 *          instrumentation initialization that should happen before the task loop
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void InstrManagerUserInit( void )
{
    sInitialTime = os_time;

    ASFTimerStart( INSTR_MANAGER_TASK_ID, TIMER_REF_RTC_UPDATE, TICS_PER_SEC, &sRtcUpdateTimer );
}


/****************************************************************************************************
 * @fn      InstrManagerUserHandler
 *          This handler is called from the instrumentation task loop to handle application specific
 *          instrumentation requirements.
 *
 * @param   none
 *
 * @return  true if message was handled by the user, false otherwise
 *
 ***************************************************************************************************/
osp_bool_t InstrManagerUserHandler( MessageBuffer *pMsg )
{
    osp_bool_t msgHandled = false;

    switch (pMsg->msgId)
    {
    case MSG_TIMER_EXPIRY: //Note: this message is common so we don't mark it handled
        switch (pMsg->msg.msgTimerExpiry.userValue)
        {
        case TIMER_REF_RTC_UPDATE:
            UpdateRTC();

            /* Restart timer for periodic output */
            ASFTimerStart( INSTR_MANAGER_TASK_ID, TIMER_REF_RTC_UPDATE, TICS_PER_SEC, &sRtcUpdateTimer );
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return msgHandled;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
