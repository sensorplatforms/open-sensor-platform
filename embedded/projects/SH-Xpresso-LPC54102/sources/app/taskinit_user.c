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
#include "common.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* These are the tasks that will be created on startup. This list allows you to disable creation of
   one or more tasks in the system if so desired. Allows to have a different run modes with different
   task selection list */
const uint8_t NormalModeTaskList[] = {
    INSTR_MANAGER_TASK_ID,
    CMD_HNDLR_TASK_ID,
    SENSOR_ACQ_TASK_ID,
    I2CSLAVE_COMM_TASK_ID,
    ALGORITHM_TASK_ID,
    ALG_BG_TASK_ID
};
const uint8_t NormalModeTaskListSize = sizeof(NormalModeTaskList);

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

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
 * @fn      GetTaskList
 *          Returns info on task list table and size based on application mode
 *
 * @param   pointer to task list
 *
 * @return  task count
 *
 ***************************************************************************************************/
uint8_t GetTaskList( uint8_t **pTaskList )
{
    *pTaskList = (uint8_t*)NormalModeTaskList;
    return NormalModeTaskListSize;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
