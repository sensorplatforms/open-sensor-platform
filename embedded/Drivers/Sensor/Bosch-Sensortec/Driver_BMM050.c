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
#include "Driver_Sensor.h"
#include "bmm050.h"
#include "bosch_i2c_adapter.h"
#include "chip.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

#define OSP_SENSOR_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR( 1,0 )

#define ACTIVE_HIGH         1
#define ACTIVE_LOW          0

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    OSP_SENSOR_API_VERSION,
    OSP_SENSOR_DRV_VERSION
};

static struct bmm050 bmc150mag;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      OSP_Mag_GetVersion
 *          Get driver version
 *
 * @param   data     Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION OSP_Mag_GetVersion( void *data )
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Mag_DumpRegisters
 *          Dumps the registers of the chosen Magnetometer device
 *
 * @param   None
 *
 * @return  None
 *
 ***************************************************************************************************/
static void _Mag_DumpRegisters( void )
{
    //TODO
    return;
}

/****************************************************************************************************
 * @fn      _Mag_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Magnetometer.
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
static void _Mag_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( MAG_INPUT_SENSOR );
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( MAG_INPUT_SENSOR );
    }
}

/****************************************************************************************************
 * @fn      OSP_Mag_Initialize
 *          Initialization function for Mag
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_Initialize( OSP_Sensor_SignalEvent_t cb, void *data, void *priv )
{
    uint8_t value;

    _Mag_HardwareSetup( true );

    /* Init the Bosch Driver for mag */
    bmc150mag.bus_write  = dev_i2c_write;
    bmc150mag.bus_read   = dev_i2c_read;
    bmc150mag.delay_msec = dev_i2c_delay;

    /* Initialize and set key registers to default on startup */
    bmm050_init( &bmc150mag );

    /* Register state before changes */
    _Mag_DumpRegisters();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    /* Enable all axis & Set DR Polarity to active high */
    value = 0x04;
    bmm050_write_register( BMM050_SENS_CNTL, &value, 1 );

    /* Set data rate */
    bmm050_set_datarate( BMM050_DR_25HZ );

    bmm050_set_functional_state( BMM050_NORMAL_MODE );
    bmc150mag.delay_msec( 1 );

    /* Registers after initialization */
    _Mag_DumpRegisters();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Mag_Uninitialize
 *          Uninitialize the Mag
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_Uninitialize( void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Mag_Activate
 *          Activate the sensor
 *
 * @param   flags               Sensor Flag - SENSOR_FLAG* (Unused)
 * @param   us_delay            Delay before actual activation
 * @param   fifo_num_samples    Fifo size
 * @param   max_report_latency  Maximum latency for reporting the data
 * @param   priv                Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_Activate( uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv )
{
    bmm050_set_mag_drdy_interrupt( true, ACTIVE_HIGH );

    /* Enable interrupt in the NVIC */
    NVIC_EnableIRQ( MAG_PINT_IRQn );

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Mag_ReadData
 *          Read data from Mag
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_ReadData( void *data, uint16_t num_samples, void *priv )
{
    MsgMagData *pxyzData = data;
    struct bmm050_mdata_s32 mdata;
    uint16_t i = 0;

    for (; i<num_samples; i++ )
    {
        bmm050_read_mdataXYZ_s32( &mdata );

        pxyzData->X = mdata.datax;
        pxyzData->Y = mdata.datay;
        pxyzData->Z = mdata.dataz;

        //add timestamp here!
        pxyzData->timeStamp = RTC_GetCounter();
        pxyzData++;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Mag_Deactivate
 *          Deactivate Mag
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_Deactivate( void *priv )
{
    bmm050_set_mag_drdy_interrupt( false, ACTIVE_HIGH );
    NVIC_DisableIRQ( MAG_PINT_IRQn );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Mag_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_PowerControl( ARM_POWER_STATE state, void *priv )
{
    switch ( state )
    {
        case ARM_POWER_OFF:
            OSP_Mag_Deactivate( NULL );
            break;

        case ARM_POWER_LOW:
            bmm050_set_functional_state( BMM050_SUSPEND_MODE );
            break;

        case ARM_POWER_FULL:
            OSP_Mag_Activate( SENSOR_FLAG_ONE_SHOT_MODE,NULL /* unused */,NULL /* unused */,NULL /* unused */, NULL );
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Mag_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_GetDetails( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Mag_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Mag_InjectData( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Mag Driver Definition */
OSP_BUILD_DRIVER_SENSOR( Mag, OSP_Mag, NULL );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
