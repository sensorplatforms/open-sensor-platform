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
#include "common.h"
#include "debugprint.h"
#include "osp-sensors.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void RxBytesToBuff( PortInfo *pPort, uint8_t byte );
void I2C_IRQHandler(LPC_I2C_T *pI2C);
void SPISlaveIRQHandler(void);

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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    printf("!!HARD FAULT!!");
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
        __IO U32 i;
        //Fast blink the front LED
        LED_On( HARD_FAULT_LED );
        for (i = 0; i < 500000; i++);
        LED_Off( HARD_FAULT_LED );
        for (i = 0; i < 500000; i++);
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles DMA interrupts.
  * @param  None
  * @retval None
  */
void DMA_IRQHandler( void )
{
#ifdef UART_DMA_ENABLE
    void *pNewBuf;
    uint8_t *pPrintBuf;
    uint32_t intA;
#endif


    /* Error interrupt on channel */
    if ((Chip_DMA_GetIntStatus(LPC_DMA) & DMA_INTSTAT_ACTIVEERRINT) != 0)
    {
#ifdef UART_DMA_ENABLE
        /* This shouldn't happen normally. We can do an abort. */
        Chip_DMA_DisableChannel(LPC_DMA, gDbgUartPort.DMAChannel);
        while ((Chip_DMA_GetBusyChannels(LPC_DMA) & (1 << gDbgUartPort.DMAChannel)) != 0) {}
        Chip_DMA_AbortChannel(LPC_DMA, gDbgUartPort.DMAChannel);
        Chip_DMA_ClearErrorIntChannel(LPC_DMA, gDbgUartPort.DMAChannel);
        Chip_DMA_EnableChannel(LPC_DMA, gDbgUartPort.DMAChannel);
#endif
    }
    
    /* Check for active interrupt & clear DMA interrupt for the channel */
    if ((Chip_DMA_GetIntStatus(LPC_DMA) & DMA_INTSTAT_ACTIVEINT) != 0)
    {
#ifdef UART_DMA_ENABLE
        intA = LPC_DMA->DMACOMMON[0].INTA;
        if ( (intA & BIT_POSITION(gDbgUartPort.DMAChannel)) != 0 )
        {
            Chip_DMA_ClearActiveIntAChannel(LPC_DMA, gDbgUartPort.DMAChannel);
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
#endif
    }
}


/**
  * @brief  This function handles UART0 interrupts.
  * @param  None
  * @retval None
  */
void UART0_IRQHandler(void)
{
    uint32_t intsrc;
    uint8_t nextByte;

    /* Determine the interrupt source */
    intsrc = Chip_UART_GetIntStatus(DBG_IF_UART);

    // Receive Data Available or Character time-out
    if (intsrc & UART_INTEN_RXRDY)
    {
        /* Read one byte from the receive data register */
        nextByte = DbgUartReadByte();
        RxBytesToBuff( &gDbgUartPort, nextByte );
    }

#ifndef UART_DMA_ENABLE
    // Transmit Holding Empty
    if (intsrc & UART_INTEN_TXRDY)
    {
        if (GetNextByteToTx( &nextByte ))
        {
            /* Write one byte to the transmit data register */
            DbgUartSendByte(nextByte);
        }
        else
        {
            // Disable Transmit interrupt
            DisableDbgUartTxBufferEmptyInterrupt();
        }
    }
#endif

}


/****************************************************************************************************
 * @fn      ACCEL_IRQHandler
 *          ISR Handler for Accelerometer DRDY input. See hw_setup_xpresso_lpc54102.h for mapping
 *
 ***************************************************************************************************/
void ACCEL_IRQHandler(void)
{
    uint32_t currTime = RTC_GetCounter();

    Chip_PININT_ClearIntStatus(LPC_PININT, ACCEL_PINT_CH);

    SendDataReadyIndication(ACCEL_INPUT_SENSOR, currTime);

    /* The following logic maybe used to decimate Accel data rate if higher BW signal is required */
    //static uint32_t sampleCount = 0;
    //if ((sampleCount++ % 5) == 0) //Decimate 250Hz to 50Hz
    //{
    //    uint32_t timeStamp = RTC_GetCounter();
    //    SendDataReadyIndication( SENSOR_TYPE_ACCELEROMETER, timeStamp );
    //}
}


/****************************************************************************************************
 * @fn      MAG_IRQHandler
 *          ISR Handler for Magnetometer DRDY input. See hw_setup_xpresso_lpc54102.h for mapping
 *
 ***************************************************************************************************/
void MAG_IRQHandler(void)
{
    uint32_t currTime = RTC_GetCounter();

    Chip_PININT_ClearIntStatus(LPC_PININT, MAG_PINT_CH);

    SendDataReadyIndication(MAG_INPUT_SENSOR, currTime);
}


/****************************************************************************************************
 * @fn      GYRO_IRQHandler
 *          ISR Handler for Gyro DRDY input. See hw_setup_xpresso_lpc54102.h for mapping
 *
 ***************************************************************************************************/
void GYRO_IRQHandler(void)
{
    uint32_t currTime = RTC_GetCounter();

    Chip_PININT_ClearIntStatus(LPC_PININT, GYRO_PINT_CH);

    SendDataReadyIndication(GYRO_INPUT_SENSOR, currTime);
}


/****************************************************************************************************
 * @fn      SPI1_IRQHandler
 *          ISR Handler for SPI1 interface
 *
 ***************************************************************************************************/
void SPI1_IRQHandler(void)
{
    SPISlaveIRQHandler();
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
