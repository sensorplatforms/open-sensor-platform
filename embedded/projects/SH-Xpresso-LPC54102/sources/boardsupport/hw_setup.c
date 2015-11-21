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
#include "timer_5410x.h"
#include "i2c_driver.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
const uint32_t ExtClockIn = 0;
const GpioInfo_t DiagLEDs[NUM_LEDS] = {PINS_LEDS};

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Pin muxing table, only items that need changing from their default pin
   state are in this table. Not every pin is mapped. */
//TODO Pin init should be moved to respective modules handling the pin function
STATIC const PINMUX_GRP_T pinmuxing[] = {

    /* I2C1 standard/fast (bridge) */
    {0, 27, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)},    /* BRIDGE_SCL (SCL) */
    {0, 28, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)},    /* BRIDGE_SDA (SDA) */

    /* Sensor related */
    {0, 4,  (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* GYR_INT1 (GPIO input) */
    {0, 18, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)},    /* CT32B0_MAT0-ACCL_INT1 */
    {0, 22, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN | IOCON_DIGITAL_EN)}, /* MAG_DRDY_INT (GPIO input) */

};

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
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      SystemInit
 *          Set up and initialize hardware prior to call to main
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void SystemInit(void)
{
#if defined(__CODE_RED)
    extern void(*const g_pfnVectors[]) (void);
    SCB->VTOR = (uint32_t) &g_pfnVectors;
#else
    extern void *__Vectors;
    SCB->VTOR = (uint32_t) &__Vectors;
#endif

#if defined(CORE_M4)
#if defined(__FPU_PRESENT) && __FPU_PRESENT == 1
    fpuInit();
#endif
#endif

#if !defined(__MULTICORE_M0SLAVE) && !defined(__MULTICORE_M4SLAVE)
    /* Chip specific SystemInit */
    /* Enable IOCON clock */
    Chip_Clock_EnablePeriphClock(SYSCON_CLOCK_IOCON);

    /* Setup internal RC clocking to given System freq */
    Chip_SetupIrcClocking(SYSTEM_CLOCK_FREQ);

    /* Select the CLKOUT clocking source */
    //Chip_Clock_SetCLKOUTSource(SYSCON_CLKOUTSRC_MAINCLK, 1);
#endif
}


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
    uint8_t index;

    for (index = 0; index < NUM_LEDS; index++)
    {
        /* Setup Pins in the mux */
        Chip_IOCON_PinMuxSet(LPC_IOCON, DiagLEDs[index].PinDef.port, DiagLEDs[index].PinDef.pin,
            DiagLEDs[index].PinDef.modefunc);

        Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, DiagLEDs[index].PinDef.port, DiagLEDs[index].PinDef.pin);

        LED_Off(index);
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
    /* Set all GPIOs to analog input mode to begin with */
    GPIO_AINConfig();

    /* Enable the peripheral clock in the PMC */
    Chip_GPIO_Init(LPC_GPIO_PORT);

    /* Setup the Sensor Hub interrupt pin as output */
    //Chip_IOCON_Config(LPC_IOCON, SH_INT_PIN);
    //Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, SH_INT_GPIO_GRP, SH_INT_GPIO_PIN);
    //SensorHubIntLow(); //Deassert on startup

    /* TODO: Catch ALL for uninitialized pins... should be moved to respective modules */
    Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

    /* Setup DMA Common here since its not specific to any peripheral */
    Chip_DMA_Init(LPC_DMA);
    //Chip_SYSCON_PeriphReset(RESET_DMA);
    Chip_DMA_Enable(LPC_DMA);
    Chip_DMA_SetSRAMBase(LPC_DMA, DMA_ADDR(Chip_DMA_Table));
}


/****************************************************************************************************
 * @fn      SystemInterruptConfig
 *          Configures the nested vectored interrupt controller.
 *
 ***************************************************************************************************/
void SystemInterruptConfig( void )
{
    NVIC_SetPriorityGrouping( SYSTEM_INTERRUPT_PRIORITY_GRP );

    /* Enable Pin interrupt sources */
    Chip_PININT_Init(NULL); /* Note: input arg is ignored! */
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
    uint32_t preScale;
/* Note: Be careful with prescalar setup here if RTC counter is changed. T2,T3 & T4 use Sync APB clock
   (System Clock) while T0 & T1 use Async APB clock */

    Chip_TIMER_Init(RTC_COUNTER);
    /* Calculated prescalar based on PCLK rate (System Clock) for the selected timer (2,3 or 4) */
    preScale = ((Chip_Clock_GetSystemClockRate()/1000000) * US_PER_RTC_TICK) - 1;
    Chip_TIMER_PrescaleSet(RTC_COUNTER, preScale);
    /* Reset Timer */
    Chip_TIMER_Reset(RTC_COUNTER);
    /* Start timer */
    Chip_TIMER_Enable(RTC_COUNTER);
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
uint32_t RTC_GetCounter( void )
{
    return RTC_COUNTER->TC;
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
    // init UARTx pins and config
    Chip_IOCON_PinMuxSet(LPC_IOCON, DBG_UART_RX_PIN_DEF); // U0_RXD
    Chip_IOCON_PinMuxSet(LPC_IOCON, DBG_UART_TX_PIN_DEF); // U0_TXD

    Chip_UART_Init(DBG_IF_UART);
    Chip_UART_ConfigData(DBG_IF_UART, UART_CFG_ENABLE | dataLen |
                         parity | stopBits);
    Chip_UART_SetBaud(DBG_IF_UART, baud);


#ifdef UART_DMA_ENABLE
    /* DMA Interrupt configuration section ------------------------------------------------- */

    /* Initialize DMA for UART - Note: common stuff for DMA done in SystemGPIOConfig */
    Chip_DMA_SetupChannelConfig(LPC_DMA, DBG_UART_TX_DMA_Channel, DMA_CFG_PERIPHREQEN | DMA_CFG_TRIGBURST_SNGL |
                                DMA_CFG_CHPRIORITY(DBG_UART_DMA_PRIORITY));

    /* Setup DMA interrupt */
    NVIC_DisableIRQ (DMA_INT_IRQChannel);
    /* Setup preemption and sub-priority for DMA */
    NVIC_SetPriority(DMA_INT_IRQChannel, DMA_INT_PREEMPT_PRIO);

    // Enable interrupt for DMA
    NVIC_EnableIRQ(DMA_INT_IRQChannel);
#endif

    /* Enable receiver & transmitter */
    Chip_UART_Enable(DBG_IF_UART);

    /* Set interrupt priority */
    NVIC_DisableIRQ(DBG_UART_IRQChannel);
    NVIC_ClearPendingIRQ(DBG_UART_IRQChannel);

    /* Setup preemption and sub-priority for UART */
    NVIC_SetPriority(DBG_UART_IRQChannel, DBG_UART_INT_PRIORITY);

    /* Enable receive interrupt, TX interrupt in enabled in write */
    Chip_UART_IntEnable(DBG_IF_UART, UART_INTEN_RXRDY); //NOTE: Receive disabled for visualization tool

    /* Enable the interrupt channel */
    NVIC_EnableIRQ( DBG_UART_IRQChannel );
}


#ifdef UART_DMA_ENABLE
/****************************************************************************************************
 * @fn      UartDMAConfiguration
 *          Configures the DMA for use with the selected UART
 *
 ***************************************************************************************************/
void UartDMAConfiguration( PortInfo *pPort, uint8_t *pTxBuffer, uint16_t txBufferSize )
{
    DMA_CHDESC_T dmaDesc;
    pTxBuffer += txBufferSize - 1;
    /* DMA descriptor for M2P operation - note that addresses must
       be the END address for src and destination, not the starting address.
       DMA operations moves from end to start. */
    dmaDesc.source  = DMA_ADDR(pTxBuffer);
    dmaDesc.dest    = DMA_ADDR(&DBG_IF_UART->TXDAT);
    dmaDesc.next    = DMA_ADDR(0); //No chaining
    dmaDesc.xfercfg = (DMA_XFERCFG_CFGVALID | DMA_XFERCFG_SETINTA | DMA_XFERCFG_SWTRIG |
        DMA_XFERCFG_WIDTH_8 | DMA_XFERCFG_SRCINC_1 | DMA_XFERCFG_DSTINC_0 |
        DMA_XFERCFG_XFERCOUNT(txBufferSize));


    /* Setup transfer descriptor and validate it */
    Chip_DMA_SetupTranChannel(LPC_DMA, pPort->DMAChannel, &dmaDesc);

    /* Note: DMA is triggered in EnableUartDMAChannel() call */
    Chip_DMA_SetupChannelTransfer(LPC_DMA, pPort->DMAChannel, dmaDesc.xfercfg);
    Chip_DMA_SetValidChannel(LPC_DMA, pPort->DMAChannel);
}
#endif

/****************************************************************************************************
 * @fn      Board_SensorIfInit
 *          Configures the peripherals and IOs for the given Sensor
 *
 ***************************************************************************************************/
void Board_SensorIfInit( InputSensor_t ifID )
{

    /* Initialize common interfaces */
    /* Initialize the I2C Driver interface */
    ASF_assert( true == I2C_HardwareSetup( I2C_SENSOR_BUS ) );

    /* Initialize IOs - interrupts, wakeup, etc. */
    switch (ifID)
    {
    case ACCEL_INPUT_SENSOR:
        /* ACCEL INT1 irq setup */
        NVIC_DisableIRQ(ACCEL_PINT_IRQn);
        NVIC_SetPriority(ACCEL_PINT_IRQn, ACCEL_INT_IRQ_PRIORITY);

        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, ACCEL_INT_PORT, ACCEL_INT_PIN, false);

        Chip_INMUX_PinIntSel(ACCEL_PINT_SEL, ACCEL_INT_PORT, ACCEL_INT_PIN);
        Chip_PININT_SetPinModeEdge(LPC_PININT, ACCEL_PINT_CH); /* edge sensitive */
        Chip_PININT_EnableIntHigh(LPC_PININT, ACCEL_PINT_CH);  /* Rising edge interrupt */

        //pinDef.pin = ENCODE_PORT_PIN(ACCEL_INT2_PORT, ACCEL_INT2_PIN);
        //gpio_dir(&pinDef, PIN_INPUT);

        //Chip_SYSCON_EnableWakeup(ACCEL_WAKE); /* enable to wake from sleep */

        //gpio_irq_disable(&gpioIrq);
        break;

    case MAG_INPUT_SENSOR:
        NVIC_DisableIRQ(MAG_PINT_IRQn);
        NVIC_SetPriority(MAG_PINT_IRQn, MAG_INT_IRQ_PRIORITY);

        /* MAG INT2 irq setup */
        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, MAG_INT_PORT, MAG_INT_PIN, false);

        Chip_INMUX_PinIntSel(MAG_PINT_SEL, MAG_INT_PORT, MAG_INT_PIN);
        Chip_PININT_SetPinModeEdge(LPC_PININT, MAG_PINT_CH); /* edge sensitive */
        Chip_PININT_EnableIntHigh(LPC_PININT, MAG_PINT_CH);  /* Rising edge interrupt */

        //Chip_SYSCON_EnableWakeup(MAG_WAKE); /* enable to wake from sleep */
        //Chip_SYSCON_EnableWakeup(SYSCON_STARTER_WWDT);  /* enable to wake from sleep */

        //pinDef.pin = ENCODE_PORT_PIN(MAG_INT3_PORT, MAG_INT3_PIN);
        //gpio_dir(&pinDef,PIN_INPUT);
        break;

    case GYRO_INPUT_SENSOR:
        NVIC_DisableIRQ(GYRO_PINT_IRQn);
        NVIC_SetPriority(GYRO_PINT_IRQn, GYRO_INT_IRQ_PRIORITY);

        /* Gyro interrupt setup */
        Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, GYRO_INT_PORT, GYRO_INT_PIN, false);

        Chip_INMUX_PinIntSel(GYRO_PINT_SEL, GYRO_INT_PORT, GYRO_INT_PIN);
        Chip_PININT_SetPinModeEdge(LPC_PININT, GYRO_PINT_CH); /* edge sensitive */
        Chip_PININT_EnableIntHigh(LPC_PININT, GYRO_PINT_CH);  /* Rising edge interrupt */

        //Chip_SYSCON_EnableWakeup(GYRO_WAKE); 
        break;

    case PRESSURE_INPUT_SENSOR:
        /* Pressure sensor is not interrupt based - nothing to do here */
        break;

    default:
        /* do nothing, just return */
        D0_printf("WARNING! No HW initialization defined for sensor: %d\r\n", ifID);
        break;

    }

}


/****************************************************************************************************
 * @fn      Board_SensorIfDeinit
 *          Releases the peripherals and IOs for the given Sensor
 *
 ***************************************************************************************************/
void Board_SensorIfDeinit( InputSensor_t ifID )
{
    //TODO
    /* This is not a plug & play system so this is likely not needed in typical systems */
}


/****************************************************************************************************
 * @fn      Chip_FIFOSPI_Configure
 *          Helper function (missing in NXPs sources) to configure FIFO for SPI use
 *
 ***************************************************************************************************/
void Chip_FIFOSPI_Configure(LPC_FIFO_T *pFIFO, int spiIndex, LPC_FIFO_CFG_T *pSPICfg)
{
    pFIFO->spi[spiIndex].CFG =
        (pSPICfg->noTimeoutContWrite << 4) |
        (pSPICfg->noTimeoutContEmpty << 5) |
        (pSPICfg->timeoutBase << 8) |
        (pSPICfg->timeoutValue << 12) |
        (pSPICfg->rxThreshold << 16) |
        (pSPICfg->txThreshold << 24);
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
