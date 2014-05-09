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
#if !defined (MAIN_H)
#define   MAIN_H

#define I2C_DRIVER                  /* Include I2C Driver */
#define INTERRUPT_BASED_SAMPLING    /* Sensor sampling is interrupt driver */
//#define PUBLISH_RAW               /* Publish raw sensor counts instead of standard units */
#define TRIGGERED_MAG_SAMPLING      /* Magnetometer sampling is software triggered */

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"              /* STM32F1xx Library Definitions     */
#include "appversion.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#ifdef ANDROID_DEMO
# define ANDROID_COMM_TASK              I2CSLAVE_COMM_TASK_ID
#endif
#define ALGORITHM_TASK                  ALGORITHM_TASK_ID

#ifdef DEBUG_BUILD
# ifndef DEBUG_OUTPUT
# define DEBUG_OUTPUT
# endif
#endif

//#define RESET_ON_ASSERT

#ifdef RESET_ON_ASSERT
# define SysRESET()         NVIC_SystemReset()
#else
# define SysRESET()         while(1)
#endif


#define MAX_SYSTEM_MESSAGES                     30     ///< Max number of queued messages in the system

/* All timer references (arbitrary unique identifiers for each timer)*/
#define TIMER_REF_RTC_UPDATE                    0x55A5
#define TIMER_REF_SENSOR_READ                   0x55B0

/* Printf and assert support for debugging */
#ifdef DEBUG_OUTPUT
# define DEBUG_LVL      1
#else
# ifndef DEBUG_LVL
#  define DEBUG_LVL     0
# endif
#endif

#if (DEBUG_LVL > 0)
# ifdef UART_DMA_ENABLE
#  define MAX_DPRINTF_MESSAGES                  30  ///< Max printf messages allowed at a given time
# else
#  define TX_BUFFER_SIZE                        512
# endif
# define RX_BUFFER_SIZE                         32
# define DPRINTF_BUFF_SIZE                      200

#else //DEBUG_LVL = 0

# ifndef UART_DMA_ENABLE
#  define TX_BUFFER_SIZE                        512
# endif
# define RX_BUFFER_SIZE                         200
# define MAX_DPRINTF_MESSAGES                   10   ///< Max printf messages allowed at a given time
# define DPRINTF_BUFF_SIZE                      100

#endif

/* Defines for command handler */
#define COMMAND_LINE_SIZE           32

/* Sensor acquisition related definitions */

#if !defined INTERRUPT_BASED_SAMPLING
# define SENSOR_SAMPLE_PERIOD           MSEC_TO_TICS(20)   //tick
# define MAG_DECIMATE_FACTOR            1
# define ACCEL_SAMPLE_DECIMATE          1
# define GYRO_SAMPLE_DECIMATE           1
#else
# define MAG_DECIMATE_FACTOR            2
# define ACCEL_SAMPLE_DECIMATE          2
# define GYRO_SAMPLE_DECIMATE           2
#endif

#ifdef TRIGGERED_MAG_SAMPLING
# define MAG_TRIGGER_RATE_DECIMATE      2 //1/2 of Accel ODR
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Serial command parser tokens */
enum ParserTokensTag {
    TOKEN_NULL = 0,
    TOKEN_1 = 'l',
    TOKEN_2 = 'o',
    TOKEN_3 = 'g',
    TOKEN_4 = '=',
    TOKEN_STATS = '\r',
    TOKEN_PARAM = 0xAA
};

/* RTC clock */
typedef struct RtcClockTag
{
    uint16_t hours;
    uint8_t  minutes;
    uint8_t  seconds;
    uint16_t msec;
} RtcClock_t;

typedef MsgAccelData AccelData_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern RCC_ClocksTypeDef gRccClockInfo;
extern uint32_t g_logging;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Updates RTC counter based on system tick counter */
void UpdateRTC( void );
int Print_LIPS( const char *fmt, ... ); //Serial protocol for data results out


#endif /* MAIN_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
