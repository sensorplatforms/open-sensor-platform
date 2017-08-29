/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2017 Knowles Electronics, LLC
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
#include "common.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern void *AsfTimerList[];
extern U32 const os_timernum;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Maintain the list of registered timer currently running */
static AsfTimer **_pAsfTimerList = (AsfTimer**)&AsfTimerList[0];

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      AddTimerToList
 *          Called by Timer Start to add timer in active timer list
 *
 * @param   pTimer  Pointer to the timer control block
 *
 * @return  Internal reference to the timer (index of the timer list)
 *
 ***************************************************************************************************/
static uint16_t AddTimerToList(AsfTimer *pTimer)
{
    uint16_t i;
    const uint16_t numTimers = os_timernum & 0xFFFF;

    for ( i = 0; i < numTimers; i++ )
    {
        if ( _pAsfTimerList[i] == NULL )
        {
            _pAsfTimerList[i] = pTimer;
            return i;
        }
    }

    /* More timers created than declared in rtx_conf_cm.c */
    ASF_assert(FALSE);
}


/****************************************************************************************************
 * @fn      GetTimerAndRemoveFromList
 *          Called by Timer Expiry function to remove timer for active timer list and return the 
 *          pointer to the timer that expired
 *
 * @param   info  Internal reference to the timer that expired
 *
 * @return  Pointer to the timer control block
 *
 ***************************************************************************************************/
static AsfTimer*  GetTimerAndRemoveFromList( uint16_t info)
{
    const uint16_t numTimers = os_timernum & 0xFFFF;
    AsfTimer *pTimer;

    if ( info < numTimers )
    {
        pTimer = _pAsfTimerList[info];
        _pAsfTimerList[info] = NULL;
        return pTimer;
    }

    return (AsfTimer*)NULL;
}


/****************************************************************************************************
 * @fn      RemoveTimerFromList
 *          Called by KillTimer to remove timer for active timer list
 *
 * @param   pTimer  Pointer to the timer control block
 *
 * @return  none
 *
 ***************************************************************************************************/
static void RemoveTimerFromList(AsfTimer *pTimer)
{
    uint16_t i;
    const uint16_t numTimers = os_timernum & 0xFFFF;

    for ( i = 0; i < numTimers; i++ )
    {
        if ( _pAsfTimerList[i] == pTimer )
        {
            _pAsfTimerList[i] = NULL;
            return;
        }
    }

    /* Timer not found! */
    ASF_assert(FALSE);
}


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

    ASF_assert( ASFCreateMessage( MSG_TIMER_EXPIRY, sizeof(MsgTimerExpiry), &pSendMsg ) == ASF_OK );
    pSendMsg->msg.msgTimerExpiry.userValue = pTimer->userValue;
    pSendMsg->msg.msgTimerExpiry.timerId   = pTimer->timerId;
    ASF_assert( ASFSendMessage( pTimer->owner, pSendMsg ) == ASF_OK );
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
static void _TimerStart ( AsfTimer *pTimer, char *_file, int _line )
{
    uint16_t info;
    OS_SETUP_CRITICAL();

    ASF_assert( pTimer != NULLP );
    ASF_assert( pTimer->sysUse != TIMER_SYS_ID ); //In case we are trying to restart a running timer
    OS_ENTER_CRITICAL();
    pTimer->sysUse = TIMER_SYS_ID;
    info = AddTimerToList(pTimer); // Add timer to list & get index
    OS_LEAVE_CRITICAL();
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
osp_bool_t ASFTimerStarted ( AsfTimer *pTimer )
{
    return (pTimer->sysUse == TIMER_SYS_ID? true : false);
}


/****************************************************************************************************
 * @fn      ASFTimerStart
 *          Creates a timer with given reference and tick value assigned to the owner.
 *
 * @param   owner  Task ID of the task that will receive the expiry message
 * @param   ref  Unique reference number for the timer
 * @param   tick  Tick count in OS ticks
 * @param   pTimer  Pointer to timer type
 *
 * @return  none
 *
 * @see     ASFTimerKill()
***************************************************************************************************/
void _ASFTimerStart( TaskId owner, uint16_t ref, uint16_t tick, AsfTimer *pTimer, char *_file, int _line  )
{
    pTimer->owner = owner;
    pTimer->ticks = tick;
    pTimer->userValue = ref;
    _TimerStart( pTimer, _file, _line );
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
    pTimer = GetTimerAndRemoveFromList(info);

    //Look for our magic number to be sure we got the right pointer
    ASF_assert_var( pTimer->sysUse == TIMER_SYS_ID,  pTimer->ticks, pTimer->userValue, pTimer->owner);
    pTimer->sysUse = (uint32_t)-1; //Timer no longer in use
    SendTimerExpiry( pTimer );
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
    OS_SETUP_CRITICAL();

    ASF_assert( pTimer != NULLP );
    ret = os_tmr_kill( pTimer->timerId );
    ASF_assert( ret == NULL );
    OS_ENTER_CRITICAL();
    pTimer->sysUse = (uint32_t)-1; //Timer no longer in use
    RemoveTimerFromList( pTimer );
    OS_LEAVE_CRITICAL();
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
