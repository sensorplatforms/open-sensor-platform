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
#include "acc_common.h"
#include "acc_bmc150_i2c.h"
#include "i2c_driver.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern uint32_t AccelTimeExtend;

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
static bma2x2_t bma2x2;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Accel_DumpRegisters
 *          Dumps the registers of the chosen Accelerometer device
 *
 ***************************************************************************************************/
static void Accel_DumpRegisters( void )
{
    //TODO
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Accel_Initialize
 *          Configures the Accelerometer device params & registers for the application.
 *
 * @param   option Normal/Motion sense or Sleep modes for the accels.
 *
 * @return  none
 *
 ***************************************************************************************************/
void Accel_Initialize( AccelInitOption option )
{
    /* Init the Bosch Driver for accel */
    bma2x2.bus_write  = dev_i2c_write;
    bma2x2.bus_read   = dev_i2c_read;
    bma2x2.delay_msec = dev_i2c_delay;
    bma2x2_init(&bma2x2);

    /* Register state before changes */
    Accel_DumpRegisters();

    /* Set key registers to default on startup */
    Accel_ResetDevice();

    /* Clear any existing interrupts */
    Accel_ClearDataInt();

    bma2x2_set_mode(BMA2x2_MODE_NORMAL);        // Normal power
    bma2x2_set_range(BMA2x2_RANGE_4G);          // set range 4g for grange
    //bma2x2_set_bandwidth(BMA2x2_BW_125HZ );     // BW set to 125Hz, ODR = BW*2
    //bma2x2_set_bandwidth(BMA2x2_BW_62_50HZ );     // BW set to 62.5Hz, ODR = BW*2
    bma2x2_set_bandwidth(BMA2x2_BW_31_25HZ );     // BW set to 31.25Hz, ODR = BW*2 (62.5Hz)

    /* Setup interrupt selections */
    bma2x2_set_newdata( BMA2x2_INT1_NDATA, 1 ); //DRDY interrupt on INT1 pad
    /* Latched with 250uS duration */
    bma2x2_set_latch_int(BMA2x2_LATCH_DUR_250US);

    /* Registers after initialization */
    Accel_DumpRegisters();

    /* Set to suspend mode till application requests data by enabling interrupt */
    //bma2x2_set_mode(BMA2x2_MODE_DEEP_SUSPEND); --> This requires additional setup in IntEnable call
}


/****************************************************************************************************
 * @fn      Accel_HardwareSetup
 *          Configures the interrupt GPIO and regulators (if any) for the Accelerometer.
 *
 * @param   enable - Enables or disables the GPIOs / Communications interface
 *
 * @return  none
 *
 ***************************************************************************************************/
void Accel_HardwareSetup( osp_bool_t enable )
{
    if (enable == true)
    {
        /* Initialize the Sensor interface HW (typically I2C or SPI) */
        Board_SensorIfInit( ACCEL_INPUT_SENSOR );
        /* Note: Int enabled via Accel_ConfigDataInt() */
    }
    else
    {
        /* Disable interrupts and free GPIOs */
        Board_SensorIfDeinit( ACCEL_INPUT_SENSOR );
    }
}



/****************************************************************************************************
 * @fn      Accel_ReadData
 *          Reads data from Accelerometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Accel_ReadData( AccelData_t *pxyzData )
{
    bma2x2acc_t acc;                //create object of type bma250acc_t

    bma2x2_read_accel_xyz(&acc);    //get acceleration data from sensor

    pxyzData->X = acc.x;
    pxyzData->Y = acc.y;
    pxyzData->Z = acc.z;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Accel_ClearDataInt
 *          Resets the New Data interrupt on the Accelerometer.
 *
 ***************************************************************************************************/
void Accel_ClearDataInt( void )
{
    /* Note: DRDY is normally cleared on Data read but otherwise since we don't latch DRDY interrupt,
       we don't need to do data read to clear any pending interrupts */
}


/****************************************************************************************************
 * @fn      Accel_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the Accelerometer.
 *          Note: If device is set in low power/suspend mode then additional device needs to be
 *          reinitialized to the last operating mode
 *
 ***************************************************************************************************/
void Accel_ConfigDataInt( osp_bool_t enInt )
{
    if (enInt)
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
}


/****************************************************************************************************
 * @fn      Accel_ResetDevice
 *          Soft resets the Accelerometer.
 *
 ***************************************************************************************************/
void Accel_ResetDevice( void )
{
    /* Soft Reset device */
    bma2x2_soft_reset();  // Problem with I2C at 1.8V
    dev_i2c_delay(1);
}


/****************************************************************************************************
 * @fn      Accel_SelfTest
 *          Runs self test on the Accelerometer. Returns true if passed.
 *
 ***************************************************************************************************/
osp_bool_t Accel_SelfTest( void )
{
    return true; //Not supported
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
