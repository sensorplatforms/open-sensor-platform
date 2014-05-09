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
#include "i2c_driver.h"
#include "bmm050.h"
#include "bosch_i2c_adapter.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

//#define _DEBUG_


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern uint32_t MagTimeExtend;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define delay_ms(msec)      os_dly_wait(MSEC_TO_TICS(msec))

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
#if 0
/****************************************************************************************************
 * @fn      WriteMagReg
 *          Sends data to digital Magnetometer's specified register address
 *
 ***************************************************************************************************/
static void WriteMagReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_A_I2C_ADDRESS_7BIT, regAddr, &data, 1, I2C_MASTER_WRITE );
    APP_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ReadMagReg
 *          Reads data from digital Magnetometer's specified register address
 *
 ***************************************************************************************************/
static uint8_t ReadMagReg( uint8_t regAddr )
{
    uint8_t result;
    uint8_t retVal;

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( _SENSOR_ADDR_7BIT_, regAddr, &retVal, 1, I2C_MASTER_READ );
    APP_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();

    return retVal;
}


/****************************************************************************************************
 * @fn      ReadMagMultiByte
 *          Reads data > 1 byte from digital Magnetometer's specified register start address
 *
 ***************************************************************************************************/
static void ReadMagMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
{
    uint8_t result;
    regAddr = regAddr | 0x80;     /* For multi-byte read MSB of sub-addr field should be 1 */

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( _SENSOR_ADDR_7BIT_, regAddr, pBuffer, count, I2C_MASTER_READ );
    APP_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();
}
#endif

/****************************************************************************************************
 * @fn      Mag_DumpRegisters
 *          Dumps the registers of the chosen Magnetometer device
 *
 ***************************************************************************************************/
static void Mag_DumpRegisters( void )
{
#ifdef _DEBUG_
    D0_printf("\n------- __SENSOR__ Register Dump -------\r\n");
    D0_printf("\t CTRL_REG1           (0x20) %02X\r\n", ReadMagReg( REG1          ));
    delay_ms(50);
    D0_printf("\t FIFO_CTRL_REG       (0x2E) %02X\r\n", ReadMagReg( REG2          ));
    D0_printf("----------------------------------------------\r\n");
    delay_ms(50);
#endif
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
    bmm050_init(&bmc150mag);


    /* Register state before changes */
    Mag_DumpRegisters();

    /* Set key registers to default on startup */
    //Mag_ResetDevice(); //Done in init call properly

    /* Clear any existing interrupts */
    Mag_ClearDataInt();

    /* Enable all axis & Set DR Polarity to active high */
    value = 0x04;
    bmm050_write_register(BMM050_SENS_CNTL, &value, 1);

    //bmm050_set_functional_state(BMM050_FORCED_MODE);
    bmm050_set_functional_state(BMM050_NORMAL_MODE);
    dev_i2c_delay(1);
    bmm050_set_presetmode(BMM050_PRESETMODE_REGULAR);
    //bmm050_set_repetitions_XY(BMM050_REGULAR_REPXY); //Already done in PRESETMODE_REGULAR
    //bmm050_set_repetitions_Z(BMM050_REGULAR_REPZ);   //Already done in PRESETMODE_REGULAR


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
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( I2C_SENSOR_BUS ) );

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_MAG_INT_GPIO, ENABLE );

        /* Configure INT interrupt Pin */
        GPIO_InitStructure.GPIO_Pin   = MAG_INT;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; //Don't care for input mode
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //Don't care for input mode
        GPIO_Init (MAG_INT_GPIO_GRP, &GPIO_InitStructure);

        /* Enable SYSCFG clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        /* Connect the EXTI Line to the port pin */
        SYSCFG_EXTILineConfig(EXTI_PORT_SRC_MAG_INT, EXTI_PIN_SRC_MAG_INT);

        EXTI_ClearFlag(EXTI_LINE_MAG_INT);

        EXTI_InitStructure.EXTI_Line = EXTI_LINE_MAG_INT;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(MAG_INT_IRQChannel);
        NVIC_InitStructure.NVIC_IRQChannel = MAG_INT_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MAG_INT_PREEMPT_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = MAG_INT_SUB_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        /* Int enabled via Mag_ConfigDataInt() */
    }
    else //TODO: disable interrupts and free GPIOs
    {
        //TODO!!
    }

}



/****************************************************************************************************
 * @fn      Mag_ReadData
 *          Reads data from Magnetometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Mag_ReadData( MagData_t *pxyzData )
{
    static uint32_t lastRtc = 0;
    struct bmm050_mdata_s32 mdata;

    //bmm050_set_functional_state(BMM050_FORCED_MODE);
    bmm050_read_mdataXYZ_s32(&mdata);

    pxyzData->X = mdata.datax;
    pxyzData->Y = mdata.datay;
    pxyzData->Z = mdata.dataz;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
    if (pxyzData->timeStamp < lastRtc)
    {
        MagTimeExtend++; //Rollover counter
    }
    lastRtc = pxyzData->timeStamp;
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
 *
 ***************************************************************************************************/
void Mag_ConfigDataInt( osp_bool_t enInt )
{
    if (enInt)
    {
        bmm050_config_drdy_int(1);
    }
    else
    {
        bmm050_config_drdy_int(0);
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
 * @fn      Mag_SetLowPowerMode
 *          Configures mag to be in power saving mode
 *
 ***************************************************************************************************/
void Mag_SetLowPowerMode( void )
{
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
