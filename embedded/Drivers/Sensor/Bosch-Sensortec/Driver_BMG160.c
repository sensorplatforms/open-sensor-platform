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
#include "bmg160.h"
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

static struct bmg160_t bmg160;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      OSP_Gyro_GetVersion
 *          Get driver version.
 *
 * @param   data     Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION OSP_Gyro_GetVersion( void *data )
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Gyro_DumpRegisters
 *          Dumps the registers of the chosen Gyroscope device
 *
 * @param   None
 *
 * @return  None
 *
 ***************************************************************************************************/
static void _Gyro_DumpRegisters( void )
{
    //TODO
    return;
}

/****************************************************************************************************
 * @fn      _Gyro_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Gyroscope.
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
static void _Gyro_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( GYRO_INPUT_SENSOR );
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( GYRO_INPUT_SENSOR );
    }
}

/****************************************************************************************************
 * @fn      OSP_Gyro_Initialize
 *          Initialization function for Gyro
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_Initialize( OSP_Sensor_SignalEvent_t cb, void *data, void *priv )
{

    /* Configure the interrupt GPIO and regulators (if any) for the Gyroscope */
    _Gyro_HardwareSetup( true );

    /* Init the Bosch Driver for Gyro */
    bmg160.bus_write  = dev_i2c_write;
    bmg160.bus_read   = dev_i2c_read;
    bmg160.delay_msec = dev_i2c_delay;
    bmg160_init( &bmg160 );

    /* Register state before changes */
    _Gyro_DumpRegisters();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    bmg160_set_mode( BMG160_MODE_NORMAL );
    bmg160_set_bw( C_BMG160_BW_32Hz_U8X ); //ODR = 100Hz
    bmg160_set_range_reg( C_BMG160_Zero_U8X ); //Zero = 2000dps

    /* Interrupt setup */
    bmg160_set_int_data( BMG160_INT1_DATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bmg160_set_latch_int( BMG160_LATCH_DUR_250US );

    /* Registers after initialization */
    _Gyro_DumpRegisters();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_Uninitialize
 *          Uninitialize the Gyro
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_Uninitialize( void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_Activate
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
static int32_t OSP_Gyro_Activate( uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv )
{
    /* Enables the New Data interrupt on the Gyroscope */
    bmg160_set_data_en( 1 );

    /* Enable interrupt in the NVIC */
    NVIC_EnableIRQ( GYRO_PINT_IRQn );

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_ReadData
 *          Reads data from Gyroscope's X, Y, Z data registers
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_ReadData( void *data, uint16_t num_samples, void *priv )
{
    MsgGyroData *pxyzData = data;
    struct    bmg160_data_t gyro_raw;
    uint16_t i = 0;

    for (; i<num_samples; i++)
    {
        bmg160_get_dataXYZ( &gyro_raw );
        pxyzData->X = gyro_raw.datax;
        pxyzData->Y = gyro_raw.datay;
        pxyzData->Z = gyro_raw.dataz;
        //add timestamp here!
        pxyzData->timeStamp = RTC_GetCounter();
        pxyzData++;
    }

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_Deactivate
 *          Deactivate Gyro Interrupt
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_Deactivate( void *priv )
{
    NVIC_DisableIRQ( GYRO_PINT_IRQn );
    bmg160_set_data_en( 0 );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_PowerControl( ARM_POWER_STATE state, void *priv )
{
    switch (state)
    {
        case ARM_POWER_OFF:
            OSP_Gyro_Deactivate( NULL );
            break;

        case ARM_POWER_LOW:
            bmg160_set_mode( BMG160_MODE_SUSPEND );
            break;

        case ARM_POWER_FULL:
            OSP_Gyro_Activate( SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */, NULL /* unused */,NULL );
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_GetDetails( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Gyro_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Gyro_InjectData( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Gyro Driver Definition */
OSP_BUILD_DRIVER_SENSOR( Gyro, OSP_Gyro, NULL );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
