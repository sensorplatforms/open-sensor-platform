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


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

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
 * @fn      ValidInputData
 *          Validate the input data against the expected value as per the parsing algorithm.
 *
 ***************************************************************************************************/
osp_bool_t ValidInputData( uint8_t inByte )
{
    static uint8_t curState = TOKEN_NULL;

    switch (curState)
    {
        case TOKEN_NULL:
            if (inByte == TOKEN_1)
            {
                curState = TOKEN_1;
                return true;
            }
            break;

        case TOKEN_1:
            if (inByte == TOKEN_2)
            {
                curState = TOKEN_2;
                return true;
            }
            break;

        case TOKEN_2:
            if (inByte == TOKEN_3)
            {
                curState = TOKEN_3;
                return true;
            }
            break;

        case TOKEN_3:
            if (inByte == TOKEN_4)
            {
                curState = TOKEN_PARAM;
                return true;
            }
            break;

        case TOKEN_PARAM: //Remain in this state till you get CR/LF
            if ((inByte == '\r') || (inByte == '\n'))
            {
                curState = TOKEN_NULL;
            }
            return true;
    }
    return false;
}


/****************************************************************************************************
 * @fn      CmdParse_User
 *          Called by command handler task to parse input serial string
 *
 * @param   pBuffer: pointer to received buffer
 * @param   size:    number of bytes received (always < COMMAND_LINE_SIZE)
 * @param   appMode: application mode (allows mode specific parsing)
 *
 * @return  none
 *
 ***************************************************************************************************/
void CmdParse_User( int8_t *pBuffer, uint16_t size )
{
    pBuffer[size] = '\0';
    sscanf((char*)pBuffer, "log=%x", &g_logging);
    D1_printf("Received %d bytes: %s\r\n", size, pBuffer);
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
