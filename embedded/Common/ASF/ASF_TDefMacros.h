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
/* This file maybe included more than once */

/*
** ASF_TASK_DEF_TYPE should be defined to be either
**
**     ASF_TASK_IDS
**     ASF_QUEUE_IDS
**     ASF_STACK_SETUP
**     ASF_TASK_SETUP
**     ASF_TASK_DECLARE
**
** by the including file
*/


/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#if !defined (IN_ASF_TASK_DEF)
# error This file can only be included in ASF_TaskDefType.h
#endif

#if ASF_TASK_DEF_TYPE == ASF_TASK_IDS
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize ) ThreadId,
#endif

#if ASF_TASK_DEF_TYPE == ASF_STACK_SETUP
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize )           \
    const uint32_t ThreadId##_StkSize = StackSize;
#endif

#if ASF_TASK_DEF_TYPE == ASF_QUEUE_SETUP
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize )           \
    os_mbx_declare( Q_##ThreadId, QueueSize );
#endif

#if ASF_TASK_DEF_TYPE == ASF_TASK_DECLARE
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize )             \
       extern void EntryFunction(void);
#endif

#if ASF_TASK_DEF_TYPE == ASF_TASK_SETUP
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize )             \
        { ThreadId, EntryFunction, #EntryFunction, Q_##ThreadId, sizeof(Q_##ThreadId), StackSize, Priority, #ThreadId },
#endif

#if ASF_TASK_DEF_TYPE == ASF_TOTAL_STACK_NEEDED
# define ASF_TASK_STATIC( ThreadId, EntryFunction, Priority, StackSize, QueueSize )           \
    +ThreadId##_StkSize
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
