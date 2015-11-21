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
#include "Queue.h"
#include "BlockMemory.h"
#include <string.h> //for memset


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define NUM_APPLICATION_QUEUES          NUM_QUEUE_TYPE

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static uint8_t QPoolInitialized = 0;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
DECLARE_BLOCK_POOL( QObjectPool, sizeof(Queue_t), NUM_APPLICATION_QUEUES );

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      QueueCreate
 *          Creates a queue object with the given parameters. No memory is allocated for the given
 *          capacity. It only serves as a control point. Buffers must be allocated by application
 *
 * @param   [IN]capacity - Max capacity of the queue
 * @param   [IN]lowThreshold - Queue size low threshold. Used to trigger low threshold callback
 * @param   [IN]highThreshold - Queue size high threshold. Used to trigger high threshold callback
 *
 * @return  Pointer to the queue object created. Null otherwise.
 *
 ***************************************************************************************************/
Queue_t *QueueCreate( uint32_t capacity, uint32_t lowThreshold, uint32_t highThreshold )
{
    Queue_t *pQ;

    /* Sanity check capacity and threshold values */
    if ((capacity <= lowThreshold) || (capacity <= highThreshold) || (lowThreshold >= highThreshold))
    {
        return NULL;
    }

    /* Initialize the memory pool for the queue objects */
    if (!QPoolInitialized)
    {
        InitBlockPool( QObjectPool, sizeof(QObjectPool), sizeof(Queue_t) );
        QPoolInitialized = 1;
    }

    /* Allocate memory for queue object */
    pQ = AllocBlock(QObjectPool);
    if (pQ == NULL)
    {
        return NULL;
    }
    else
    {
        /* It is important to clear the queue structure */
        memset( pQ, 0, sizeof(Queue_t));
    }

    /* Initialize queue */
    pQ->Capacity = capacity;
    pQ->HighThres = highThreshold;
    pQ->LowThres = lowThreshold;

    /* Return reference to the queue */
    return pQ;
}


/****************************************************************************************************
 * @fn      EnQueue
 *          Called by application to enqueue the given buffer in the queue (FIFO order). Note that
            buffers must be allocated & filled by application before queuing.
 *
 * @param   [IN]pMyQ - Pointer to a queue previously created
 * @param   [IN]pBuf - Pointer to user buffer that needs to be queued. This must remain valid until
 *                     dequeued!
 *
 * @return  OSP_STATUS_OK or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t EnQueue( Queue_t *pMyQ, Buffer_t *pBuf )
{
    SETUP_CRITICAL_SECTION();

    ENTER_CRITICAL_SECTION();
    /* Check if queue is already full */
    if (pMyQ->Size == pMyQ->Capacity)
    {
        EXIT_CRITICAL_SECTION();
        return (OSP_STATUS_QUEUE_FULL);
    }

    /* Enqueue the buffer */
    if (pMyQ->Size == 0)
    {
        pMyQ->pHead = pBuf; //Add to head
    }
    else
    {
        pMyQ->pTail->Header.pNext = (uint8_t*)pBuf;
    }
    /* Update links */
    pMyQ->pTail = pBuf;
    pBuf->Header.pNext = NULL;
    pMyQ->Size++;

    /* Check for high threshold */
    if ((pMyQ->HighThres < pMyQ->Capacity) && (pMyQ->Size == pMyQ->HighThres))
    {
        EXIT_CRITICAL_SECTION();

        /* Invoke high threshold callback if registered */
        if (pMyQ->pfCB[QUEUE_HIGH_THRESHOLD_CB] != NULL)
        {
            pMyQ->pfCB[QUEUE_HIGH_THRESHOLD_CB]( pMyQ->pCbArg[QUEUE_HIGH_THRESHOLD_CB] );
            return OSP_STATUS_OK;
        }
        /* Note that the status of OSP_STATUS_QUEUE_HIGH_THRESHOLD is returned only if CB is not provided */
        return (OSP_STATUS_QUEUE_HIGH_THRESHOLD);
    }

    /* Check if queue is full to its capacity */
    if (pMyQ->Size == pMyQ->Capacity)
    {
        EXIT_CRITICAL_SECTION();
        /* Invoke callback if one was registered */
        if (pMyQ->pfCB[QUEUE_FULL_CB] != NULL)
        {
            pMyQ->pfCB[QUEUE_FULL_CB]( pMyQ->pCbArg[QUEUE_FULL_CB] );
        }
        return OSP_STATUS_OK;
    }
    EXIT_CRITICAL_SECTION();

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      DeQueue
 *          Called by application to dequeue the oldest buffer in the queue (FIFO order)
 *
 * @param   [IN]pMyQ - Pointer to a queue previously created
 * @param   [OUT]pBuf - Pointer that returns the dequeued buffer
 *
 * @return  OSP_STATUS_OK or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t DeQueue( Queue_t *pMyQ, Buffer_t **pBuf )
{
    Buffer_t *pTemp;
    SETUP_CRITICAL_SECTION();

    ENTER_CRITICAL_SECTION();
    if (pMyQ->Size == 0)
    {
        EXIT_CRITICAL_SECTION();

        *pBuf = NULL;
        return (OSP_STATUS_QUEUE_EMPTY);
    }
    else
    {
        pTemp = pMyQ->pHead;
        pMyQ->pHead = (Buffer_t*)pTemp->Header.pNext;
        *pBuf = pTemp; //set return pointer

        /* update tail if this was the last buffer dequeued */
        if (pMyQ->pHead == NULL)
        {
            pMyQ->pTail = NULL;
        }

        pMyQ->Size--;
        /* Invoke relevant callbacks... */
        if (pMyQ->Size == 0)
        {
            EXIT_CRITICAL_SECTION();

            /* Invoke queue empty callback if registered */
            if (pMyQ->pfCB[QUEUE_EMPTY_CB] != NULL)
            {
                pMyQ->pfCB[QUEUE_EMPTY_CB]( pMyQ->pCbArg[QUEUE_EMPTY_CB] );
            }
            return OSP_STATUS_OK;
        }

        if ((pMyQ->LowThres > 0) && (pMyQ->Size == pMyQ->LowThres))
        {
            EXIT_CRITICAL_SECTION();

            /* Invoke low threshold callback if registered */
            if (pMyQ->pfCB[QUEUE_LOW_THRESHOLD_CB] != NULL)
            {
                pMyQ->pfCB[QUEUE_LOW_THRESHOLD_CB]( pMyQ->pCbArg[QUEUE_LOW_THRESHOLD_CB] );
                return OSP_STATUS_OK;
            }
            return (OSP_STATUS_QUEUE_LOW_THRESHOLD);
        }
    }

    EXIT_CRITICAL_SECTION();
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      QueueRegisterCallBack
 *          Allows user to register callback for queue related events (low/high threshold, empty/full)
 *          Callbacks can be registered one at a time.
 *
 * @param   [IN]pMyQ - Pointer to a queue previously created
 * @param   [IN]cbid - Identifier for the callback being registered
 * @param   [IN]pFunc - Callback function pointer
 * @param   [IN]pUser - User provided argument that is passed to the callback when invoked.
 *
 * @return  OSP_STATUS_OK if callback was registered; OSP_STATUS_INVALID_PARAMETER otherwise
 *
 ***************************************************************************************************/
int16_t QueueRegisterCallBack( Queue_t *pMyQ, Q_CBId_t cbid, fpQueueEvtCallback_t pFunc, void *pUser )
{
    if ((pFunc != NULL) && (cbid < NUM_CB_IDS))
    {
        pMyQ->pfCB[cbid] = pFunc;
        pMyQ->pCbArg[cbid] = pUser;
        return OSP_STATUS_OK;
    }
    return (OSP_STATUS_INVALID_PARAMETER);
}


/****************************************************************************************************
 * @fn      QueueHighThresholdSet
 *          Allows user to change high threshold value for given Q. This value should always be less
 *          than the Q capacity.
 *
 * @param   [IN]pMyQ - Pointer to a queue previously created
 * @param   [IN]highThreshold - High Threshold value to be set
 *
 * @return  OSP_STATUS_OK if threshold set; OSP_STATUS_INVALID_PARAMETER otherwise
 *
 ***************************************************************************************************/
int16_t QueueHighThresholdSet( Queue_t *pMyQ, uint32_t highThreshold )
{
    SETUP_CRITICAL_SECTION();

    /* Check high threshold value it should not be greater than or equal to Q capacity */
    if ( pMyQ->Capacity <= highThreshold )
    {
        return (OSP_STATUS_INVALID_PARAMETER);
    }

    ENTER_CRITICAL_SECTION();

    /* Set high threshold value */
    pMyQ->HighThres = highThreshold;

    EXIT_CRITICAL_SECTION();

    return OSP_STATUS_OK;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
