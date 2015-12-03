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

#include "common.h"
#include "Driver_Sensor.h"
#include "bma2x2.h"
#include "bmm050.h"
#include "bmg160.h"
#include "bosch_i2c_adapter.h"
#include "chip.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

#define OSP_SENSOR_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)

#define ACTIVE_HIGH         1
#define ACTIVE_LOW          0

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
  OSP_SENSOR_API_VERSION,
  OSP_SENSOR_DRV_VERSION
};

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static bma2x2_t bma2x2;
static struct bmm050 bmc150mag;
static struct bmg160_t bmg160;

/***************************************************************************************************
 *                                       ACCEL HAL APIs                                            *
 ***************************************************************************************************/

/****************************************************************************************************
 * @fn      Accx_GetVersion
 *          Get driver version.
 *
 * @param   data     Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION Accx_GetVersion (void *data)
{
  return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Accel_DumpRegisters
 *          Dumps the registers of the chosen Accelerometer device
 *
 ***************************************************************************************************/
static void _Accel_DumpRegisters( void )
{
    //TODO
    return;
}

/****************************************************************************************************
 * @fn      _Accel_ResetDevice
 *          Soft resets the Accelerometer.
 *
 ***************************************************************************************************/
static void _Accel_ResetDevice( void )
{
    /* Soft Reset device */
    bma2x2_soft_reset();  // Problem with I2C at 1.8V
    bma2x2.delay_msec(1);
}

/****************************************************************************************************
 * @fn      _Accel_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Accelerometer.
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
 * @fn      Accx_Initialize
 *          Initialization function for Accel
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Accx_Initialize(OSP_Sensor_SignalEvent_t cb, void *data, void *priv)
{
    _Accel_HardwareSetup(true);

    /* Init the Bosch Driver for accel */
    bma2x2.bus_write  = dev_i2c_write;
    bma2x2.bus_read   = dev_i2c_read;
    bma2x2.delay_msec = dev_i2c_delay;
    bma2x2_init(&bma2x2);

    /* Register state before changes */
    _Accel_DumpRegisters();

    /* Set key registers to default on startup */
    _Accel_ResetDevice();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    bma2x2_set_mode(BMA2x2_MODE_NORMAL);        // Normal power
    bma2x2_set_range(BMA2x2_RANGE_4G);          // set range 4g for grange
    bma2x2_set_bandwidth(BMA2x2_BW_31_25HZ );     // BW set to 31.25Hz, ODR = BW*2 (62.5Hz)

    /* Setup interrupt selections */
    bma2x2_set_newdata( BMA2x2_INT1_NDATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bma2x2_set_latch_int(BMA2x2_LATCH_DUR_250US);

    /* Registers after initialization */
    _Accel_DumpRegisters();

    /* Set to suspend mode till application requests data by enabling interrupt */
    //bma2x2_set_mode(BMA2x2_MODE_DEEP_SUSPEND); --> This requires additional setup in IntEnable call

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Accx_Uninitialize
 *          Uninitialize the Accel
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Accx_Uninitialize(void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Accx_Activate
 *          Activate the sensor
 *
 * @param   flags               Enable/Disable flag
 * @param   us_delay            Delay before actual activation
 * @param   fifo_num_samples    Fifo size
 * @param   max_report_latency  Maximum latency for reporting the data
 * @param   priv                Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Accx_Activate (uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv)
{
    /* Enables/Disables the New Data interrupt on the Accelerometer.
    Note: If device is set in low power/suspend mode then additional device needs to be
    reinitialized to the last operating mode */
    if (flags)
    {
        bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 1 );

        NVIC_ClearPendingIRQ(ACCEL_PINT_IRQn);

        /* Enable interrupt in the NVIC */
        NVIC_EnableIRQ(ACCEL_PINT_IRQn);
    }
    else
    {
        bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 0 );

        NVIC_DisableIRQ(ACCEL_PINT_IRQn);
        /* Set to suspend mode till application requests data by enabling interrupt */
        //bma2x2_set_mode(BMA2x2_MODE_DEEP_SUSPEND); --> This requires additional setup in enable
    }

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Accx_ReadData
 *          Reads data from Accelerometer's X, Y, Z data registers
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Accx_ReadData (void *data, uint16_t num_samples, void *priv)
{
    MsgAccelData *pxyzData = data;

    bma2x2acc_t acc;                //create object of type bma250acc_t

    bma2x2_read_accel_xyz(&acc);    //get acceleration data from sensor

    pxyzData->X = acc.x;
    pxyzData->Y = acc.y;
    pxyzData->Z = acc.z;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Accx_Deactivate
 *          Deactivate Accelerometer
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Accx_Deactivate (void *priv)
{
    bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 0 );
    NVIC_DisableIRQ(ACCEL_PINT_IRQn);
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Accx_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Accx_PowerControl(ARM_POWER_STATE state, void *priv)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            Accx_Deactivate(NULL);
            break;

        case ARM_POWER_LOW:
            bma2x2_set_mode(BMA2x2_MODE_STANDBY);
            break;

        case ARM_POWER_FULL:
            Accx_Activate(true,0,0,0,NULL);
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Accx_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Accx_GetDetails (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Accx_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Accx_InjectData (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Accel Driver Definition */
OSP_BUILD_DRIVER_SENSOR(Acc, Accx, NULL);

/***************************************************************************************************
 *                                         MAG HAL APIs                                            *
 ***************************************************************************************************/

/****************************************************************************************************
 * @fn      Magx_GetVersion
 *          Get driver version.
 *
 * @param   data     Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION Magx_GetVersion (void *data)
{
  return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Mag_DumpRegisters
 *          Dumps the registers of the chosen Magnetometer device
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
 * @fn      Magx_Initialize
 *          Initialization function for Mag
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Magx_Initialize(OSP_Sensor_SignalEvent_t cb, void *data, void *priv)
{
    uint8_t value;

    _Mag_HardwareSetup(true);

    /* Init the Bosch Driver for mag */
    bmc150mag.bus_write  = dev_i2c_write;
    bmc150mag.bus_read   = dev_i2c_read;
    bmc150mag.delay_msec = dev_i2c_delay;

    /* Initialize and set key registers to default on startup */
    bmm050_init(&bmc150mag);

    /* Register state before changes */
    _Mag_DumpRegisters();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    /* Enable all axis & Set DR Polarity to active high */
    value = 0x04;
    bmm050_write_register(BMM050_SENS_CNTL, &value, 1);

    /* Set data rate */
    bmm050_set_datarate(BMM050_DR_25HZ);

    bmm050_set_functional_state(BMM050_NORMAL_MODE);
    bmc150mag.delay_msec(1);

    /* Registers after initialization */
    _Mag_DumpRegisters();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Magx_Uninitialize
 *          Uninitialize the Mag
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Magx_Uninitialize(void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Magx_Activate
 *          Activate the sensor
 *
 * @param   flags               Enable/Disable flag
 * @param   us_delay            Delay before actual activation
 * @param   fifo_num_samples    Fifo size
 * @param   max_report_latency  Maximum latency for reporting the data
 * @param   priv                Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Magx_Activate (uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv)
{
    if (flags)
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
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Magx_ReadData
 *          Read data from Mag
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Magx_ReadData (void *data, uint16_t num_samples, void *priv)
{
    MsgMagData *pxyzData = data;
    struct bmm050_mdata_s32 mdata;

    bmm050_read_mdataXYZ_s32(&mdata);

    pxyzData->X = mdata.datax;
    pxyzData->Y = mdata.datay;
    pxyzData->Z = mdata.dataz;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Magx_Deactivate
 *          Deactivate Mag
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Magx_Deactivate (void *priv)
{
    bmm050_set_mag_drdy_interrupt( false, ACTIVE_HIGH );
    NVIC_DisableIRQ(MAG_PINT_IRQn);
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Magx_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Magx_PowerControl(ARM_POWER_STATE state, void *priv)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            Magx_Deactivate(NULL);
            break;

        case ARM_POWER_LOW:
            bmm050_set_functional_state(BMM050_SUSPEND_MODE);
            break;

        case ARM_POWER_FULL:
            Magx_Activate(true,0,0,0, NULL);
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Magx_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Magx_GetDetails (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Magx_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Magx_InjectData (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Mag Driver Definition */
OSP_BUILD_DRIVER_SENSOR(Mag, Magx, NULL);

/***************************************************************************************************
 *                                         GYRO HAL APIs                                           *
 ***************************************************************************************************/

/****************************************************************************************************
 * @fn      Gyrox_GetVersion
 *          Get driver version.
 *
 * @param   data     Private Argument (Unused)
 *
 * @return  Driver Version
 *
 ***************************************************************************************************/
static ARM_DRIVER_VERSION Gyrox_GetVersion (void *data)
{
  return DriverVersion;
}

/****************************************************************************************************
 * @fn      _Gyro_DumpRegisters
 *          Dumps the registers of the chosen Gyroscope device
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
 * @fn      Gyrox_Initialize
 *          Initialization function for Gyro
 *
 * @param   cb      Event Handler
 * @param   data    Data required for initialization
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Gyrox_Initialize(OSP_Sensor_SignalEvent_t cb, void *data, void *priv)
{

    /* Configure the interrupt GPIO and regulators (if any) for the Gyroscope */
    _Gyro_HardwareSetup(true);

    /* Init the Bosch Driver for Gyro */
    bmg160.bus_write  = dev_i2c_write;
    bmg160.bus_read   = dev_i2c_read;
    bmg160.delay_msec = dev_i2c_delay;
    bmg160_init(&bmg160);

    /* Register state before changes */
    _Gyro_DumpRegisters();

    /* Clear any existing interrupts */
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */

    bmg160_set_mode(BMG160_MODE_NORMAL);
    bmg160_set_bw(C_BMG160_BW_32Hz_U8X); //ODR = 100Hz
    bmg160_set_range_reg(C_BMG160_Zero_U8X); //Zero = 2000dps

    /* Interrupt setup */
    bmg160_set_int_data( BMG160_INT1_DATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bmg160_set_latch_int(BMG160_LATCH_DUR_250US);

    /* Registers after initialization */
    _Gyro_DumpRegisters();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Gyrox_Uninitialize
 *          Uninitialize the Gyro
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Gyrox_Uninitialize(void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Gyrox_Activate
 *          Activate the sensor
 *
 * @param   flags               Enable/Disable flag
 * @param   us_delay            Delay before actual activation
 * @param   fifo_num_samples    Fifo size
 * @param   max_report_latency  Maximum latency for reporting the data
 * @param   priv                Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Gyrox_Activate (uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency, void *priv)
{
    /* Enables/Disables the New Data interrupt on the Gyroscope */
    if (flags)
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
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Gyrox_ReadData
 *          Reads data from Gyroscope's X, Y, Z data registers
 *
 * @param   *data        Data pointer
 * @param   num_samples  Number of samples to be read
 * @param   priv         Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Gyrox_ReadData (void *data, uint16_t num_samples, void *priv)
{
    MsgGyroData *pxyzData = data;
    struct    bmg160_data_t gyro_raw;

    bmg160_get_dataXYZ(&gyro_raw);
    pxyzData->X = gyro_raw.datax;
    pxyzData->Y = gyro_raw.datay;
    pxyzData->Z = gyro_raw.dataz;
    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();

    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Gyrox_Deactivate
 *          Deactivate Gyro Interrupt
 *
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Gyrox_Deactivate (void *priv)
{
    NVIC_DisableIRQ(GYRO_PINT_IRQn);
    bmg160_set_data_en(0);
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Gyrox_PowerControl
 *          Control the power state of the sensor
 *
 * @param   state   Power state
 * @param   priv    Private Argument (Unused)
 *
 * @return  Return code: On success return ARM_DRIVER_OK
 *
 ***************************************************************************************************/
static int32_t Gyrox_PowerControl(ARM_POWER_STATE state, void *priv)
{
    switch (state)
    {
        case ARM_POWER_OFF:
            Gyrox_Deactivate(NULL);
            break;

        case ARM_POWER_LOW:
            bmg160_set_mode(BMG160_MODE_SUSPEND);
            break;

        case ARM_POWER_FULL:
            Gyrox_Activate(true,0,0,0,NULL);
            break;

        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }
    return ARM_DRIVER_OK;
}

/****************************************************************************************************
 * @fn      Gyrox_GetDetails
 *          Get sensor details
 *
 * @param   data    Pointer to the sensor details
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Gyrox_GetDetails (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/****************************************************************************************************
 * @fn      Gyrox_InjectData
 *          Feed dummy data
 *
 * @param   data    Simulated data
 * @param   priv    Private Argument (Unused)
 *
 * @return  Returns ARM_DRIVER_ERROR_UNSUPPORTED since function is currently not supported
 *
 ***************************************************************************************************/
static int32_t Gyrox_InjectData (void *data, void *priv)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;    //Do nothing
}

/* Gyro Driver Definition */
OSP_BUILD_DRIVER_SENSOR(Gyro, Gyrox, NULL);
