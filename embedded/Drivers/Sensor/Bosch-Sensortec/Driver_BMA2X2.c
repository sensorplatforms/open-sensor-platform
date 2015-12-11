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
#include "bma2x2.h"
#include "bosch_i2c_adapter.h"
#include "chip.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

#define OSP_SENSOR_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR( 1,0 )

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    OSP_SENSOR_API_VERSION,
    OSP_SENSOR_DRV_VERSION
};

static bma2x2_t bma2x2;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      OSP_Accel_GetVersion
 *          Get driver version
 *
 * @param   data    Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION OSP_Accel_GetVersion( void *data )
{
    return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Accel_DumpRegisters
 *          Dumps the registers of the chosen Accelerometer device
 *
 * @param   None
 *
 * @return  None
 *
 ***************************************************************************************************/
static void _Accel_DumpRegisters( void )
{
    //TODO
    return;
}

/****************************************************************************************************
 * @fn      _Accel_ResetDevice
 *          Soft resets the Accelerometer
 *
 * @param   None
 *
 * @return  None
 *
 ***************************************************************************************************/
static void _Accel_ResetDevice( void )
{
    /* Soft Reset device */
    bma2x2_soft_reset();  // Problem with I2C at 1.8V
    bma2x2.delay_msec( 1 );
}

/****************************************************************************************************
 * @fn      _Accel_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Accelerometer
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
static void _Accel_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( ACCEL_INPUT_SENSOR );
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( ACCEL_INPUT_SENSOR );
    }
}

/****************************************************************************************************
 * @fn      OSP_Accel_Initialize
 *          Initialization function for Accel
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_Initialize( OSP_Sensor_SignalEvent_t cb, void *data, void *priv )
{
    _Accel_HardwareSetup( true );

    /* Init the Bosch Driver for accel */
    bma2x2.bus_write  = dev_i2c_write;
    bma2x2.bus_read   = dev_i2c_read;
    bma2x2.delay_msec = dev_i2c_delay;
    bma2x2_init( &bma2x2 );

    /* Register state before changes */
    _Accel_DumpRegisters();

    /* Set key registers to default on startup */
    _Accel_ResetDevice();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    bma2x2_set_mode( BMA2x2_MODE_NORMAL );        // Normal power
    bma2x2_set_range( BMA2x2_RANGE_4G );          // set range 4g for grange
    bma2x2_set_bandwidth( BMA2x2_BW_31_25HZ );    // BW set to 31.25Hz, ODR = BW*2 (62.5Hz)

    /* Setup interrupt selections */
    bma2x2_set_newdata( BMA2x2_INT1_NDATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bma2x2_set_latch_int( BMA2x2_LATCH_DUR_250US );

    /* Registers after initialization */
    _Accel_DumpRegisters();

    /* Set to suspend mode till application requests data by enabling interrupt */
    //bma2x2_set_mode( BMA2x2_MODE_DEEP_SUSPEND ); --> This requires additional setup in IntEnable call

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Accel_Uninitialize
 *          Uninitialize the Accel
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_Uninitialize( void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Accel_Activate
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
static int32_t OSP_Accel_Activate( uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv )
{
    /* Enables/Disables the New Data interrupt on the Accelerometer.
    Note: If device is set in low power/suspend mode then additional device needs to be
    reinitialized to the last operating mode */
    bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 1 );

    NVIC_ClearPendingIRQ( ACCEL_PINT_IRQn );

    /* Enable interrupt in the NVIC */
    NVIC_EnableIRQ( ACCEL_PINT_IRQn );

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Accel_ReadData
 *          Reads data from Accelerometer's X, Y, Z data registers
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_ReadData( void *data, uint16_t num_samples, void *priv )
{
    MsgAccelData *pxyzData = data;
    uint16_t i = 0;

    bma2x2acc_t acc;                //create object of type bma250acc_t

    for (; i<num_samples; i++)
    {
        bma2x2_read_accel_xyz( &acc );    //get acceleration data from sensor

        pxyzData->X = acc.x;
        pxyzData->Y = acc.y;
        pxyzData->Z = acc.z;

        //add timestamp here!
        pxyzData->timeStamp = RTC_GetCounter();
        pxyzData++;
    }

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Accel_Deactivate
 *          Deactivate Accelerometer
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_Deactivate( void *priv )
{
    bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 0 );
    NVIC_DisableIRQ( ACCEL_PINT_IRQn );
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Accel_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_PowerControl( ARM_POWER_STATE state, void *priv )
{
    switch (state)
    {
        case ARM_POWER_OFF:
            OSP_Accel_Deactivate( NULL );
            break;

        case ARM_POWER_LOW:
            bma2x2_set_mode( BMA2x2_MODE_STANDBY );
            break;

        case ARM_POWER_FULL:
            OSP_Accel_Activate( SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */,NULL /* unused */,NULL );
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      OSP_Accel_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_GetDetails( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      OSP_Accel_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t OSP_Accel_InjectData( void *data, void *priv )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Accel Driver Definition */
OSP_BUILD_DRIVER_SENSOR( Accel, OSP_Accel, NULL );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
