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
#if !defined (QUEUE_H)
#define   QUEUE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <stdint.h>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* Macro to calculate required buffer size to allocate from given packet size. Use this size to
 * allocate buffers before queuing
 */
#define M_CalcBufferSize(sz)            (sz + sizeof(BufferHeader_t))
#define M_GetBufferDataStart(bufptr)    ((uint8_t*)((uint8_t*)bufptr + sizeof(BufferHeader_t)))

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Callback function type */
typedef void (*fpQueueEvtCallback_t)( void *pUser );

typedef enum {
    QUEUE_EMPTY_CB,          //Queue empty callback id
    QUEUE_LOW_THRESHOLD_CB,  //Callback id for low threshold
    QUEUE_HIGH_THRESHOLD_CB, //Callback id for high threshold
    QUEUE_FULL_CB,           //Callback id for queue full
    NUM_CB_IDS
} Q_CBId_t;

/* Buffer structure for use in the queue for forming linked list */
typedef struct _BufferHeader {
    uint8_t  *pNext;     //Pointer to next buffer
    uint32_t Length;     //Store length of the payload
} BufferHeader_t;

typedef struct _Buffer {
    BufferHeader_t Header;
    uint8_t        DataStart;   //Place holder for data payload start
} Buffer_t;

/* General purpose queue structure. Holds any buffer or packet defined as Buffer_t */
typedef struct _Queue {
    Buffer_t    *pHead;     //Head
    Buffer_t    *pTail;     //Tail
    uint32_t    Capacity;   //Max capacity of the queue (application defined)
    uint32_t    LowThres;   //Used to trigger event/callback when this number is hit while dequeue
    uint32_t    HighThres;  //Used to trigger event/callback when this number is hit while enqueue
    uint32_t    Size;       //Number of buffers currently on the queue
    fpQueueEvtCallback_t pfCB[NUM_CB_IDS];
    void        *pCbArg[NUM_CB_IDS];
} Queue_t;


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
Queue_t *QueueCreate( uint32_t capacity, uint32_t lowThreshold, uint32_t highThreshold );
int16_t EnQueue( Queue_t *myQ, Buffer_t *pBuf );
int16_t DeQueue( Queue_t *myQ, Buffer_t **pBuf );
int16_t QueueRegisterCallBack( Queue_t *pMyQ, Q_CBId_t cbid, fpQueueEvtCallback_t pFunc, void *pUser );
int16_t QueueHighThresholdSet( Queue_t *pMyQ, uint32_t highThreshold );
int16_t QueueGetSize( Queue_t *pMyQ, uint32_t *size );

#endif /* QUEUE_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
