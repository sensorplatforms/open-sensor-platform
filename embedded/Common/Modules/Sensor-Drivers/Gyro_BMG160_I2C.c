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
#include "Gyro_Common.h"
#include "Gyro_BMG160_I2C.h"
#include "I2C_Driver.h"
#include "BMG160.h"
#include "Bosch_I2C_Adapter.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

//#define _DEBUG_


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
#define delay_ms(msec)      os_dly_wait(MSEC_TO_TICS(msec))

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
#if 0
/****************************************************************************************************
 * @fn      WriteGyroReg
 *          Sends data to digital Gyroscope's specified register address
 *
 ***************************************************************************************************/
static void WriteGyroReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( _SENSOR_ADDR_7BIT_, regAddr, &data, 1, I2C_MASTER_WRITE );
    APP_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ReadGyroReg
 *          Reads data from digital Gyroscope's specified register address
 *
 ***************************************************************************************************/
static uint8_t ReadGyroReg( uint8_t regAddr )
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
 * @fn      ReadGyroMultiByte
 *          Reads data > 1 byte from digital Gyroscope's specified register start address
 *
 ***************************************************************************************************/
static void ReadGyroMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
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
 * @fn      Gyro_DumpRegisters
 *          Dumps the registers of the chosen Gyroscope device
 *
 ***************************************************************************************************/
static void Gyro_DumpRegisters( void )
{
#ifdef _DEBUG_
    D0_printf("\n------- __SENSOR__ Register Dump -------\r\n");
    D0_printf("\t CTRL_REG1           (0x20) %02X\r\n", ReadGyroReg( REG1          ));
    delay_ms(50);
    D0_printf("\t FIFO_CTRL_REG       (0x2E) %02X\r\n", ReadGyroReg( REG2          ));
    D0_printf("----------------------------------------------\r\n");
    delay_ms(50);
#endif
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
    //Gyro_ResetDevice(); //Causing I2C problems

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
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( I2C_SENSOR_BUS ) );

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_GYRO_INT_GPIO, ENABLE );

        /* Configure INT interrupt Pin */
        GPIO_InitStructure.GPIO_Pin   = GYRO_INT;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; //Don't care for input mode
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //Don't care for input mode
        GPIO_Init (GYRO_INT_GPIO_GRP, &GPIO_InitStructure);

        /* Enable SYSCFG clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        /* Connect the EXTI Line to the port pin */
        SYSCFG_EXTILineConfig(EXTI_PORT_SRC_GYRO_INT, EXTI_PIN_SRC_GYRO_INT);

        EXTI_ClearFlag(EXTI_LINE_GYRO_INT);

        EXTI_InitStructure.EXTI_Line = EXTI_LINE_GYRO_INT;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(GYRO_INT_IRQChannel);
        NVIC_InitStructure.NVIC_IRQChannel = GYRO_INT_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = GYRO_INT_PREEMPT_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = GYRO_INT_SUB_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        /* Int enabled via Gyro_ConfigDataInt() */
    }
    else //TODO: disable interrupts and free GPIOs
    {
        //TODO!!
    }

}



/****************************************************************************************************
 * @fn      Gyro_ReadData
 *          Reads data from Gyroscope's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Gyro_ReadData( GyroData_t *pxyzData )
{
    static uint32_t lastRtc = 0;
    struct    bmg160_data_t gyro_raw;

    bmg160_get_dataXYZ(&gyro_raw);
    pxyzData->X = gyro_raw.datax;
    pxyzData->Y = gyro_raw.datay;
    pxyzData->Z = gyro_raw.dataz;
    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
    if (pxyzData->timeStamp < lastRtc)
    {
        GyroTimeExtend++; //Rollover counter
    }
    lastRtc = pxyzData->timeStamp;
}


/****************************************************************************************************
 * @fn      Gyro_ClearDataInt
 *          Resets the New Data interrupt on the Gyroscope.
 *
 ***************************************************************************************************/
void Gyro_ClearDataInt( void )
{
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
    }
    else
    {
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
    bmg160_set_soft_reset(); // Problem with I2C at 1.8V, OK for YS-SH
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
