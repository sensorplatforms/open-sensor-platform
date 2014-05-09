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
#include "common.h"
#include "asf_taskstruct.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern const AsfTaskInitDef C_gAsfTaskInitTable[NUMBER_OF_TASKS];

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define MESSAGE_BLOCK_SIZE  (sizeof(MessageBlock))

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/**
 * The _declare_box macro declares an array of bytes that can be used as a memory pool for fixed
 * block allocation.
 */
_declare_box( mpool, ///< this memory pool will be used to allocate the messages
    MESSAGE_BLOCK_SIZE, /**< this is the size of the regular messages.
          To avoid variable length, we allocate from this fixed size. If memory usage become issue
          we can divide the messages among various pool size that would optimize memory usage */
    MAX_SYSTEM_MESSAGES   //< Max (non dprintf) messages in the system
    );


/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      ASFDeleteMessage
 *          This function releases the buffer memory associated with the message contents. The caller
 *          must call this function after a message's contents have been read to free the memory
 *          associated with the message.
 *
 * @param   pMbuf Message buffer pointer containing the message to be deleted.
 *
 * @return  none
 *
 * @see     ASFCreateMessage(), ASFSendMessage(), ASFReceiveMessage()
 ***************************************************************************************************/
static void _ASFDeleteMessage ( MessageBuffer **pMbuf, char *_file, int _line )
{
    MessageBlock *pBlock;

    /** This is where we release the memory allocated when the message was created */
    if (*pMbuf != NULLP)
    {
        /* Get the block pointer */
        M_GetMsgBlockFromBuffer (pBlock, *pMbuf);

        ASF_assert( _free_box( mpool, pBlock ) == 0 );
    }

    *pMbuf = NULLP;
}



/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      ASFMessagingInit
 *          Initializes the messaging scheme in the system
 *
 ***************************************************************************************************/
void ASFMessagingInit( void )
{
    _init_box( mpool, sizeof(mpool), MESSAGE_BLOCK_SIZE );
}


/****************************************************************************************************
 * @fn      ASFCreateMessage
 *          This function is called to create (allocate) the space for the message type specified.
 *          This is the first step before the message parameters are filled in and the message is
 *          sent via the ASFSendMessage() call.
 *
 * @param   msgId   Message ID of the message to be created.
 * @param   msgSize size (in bytes) of the message corresponding to the ID. This is typically the
 *                  sizeof structure corresponding to the message.
 *
 * @return  pMbuf   This pointer returns the allocated buffer space for the message type specified.
 *
 * @see     ASFSendMessage(), ASFReceiveMessage()
 ***************************************************************************************************/
AsfResult_t _ASFCreateMessage( MessageId msgId, uint16_t msgSize, MessageBuffer **pMbuf, char *_file, int _line )
{
    MessageBlock   *pBlock;

    /* At this time it is assumed that the memory pool for message allocation has been
       created and initialized */
    ASF_assert_var( *pMbuf == NULLP, msgId, 0, 0 );

    pBlock = _alloc_box(mpool);
    if (pBlock == NULLP) return ASF_ERR_MSG_BUFF;

    pBlock->header.length = msgSize;
    pBlock->rec.msgId     = msgId;

    /* Set the user message buffer now */
    *pMbuf = (MessageBuffer *)&pBlock->rec;
    return ASF_OK;
}


/****************************************************************************************************
 * @fn      ASFSendMessage
 *          This function is called after a message has been created and the message parameters
 *          filled in. It sends the message to the destination task queue without blocking.
 *
 * @param   destTask  TaskId of the destination task which will receive the message.
 * @param   pMbuf Message buffer pointer containing the message to send.
 * @param   cntxt Specify if the message is being sent from ISR or Thread context
 *
 * @return  none
 *
 * @see     ASFCreateMessage(), ASFReceiveMessage()
 ***************************************************************************************************/
AsfResult_t _ASFSendMessage ( TaskId destTask, MessageBuffer *pMbuf, char *_file, int _line )
{
    MessageBlock *pBlock;
    OS_RESULT err;

    /* Check for the usual - null pointers etc. */
    ASF_assert_var( pMbuf != NULLP, pMbuf->msgId, 0, 0 );

    /* Get the block pointer */
    M_GetMsgBlockFromBuffer (pBlock, pMbuf);

    pBlock->header.destTask = destTask;

    /* Send the message without pending */
    if ( GetContext() != CTX_ISR )
    {
        err = os_mbx_send( C_gAsfTaskInitTable[destTask].queue, pMbuf, 0 );
        if (err != OS_R_OK) //Mailbox is not valid or full
        {
            ASF_assert( _free_box( mpool, pBlock ) == 0 );
            return ASF_ERR_Q_FULL;
        }
    }
    else
    {
        err = isr_mbx_check( C_gAsfTaskInitTable[destTask].queue );
        ASF_assert_var(err != 0, err, pMbuf->msgId, destTask);
        isr_mbx_send( C_gAsfTaskInitTable[destTask].queue, pMbuf );
    }
    return ASF_OK;
}


/****************************************************************************************************
 * @fn      ASFReceiveMessage
 *          This function blocks until a message is received on the queue of the calling task.
 *
 * @param   rcvTask  TaskId of the receiving task which will receive the message. This id must be
 *                   task's own id.
 *
 * @param   pMbuf Message buffer pointer that will return the message received. Note that the
 *                pointer to the memory allocated for the message contents is passed in the
 *                message buffer structure passed in.
 *
 * @return  none
 *
 * @see     ASFCreateMessage(), ASFSendMessage(), ASFDeleteMessage(), ASFReceiveMessagePoll()
 ***************************************************************************************************/
void _ASFReceiveMessage ( TaskId rcvTask, MessageBuffer **pMbuf, char *_file, int _line )
{
    OS_RESULT   err;

    /* Delete old/previous message to release its buffer */
    _ASFDeleteMessage( pMbuf, _file, _line );

    /* Wait for receive */
    err = os_mbx_wait( C_gAsfTaskInitTable[rcvTask].queue, (void **)pMbuf, OS_WAIT_FOREVER );
    ASF_assert_var(((err == OS_R_OK) || (err == OS_R_MBX)), err, 0, 0);
}


/****************************************************************************************************
 * @fn      ASFReceiveMessagePoll
 *          This function tries to receive a message on the queue of the calling task without blocking.
 *
 * @param   rcvTask  TaskId of the receiving task which will receive the message. This id must be
 *                   task's own id.
 *
 * @param   pMbuf Message buffer pointer that will return the message received. Note that the
 *                pointer to the memory allocated for the message contents is passed in the
 *                message buffer structure passed in.
 *
 * @return  true if message was received.
 *
 * @see     ASFCreateMessage(), ASFSendMessage(), ASFDeleteMessage(), ASFReceiveMessage()
 ***************************************************************************************************/
osp_bool_t _ASFReceiveMessagePoll ( TaskId rcvTask, MessageBuffer **pMbuf, char *_file, int _line )
{
    OS_RESULT   err;

    /* Delete old message to release its buffer */
    _ASFDeleteMessage( pMbuf, _file, _line );

    /* Wait for receive */
    err = os_mbx_wait( C_gAsfTaskInitTable[rcvTask].queue, (void **)pMbuf, OS_WAIT_NEVER );
    if (err == OS_R_TMO)
    {
        return false;
    }
    ASF_assert_var(((err == OS_R_OK) || (err == OS_R_MBX)), err, 0, 0);
    return true;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
