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


/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/*
** ASF_TASK_DEF_TYPE should be defined to be either
**
**     ASF_TASK_IDS
**     ASF_QUEUE_SETUP
**     ASF_STACK_SETUP
**     ASF_TASK_SETUP
**     ASF_TASK_DECLARE
**
** by the including file
*/
#define ASF_TASK_IDS        1
#define ASF_STACK_SETUP     2
#define ASF_QUEUE_SETUP     3
#define ASF_TASK_SETUP      4
#define ASF_TASK_DECLARE    5

#define IN_ASF_TASK_DEF

#if defined (ASF_TASK_STATIC)
# undef ASF_TASK_STATIC
#endif

/***************************************************************************
 * Include OS specific macros
 **************************************************************************/

#  include "ASF_TDefMacros.h"


#if !defined (ASF_TASK_STATIC)
# error ASF_TASK_STATIC must be defined
#endif

#undef ASF_TASK_DEF_TYPE
#undef IN_ASF_TASK_DEF

#define ASF_TASK        __task
#define ASF_TASK_ARG    void *argv


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
