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
#if !defined (DEBUGPRINT_H)
#define   DEBUGPRINT_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Common.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern PortInfo gDbgUartPort;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Platform/Device dependent macros */
void UartDMAConfiguration( PortInfo *pPort, uint8_t *pTxBuffer, uint16_t txBufferSize );

#ifndef UART_DMA_ENABLE
Bool GetNextByteToTx( uint8_t* pucByte );
#else
void *GetNextBuffer( PortInfo *pPort );
#endif

/* Support functions for DEBUG Uart */
static __inline void DisableDbgUartInterrupt( void ) {
    NVIC_DisableIRQ(DBG_UART_IRQChannel);
}

static __inline void EnableDbgUartInterrupt( void ) {
    NVIC_EnableIRQ(DBG_UART_IRQChannel);
}

#ifdef UART_DMA_ENABLE
static __inline void EnableDbgUartDMAxferCompleteInt( void ) {
    DMA_ITConfig(DBG_UART_TX_DMA_Channel, DMA_IT_TC, ENABLE);
}

static __inline void EnableDbgUartDMAChannel( void ) {
    DMA_Cmd(DBG_UART_TX_DMA_Channel, ENABLE);
}

static __inline void DisableDbgUartDMAChannel( void ) {
    DMA_Cmd(DBG_UART_TX_DMA_Channel, DISABLE);
}

static __inline void EnableDbgUartDMATxRequest( void ) {
    USART_DMACmd(DBG_IF_UART, USART_DMAReq_Tx, ENABLE);
}
#endif

static __inline void DbgUartSendByte( uint8_t byte ) {
    USART_SendData(DBG_IF_UART, byte);
}

static __inline uint8_t DbgUartReadByte( void ) {
    return (uint8_t) (USART_ReceiveData(DBG_IF_UART) & 0xFF);
}

static __inline Bool DbgUartTransmitBufferEmpty( void ) {
    if (USART_GetFlagStatus(DBG_IF_UART, USART_FLAG_TXE) != RESET) {
        return true;
    }
    return false;
}

static __inline void EnableDbgUartTxBufferEmptyInterrupt( void ) {
    USART_ITConfig(DBG_IF_UART, USART_IT_TXE, ENABLE);
}

static __inline void DisableDbgUartTxBufferEmptyInterrupt( void ) {
    USART_ITConfig(DBG_IF_UART, USART_IT_TXE, DISABLE);
}



#endif /* DEBUGPRINT_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
