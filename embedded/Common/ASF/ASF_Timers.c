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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#ifndef RAM_START
# define RAM_START           NVIC_VectTab_RAM
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      SendTimerExpiry
 *          Sends the timer expiry message to the owner of the timer
 *
 * @param   pTimer  Pointer to the timer control block
 *
 * @return  none
 *
 ***************************************************************************************************/
static void SendTimerExpiry ( AsfTimer *pTimer )
{
    MessageBuffer *pSendMsg = NULLP;

    ASFCreateMessage( MSG_TIMER_EXPIRY, sizeof(MsgTimerExpiry), &pSendMsg );
    pSendMsg->msg.msgTimerExpiry.userValue = pTimer->userValue;
    pSendMsg->msg.msgTimerExpiry.timerId   = pTimer->timerId;
    ASFSendMessage( pTimer->owner, pSendMsg, CTX_ISR );
}



/****************************************************************************************************
 * @fn      ASFTimerStart
 *          Creates a new timer in the system with the given attributes.
 *
 * @param   pTimer  Pointer to timer control block containing the attributes of the timer to be
 *                  created.
 *
 * @return  none
 *
 * @see     ASFDeleteTimer()
 ***************************************************************************************************/
void _ASFTimerStart ( AsfTimer *pTimer, char *_file, int _line )
{
    uint16_t info = (uint16_t)((uint32_t)pTimer); //We only need to store the LSB16 of the pointer as the system RAM is < 64K
    ASF_assert( pTimer != NULLP );
    ASF_assert( pTimer->sysUse != TIMER_SYS_ID ); //In case we are trying to restart a running timer
    pTimer->sysUse = TIMER_SYS_ID;
    pTimer->timerId = os_tmr_create( pTimer->ticks, info );
    ASF_assert( pTimer->timerId != NULL );
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      ASFTimerStarted
 *          Checks if the timer has already been started
 *
 * @param   pTimer  Pointer to timer control block containing the attributes of the timer to be
 *                  created.
 *
 * @return  true - Timer already started; false otherwise
 *
 * @see     ASFTimerStart()
 ***************************************************************************************************/
Bool ASFTimerStarted ( AsfTimer *pTimer )
{
    return (pTimer->sysUse == TIMER_SYS_ID? true : false);
}


/****************************************************************************************************
 * @fn      TimerStart
 *          Creates a timer with given reference and tick value assigned to the owner.
 *
 * @param   owner  Task ID of the task that will receive the expiry message
 * @param   ref  Unique reference number for the timer
 * @param   tick  Tick count in OS ticks
 * @param   pTimer  Pointer to timer type
 *
 * @return  none
 *
 * @see     ASFTimerStart()
***************************************************************************************************/
void _TimerStart( TaskId owner, uint16_t ref, uint16_t tick, AsfTimer *pTimer, char *_file, int _line  )
{
    pTimer->owner = owner;
    pTimer->ticks = tick;
    pTimer->userValue = ref;
    _ASFTimerStart( pTimer, _file, _line );
}


/****************************************************************************************************
 * @fn      ASFTimerExpiry
 *          Handles the timer expiry by sending message to the task that created the timer
 *
 * @param   info  pseudo pointer to timer control block of the timer that expired.
 *
 * @return  none
 *
 * @see     ASFKillTimer()
 ***************************************************************************************************/
void _ASFTimerExpiry ( uint16_t info, char *_file, int _line )
{
    AsfTimer *pTimer;
    int wasMasked = __disable_irq();
    pTimer = (AsfTimer *)(RAM_START + info);
    //Look for our magic number to be sure we got the right pointer
    ASF_assert_var( pTimer->sysUse == TIMER_SYS_ID,  pTimer->ticks, pTimer->userValue, pTimer->owner);
    SendTimerExpiry( pTimer );
    pTimer->sysUse = (uint32_t)-1; //Timer no longer in use
    if (!wasMasked) __enable_irq();
}


/****************************************************************************************************
 * @fn      ASFKillTimer
 *          Kills the timer that was created earlier
 *
 * @param   pTimer  Pointer to timer control block containing the attributes of the timer to be
 *                  created.
 *
 * @return  none
 *
 * @see     ASFTimerStart()
 ***************************************************************************************************/
void _ASFKillTimer ( AsfTimer *pTimer, char *_file, int _line )
{
    TimerId ret;
    ASF_assert( pTimer != NULLP );
    ret = os_tmr_kill( pTimer->timerId );
    ASF_assert( ret == NULL );
    pTimer->sysUse = (uint32_t)-1; //Timer no longer in use
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
