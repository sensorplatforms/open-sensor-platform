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
uint32_t g_logging = 0; //0x40;

LedsInfo_t DiagLEDs[NUM_LEDS] = {
    { //LED_GREEN
        RCC_APB2Periph_GPIOC,
        GPIOC,
        GPIO_Pin_10
    },
    { //LED_RED
        RCC_APB2Periph_GPIOC,
        GPIOC,
        GPIO_Pin_11
    },
    { //LED_YELLOW
        RCC_APB2Periph_GPIOB,
        GPIOB,
        GPIO_Pin_5
    },
};

uint32_t AccelTimeExtend = 0;
uint32_t MagTimeExtend = 0;
uint32_t GyroTimeExtend = 0;
uint32_t QuatTimeExtend = 0;


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define RCC_APB2Periph_ALLGPIO             (RCC_APB2Periph_GPIOA \
                                          | RCC_APB2Periph_GPIOB \
                                          | RCC_APB2Periph_GPIOC \
                                          | RCC_APB2Periph_GPIOD)

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

/****************************************************************************************************
 * @fn      GPIO_AINConfig
 *          Configures all IOs as AIN to reduce the power consumption.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
static void GPIO_AINConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable all GPIOs Clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, ENABLE);

    /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Disable all GPIOs Clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, DISABLE);
}


/****************************************************************************************************
 * @fn      ConfigRTCandBkupDomain
 *          Configures the RTC & Backup domain clocks and checks for Standby flag.
 *
 * @param   none
 *
 * @return  True - if Standby flag is set
 *
 ***************************************************************************************************/
static osp_bool_t ConfigRTCandBkupDomain( void )
{
    osp_bool_t standbyState = false;

    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    /* Enable WKUP pin */
    PWR_WakeUpPinCmd(ENABLE);

    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);

    /* Check if the StandBy flag is set */
    if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET)
    {/* System resumed from STANDBY mode */

        standbyState = true;

        /* Clear StandBy flag */
        PWR_ClearFlag(PWR_FLAG_SB);
    }

    return standbyState;
}


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
            RCC_APB2PeriphClockCmd( DiagLEDs[index].rccPeriph, ENABLE );
            LED_Off(index);
            GPIO_InitStructure.GPIO_Pin = DiagLEDs[index].pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(DiagLEDs[index].grp, &GPIO_InitStructure);
        }
    }
}

#ifndef USE_HSI_CLOCK
/****************************************************************************************************
 * @fn      SystemClkConfig
 *          Configures the various clock domains on the chip.
 *
 * @param   none
 *
 * @return  none
 *
 * @see     none
 ***************************************************************************************************/
void SystemClkConfig( void )
{
    ErrorStatus HSEStartUpStatus;
#if (SYSTEM_CLOCK_FREQ == 72000000)
    uint32_t rccMul = RCC_PLLMul_9;
#else
    uint32_t rccMul = RCC_PLLMul_6;
#endif


    /* RCC system reset (clear BootLoader settings, if any and enables HSI clock) */
    RCC_DeInit();

#ifdef RTC_ON_LSI_CLOCK
    /* Enable LSI for use by RTC */
    RCC_LSICmd( ENABLE );
#endif

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);

        /* HCLK (AHB) = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 (APB2) = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div4); //18MHz

        /* PCLK1 (APB1) = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* ADCCLK */
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);  //9Mhz

        if (HSE_VALUE == 8000000)
        {
            /* PLLCLK = 8MHz * rccMul = 72 or 48 MHz (Crystal = 8MHz) */
            RCC_PLLConfig(RCC_PLLSource_HSE_Div1, rccMul);
        }
        else
        {
            /* PLLCLK = 16MHz/2 * rccMul = 72 or 48 MHz (Crystal = 16MHz) */
            RCC_PLLConfig(RCC_PLLSource_HSE_Div2, rccMul);
        }

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    /* Note enabling individual periperals is responsibility of the modules */
    /* Enable alternate function IO clock here as it is common for most alternate function IOs */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );

#ifdef RTCClockOutput_Enable
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
#endif

    /* Configure RTC and backup domain */
    gExitFromStandby = ConfigRTCandBkupDomain();
}
#else //If internal oscillator needs to be used
/****************************************************************************************************
 * @fn      SystemClkConfig
 *          Configures the various clock domains on the chip.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void SystemClkConfig( void )
{
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();  //This enables HSI by default

    /* Enable LSI for use by RTC */
    RCC_LSICmd( ENABLE );

    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency( FLASH_Latency_2 );

    /* HCLK (AHB) = SYSCLK (64MHz with HSI) */
    RCC_HCLKConfig( RCC_SYSCLK_Div1 );

    /* PCLK2 (APB2) = HCLK (16MHz with HSI) */
    RCC_PCLK2Config( RCC_HCLK_Div4 );

    /* PCLK1 (APB1) = HCLK/2 (32MHz with HSI) */
    RCC_PCLK1Config( RCC_HCLK_Div2 );

    /* ADCCLK */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);  //8Mhz

    /* PLLCLK = HSI/2 * 16 = 64 MHz (HSI = 8MHz) */
    RCC_PLLConfig( RCC_PLLSource_HSI_Div2, RCC_PLLMul_16 );

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }

    /* Note enabling individual periperals is responsibility of the modules */
    /* Enable alternate function IO clock here as it is common for most alternate function IOs */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );

#ifdef RTCClockOutput_Enable
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE );
#endif

    /* Configure RTC and backup domain */
    gExitFromStandby = ConfigRTCandBkupDomain();
}
#endif

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
    /* Set all GPIOs to analog input mode to begin with */
    GPIO_AINConfig();

#ifdef REMAP_JTAG
    /* Only supports SWJ interface */
    GPIO_PinRemapConfig( GPIO_Remap_SWJ_JTAGDisable, ENABLE );
#endif
}


/****************************************************************************************************
 * @fn      SystemInterruptConfig
 *          Configures the nested vectored interrupt controller.
 *
 ***************************************************************************************************/
void SystemInterruptConfig( void )
{

#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x20000000 */
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at given offset from 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

    /* Initialize EXTI registers to known state */
    EXTI_DeInit();

    /* Setup the global interrupt priority grouping */
    NVIC_PriorityGroupConfig( SYSTEM_INTERRUPT_PRIORITY_GRP );
}


/****************************************************************************************************
 * @fn      RTC_Configuration
 *          Configures the RTC.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void RTC_Configuration( void )
{
    /* System resumed from STANDBY mode */
    if(gExitFromStandby)
    {
        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();
        /* No need to configure the RTC as the RTC configuration(clock source, enable,
        prescaler,...) is kept after wake-up from STANDBY */
    }
    else
    {/* StandBy flag is not set */
        /* Select HSE as RTC Clock Source */
#if defined USE_HSI_CLOCK || defined RTC_ON_LSI_CLOCK
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#elif defined RTC_ON_LSE_CLOCK
        RCC_LSEConfig(RCC_LSE_ON);
        /* Wait till LSE is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
#endif

        /* Enable RTC Clock */
        RCC_RTCCLKCmd(ENABLE);

        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        /* Set RTC prescaler */
        RTC_SetPrescaler(RTC_PRESCALAR); /* RTC period = RTCCLK/(RTC_PRESCALAR+1) */

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        /* Reset the counter to 0 */
        RTC_SetCounter(0);

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }
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
    RCC_APB2PeriphClockCmd( RCC_Periph_UART_GPIO, ENABLE );
    if (DBG_IF_UART == USART1)
    {
        RCC_APB2PeriphClockCmd( RCC_Periph_DBG_UART, ENABLE );
    }
    else
    {
        RCC_APB1PeriphClockCmd( RCC_Periph_DBG_UART, ENABLE );
    }

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = DBG_UART_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(DBG_UART_GPIO_GRP, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = DBG_UART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DBG_UART_GPIO_GRP, &GPIO_InitStructure);

#ifdef UART_DMA_ENABLE
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_UART_Periph_DMA, ENABLE);

    DMA_DeInit(DBG_UART_TX_DMA_Channel);

    /* Enable the DMA1 Channel 4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DBG_UART_TX_DMA_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DBG_UART_DMA_INT_PREEMPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = DBG_UART_TX_DMA_INT_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DBG_UART_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DBG_UART_INT_PREEMPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = DBG_UART_INT_SUB_PRIORITY;
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
