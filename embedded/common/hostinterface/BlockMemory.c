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
#include "BlockMemory.h"
#include "osp-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct _BlkMem {
    void *pFree;                    /* Pointer to first free memory block       */
    void *pEnd;                     /* Pointer to memory block end              */
    uint32_t  BlkSz;                /* Memory block size                        */
    uint32_t  TotalCnt;             /* Total memory blocks in the pool          */
    uint32_t  UsedCnt;              /* Total allocated block count              */
} BlkMem_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      InitBlockPool
 *          Initialize the memory pool with the given block sizes. Memory pool can be a static array
 *          or one time allocation from a bigger pool/heap. Pool pointer must be 32-bit aligned.
 *          Memory pool must be initialized using this call after a DECLARE_BLOCK_POOL
 *
 * @param   [IN]pPool - Pointer (32-bit aligned) to the memory pool that needs to be initialized
 * @param   [IN]poolSize - Total size of the memory pool in bytes that will be divided into blocks
 * @param   [IN]blkSize - Individual memory block sizes that will be allocated from the pool
 *
 * @return  OSP_STATUS_OK if initialization successful, OSP_STATUS_INVALID_PARAMETER otherwise
 *
 ***************************************************************************************************/
int16_t InitBlockPool( void *pPool, uint32_t poolSize, uint32_t blkSize )
{
    /* Initialize memory block system */
    BlkMem_t *pBlkPool = (BlkMem_t*)pPool;
    void *pEnd;
    void *pBlk;
    void *pNext;

    /* Adjust block size to 32-bit boundary */
    blkSize = (blkSize + 3) & ~3;
    if (blkSize == 0)
    {
        return (OSP_STATUS_INVALID_PARAMETER);
    }

    if ((blkSize + sizeof(BlkMem_t)) > poolSize)
    {
        return (OSP_STATUS_INVALID_PARAMETER);
    }

    /* Initialize the block pool structure. */
    pBlk = ((uint8_t*)pPool) + sizeof(BlkMem_t);
    pBlkPool->pFree = pBlk;
    pEnd = ((uint8_t*)pPool) + poolSize;
    pBlkPool->pEnd = pEnd;
    pBlkPool->BlkSz = blkSize;
    pBlkPool->TotalCnt = 0;
    pBlkPool->UsedCnt = 0;

    /* Link all free blocks using offsets. */
    pEnd = ((uint8_t*)pEnd) - blkSize;
    while (1)
    {
        pNext = ((uint8_t*)pBlk) + blkSize;
        pBlkPool->TotalCnt++;
        if (pNext > pEnd)
            break;
        *((void **)pBlk) = pNext;
        pBlk = pNext;
    }

    /* End marker */
    *((void **)pBlk) = 0;
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      AllocBlock
 *          Allocate a memory block from the given memory pool
 *
 * @param   [IN]pPool - Pointer to the memory pool from which fixed size memory block is requested
 *
 * @return  Pointer to the allocated memory block
 *
 ***************************************************************************************************/
void *AllocBlock( void *pPool )
{
    /* Allocate a memory block and return start address. */
    void **free;
    SETUP_CRITICAL_SECTION();

    ENTER_CRITICAL_SECTION();
    free = ((BlkMem_t*)pPool)->pFree;
    if (free)
    {
        ((BlkMem_t*)pPool)->pFree = *free;
        ((BlkMem_t*)pPool)->UsedCnt++;
    }
    EXIT_CRITICAL_SECTION();
    return (free);
}


/****************************************************************************************************
 * @fn      FreeBlock
 *          Return to the memory pool a block of memory that was previously allocated from it.
 *
 * @param   [IN]pPool - Pointer to the memory pool to which the previously allocated memory block is
 *              being returned
 * @param   [IN]pBlock - Pointer to the memory block that is being returned to the pool
 *
 * @return  OSP_STATUS_OK if successful, OSP_STATUS_INVALID_PARAMETER otherwise
 *
 ***************************************************************************************************/
int16_t FreeBlock( void *pPool, void *pBlock )
{
    SETUP_CRITICAL_SECTION();
    /* Check if the block belongs to pool before trying to free it */
    if ((pBlock < pPool) || (pBlock >= ((BlkMem_t*)pPool)->pEnd))
    {
        return (OSP_STATUS_INVALID_PARAMETER);
    }

    ENTER_CRITICAL_SECTION();
    *((void **)pBlock) = ((BlkMem_t*)pPool)->pFree;
    ((BlkMem_t*)pPool)->pFree = pBlock;
    ((BlkMem_t*)pPool)->UsedCnt--;
    EXIT_CRITICAL_SECTION();
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      GetPoolStats
 *          Returns the total block count and used blocks count from the given pool
 *
 * @param   [IN]pPool - Pointer to the memory pool that has been initialized
 * @param   [OUT]pTotalCount - return the total number of blocks in the given pool
 * @param   [OUT]pUsedCount - return the current used block count
 *
 * @return  none
 *
 ***************************************************************************************************/
void GetPoolStats( void *pPool, uint32_t *pTotalCount, uint32_t *pUsedCount )
{
    SETUP_CRITICAL_SECTION();
    ENTER_CRITICAL_SECTION();
    if (pTotalCount)
    {
        *pTotalCount = ((BlkMem_t*)pPool)->TotalCnt;
    }
    if (pUsedCount)
    {
        *pUsedCount = ((BlkMem_t*)pPool)->UsedCnt;
    }
    EXIT_CRITICAL_SECTION();
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
