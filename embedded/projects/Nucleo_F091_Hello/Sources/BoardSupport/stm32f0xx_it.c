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
#include "stm32f0xx_it.h"
#include "common.h"
#include "debugprint.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void RxBytesToBuff( PortInfo *pPort, uint8_t byte );


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

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
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* NONE */

/*-------------------------------------------------------------------------------------------------*\
 |    Cortex-M4 Processor Exceptions Handlers
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      NMI_Handler
 *          This function handles NMI exception.
 *
 ***************************************************************************************************/
void NMI_Handler(void)
{
}


/****************************************************************************************************
 * @fn      HardFault_Handler
 *          This function handles Hard Fault exception.
 *
 ***************************************************************************************************/
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    printf("\r\n!! HARD FAULT !!");
    while (1)
    {
        __IO uint32_t i;
        //Fast blink the indicator LED
        LED_On( HARD_FAULT_LED );
        for (i = 0; i < 200000; i++);
        LED_Off( HARD_FAULT_LED );
        for (i = 0; i < 200000; i++);
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/* ### DEFINED BY RTX in HAL_CM1.C ### */

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* ### DEFINED BY RTX in HAL_CM1.C ### */




/*-------------------------------------------------------------------------------------------------*\
 |    STM32F0x Peripherals Interrupt Handlers
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      DBG_UART_TX_DMA_IRQHandler
 *          This function handles DMAx Channel interrupt request.
 *
 ***************************************************************************************************/
void DBG_UART_TX_DMA_IRQHandler(void)
{
#if defined NUCLEO_F091_BOARD
# ifdef UART_DMA_ENABLE
    void *pNewBuf;
    uint8_t *pPrintBuf;

    if(DMA_GetFlagStatus(UART_TX_DMA_TC_FLAG))
    {
        DMA_ClearFlag(UART_TX_DMA_TC_FLAG);

        /* Disable DMA Channel */
        DMA_Cmd(gDbgUartPort.DMAChannel, DISABLE);

        /*Get the next print buffer */
        pNewBuf = GetNextBuffer(&gDbgUartPort);
        if (pNewBuf != NULL)
        {
            pPrintBuf = M_GetBuffStart(pNewBuf);
            UartDMAConfiguration(&gDbgUartPort, pPrintBuf, M_GetBuffLen(pNewBuf));
            /* Enable DMA Channel Transfer complete interrupt */
            DMA_ITConfig(gDbgUartPort.DMAChannel, DMA_IT_TC, ENABLE);

            /* Enable the DMA Channel */
            DMA_Cmd(gDbgUartPort.DMAChannel, ENABLE);
        }
        else
        {
            /* Disable DMA Channel Transfer complete interrupt */
            //DMA_ITConfig(UART_TX_DMA_Channel, DMA_IT_TC, DISABLE);
        }
    }
# endif //UART_DMA_ENABLE
#endif
}

/**
  * @brief  This function handles Debug UART global interrupt request.
  * @param  None
  * @retval None
  */
void DBG_UART_IRQHandler(void)
{
#if defined NUCLEO_F091_BOARD
    uint8_t nextByte;

    if(USART_GetFlagStatus(DBG_IF_UART, USART_FLAG_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        nextByte = DbgUartReadByte();
        RxBytesToBuff( &gDbgUartPort, nextByte );
    }

    /* Check for Overrun Error - if this occurs and is not cleared then the
       System is stuck in servicing UART interrupt forever */
    if(USART_GetFlagStatus(DBG_IF_UART, USART_FLAG_ORE) != RESET)
    {
        USART_ClearFlag(DBG_IF_UART, USART_FLAG_ORE);
        D0_printf("!!Warning!! Uart Overrun\r\n");
    }


# ifndef UART_DMA_ENABLE
    /** If there's more data waiting to be transmitted, get the
    * next byte and start transmission. Otherwise, disable the
    * TX interrupt. It will be re-enabled again when sioWrite()
    * is called to transmit new data.
    */
    if (USART_GetFlagStatus(DBG_IF_UART, USART_FLAG_TXE) != RESET)
    {
        if (GetNextByteToTx( &nextByte ))
        {
            /* Write one byte to the transmit data register */
            USART_SendData(DBG_IF_UART, nextByte);
        }
        else
        {
            /* Disable the USART1 Transmit interrupt */
            USART_ITConfig(DBG_IF_UART, USART_IT_TXE, DISABLE);
        }
    }
# endif
#endif //NUCLEO_F091_BOARD
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
