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
#include <stdlib.h>


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void ASFMessagingInit( void );
extern uint8_t GetTaskList( uint8_t **pTaskList );

extern Bool gExitFromStandby;

#define STACK_INCREASE                  0


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Declare all the thread function prototypes here */
#define ASF_TASK_DEF_TYPE ASF_TASK_DECLARE
#include "ASF_TaskDefType.h"
#include "ASF_Tasks.h"

/**
 * Declare the task stack
 */
#define ASF_TASK_DEF_TYPE ASF_STACK_SETUP
#include "ASF_TaskDefType.h"
#include "ASF_Tasks.h"


/**
 * Declare the queues associated with each task
 */
#define ASF_TASK_DEF_TYPE ASF_QUEUE_SETUP
#include "ASF_TaskDefType.h"
#include "ASF_Tasks.h"


/**
 * This is the task initialization table which details all the information
 * pulled from ASF_TASK_STATIC/ ASF_TASK_DYNAMIC that is required to create
 * the tasks.
 * NOTE: this array is marked as constant so that it is placed in ROM.
 */
#define ASF_TASK_DEF_TYPE ASF_TASK_SETUP
#include "ASF_TaskDefType.h"
const AsfTaskInitDef C_gAsfTaskInitTable[NUMBER_OF_TASKS] =
{
#include "ASF_Tasks.h"
};

/**
 * This table will hold the Task handle (RTX task type)corresponding to
 * the TaskId for each task. This is initialized during AsfInitialiseTasks()
 */
AsfTaskHandle asfTaskHandleTable[NUMBER_OF_TASKS];

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* IMPORTANT: The total stack needed must include stack sizes of all tasks that can be created in
   a given mode. */
#define ASF_TASK_DEF_TYPE ASF_TOTAL_STACK_NEEDED
#include "ASF_TaskDefType.h"
const uint32_t TotalStkNeeded =
(
    128 /* System overhead */
#include "ASF_Tasks.h"
);

/* Heap Area defined here */
U64 NewHeap[TotalStkNeeded/8] = {0};


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
 * @fn      InitializeTasks
 *          Called from initial task to spawn the rest of the tasks in the system
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void InitializeTasks( void )
{
    uint8_t  taskCounter, numTasks;
    TaskId tid;
    uint8_t *pTaskTable;
    uint32_t *pU64Aligned;

    /* Create tasks  based on the mode we are in */
    numTasks = GetTaskList( &pTaskTable );
    for (taskCounter = 0; taskCounter < numTasks; taskCounter++)
    {
        tid = (TaskId)pTaskTable[taskCounter];

        if (tid != INSTR_MANAGER_TASK_ID)
        {
            /* Allocate task stack from heap */
            /* NOTE: All mallocs are 8-byte aligned as per ARM stack alignment requirements */
            pU64Aligned = malloc( C_gAsfTaskInitTable[tid].stackSize );
            ASF_assert( pU64Aligned != NULL );
            ASF_assert( ((uint32_t)pU64Aligned & 0x7) == 0 ); //Ensure 64-bit aligned

            asfTaskHandleTable[tid].handle  = os_tsk_create_user( C_gAsfTaskInitTable[tid].entryPoint,
                C_gAsfTaskInitTable[tid].priority, pU64Aligned, C_gAsfTaskInitTable[tid].stackSize);
            ASF_assert( asfTaskHandleTable[tid].handle != 0 );
            asfTaskHandleTable[tid].stkSize = C_gAsfTaskInitTable[tid].stackSize;
            asfTaskHandleTable[tid].pStack = pU64Aligned; /* Keep track of our stack pointer */
        }

        /* Initialize the associated queue */
        if (asfTaskHandleTable[tid].handle != 0)
        {
            os_mbx_init( C_gAsfTaskInitTable[tid].queue, C_gAsfTaskInitTable[tid].queueSize );
        }
    }

    /* Initialize the messaging */
    ASFMessagingInit();

    /* Switch the priority to be lowest now */
    os_tsk_prio_self( C_gAsfTaskInitTable[INSTR_MANAGER_TASK_ID].priority );
}


/****************************************************************************************************
 * @fn      AsfInitialiseTasks
 *          This function creates all the Tasks (via the initialTask) as defined in the
 *          C_gAsfTaskInitTable and starts the RTX ticking. This function should be the last to be
 *          called from main() as it will not return.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void AsfInitialiseTasks ( void )
{
    uint32_t *pU64Aligned;

    /* NOTE: All mallocs are 8-byte aligned as per ARM stack alignment requirements */
    pU64Aligned = malloc( C_gAsfTaskInitTable[INSTR_MANAGER_TASK_ID].stackSize );
    ASF_assert( ((uint32_t)pU64Aligned & 0x7) == 0 ); //Ensure 64-bit aligned

    asfTaskHandleTable[INSTR_MANAGER_TASK_ID].handle  = 1; //Initial task always gets this OS_ID
    asfTaskHandleTable[INSTR_MANAGER_TASK_ID].stkSize = C_gAsfTaskInitTable[INSTR_MANAGER_TASK_ID].stackSize;
    asfTaskHandleTable[INSTR_MANAGER_TASK_ID].pStack = pU64Aligned;

    /* Initialize RTX and start initialTask */
    os_sys_init_user( InstrManagerTask, 254, pU64Aligned, C_gAsfTaskInitTable[INSTR_MANAGER_TASK_ID].stackSize );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
