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
#if !defined (TYPES_H)
#define   TYPES_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <stdint.h> /* Try to use std int types in the application code */
#include "RTL.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

#define NULLP               ((void *) 0)
#ifndef true
# define true               (1 == 1)
#endif
#ifndef false
# define false              (!true)
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Standard typedefs */
typedef double              DBL;
typedef float               FLT;
typedef unsigned char       Bool;

/*----------------------------------------------------------------------------*\
 | Task Handle type. This type is OS-dependent.
\*----------------------------------------------------------------------------*/
typedef struct AsfTaskHandleTag {
    OS_TID   handle;
    void     *pStack;
    uint16_t stkSize;
} AsfTaskHandle;

/*----------------------------------------------------------------------------*\
 | Semaphore ID type. This type is OS-dependent.
\*----------------------------------------------------------------------------*/
typedef OS_SEM* AsfSemIdType;

/*----------------------------------------------------------------------------*\
 | Timer ID type. This type is OS-dependent.
\*----------------------------------------------------------------------------*/
typedef OS_ID TimerId;


#endif /* TYPES_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
