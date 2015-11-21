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
#include "gyro_common.h"
#include "gyro_bmg160_i2c.h"
#include "i2c_driver.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif



/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern uint32_t GyroTimeExtend;

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
static struct bmg160_t bmg160;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Gyro_DumpRegisters
 *          Dumps the registers of the chosen Gyroscope device
 *
 ***************************************************************************************************/
static void Gyro_DumpRegisters( void )
{
    //TODO
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Gyro_Initialize
 *          Configures the Gyroscope device params & registers for the application.
 *
 * @param   option Normal/Motion sense or Sleep modes for the accels.
 *
 * @return  none
 *
 ***************************************************************************************************/
void Gyro_Initialize( void )
{
    /* Init the Bosch Driver for Gyro */
    bmg160.bus_write  = dev_i2c_write;
    bmg160.bus_read   = dev_i2c_read;
    bmg160.delay_msec = dev_i2c_delay;
    bmg160_init(&bmg160);


    /* Register state before changes */
    Gyro_DumpRegisters();

    /* Set key registers to default on startup */
    //Gyro_ResetDevice(); //Causing I2C problems- Bosch says its causes SDA to get stuck low

    /* Clear any existing interrupts */
    Gyro_ClearDataInt();

    bmg160_set_mode(BMG160_MODE_NORMAL);
    bmg160_set_bw(C_BMG160_BW_32Hz_U8X); //ODR = 100Hz
    bmg160_set_range_reg(C_BMG160_Zero_U8X); //Zero = 2000dps

    /* Interrupt setup */
    bmg160_set_int_data( BMG160_INT1_DATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bmg160_set_latch_int(BMG160_LATCH_DUR_250US);

    /* Registers after initialization */
    Gyro_DumpRegisters();
}


/****************************************************************************************************
 * @fn      Gyro_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Gyroscope.
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
void Gyro_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( GYRO_INPUT_SENSOR );
        /* Note: Int enabled via Accel_ConfigDataInt() */
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( GYRO_INPUT_SENSOR );
    }
}



/****************************************************************************************************
 * @fn      Gyro_ReadData
 *          Reads data from Gyroscope's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Gyro_ReadData( GyroData_t *pxyzData )
{
    struct    bmg160_data_t gyro_raw;

    bmg160_get_dataXYZ(&gyro_raw);
    pxyzData->X = gyro_raw.datax;
    pxyzData->Y = gyro_raw.datay;
    pxyzData->Z = gyro_raw.dataz;
    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Gyro_ClearDataInt
 *          Resets the New Data interrupt on the Gyroscope.
 *
 ***************************************************************************************************/
void Gyro_ClearDataInt( void )
{
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */
}


/****************************************************************************************************
 * @fn      Gyro_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the Gyroscope.
 *
 ***************************************************************************************************/
void Gyro_ConfigDataInt( osp_bool_t enInt )
{
    if (enInt)
    {
        bmg160_set_data_en(1);

        /* Enable interrupt in the NVIC */
        NVIC_EnableIRQ(GYRO_PINT_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(GYRO_PINT_IRQn);
        bmg160_set_data_en(0);
    }
}


/****************************************************************************************************
 * @fn      Gyro_ResetDevice
 *          Soft resets the Gyroscope.
 *
 ***************************************************************************************************/
void Gyro_ResetDevice( void )
{
    /* Soft Reset device */
    bmg160_set_soft_reset(); // Problem with I2C at 1.8V
    dev_i2c_delay(10);
}


/****************************************************************************************************
 * @fn      Gyro_SelfTest
 *          Runs self test on the Gyroscope. Returns true if passed.
 *
 ***************************************************************************************************/
osp_bool_t Gyro_SelfTest( void )
{
    return true; //Not supported
}

/****************************************************************************************************
 * @fn      Gyro_SetLowPowerMode
 *          Configures gyro to be in power saving mode
 *
 ***************************************************************************************************/
void Gyro_SetLowPowerMode( void )
{
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
