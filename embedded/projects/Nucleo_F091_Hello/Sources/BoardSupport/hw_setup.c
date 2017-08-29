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
#include "hw_setup.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern RCC_ClocksTypeDef gRccClockInfo;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
osp_bool_t gExitFromStandby = false;
DeviceUid_t *gDevUniqueId = (DeviceUid_t *)(DEV_UID_OFFSET);

GPioPinInfo_t DiagLEDs[NUM_LEDS] = {
    { //LED_LD2
        RCC_AHBPeriph_GPIOA,
        GPIOA,
        GPIO_Pin_5
    },
    { //LED_LD3
        RCC_AHBPeriph_GPIOC,
        GPIOC,
        GPIO_Pin_8
    },
};


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
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      LED_Init
 *          Configures the diagnostic LEDs in the system
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void LED_Init( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    uint8_t index;

    if (NUM_LEDS > 0)
    {
        for (index = 0; index < NUM_LEDS; index++)
        {
            RCC_AHBPeriphClockCmd( DiagLEDs[index].rccPeriph, ENABLE );
            GPIO_InitStructure.GPIO_Pin = DiagLEDs[index].pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            GPIO_Init(DiagLEDs[index].grp, &GPIO_InitStructure);
        }
    }
}


/****************************************************************************************************
 * @fn      SystemGPIOConfig
 *          Configures the various GPIO ports on the chip according to the usage by various
 *          peripherals.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void SystemGPIOConfig( void )
{
}


/****************************************************************************************************
 * @fn      SystemInterruptConfig
 *          Configures the nested vectored interrupt controller.
 *
 ***************************************************************************************************/
void SystemInterruptConfig( void )
{
    /* Initialize EXTI registers to known state */
    EXTI_DeInit();
}


/****************************************************************************************************
 * @fn      RTC_Configuration
 *          Configures the RTC (or any general purpose timer used for the purpose).
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void RTC_Configuration( void )
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    uint16_t PrescalerValue = 0;

    /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RTC_TIMER_CLK_RCC, ENABLE);

    /* Compute the prescaler value */
    PrescalerValue = (uint16_t)RTC_PRESCALAR;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 0xFFFFFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 0; //Done separately below
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(RTC_TIMER, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_PrescalerConfig(RTC_TIMER, PrescalerValue, TIM_PSCReloadMode_Immediate);

    /* Enable Timer counter */
    TIM_Cmd(RTC_TIMER, ENABLE);
}


/****************************************************************************************************
 * @fn      RTC_GetCounter
 *          Returns the counter value of the timer register
 *
 * @param   none
 *
 * @return  32-bit counter value
 *
 ***************************************************************************************************/
int32_t RTC_GetCounter( void )
{
    return TIM_GetCounter( RTC_TIMER );
}


/****************************************************************************************************
 * @fn      DebugUARTConfig
 *          Configures the selected USART for UART operation with the given baud rate parameters.
 *          Note that flow control is not supported due to I/O limitations.
 *
 * @param   baud    Baud rate value
 * @param   dataLen Word length of the serial data
 * @param   stopBits Number of stop bits
 * @param   parity  Parity - Odd/Even/None
 *
 * @return  none
 *
 * @see     none
 ***************************************************************************************************/
void DebugUARTConfig( uint32_t baud, uint32_t dataLen, uint32_t stopBits, uint32_t parity )
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Setup the USART configuration structure */
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = dataLen;
    USART_InitStructure.USART_StopBits = stopBits;
    USART_InitStructure.USART_Parity = parity;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Need to enable the GPIO and USART clocks first */
    RCC_AHBPeriphClockCmd( RCC_Periph_UART_GPIO, ENABLE );
    RCC_ClkFtn_DBG_UART( RCC_Periph_DBG_UART, ENABLE );

    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(DBG_UART_GPIO_TX_GRP, DBG_UART_TX_PIN_SRC, DBG_UART_TX_AF);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(DBG_UART_GPIO_RX_GRP, DBG_UART_RX_PIN_SRC, DBG_UART_RX_AF);
  
    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = DBG_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(DBG_UART_GPIO_TX_GRP, &GPIO_InitStructure);

    /* Configure USART Rx as Alternate function. OType is ignored */
    GPIO_InitStructure.GPIO_Pin = DBG_UART_RX_PIN;
    GPIO_Init(DBG_UART_GPIO_RX_GRP, &GPIO_InitStructure);

#ifdef UART_DMA_ENABLE
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_DBG_UART_Periph_DMA, ENABLE);

    DMA_DeInit(DBG_UART_TX_DMA_Channel);

    /* Enable the DMA1 Channel 4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DBG_UART_TX_DMA_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPriority = DBG_UART_DMA_INT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Remap USART1 Tx DMA requests from channel2 to channel4 */
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    //SYSCFG_DMAChannelRemapConfig( SYSCFG_DMARemap_USART1Tx, ENABLE );
#endif

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DBG_UART_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPriority = DBG_UART_INT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USART Receive interrupt: this interrupt is generated when the
     USART receive data register is not empty. Transmit interrupt is enabled in write routine */
    USART_ITConfig(DBG_IF_UART, USART_IT_RXNE, ENABLE);

    /* Configure the selected USART port */
    USART_Init(DBG_IF_UART, &USART_InitStructure);

    /* Enable the selected USART */
    USART_Cmd(DBG_IF_UART, ENABLE);
}


#ifdef UART_DMA_ENABLE
/****************************************************************************************************
 * @fn      UartDMAConfiguration
 *          Configures the DMA for use with the selected UART
 *
 ***************************************************************************************************/
void UartDMAConfiguration( PortInfo *pPort, uint8_t *pTxBuffer, uint16_t txBufferSize )
{
    DMA_InitTypeDef DMA_InitStructure;

    /* UART TX DMA  Channel (triggered by USART Tx event) Config */
    DMA_InitStructure.DMA_PeripheralBaseAddr = pPort->UartBaseAddress;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pTxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = txBufferSize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(pPort->DMAChannel, &DMA_InitStructure);
}
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
