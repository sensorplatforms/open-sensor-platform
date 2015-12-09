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
#include "mag_common.h"
#include "mag_bmc150_i2c.h"

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
#define ACTIVE_HIGH         1
#define ACTIVE_LOW          0

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static struct bmm050 bmc150mag;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Mag_DumpRegisters
 *          Dumps the registers of the chosen Magnetometer device
 *
 ***************************************************************************************************/
static void Mag_DumpRegisters( void )
{
    //TODO
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Mag_Initialize
 *          Configures the Magnetometer device params & registers for the application.
 *
 * @param   option Normal/Motion sense or Sleep modes for the accels.
 *
 * @return  none
 *
 ***************************************************************************************************/
void Mag_Initialize( void )
{
    uint8_t value;

    /* Init the Bosch Driver for mag */
    bmc150mag.bus_write  = dev_i2c_write;
    bmc150mag.bus_read   = dev_i2c_read;
    bmc150mag.delay_msec = dev_i2c_delay;

    /* Initialize and set key registers to default on startup */
    bmm050_init(&bmc150mag);

    /* Register state before changes */
    Mag_DumpRegisters();

    /* Clear any existing interrupts */
    Mag_ClearDataInt();

    /* Enable all axis & Set DR Polarity to active high */
    value = 0x04;
    bmm050_write_register(BMM050_SENS_CNTL, &value, 1);

    /* Set data rate */
    bmm050_set_datarate(BMM050_DR_25HZ);

    bmm050_set_functional_state(BMM050_NORMAL_MODE);
    dev_i2c_delay(1);

    /* Registers after initialization */
    Mag_DumpRegisters();
}


/****************************************************************************************************
 * @fn      Mag_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Magnetometer.
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
void Mag_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( MAG_INPUT_SENSOR );
        /* Note: Int enabled via Accel_ConfigDataInt() */
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( MAG_INPUT_SENSOR );
    }
}



/****************************************************************************************************
 * @fn      Mag_ReadData
 *          Reads data from Magnetometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Mag_ReadData( MagData_t *pxyzData )
{
    struct bmm050_mdata_s32 mdata;

    bmm050_read_mdataXYZ_s32(&mdata);

    pxyzData->X = mdata.datax;
    pxyzData->Y = mdata.datay;
    pxyzData->Z = mdata.dataz;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Mag_ClearDataInt
 *          Resets the New Data interrupt on the Magnetometer.
 *
 ***************************************************************************************************/
void Mag_ClearDataInt( void )
{
    volatile uint8_t value;
    /* Interrupts for DRDY is cleared on data reads */
    bmm050_read_register(BMM050_DATAX_LSB, (uint8_t*)&value, 1);
    bmm050_read_register(BMM050_DATAX_MSB, (uint8_t*)&value, 1);
    bmm050_read_register(BMM050_R_LSB, (uint8_t*)&value, 1);
    value = value;
}


/****************************************************************************************************
 * @fn      Mag_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the Magnetometer.
 *          Note: If device is set in low power/suspend mode then additional device needs to be
 *          reinitialized to the last operating mode
 *
 ***************************************************************************************************/
void Mag_ConfigDataInt( osp_bool_t enInt )
{
    if (enInt)
    {
        bmm050_set_mag_drdy_interrupt( true, ACTIVE_HIGH );
        /* Enable interrupt in the NVIC */
        NVIC_EnableIRQ(MAG_PINT_IRQn);
    }
    else
    {
        bmm050_set_mag_drdy_interrupt( false, ACTIVE_HIGH );
        NVIC_DisableIRQ(MAG_PINT_IRQn);
    }
}


/****************************************************************************************************
 * @fn      Mag_ResetDevice
 *          Soft resets the Magnetometer.
 *
 ***************************************************************************************************/
void Mag_ResetDevice( void )
{
    /* Soft Reset device */
    bmm050_soft_reset();  // Problem with I2C at 1.8V, OK for YS-SH
    dev_i2c_delay(1);
}


/****************************************************************************************************
 * @fn      Mag_SelfTest
 *          Runs self test on the Magnetometer. Returns true if passed.
 *
 ***************************************************************************************************/
osp_bool_t Mag_SelfTest( void )
{
    return true; //Not supported
}

/****************************************************************************************************
 * @fn      Mag_TriggerDataAcq
 *          Configures mag to be in forced triggered mode for sampling.
 *
 ***************************************************************************************************/
void Mag_TriggerDataAcq( void )
{
    MagData_t xyzData;

    Mag_ReadData(NULL);
    Mag_ConfigDataInt(true);
    bmm050_set_functional_state(BMM050_FORCED_MODE);

    (void)xyzData; //Avoid compiler warning
}


/****************************************************************************************************
 * @fn      Mag_SetLowPowerMode
 *          Configures mag to be in power saving mode
 *
 ***************************************************************************************************/
void Mag_SetLowPowerMode( void )
{
    bmm050_set_functional_state(BMM050_SUSPEND_MODE);
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
