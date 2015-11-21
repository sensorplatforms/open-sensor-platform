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
#include "Common.h"
#include "I2C_Driver.h"
#include "Bosch_I2C_Adapter.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

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
 * @fn      dev_i2c_delay
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
void dev_i2c_delay(unsigned int msec)
{
    os_dly_wait(MSEC_TO_TICS(msec));
}


/****************************************************************************************************
 * @fn      dev_i2c_write
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
char dev_i2c_write(unsigned char dev_addr,unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt)
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( dev_addr, reg_addr, reg_data, cnt, I2C_MASTER_WRITE );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();

    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      dev_i2c_read
 *          adaptation of the Bosch API functions to the BOARD specific function
 *
 ***************************************************************************************************/
char dev_i2c_read(unsigned char dev_addr,unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt)
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( dev_addr, reg_addr, reg_data, cnt, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();

    return I2C_ERR_OK;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
