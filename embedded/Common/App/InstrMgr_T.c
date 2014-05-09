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
#include "ASF_TaskStruct.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern const uint16_t C_gOsStkSize;
extern uint32_t gStackMem;
extern uint32_t gStackSize;

extern const AsfTaskInitDef C_gAsfTaskInitTable[NUMBER_OF_TASKS];
extern uint32_t gSystemRTCRefTime;
extern struct OS_TCB os_idle_TCB; //RTX internal
extern void *os_active_TCB[];     //RTX internal

extern void InitializeTasks( void );
extern uint8_t GetTaskList( uint8_t **pTaskList );

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define i_printf           D1_printf

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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      InstrManagerTask
 *          This task is for instrumentation. At startup initializes
 *          other tasks and OS resources in the system. This task is created by the main OS entry
 *          function called from main().
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK void InstrManagerTask( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;
    osp_bool_t msgHandled;

    /* Create other tasks & OS resources in the system */
    InitializeTasks();

    /* Call user init related to instrumentation (for stuff needing to be done before the while loop) */
    InstrManagerUserInit();

    while(1)
    {
        /* User instrumentation such as LED indications, RTC updates, etc. */
        ASFReceiveMessage( INSTR_MANAGER_TASK_ID, &rcvMsg );

        msgHandled = InstrManagerUserHandler( rcvMsg );

        if (!msgHandled)
        {
            switch (rcvMsg->msgId)
            {
            case MSG_TIMER_EXPIRY:
                break;

            default:
                /* Unhandled messages */
                D1_printf("INSTR:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
                break;

            }
        }
    }
}






/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
