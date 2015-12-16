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
#if !defined (BLOCK_MEMORY_H)
#define   BLOCK_MEMORY_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "common.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SETUP_CRITICAL_SECTION()        OS_SETUP_CRITICAL()
#define ENTER_CRITICAL_SECTION()        OS_ENTER_CRITICAL()
#define EXIT_CRITICAL_SECTION()         OS_LEAVE_CRITICAL()

/* The DECLARE_BLOCK_POOL macro declares an array of bytes that can be used as a memory pool for fixed
 * block allocation.
 * @param [IN]pool - User defined unique C-Style name of the pool
 * @param [IN]size - Size in bytes of each memory block that will be allocated from the pool
 * @param [IN]cnt - Number of blocks the pool should contain
 */
#define DECLARE_BLOCK_POOL(pool,size,cnt)     uint32_t pool[(((size)+3)/4)*(cnt) + 5]
/* Note: +5 is to account for size (in 32-bit values) of local block structure (BlkMem_t) */


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
int16_t InitBlockPool( void *pPool, uint32_t poolSizeBytes, uint32_t blkSize );
void *AllocBlock( void *pPool );
int16_t FreeBlock( void *pPool, void *pBlock );
void GetPoolStats( void *pPool, uint32_t *pTotalCount, uint32_t *pUsedCount );


#endif /* BLOCK_MEMORY_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
