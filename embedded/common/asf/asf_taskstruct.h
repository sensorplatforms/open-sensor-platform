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
#if !defined (ASF_TASKSTRUCT_H)
#define   ASF_TASKSTRUCT_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#if !defined (ASF_TQENUM_H)
# include "asf_tqenum.h"
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/**
 * This structure is used to hold information about a task that needs to be
 * created at run time. This is the base unit of the C_gAsfTaskInitTable array.
 * NOTE: that this must hold only things that are known at compile time as
 * the C_gAsfTaskInitTable array is marked as constant i.e. in ROM.
 */
typedef struct AsfTaskInitDefTag
{
    TaskId      taskId;
    void        (*entryPoint)();
    char        *tskName;
    OS_ID       queue;
    uint16_t    queueSize;
    uint16_t    stackSize;
    uint8_t     priority;
    char        *tidString; /* String equivalent of the TASK_ID enum */

} AsfTaskInitDef;
 

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/**
 * This table holds the Task handle provided by the RTOS. The table is
 * filled in when the tasks are created, see ASFInitialiseTasks().
 */
extern AsfTaskHandle asfTaskHandleTable[NUMBER_OF_TASKS];

/**
 * This is the task initialization table which details all the information
 * that is required to create the tasks.
 * NOTE: this array is marked as constant i.e. in ROM.
 */
extern const AsfTaskInitDef C_gAsfTaskInitTable[NUMBER_OF_TASKS];


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
/**
 * This function is used to initialize all the defined tasks and their 
 * associated queues.
 */
void ASFInitialiseTasks ( void );


#endif /* ASF_TASKSTRUCT_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
