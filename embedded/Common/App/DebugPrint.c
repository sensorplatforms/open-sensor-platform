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
#include "DebugPrint.h"
#include "ASF_TaskStruct.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern osp_bool_t ValidInputData( uint8_t inByte );

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
PortInfo gDbgUartPort;      //Debug information port

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/**
 * Declare separate pool for debug printf messages
 */
#ifdef UART_DMA_ENABLE
# define DPRINTF_MPOOL_SIZE         (DPRINTF_BUFF_SIZE + 8)
_declare_box( gMemPoolDprintf, DPRINTF_MPOOL_SIZE, MAX_DPRINTF_MESSAGES);
#endif


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

#ifdef UART_DMA_ENABLE
/****************************************************************************************************
 * @fn      AddToList
 *          Adds the buffer to list and starts DMA transfer if this was the first buffer
 *
 ***************************************************************************************************/
void AddToList( PortInfo *pPort, void *pPBuff, uint16_t length )
{
    /* New printf buffers will be added to the list of buffers that will eventually be used for
       DMAing out the data */
    int wasMasked;
    Address *pTemp;
    Address *pObj = M_GetBuffBlock(pPBuff);
    M_SetBuffLen(pObj, length);

    wasMasked = __disable_irq();
    if (pPort->pHead == NULL)
    {
        pPort->pTail = pPort->pHead = pObj; //First DWORD is reserved for list management
        M_NextBlock(pObj) = NULL;

        /* Start the first DMA Xfer */
        UartDMAConfiguration( pPort, pPBuff, length );

        /* Enable UART DMA TX request */
        pPort->EnableDMATxRequest();

        /* Enable DMA Channel Transfer complete interrupt */
        pPort->EnableDMAxferCompleteInt();

        /* Enable UART DMA channel */
        pPort->EnableDMAChannel();
    }
    else
    {
        pTemp = pPort->pTail;
        pPort->pTail = pObj;
        M_NextBlock(pObj) = NULL;
        M_NextBlock(pTemp) = pObj;
    }
    if (!wasMasked) __enable_irq();
}


/****************************************************************************************************
 * @fn      RemoveFromList
 *          Removes head object (in FIFO order) from list and returns pointer to the list object
 *
 ***************************************************************************************************/
void *RemoveFromList( PortInfo *pPort )
{
    /* At this point the DMA has consumed this buffer. It will be removed from the list and the
       returned buffer pointer will be used to free the printf buffer. The DMA completion handler
       should call this function, free the memory and then use the buffer pointed by the Head pointer */
    void *pTemp;
    int wasMasked;
    ASF_assert(pPort->pHead != NULL);
    wasMasked = __disable_irq();
    pTemp = pPort->pHead;
    pPort->pHead = (void *)M_NextBlock(pPort->pHead); //If this is the last element then spHead will now be NULL
    /* Here we should check if spHead is NULL and correspondingly set spTail to NULL but since
       we probably won't check for spTail == NULL, we skip that step here. */
    if (!wasMasked) __enable_irq();
    return pTemp;
}
#endif


/****************************************************************************************************
 * @fn      DebugPortInit
 *          Initializes the Debug Uart port data structure.
 *
 ***************************************************************************************************/
void DebugPortInit( void )
{
    _init_box( gMemPoolDprintf, sizeof(gMemPoolDprintf), DPRINTF_MPOOL_SIZE );
    gDbgUartPort.pBuffPool = gMemPoolDprintf;
    gDbgUartPort.rxWriteIdx = 1;
    gDbgUartPort.rxReadIdx  = 0;
    gDbgUartPort.rcvTask = CMD_HNDLR_TASK_ID;
#ifndef UART_DMA_ENABLE
    gDbgUartPort.txReadIdx  = 0;
    gDbgUartPort.txWriteIdx = 1;
#else
    gDbgUartPort.pHead = NULL;
    gDbgUartPort.pTail = NULL;
    gDbgUartPort.DMAChannel = DBG_UART_TX_DMA_Channel;
    gDbgUartPort.UartBaseAddress = DBG_UART_DR_Base;
    gDbgUartPort.ValidateInput = ValidInputData;
    /* Note functions can be empty but not NULL (coz we dont' check for null) */
    gDbgUartPort.EnableDMAChannel = EnableDbgUartDMAChannel;
    gDbgUartPort.EnableDMATxRequest = EnableDbgUartDMATxRequest;
    gDbgUartPort.EnableDMAxferCompleteInt = EnableDbgUartDMAxferCompleteInt;
#endif
}

/****************************************************************************************************
 * @fn      RxBytesToBuff
 *          This function receives bytes into the RX buffer.  It is called from the receive ISR
 *
 * @param   byte received byte
 *
 * @return  none
 *
 ***************************************************************************************************/
void RxBytesToBuff( PortInfo *pPort, uint8_t byte )
{
    uint16_t  left;
    uint16_t  readIdx, writeIdx;

    /* Snapshot the two index values for local use. */
    readIdx = pPort->rxReadIdx;
    writeIdx = pPort->rxWriteIdx;

    /* Check if enough room in the buffer to store the new data. */
    left = readIdx - writeIdx;
    if(readIdx < writeIdx)
    {
        left += RX_BUFFER_SIZE;
    } /* Here, ulLeft should be correct (between 0 and RX_BUFFER_SIZE). */

    if ((left > 0) && pPort->ValidateInput(byte))
    {
        pPort->rxBuffer[writeIdx] = byte;

        /* Check if a task is waiting for it. */
        if (pPort->rcvTask != UNKNOWN_TASK_ID)
        {
            if (byte == '\r' || byte == '\n')
            {
                isr_evt_set( UART_CRLF_RECEIVE, asfTaskHandleTable[pPort->rcvTask].handle );
            }
            else
            {
                /* Wake up the task. */
                isr_evt_set( UART_CMD_RECEIVE, asfTaskHandleTable[pPort->rcvTask].handle );
            }
        }

        if (writeIdx < RX_BUFFER_SIZE-1)
        {
            ++writeIdx;
        }
        else
        {
            writeIdx = 0;
        }
    }

    /* Update the port control block values */
    pPort->rxWriteIdx = writeIdx;
}


#ifdef UART_DMA_ENABLE
/****************************************************************************************************
 * @fn      GetNextBuffer
 *          Called from DMA TC ISR to request next buffer to process
 *
 ***************************************************************************************************/
void *GetNextBuffer( PortInfo *pPort )
{
    void *pFreeBuff = RemoveFromList( pPort );
    ASF_assert(_free_box( pPort->pBuffPool, pFreeBuff ) == 0); //Free the current consumed buffer
    return pPort->pHead; //Return the current head of the list
}
#endif

/****************************************************************************************************
 * @fn      Print_LIPS
 *          Helper function that prints the LibFM Inline Parseable Serial protocol for external use
 *
 * @param   fmt printf style variable length parameters
 *
 * @return  length of the string printed.
 *
 ***************************************************************************************************/
int Print_LIPS( const char *fmt, ... )
{
    va_list args;
    uint16_t i, len = 0;
    int8_t *pPrintBuff;
    PortInfo *pPort = &gDbgUartPort;
    uint16_t maxsize = DPRINTF_BUFF_SIZE - 8 - 4; /*4 for header & footer, 2 for "\r\n", 4 for
                                                       checksum value, 1 for ',' & 1 for null termination */
    uint8_t chkSum = 0;
#if defined UART_DMA_ENABLE
    int8_t *pNewBuff;
#else
    //Not implemented
#endif

    va_start( args, fmt );

#if defined UART_DMA_ENABLE
    /* Note: Output will be truncated to allowed max size */
    pNewBuff = _alloc_box(pPort->pBuffPool);
    ASF_assert( pNewBuff != NULL );
    pPrintBuff = M_GetBuffStart(pNewBuff);
    if (pPrintBuff != NULL)
    {
        pPrintBuff[0] = '{';
        pPrintBuff[1] = '!';
        len = vsnprintf( (char*)&pPrintBuff[2], maxsize, fmt, args );

        ASF_assert( (len > 0) && (len < maxsize)); //Catch truncation of messages

        /* Checksum is XOR checksum calculated after the first '!' and upto but not inc. the ',' before
        the checksum itself */
        for (i = 0; i < len; i++)
        {
            chkSum ^= pPrintBuff[2+i];
        }

        len += 2; //account for header

        len += snprintf( (char*)&pPrintBuff[len], 11, ",0x%02X,!}\r\n", chkSum );
        /* Add the print buffer to list */
        AddToList( pPort, pPrintBuff, len );
    }

    return len;
#else
    return 0; //Not implemented!
#endif
}


/****************************************************************************************************
 * @fn      _dprintf
 *          Helper function that replaces printf functionality to dump the printf messages on UART
 *
 * @param   fmt printf style variable length parameters
 *
 * @return  length of the string printed.
 *
 ***************************************************************************************************/
int _dprintf( uint8_t dbgLvl, const char *fmt, ... )
{
    va_list args;
    const PortInfo *pPort = &gDbgUartPort;
#ifdef UART_DMA_ENABLE
    uint16_t len = 0;
    int8_t *pNewBuff, *pPrintBuff;
#else
    
#endif

    switch( dbgLvl )
    {
#if (DEBUG_LVL < 1)
        case 1:
            return 0;
#endif
#if (DEBUG_LVL < 2)
        case 2:
            return 0;
#endif

        default:
            va_start( args, fmt );

#ifdef UART_DMA_ENABLE
            /* Note: Output will be truncated to allowed max size */
            pNewBuff = _alloc_box(pPort->pBuffPool);
            ASF_assert( pNewBuff != NULL );
            if (pNewBuff != NULL)
            {
                pPrintBuff = M_GetBuffStart(pNewBuff);
                len = vsnprintf( (char*)pPrintBuff, DPRINTF_BUFF_SIZE, fmt, args );

                ASF_assert( len > 0 );

                /* Add the print buffer to list */
                AddToList( (PortInfo*)pPort, pPrintBuff, len );
                return len;
            }
            return 0;
#else
            return 0;
#endif
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
