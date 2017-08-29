/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2016 Knowles Corporation
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
#if !defined (HW_SETUP_PURPLE_HAZE_H)
#define   HW_SETUP_PURPLE_HAZE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "osp-types.h"
#include <stddef.h>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/* ########################################################################## */
/* #    T I M I N G S                                                       # */
/* ########################################################################## */
/** System clock & tick configuration */
#define SYSTEM_CLOCK_FREQ                       46080000
#define USEC_PER_TICK                           1000       ///< (in uS) = 1000Hz
#define TICS_PER_SEC                            ((uint32_t)(1000000/USEC_PER_TICK))
#define MSEC_PER_TICK                           ((uint32_t)(USEC_PER_TICK/1000))

/** RTC Configuration */
/* Note Timer-2 is used as free running counter instead of RTC */
#define US_PER_RTC_TICK                         32  //micro-seconds
#define RTC_PRESCALAR                           (((SystemCoreClock * US_PER_RTC_TICK) / 1000000) - 1)
#define RTC_TIMER                               TIM2
#define RTC_TIMER_CLK_RCC                       RCC_APB1Periph_TIM2

/* Tick conversion macros */
#define TICS_TO_SEC(T)                          ((uint32_t)(((T) + (TICS_PER_SEC/2))/TICS_PER_SEC))
#define SEC_TO_TICS(S)                          ((uint32_t)((S) * TICS_PER_SEC))
#define MSEC_TO_TICS(M)                         ((uint32_t)(((M) + (MSEC_PER_TICK-1))/MSEC_PER_TICK))

/* Timers for Haier Protocol */
#define HP_RX_TIMER                             TIM15
#define HP_RX_TIMER_CLK_RCC                     RCC_APB2Periph_TIM15
#define HP_RX_TIMER_CLK_RCC_Ftn                 RCC_APB2PeriphClockCmd
#define HP_RX_TIMER_IRQHandler                  TIM15_IRQHandler
#define HP_RX_TIMER_IRQn                        TIM15_IRQn
#define HP_RX_TIMER_IN_TRIGGER                  TIM_TS_TI2FP2
#define HP_RX_TIMER_US_PER_TICK                 1  //micro-seconds
#define HP_RX_TIMER_PRESCALAR                   (((SystemCoreClock * HP_RX_TIMER_US_PER_TICK) / 1000000) - 1)
#define HP_RX_TIMER_CH_IN                       TIM_Channel_2
#define HP_RX_TIMER_CC_INT                      TIM_IT_CC2      //For CH2 use CC2
#define HP_RX_TIMER_CH_GPIO_RCC                 RCC_AHBPeriph_GPIOB
#define HP_RX_TIMER_CH_GPIO                     GPIOB
#define HP_RX_TIMER_CH_PIN                      GPIO_Pin_15
#define HP_RX_TIMER_GPIO_AF                     GPIO_AF_1
#define HP_RX_TIMER_GPIO_PINSRC                 GPIO_PinSource15

#define HP_TX_GPIO_CLK_RCC                      RCC_AHBPeriph_GPIOC
#define HP_TX_GPIO_PIN                          GPIO_Pin_9
#define HP_TX_GPIO_GRP                          GPIOC

#define HP_TX_TIMER                             TIM3
#define HP_TX_TIMER_IRQHandler                  TIM3_IRQHandler
#define HP_TX_TIMER_CLK_RCC                     RCC_APB1Periph_TIM3
#define HP_TX_TIMER_CLK_RCC_Ftn                 RCC_APB1PeriphClockCmd
#define HP_TX_TIMER_IRQn                        TIM3_IRQn
#define HP_TX_TIMER_US_PER_TICK                 1  //micro-seconds
#define HP_TX_TIMER_PRESCALAR                   (((SystemCoreClock * HP_TX_TIMER_US_PER_TICK) / 1000000) - 1)

#ifdef ENABLE_BUTTON_BEEPS
/* Timer for Piezo Buzzer PWM generation */
# define BUZZ_TIMER                             TIM16
# define BUZZ_FREQUENCY                         2083    //Hz
# define BUZZ_TIMER_PERIOD                      ((SystemCoreClock / BUZZ_FREQUENCY ) - 1)
# define BUZZ_TIMER_CLK_RCC                     RCC_APB2Periph_TIM16
# define BUZZ_TIMER_IRQn                        TIM16_IRQn
# define BuzzTimerISRHandler                    TIM16_IRQHandler
# define BUZZ_TIMER_CH_OUT                      TIM_Channel_1
# define BUZZ_TIMER_CH_GPIO_RCC                 RCC_AHBPeriph_GPIOB
# define BUZZ_TIMER_CH_GPIO                     GPIOB
# define BUZZ_TIMER_CH_PIN                      GPIO_Pin_8      //HOST_I2C_CLK signal
# define BUZZ_TIMER_CH_AF                       GPIO_AF_2
# define BUZZ_TIMER_CH_GPIO_PINSRC              GPIO_PinSource8
#endif

/* ########################################################################## */
/* #    F L A S H  S T O R A G E                                            # */
/* ########################################################################## */

/* **** NOT PRESENT **** */


/* ########################################################################## */
/* #    D I A G N O S T I C  (LED/GPIOs) I N T E R F A C E                  # */
/* ########################################################################## */
/* Diagnostic GPIOs */

/* User Friendly LED designation - unused ones should be assigned 0xFF */
#ifdef PH_REV_F
# define FRONT_LED                              LED_BLUE
# define LED_RED                                LED_LD3
# define LED_BLUE                               LED_LD2
# define HARD_FAULT_LED                         LED_RED
#endif

#ifdef PH_REV_F
# define LED_On(led)                                    \
    if (led < NUM_LEDS) {                               \
         DiagLEDs[led].grp->BRR = DiagLEDs[led].pin;    \
    }

# define LED_Off(led)                                   \
    if (led < NUM_LEDS) {                               \
         DiagLEDs[led].grp->BSRR = DiagLEDs[led].pin;   \
    }

# define LED_Toggle(led)                                \
    if (led < NUM_LEDS) {                               \
        if (DiagLEDs[led].grp->ODR & DiagLEDs[led].pin) \
            DiagLEDs[led].grp->BSRR = DiagLEDs[led].pin;\
        else                                            \
            DiagLEDs[led].grp->BRR = DiagLEDs[led].pin; \
    }

/* Assert LED assignment */
# define AssertIndication()                     LED_Off(LED_BLUE)
#else
# define LED_On(led)

# define LED_Off(led)

# define LED_Toggle(led)

/* Assert LED assignment */
# define AssertIndication()
#endif


/* ########################################################################## */
/* #    M I S C  I N T E R F A C E                                          # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    LED  A S S I G N M E N T S                                          # */
/* ########################################################################## */
/* TODO - There are not direct connected LEDs - these are HOST_GPIO lines for now! */
#ifdef PH_REV_F
# define LED_LD2_RCC                            RCC_AHBPeriph_GPIOA
# define LED_LD2_BASE                           GPIOA
# define LED_LD2_PIN                            GPIO_Pin_15

# define LED_LD3_RCC                            RCC_AHBPeriph_GPIOC
# define LED_LD3_BASE                           GPIOC
# define LED_LD3_PIN                            GPIO_Pin_15
#else
#endif

/* ########################################################################## */
/* #    S P I   I N T E R F A C E                                           # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    I 2 C   I N T E R F A C E                                           # */
/* ########################################################################## */

/* **** NOT PRESENT **** */

/* ########################################################################## */
/* #    U A R T   I N T E R F A C E                                         # */
/* ########################################################################## */
/** UART configuration */
/* TX data register addr for DMA operations */
#define USART1_DR_Base                          ((uint32_t)(USART1_BASE + offsetof(USART_TypeDef, TDR)))
#define USART2_DR_Base                          ((uint32_t)(USART2_BASE + offsetof(USART_TypeDef, TDR)))
#define USART3_DR_Base                          ((uint32_t)(USART3_BASE + offsetof(USART_TypeDef, TDR)))

#define DBG_UART_BAUD                           921600

#ifndef DBG_UART3_ENABLE
# define DBG_IF_UART                            USART1
# define RCC_Periph_DBG_UART                    RCC_APB2Periph_USART1
# define RCC_ClkFtn_DBG_UART                    RCC_APB2PeriphClockCmd

/* TX */
#define DBG_UART_TX_PIN                         GPIO_Pin_9      //< PA.09
#define DBG_UART_TX_PIN_SRC                     GPIO_PinSource9
#define DBG_UART_TX_AF                          GPIO_AF_1
/* RX */
#define DBG_UART_RX_PIN                         GPIO_Pin_10     //< PA.10
#define DBG_UART_RX_PIN_SRC                     GPIO_PinSource10
#define DBG_UART_RX_AF                          GPIO_AF_1

# define DBG_UART_DR_Base                       USART1_DR_Base

# define RCC_Periph_UART_GPIO                   RCC_AHBPeriph_GPIOA
# define DBG_UART_GPIO_TX_GRP                   GPIOA
# define DBG_UART_GPIO_RX_GRP                   GPIOA

/* IRQ Channel for TX and RX interrupts */
# define DBG_UART_IRQChannel                    USART1_IRQn
# define DBG_UART_IRQHandler                    USART1_IRQHandler

#else /*if debug interface is UART3 */

# define DBG_IF_UART                            USART3
# define RCC_Periph_DBG_UART                    RCC_APB1Periph_USART3
# define RCC_ClkFtn_DBG_UART                    RCC_APB1PeriphClockCmd

/* TX */
# define DBG_UART_TX_PIN                        GPIO_Pin_10     ///< PC.10
# define DBG_UART_TX_PIN_SRC                    GPIO_PinSource10
# define DBG_UART_TX_AF                         GPIO_AF_1
/* RX */
# define DBG_UART_RX_PIN                        GPIO_Pin_11     ///< PC.11
# define DBG_UART_RX_PIN_SRC                    GPIO_PinSource11
# define DBG_UART_RX_AF                         GPIO_AF_1

# define DBG_UART_DR_Base                       USART3_DR_Base

# define RCC_Periph_UART_GPIO                   RCC_AHBPeriph_GPIOC
# define DBG_UART_GPIO_TX_GRP                   GPIOC
# define DBG_UART_GPIO_RX_GRP                   GPIOC

/* IRQ Channel for TX and RX interrupts */
# define DBG_UART_IRQChannel                    USART3_8_IRQn
# define DBG_UART_IRQHandler                    USART3_8_IRQHandler

#endif /* end of DBG_UART3_ENABLE */

#ifdef UART_DMA_ENABLE
# ifndef DBG_UART3_ENABLE
/** DMA & Channel assignments */
#  define RCC_DBG_UART_Periph_DMA               RCC_AHBPeriph_DMA1
#  define DBG_UART_TX_DMA_Channel               DMA1_Channel2
#  define UART_TX_DMA_TC_FLAG                   DMA1_FLAG_TC2
/* Interrupt channel assignment */

#  define DBG_UART_TX_DMA_IRQChannel            DMA1_Ch2_3_DMA2_Ch1_2_IRQn
#  define DBG_UART_TX_DMA_IRQHandler            DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler

# else
/** DMA & Channel assignments */
#  define RCC_DBG_UART_Periph_DMA               RCC_AHBPeriph_DMA1
#  define DBG_UART_TX_DMA_Channel               DMA1_Channel7
#  define UART_TX_DMA_TC_FLAG                   DMA1_FLAG_TC7

/* Interrupt channel assignment */
#  define DBG_UART_TX_DMA_IRQChannel            DMA1_Channel4_5_6_7_IRQn //< Dependent on UART selection
#  define DBG_UART_TX_DMA_IRQHandler            DMA1_Channel4_5_6_7_IRQHandler

# endif /* end of DBG_UART3_ENABLE */
#endif

/* defining I2C related configuration */
#if defined (DEBUG_BOARD_PRESENT)
#define DBG_IF_I2C                          I2C2
#define RCC_PERIPH_DBG_I2C                  RCC_APB1Periph_I2C2
#define RCC_CLKFTN_DBG_I2C                  RCC_APB1PeriphClockCmd
#define RCC_PERIPH_RESET_CMD_I2C            RCC_APB1PeriphResetCmd
#define DBG_IF_I2C_GPIO_GRP_RCC             RCC_AHBPeriph_GPIOB
#define DBG_IF_I2C_GPIO_GRP                 GPIOB
/* CLK */
#define DBG_IF_I2C_CLK_PIN                  GPIO_Pin_10     ///< PB.10
#define DBG_IF_I2C_CLK_SRC                  GPIO_PinSource10
#define DBG_IF_I2C_CLK_AF                   GPIO_AF_1
/* SDA */
#define DBG_IF_I2C_SDA_PIN                  GPIO_Pin_11     ///< PB.11
#define DBG_IF_I2C_SDA_SRC                  GPIO_PinSource11
#define DBG_IF_I2C_SDA_AF                   GPIO_AF_1

/* define I2C event interrupts */
#define DBG_IF_I2C_IRQ_CHANNEL              I2C2_IRQn
#define DBG_IF_I2C_IRQ_HANDLER              I2C2_IRQHandler
#define DBG_IF_I2C_IRQ_CH_PRIORITY          3 /* lowest priority (valid range 0-3) */
#define DBG_IF_I2C_IRQ_CH_CMD               ENABLE /* enable or disable */
#define DBG_IF_I2C_TIMING                   ((uint32_t )(0x0050184F))
#define DBG_IF_I2C_OWN_ADDRESS              0x30F

#endif /* end of DEBUG_BOARD_PRESENT */

/* Flush macro used in assert */
#ifdef UART_DMA_ENABLE
# define FlushUart()                            DMA_Cmd(DBG_UART_TX_DMA_Channel, DISABLE)
#else
# define FlushUart()
#endif


/* ########################################################################## */
/* #    I N T E R R U P T  A S S I G N M E N T S                            # */
/* ########################################################################## */
/** Interrupt Channels, Group & Priority assignments:
 Note that lower number = higher priority (subpriority). All priorities are in relationship to
 each other & the priority group so changing one may affect others.
 For STMF0xx series processors only priorities 0..3 are supported.
 */
#define DBG_UART_DMA_INT_PRIORITY               3   // Uart DMA channel priority
#define DBG_UART_INT_PRIORITY                   3   // Lowest priority

/* Haier Protocol Interrupt Priority */
#define HP_RX_TIMER_INT_PRIORITY                1
#define HP_TX_TIMER_INT_PRIORITY                2

#define NVIC_CH_ENABLE(IRQCh)                   NVIC_EnableIRQ(IRQCh)
#define NVIC_CH_DISABLE(IRQCh)                  NVIC_DisableIRQ(IRQCh)


/* ########################################################################## */
/* #    M I S C E L L A N E O U S                                           # */
/* ########################################################################## */
/* Device Unique ID register for STMF0xx series */
#define DEV_UID_OFFSET                          0x1FFFF7AC

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct _GPioPinInfo {
    uint32_t        rccPeriph;
    GPIO_TypeDef*   grp;
    uint16_t        pin;
} GPioPinInfo_t;

typedef enum MsgContextTag
{
    CTX_THREAD,     ///< Message sent from within a thread context
    CTX_ISR         ///< Message sent from ISR
} MsgContext;


typedef union DeviceUidTag
{
    uint32_t uidWords[3];
    uint8_t  uidBytes[12];
} DeviceUid_t;

typedef DMA_Channel_TypeDef* DMAChannel_t;

/* Diagnostic LEDs on this board */
enum _Leds {
    LED_LD2,
    LED_LD3,
    NUM_LEDS
};


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern DeviceUid_t   *gDevUniqueId;
extern GPioPinInfo_t DiagLEDs[NUM_LEDS];


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
void LED_Init(void);
void SystemClkConfig( void );

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
    extern uint32_t gStackMem;     // Note: This extern is defined in startup file

    return (__current_sp() < (uint32_t)&gStackMem)? CTX_THREAD : CTX_ISR;
}


#endif /* HW_SETUP_PURPLE_HAZE_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
