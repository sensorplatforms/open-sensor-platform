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
#include "stm32f10x_it.h"
#include "common.h"
#include "debugprint.h"
#include "osp-sensors.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void RxBytesToBuff( PortInfo *pPort, uint8_t byte );

#if defined I2C_DRIVER
void I2C_Driver_ISR_Handler(void);
void I2C_Driver_ERR_ISR_Handler(void);
#endif
#ifdef ANDROID_COMM_TASK
void I2C_Slave_Handler(I2C_TypeDef *pI2C, uint8_t irqCh);
#endif

void SendDataReadyIndication( uint8_t sensorId, uint32_t timeStamp );

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
 |    Cortex-M3 Processor Exceptions Handlers
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
        for (i = 0; i < 500000; i++);
        LED_Off( HARD_FAULT_LED );
        for (i = 0; i < 500000; i++);
    }
}


/****************************************************************************************************
 * @fn      MemManage_Handler
 *          This function handles Memory Manage exception.
 *
 ***************************************************************************************************/
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}


/****************************************************************************************************
 * @fn      BusFault_Handler
 *          This function handles Bus Fault exception.
 *
 ***************************************************************************************************/
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}


/****************************************************************************************************
 * @fn      UsageFault_Handler
 *          This function handles Usage Fault exception.
 *
 ***************************************************************************************************/
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}


/****************************************************************************************************
 * @fn      DebugMon_Handler
 *           This function handles Debug Monitor exception.
 *
 ***************************************************************************************************/
void DebugMon_Handler(void)
{
}


/*-------------------------------------------------------------------------------------------------*\
 |    STM32F10x Peripherals Interrupt Handlers
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      EXTI3_IRQHandler
 *          This function handles External interrupt Line 3 request.
 *
 ***************************************************************************************************/
void EXTI3_IRQHandler(void)
{
    // !!WARNING!!: The time stamp extension scheme implemented in osp-api.c will need to be changed
    // if timer capture is used for sensor time-stamping. Current scheme will cause time jumps if two
    // sensors are timer-captured before & after rollover but the sensor that was captured after
    // rollover is queued before the sensor that was captured before timer rollover
    if (EXTI_GetFlagStatus(EXTI_LINE_ACCEL_INT) != RESET)
    {
        /* Clear the EXTI line pending bit */
        EXTI_ClearFlag(EXTI_LINE_ACCEL_INT);
        /* Send indication to Sensor task */
        SendDataReadyIndication(ACCEL_INPUT_SENSOR, RTC_GetCounter());
        /* NOTE: No I2C transactions (rd/wr) allowed from ISR */
    }
}


/****************************************************************************************************
 * @fn      DMA1_Channel4_IRQHandler
 *          This function handles DMA1 Channel 4 interrupt request.
 *
 ***************************************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{
#if defined STEVAL_MKI109V2
# ifdef UART_DMA_ENABLE
    void *pNewBuf;
    uint8_t *pPrintBuf;

    if(DMA_GetFlagStatus(DMA1_FLAG_TC4))
    {
        DMA_ClearFlag(DMA1_FLAG_TC4);

        /* Disable DMA Channel */
        DisableDbgUartDMAChannel();

        /*Get the next print buffer */
        pNewBuf = GetNextBuffer(&gDbgUartPort);
        if (pNewBuf != NULL)
        {
            pPrintBuf = M_GetBuffStart(pNewBuf);
            UartDMAConfiguration(&gDbgUartPort, pPrintBuf, M_GetBuffLen(pNewBuf));
            /* Enable DMA Channel Transfer complete interrupt */
            gDbgUartPort.EnableDMAxferCompleteInt();

            /* Enable DMA Channel for TX */
            gDbgUartPort.EnableDMAChannel();
        }
    }
# endif //UART_DMA_ENABLE
#endif
}


/****************************************************************************************************
 * @fn      I2C1_EV_IRQHandler
 *          This function handles I2C1 Event interrupt request.
 *
 ***************************************************************************************************/
void I2C1_EV_IRQHandler(void)
{
#if defined I2C_DRIVER && (I2C_SENSOR_BUS_EVENT_IRQ_CH == I2C1_EV_IRQn)
    I2C_Driver_ISR_Handler();
#endif
}


/****************************************************************************************************
 * @fn      I2C1_ER_IRQHandler
 *          This function handles I2C1 Error interrupt request.
 *
 ***************************************************************************************************/
void I2C1_ER_IRQHandler(void)
{
#if defined I2C_DRIVER && (I2C_SENSOR_BUS_ERROR_IRQ_CH == I2C1_ER_IRQn)
    I2C_Driver_ERR_ISR_Handler();
#endif
}


/****************************************************************************************************
 * @fn      I2C2_EV_IRQHandler
 *          This function handles I2C2 Event interrupt request.
 *
 ***************************************************************************************************/
void I2C2_EV_IRQHandler(void)
{
#if defined ANDROID_COMM_TASK && (I2C_SLAVE_BUS_EVENT_IRQ_CH == I2C2_EV_IRQn)
    I2C_Slave_Handler( I2C2, I2C_SLAVE_BUS_EVENT_IRQ_CH );
#endif
}


/****************************************************************************************************
 * @fn      I2C2_ER_IRQHandler
 *          This function handles I2C2 Error interrupt request.
 *
 ***************************************************************************************************/
void I2C2_ER_IRQHandler(void)
{
#if defined ANDROID_COMM_TASK && (I2C_SLAVE_BUS_ERROR_IRQ_CH == I2C2_ER_IRQn)
    I2C_Slave_Handler( I2C2, I2C_SLAVE_BUS_ERROR_IRQ_CH );
#endif
}


/****************************************************************************************************
 * @fn      USART1_IRQHandler
 *          This function handles USART1 global interrupt request.
 *
 ***************************************************************************************************/
void USART1_IRQHandler(void)
{
#if defined STEVAL_MKI109V2
    uint8_t nextByte;

    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        nextByte = DbgUartReadByte();
        RxBytesToBuff( &gDbgUartPort, nextByte );
    }

    /** If there's more data waiting to be transmitted, get the
    * next byte and start transmission. Otherwise, disable the
    * TX interrupt. It will be re-enabled again when sioWrite()
    * is called to transmit new data.
    */
# ifndef UART_DMA_ENABLE
    if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET)
    {
        if (GetNextByteToTx( &nextByte ))
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART1, nextByte);
        }
        else
        {
            /* Disable the USART1 Transmit interrupt */
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
    }
# endif
#endif //STEVAL_MKI109V2
}


/****************************************************************************************************
 * @fn      EXTI15_10_IRQHandler
 *          This function handles External lines 15 to 10 interrupt request.
 *
 ***************************************************************************************************/
void EXTI15_10_IRQHandler(void)
{
    // !!WARNING!!: The time stamp extension scheme implemented in osp-api.c will need to be changed
    // if timer capture is used for sensor time-stamping. Current scheme will cause time jumps if two
    // sensors are timer-captured before & after rollover but the sensor that was captured after
    // rollover is queued before the sensor that was captured before timer rollover
    if (EXTI_GetFlagStatus(MAG_RDY_INT_EXTI_LINE) != RESET)
    {
        /* Clear the EXTI line pending bit */
        EXTI_ClearFlag(MAG_RDY_INT_EXTI_LINE);
        SendDataReadyIndication(MAG_INPUT_SENSOR, RTC_GetCounter());
        /* NOTE: No I2C transactions (rd/wr) allowed from ISR */
    }

    if (EXTI_GetFlagStatus(GYRO_RDY_INT_EXTI_LINE) != RESET)
    {
        /* Clear the EXTI line pending bit */
        EXTI_ClearFlag(GYRO_RDY_INT_EXTI_LINE);
        /* Send indication to Sensor task */
        SendDataReadyIndication(GYRO_INPUT_SENSOR, RTC_GetCounter());
        /* NOTE: No I2C transactions (rd/wr) allowed from ISR */
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
