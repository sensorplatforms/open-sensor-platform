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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Common.h"
#include "i2c_driver.h"
#include <string.h>

/* Charset file */
#include "Font_CP437.c"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define LED_DRIVER_I2C_ADDR             0x60

#define CONFIG_REG_ADDR                 0x00
#define COLUMN_DATA_REG                 0x01
#define UPDATE_DATA_REG                 0x0C
#define LIGHT_EFFECT_REG                0x0D
#define AUDIO_EQ_REG                    0x0F

#define PRINTABLE_ASCII_MAX             0x5E

#define BANNER_MSG                      "Knowles Intelligent Interfaces!"
#define SIZE_OF_BANNER_MSG              sizeof(BANNER_MSG)

#define SCROLL_DELAY_TIME               MSEC_TO_TICS(50)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static uint8_t _displayBuffer[APP_MESSAGE_SIZE];
static AsfTimer _scrollTimer = NULL_TIMER;
static uint8_t _msgIdx = 0;
static uint16_t _msgSz = SIZE_OF_BANNER_MSG;
static uint8_t _charIdx = 0;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      WriteLedDriverReg
 *          Sends data to LED Driver's specified register address
 *
 ***************************************************************************************************/
static void WriteLedDriverReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LED_DRIVER_I2C_ADDR, regAddr, &data, 1, I2C_MASTER_WRITE );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ClearDisplay
 *          Helper routine to turn all LEDs off
 *
 ***************************************************************************************************/
static void ClearDisplay( void )
{
    uint16_t result = I2C_ERR_OK;
    const uint8_t allOff[] = {
        0, COLUMN_DATA_REG + 1, 0, COLUMN_DATA_REG + 2, 0, COLUMN_DATA_REG + 3,
        0, COLUMN_DATA_REG + 4, 0, COLUMN_DATA_REG + 5, 0, COLUMN_DATA_REG + 6,
        0, COLUMN_DATA_REG + 7, 0, UPDATE_DATA_REG, 0
    };
    /* Turn off all LEDs */
    I2C_Start_Transfer( LED_DRIVER_I2C_ADDR, COLUMN_DATA_REG, (uint8_t*)allOff, sizeof(allOff), I2C_MASTER_WRITE );
    result = I2C_Wait_Completion();
    ASF_assert(result != I2C_ERR_FAIL);
}


/****************************************************************************************************
 * @fn      UpdateDisplay
 *          Writes a display buffer to the matrix display
 *
 ***************************************************************************************************/
static void UpdateDisplay( uint8_t buffer[] )
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        WriteLedDriverReg( COLUMN_DATA_REG+i, buffer[i] );
    }
    /* Update */
    WriteLedDriverReg( UPDATE_DATA_REG, 0 );
}

#if 0 //Old scroll function
/****************************************************************************************************
 * @fn      ScrollBanner
 *          Helper routine to scroll a message across the display
 *
 ***************************************************************************************************/
static void ScrollCharacter( uint8_t inChar, uint32_t scrollDelayms )
{
    /* This routines scrolls an ASCII character across the 8x8 matrix display */
    uint8_t idx;
    uint8_t *charBitMap;
    static union _Db
    {
        uint64_t bf64;
        uint8_t  b[8];
    } dispBuffer = {0LL};

    idx = inChar;
    charBitMap = (uint8_t *)&CharSet[idx][0];

    /* Start with the last column (C7) and bring the character in */
    for (idx = 0; idx < 8; idx++)
    {
        dispBuffer.b[7] = charBitMap[idx];
        UpdateDisplay( &dispBuffer.b[0] );
        os_dly_wait(MSEC_TO_TICS(scrollDelayms));
        dispBuffer.bf64 >>= 8;
    }
}

#else //New scroll function that does not do delay inside but depends on task timers
/****************************************************************************************************
 * @fn      ScrollBanner
 *          Helper routine to scroll a message across the display
 *
 ***************************************************************************************************/
static void ScrollBanner( void )
{
    /* This routines scrolls an ASCII character across the 8x8 matrix display */
    static uint8_t *charBitMap, currChar;
    static union _Db
    {
        uint64_t bf64;
        uint8_t  b[8];
    } dispBuffer = {0LL};

    if (_charIdx == 0)
    {
        currChar = _displayBuffer[_msgIdx];
        _msgIdx = (_msgIdx + 1) % _msgSz;
        charBitMap = (uint8_t *)&CharSet[currChar][0];
    }
    else
    {
        dispBuffer.bf64 >>= 8;
    }

    /* Start with the last column (C7) and bring the character in */
    dispBuffer.b[7] = charBitMap[_charIdx];
    UpdateDisplay( &dispBuffer.b[0] );
    _charIdx = (_charIdx + 1) % 8;
}
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      DebugIfTask
 *          This task handles interfacing to debug board.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK void DebugIfTask( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;

    I2C_HardwareSetup();
    I2C_Master_Initialise();

    D1_printf(":: Debug IF Task Ready ::\r\n");

    /* Copy our initial message to the message buffer */
    memcpy(_displayBuffer, BANNER_MSG, (SIZE_OF_BANNER_MSG > APP_MESSAGE_SIZE? APP_MESSAGE_SIZE : SIZE_OF_BANNER_MSG));
    /* Start a 50ms timer for scrolling */
    ASFTimerStart( DBG_TASK_ID, TIMER_REF_LED_SCROLL, SCROLL_DELAY_TIME, &_scrollTimer );

    while(1)
    {
        ASFReceiveMessage( DBG_TASK_ID, &rcvMsg );

        switch (rcvMsg->msgId)
        {
        case MSG_CONSOLE_CMD_RCVD:
            memcpy(_displayBuffer, rcvMsg->msg.msgConsoleData.msgBuffer, rcvMsg->msg.msgConsoleData.msgSz);
            ClearDisplay();
            _msgIdx = 0; //Reset buffer index so that next timer message will start the string from begining
            _msgSz = rcvMsg->msg.msgConsoleData.msgSz;
            _charIdx = 0;
            break;

        case MSG_TIMER_EXPIRY:
            if (rcvMsg->msg.msgTimerExpiry.userValue == TIMER_REF_LED_SCROLL) //User reference values to distinguish between multiple timers
            {
                /* Get the display rolling */
                ScrollBanner();
                /* We need to restart the timer since its a one-shot */
                ASFTimerStart( DBG_TASK_ID, TIMER_REF_LED_SCROLL, SCROLL_DELAY_TIME, &_scrollTimer );
            }
            break;

        default:
            break;
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
