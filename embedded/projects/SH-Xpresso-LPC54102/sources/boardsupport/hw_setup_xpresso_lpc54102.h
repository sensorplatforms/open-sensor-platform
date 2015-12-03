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
#ifndef __HW_SETUP_XPRESSO_LPC54102_H__
#define __HW_SETUP_XPRESSO_LPC54102_H__
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "chip.h"
#include "uart_5410x.h"
#include "clock_5410x.h"
#include "dma_5410x.h"
#include "iocon_5410x.h"
#include "osp-types.h"
#include "osp-sensors.h"
#include <stddef.h>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SENSORHUB_INT_ACTIVE_HIGH

/* ########################################################################## */
/* #    T I M I N G S                                                       # */
/* ########################################################################## */
/** System clock & tick configuration */
/* This define is now defined in build option
 * #define SYSTEM_CLOCK_FREQ                       96000000  //high-performance mode (CPU @96MHz)
 */
#define USEC_PER_TICK                           5000     ///< (in uS) = 200Hz
#define TICS_PER_SEC                            ((uint32_t)(1000000/USEC_PER_TICK))
#define MSEC_PER_TICK                           ((uint32_t)(USEC_PER_TICK/1000))

/* A timer is used as RTC counter coz internal RTC does not provide low enough resolution */
#define RTC_COUNTER                             LPC_TIMER2
#define US_PER_RTC_TICK                         25

/* Tick conversion macros */
#define TICS_TO_SEC(T)                          ((uint32_t)(((T) + (TICS_PER_SEC/2))/TICS_PER_SEC))
#define SEC_TO_TICS(S)                          ((uint32_t)((S) * TICS_PER_SEC))
#define MSEC_TO_TICS(M)                         ((uint32_t)(((M) + (MSEC_PER_TICK-1))/MSEC_PER_TICK))

/* ########################################################################## */
/* #    F L A S H  S T O R A G E                                            # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    D I A G N O S T I C  (LED/GPIOs) I N T E R F A C E                  # */
/* ########################################################################## */
/* Diagnostic GPIOs */
#define LPC_GPIO_PORT LPC_GPIO
#define NC (uint32_t)0xFFFFFFFF
typedef uint32_t PinName;

#define ENCODE_PORT_PIN(port,pin) (PinName)(((uint32_t)port << 16) + (uint16_t)pin)
#define DECODE_PORT(X) (((uint32_t)(X) >> 16) & 0xF)
#define DECODE_PIN(X)  ((uint32_t)(X) & 0xFFFF)

/* Diagnostic LEDs on this board */
enum _Leds { //Sequence must correspond to PINS_LED definition
    LED_BLUE,
    LED_RED,
    LED_GREEN,
    //DEBUG_OUT0,
    //DEBUG_OUT1,
    NUM_LEDS,
    NOT_USED
};

typedef struct _IO_info {
    PINMUX_GRP_T PinDef;
    uint8_t      Invert; //set to 1 if active low
} GpioInfo_t;

extern const GpioInfo_t DiagLEDs[NUM_LEDS];

/** D2-RGB LED Blue pin definition */
#define PIN_LED_BLUE    {0x0, 31, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN), 1}
/** D2-RGB LED Red pin definition */
#define PIN_LED_RED     {0x0, 29, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN), 1}
/** D2-RGB LED Green pin definition */
#define PIN_LED_GREEN   {0x0, 30, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN), 1}

/* Temp Debug pin */
//#define PIN_DEBUG_0     {0x1, 15, (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN), 0}
//#define PIN_DEBUG_1     {0x1, 6,  (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN), 0}

/** List of all LEDs definitions. */
#define PINS_LEDS   PIN_LED_BLUE, PIN_LED_RED, PIN_LED_GREEN  /*, PIN_DEBUG_0, PIN_DEBUG_1 */

/* User Friendly LED designation - unused ones should be assigned 0xFF */
#define FRONT_LED                               NOT_USED
#define HARD_FAULT_LED                          LED_RED
#define MAG_ANOMALY_LED                         LED_RED
#define CAL_INDICATE_LED                        LED_GREEN
#define ASSERT_LED                              LED_BLUE

#define LED_On(led)                                     \
    if (led < NUM_LEDS) {                               \
        if (DiagLEDs[led].Invert) {                     \
            LPC_GPIO_PORT->B[DiagLEDs[led].PinDef.port][DiagLEDs[led].PinDef.pin] = 0; \
        } else {                                        \
            LPC_GPIO_PORT->B[DiagLEDs[led].PinDef.port][DiagLEDs[led].PinDef.pin] = 1; \
        }                                               \
    }

#define LED_Off(led)                                    \
    if (led < NUM_LEDS) {                               \
        if (DiagLEDs[led].Invert) {                     \
            LPC_GPIO_PORT->B[DiagLEDs[led].PinDef.port][DiagLEDs[led].PinDef.pin] = 1; \
        } else {                                        \
            LPC_GPIO_PORT->B[DiagLEDs[led].PinDef.port][DiagLEDs[led].PinDef.pin] = 0; \
        }                                               \
    }

# define LED_Toggle(led)                                \
    if (led < NUM_LEDS) {                               \
        Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, DiagLEDs[led].PinDef.port, DiagLEDs[led].PinDef.pin); \
    }

/* Assert LED assignment */
#define AssertIndication()                      \
    do {                                        \
        uint32_t j;                             \
        LED_Off( MAG_ANOMALY_LED );             \
        LED_Off( CAL_INDICATE_LED );            \
        for (j = 0; j < 10; j++) {              \
            __IO uint32_t i;                    \
            LED_On( ASSERT_LED );               \
            for (i = 0; i < 10000; i++);        \
            LED_Off( ASSERT_LED );              \
            for (i = 0; i < 500000; i++);       \
        }                                       \
    } while(0);

/* ########################################################################## */
/* #    K E Y P A D  I N T E R F A C E                                      # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    A C C E L E R O M E T E R  I N T E R F A C E                        # */
/* ########################################################################## */
/* Interrupt input from Accel (INT1) */
/** Accelerometer interrupt pin defines */
#define ACCEL_INT_PORT                          0
#define ACCEL_INT_PIN                           18
#define ACCEL_PINT_SEL                          PININTSELECT0
#define ACCEL_PINT_CH                           PININTCH0
#define ACCEL_PINT_IRQn                         PIN_INT0_IRQn
#define ACCEL_WAKE                              SYSCON_STARTER_PINT0
#define ACCEL_IRQHandler                        PIN_INT0_IRQHandler
#define ACCEL_INT2_PORT                         0
#define ACCEL_INT2_PIN                          7

/* IMPORTANT! - Sensor interrupts are shared by multiple sensor devices on the
 * LPCXpresso Sensor Shield board. Particularly when Accel on BMI055 is used,
 * INT1 is connected to P0_18 via R16 (220ohm) and fights with the INT1 of BMC150
 * that also has an accel inside. To isolate the signals it is important to
 * remove the resistor from the INT pin not used. In this case R22 must be removed
 * for the setup to work properly.
 */

/* ########################################################################## */
/* #    M A G N E T O M E T E R  I N T E R F A C E  (D I G I T A L)         # */
/* ########################################################################## */
/** Magnetometer interrupt pin defines  */
#define MAG_INT_PORT                            0
#define MAG_INT_PIN                             22
#define MAG_PINT_SEL                            PININTSELECT2
#define MAG_PINT_CH                             PININTCH2
#define MAG_PINT_IRQn                           PIN_INT2_IRQn
#define MAG_WAKE                                SYSCON_STARTER_PINT2
#define MAG_IRQHandler                          PIN_INT2_IRQHandler
#define MAG_INT3_PORT                           0
#define MAG_INT3_PIN                            10

/* ########################################################################## */
/* #    G Y R O S C O P E  I N T E R F A C E                                # */
/* ########################################################################## */
/** Gyroscope interrupt pin interface */
#define GYRO_INT_PORT                           0
#define GYRO_INT_PIN                            4
#define GYRO_PINT_SEL                           PININTSELECT1
#define GYRO_PINT_CH                            PININTCH1
#define GYRO_PINT_IRQn                          PIN_INT1_IRQn
#define GYRO_WAKE                               SYSCON_STARTER_PINT1
#define GYRO_IRQHandler                         PIN_INT1_IRQHandler

/* ########################################################################## */
/* #    B A R O M E T E R  I N T E R F A C E                                # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    M I S C  I N T E R F A C E                                          # */
/* ########################################################################## */

/** Proximity interrupt pin interface */
#define PROXI_INT_PORT                          0X0
#define PROXI_INT_PIN                           9
#define PROXI_PINT_SEL                          PININTSELECT3
#define PROXI_PINT_CH                           PINTINTCH3
#define PROXI_PINT_IRQn                         PIN_INT3_IRQn
#define PROXI_WAKE                              SYSCON_STARTER_PINT3
#define PROXI_IRQHandler                        PIN_INT3_IRQHandler

/* define SENSORHUB_INT_ACTIVE_HIGH for active high INT. Leave undefined.
 * for default active low behavior.
 */
/* Assert IRQ */
#define _SensorHubIntHigh() Chip_GPIO_SetPinState(LPC_GPIO, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN, 1)
/* Deassert IRQ */
#define _SensorHubIntLow()  Chip_GPIO_SetPinState(LPC_GPIO, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN, 0)

#ifdef SENSORHUB_INT_ACTIVE_HIGH
/* Active High */
# define SensorHubAssertInt()       _SensorHubIntHigh()
# define SensorHubDeAssertInt()     _SensorHubIntLow()
#else
/* Active Low */
# define SensorHubAssertInt()       _SensorHubIntLow()
# define SensorHubDeAssertInt()     _SensorHubIntHigh()
#endif

/* ########################################################################## */
/* #    S P I   I N T E R F A C E                                           # */
/* ########################################################################## */
/* SPI Slave host interface */
#define SPI_HOSTIF_BUS                          LPC_SPI1
#define SPI_HOSTIF_IRQn                         SPI1_IRQn
#define SPI_HOSTIF_MOSI                         1, 7,   (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN) //SPI1_MOSI
#define SPI_HOSTIF_MISO                         1, 14,  (IOCON_FUNC4 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN) //SPI1_MISO
#define SPI_HOSTIF_SCK                          1, 6,   (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN) //SPI1_SCK
#define SPI_HOSTIF_SSN                          1, 15,  (IOCON_FUNC4 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN) //SPI1_SSEL0
#define SPIFIFO_IF_IDX                          1  //for SPI1 used in FIFO calls
#define SPIFIFO_SYSCON_TXEN                     SYSCON_FIFO_SPI1TXFIFOEN
#define SPIFIFO_SYSCON_RXEN                     SYSCON_FIFO_SPI1RXFIFOEN

/* ########################################################################## */
/* #    I 2 C   I N T E R F A C E                                           # */
/* ########################################################################## */
/* I2C Master interface for Sensors */
/** Sensor bus (I2C master) defines */
#define I2C_MASTER_CLOCK_DIV                    2
#define I2C_MCLOCK_SPEED                        400000 /* 400000 generates 375K, use 430000 for 400K */
/** Sensor bus interface defines */
#define I2C_SENSOR_BUS                          LPC_I2C0
#define I2C_SENSOR_BUS_SCL_PIN                  0, 23, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)  /* I2C0_SCL (SCL) */
#define I2C_SENSOR_BUS_SDA_PIN                  0, 24, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN)  /* I2C0_SDA-WAKEUP (SDA) */
#define I2C_SENSOR_CLOCK                        SYSCON_CLOCK_I2C0 // Clock for I2C used in SENSOR_IF
#define I2C_SENSOR_RESET                        RESET_I2C0
#define I2C_SENSOR_BUS_IRQn                     I2C0_IRQn
#define I2C_SENSOR_BUS_IRQHandler               I2C0_IRQHandler
#define I2C_SENSOR_BUS_DMAID                    DMAREQ_I2C0_MASTER
#define I2C_SENSOR_BUS_DMABYTELIM               (1)    /* Use DMA when sending or receiving more bytes than this define,
                                                        I2C ADDR byte is never sent using DMA. Use 0 to always use DMA. */
#define CFG_MAX_SA_TX_BUFFER                    32     /* Maximum size of the buffer transmitted to sensor devices over I2c  */

/* ########################################################################## */
/* #    H O S T  I N T E R F A C E                                          # */
/* ########################################################################## */
/* Interrupt and communication interface going to the host */
#define HOSTIF_IRQ_PORT                         0
#define HOSTIF_IRQ_PIN                          19
#define HOSTIF_SCL_PIN                          0, 28, (IOCON_FUNC1 | IOCON_DIGITAL_EN)
#define HOSTIF_SDA_PIN                          0, 27, (IOCON_FUNC1 | IOCON_DIGITAL_EN)


/* I2C Slave interface for host communication (e.g. interface with Android host) */
#define I2C_HOSTIF                              LPC_I2C2
#define I2C_HOSTIF_IRQn                         I2C2_IRQn
#define I2C_HOSTIF_IRQHandler                   I2C2_IRQHandler
#define I2C_HOSTIF_WAKE                         SYSCON_STARTER_I2C2
#define I2C_HOSTIF_CLOCK_DIV                    2
#define I2C_HOSTIF_ADDR                         (0x18)

/* ########################################################################## */
/* #    U A R T  I N T E R F A C E                                          # */
/* ########################################################################## */
/** UART configuration */
#define DBG_UART_BAUD                           921600 //Rx not working above 200K baud!
#define USART_WordLength_8b                     UART_CFG_DATALEN_8
#define USART_StopBits_1                        UART_CFG_STOPLEN_1
#define USART_Parity_No                         UART_CFG_PARITY_NONE

#define DBG_IF_UART                             LPC_USART0
/* Pin TX (Port, Pin, (Mode | Func)) */
#define DBG_UART_TX_PIN_DEF                     0, 1, (IOCON_MODE_INACT | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF) //P0.1

/* Pin RX (Port, Pin, (Mode | Func)) */
#define DBG_UART_RX_PIN_DEF                     0, 0, (IOCON_MODE_INACT | IOCON_FUNC1 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF) //P0.0

/* IRQ Channel for TX and RX interrupts */
#define DBG_UART_IRQChannel                     UART0_IRQn //!Don't forget to switch the ISR Handler!

/* Flush macro used in assert */
#ifdef UART_DMA_ENABLE
# define FlushUart()                            //TODO
# define DBG_UART_TX_DMA_Channel                DMAREQ_UART0_TX
# define DBG_UART_DR_Base                       DBG_IF_UART->TXDAT
# define DBG_UART_DMA_PRIORITY                  0
# define DMA_INT_IRQChannel                     DMA_IRQn
#else
# define FlushUart()
#endif
#define BIT_POSITION(x)                         (1<<x)

/* ########################################################################## */
/* #    I N T E R R U P T  A S S I G N M E N T S                            # */
/* ########################################################################## */
/** Interrupt Channels, & Priority assignments: M4 on LPC54XXX has 8 priorities */
#define SYSTEM_INTERRUPT_PRIORITY_GRP           0  //0 is the only valid option

#define DBG_UART_INT_PRIORITY                   7  ///< Lowest priority

/* Priorities for I2C Bus interrupts */
#define I2C_SENSOR_BUS_INT_PRIORITY             3   ///<  I2C0 IRQ
#define I2C_SLAVE_BUS_INT_PRIORITY              4   ///<  I2C2 IRQ

/* DMA Controller interface */
#define DMA_INT_PREEMPT_PRIO                    6

/* Sensors & host interface */
#define ACCEL_INT_IRQ_PRIORITY                  0   ///< Highest priority
#define MAG_INT_IRQ_PRIORITY                    0
#define GYRO_INT_IRQ_PRIORITY                   0
#define HOSTIF_IRQ_PRIORITY                     I2C_SLAVE_BUS_INT_PRIORITY

#define NVIC_CH_ENABLE(IRQCh)                   NVIC_EnableIRQ(IRQCh)
#define NVIC_CH_DISABLE(IRQCh)                  NVIC_DisableIRQ(IRQCh)

/* ########################################################################## */
/* #    P O W E R  C O N T R O L                                            # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    U S B  I N T E R F A C E                                            # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum MsgContextTag
{
    CTX_THREAD,     ///< Message sent from within a thread context
    CTX_ISR         ///< Message sent from ISR
} MsgContext;

typedef DMA_CHID_T DMAChannel_t;
typedef LPC_I2C_T I2C_TypeDef;


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
void LED_Init(void);
void Board_SensorIfInit( InputSensor_t ifID );
void Board_SensorIfDeinit( InputSensor_t ifID );

/****************************************************************************************************
 * @fn      GetContext
 *          Identifies if we are currently in ISR or Thread context and returns the corresponding
 *          enum value for it. The logic is based on the fact that ISR uses system stack and Thread
 *          use their allocated stack. We only need to read the current value of SP to figure out
 *          whether we are in a Thread or ISR. (Maybe there is a better way... but this is good
 *          enough for now)
 *
 * @param   none
 *
 * @return  CTX_THREAD or CTX_ISR
 *
 ***************************************************************************************************/
static __inline MsgContext GetContext( void )
{
    extern uint32_t gStackMem;     // Note this extern is defined in keil_startup_lpc5410x.s

    return (__current_sp() < (uint32_t)&gStackMem)? CTX_THREAD : CTX_ISR;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
#endif /*__HW_SETUP_XPRESSO_LPC54102_H__ */
