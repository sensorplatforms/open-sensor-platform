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
#ifdef ASF_PROFILING
  extern uint32_t gStackMem;
  extern uint32_t gStackSize;
  extern const char C_gStackPattern[8];
#endif

void SPISlaveSetup( void );

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef DEBUG_BUILD
  char _errBuff[ERR_LOG_MSG_SZ];
#endif
uint32_t g_logging = 0;

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
 * @fn      main
 *          Main entry point to the application firmware
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
int main( void )
{
#ifdef ASF_PROFILING
    register uint32_t *pStack = (uint32_t *)&gStackMem;
    register uint32_t stkSize = (uint32_t)&gStackSize;
    register uint32_t idx;

    /* main() is using the same stack that we are trying to initialize so we leave the last 32 bytes */
    for ( idx = 0; idx < ((stkSize-32)/sizeof(C_gStackPattern)); idx++)
    {
        *pStack++ = *((uint32_t *)C_gStackPattern);
        *pStack++ = *((uint32_t *)(C_gStackPattern+4));
    }
#endif
    /**
     Get the clocks going and setup UART1 as the basic debug port for now. If UART_DMA_ENABLE is not
     defined then Debug messages will be handled in a separate task when the RTOS takes over.
     */

    /* NVIC configuration */
    SystemInterruptConfig();

    /* System clock configuration for regular mode */
    Chip_Clock_EnablePeriphClock(SYSCON_CLOCK_INPUTMUX);

    /* Update clock information before other drivers might access it */
    SystemCoreClockUpdate();

    /* Configure the GPIO ports (non module specific) */
    SystemGPIOConfig();

    /* Set startup state of LEDs */
    LED_Init();                   /* Initialize Debug LEDs */

    /* Configure RTC */
    RTC_Configuration();

    /* Configure debug UART port - we do it here to enable assert messages early in the system */
    DebugUARTConfig( DBG_UART_BAUD, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No );
    DebugPortInit();

    /* Print version number */
    printf("\r\n### OSP 1.1.17 for LPC54102Xpresso board: Date: %s - %s ###\r\n",
        __DATE__, __TIME__);

    /* Display System clock information */
    printf("System Clocks:\r\n");
    printf("\tMain      - %ld Hz\r\n", Chip_Clock_GetMainClockRate());
    printf("\tSystem    - %ld Hz\r\n", Chip_Clock_GetSystemClockRate());
    printf("\tAsyncSCON - %ld Hz\r\n", Chip_Clock_GetAsyncSyscon_ClockRate());
    printf("\tAsyncSy_A - %ld Hz\r\n", Chip_Clock_GetMain_A_ClockRate());
    printf("\tAsyncSy_B - %ld Hz\r\n", Chip_Clock_GetMain_B_ClockRate());
    printf("\tPLL_IN    - %ld Hz\r\n", Chip_Clock_GetSystemPLLInClockRate());

    printf("\tPLL_OUT   - %ld Hz\r\n", Chip_Clock_GetSystemPLLOutClockRate(false));
    D0_printf("\tUartBase  - %ld Hz\r\n", Chip_Clock_GetUARTBaseClockRate());
    //D0_printf("\tSysTick   - %ld Hz\r\n", Chip_Clock_GetSysTickClockRate()); --> Not giving proper value!?

    /* Enable SPI Slave interface */
    SPISlaveSetup();

    /* Get the OS going - This must be the last call */
    AsfInitialiseTasks();

    /* If it got here something bad happened */
    ASF_assert_fatal(false);
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
