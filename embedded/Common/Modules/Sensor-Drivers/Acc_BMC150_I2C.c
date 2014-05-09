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
#include "Acc_Common.h"
#include "Acc_BMC150_I2C.h"
#include "bma2x2.h"
#include "I2C_Driver.h"
#include "Bosch_I2C_Adapter.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

//#define _DEBUG_


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
#define delay_ms(msec)      os_dly_wait(MSEC_TO_TICS(msec))
#define PORT_ACCIRQREQ                  4
#define PIN_ACCIRQREQ                   11
#define ACCEL_INT_IRQCh                 PIN_INT0_IRQn
#define ACCEL_INT_PRIO                  2

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

#if 0
/****************************************************************************************************
 * @fn      WriteAccReg
 *          Sends data to digital Accelerometer's specified register address
 *
 ***************************************************************************************************/
static void WriteAccReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_A_I2C_ADDRESS_7BIT, regAddr, &data, 1, I2C_MASTER_WRITE );
    APP_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ReadAccReg
 *          Reads data from digital Accelerometer's specified register address
 *
 ***************************************************************************************************/
static uint8_t ReadAccReg( uint8_t regAddr )
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
 * @fn      ReadAccMultiByte
 *          Reads data > 1 byte from digital Accelerometer's specified register start address
 *
 ***************************************************************************************************/
static void ReadAccMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
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
 * @fn      Accel_DumpRegisters
 *          Dumps the registers of the chosen Accelerometer device
 *
 ***************************************************************************************************/
static void Accel_DumpRegisters( void )
{
#ifdef _DEBUG_
    D0_printf("\n------- BOSCH BMC150 Accel Register Dump -------\r\n");
    D0_printf("\t CTRL_REG1           (0x20) %02X\r\n", ReadAccReg( REG1          ));
    delay_ms(50);
    D0_printf("\t FIFO_CTRL_REG       (0x2E) %02X\r\n", ReadAccReg( REG2          ));
    D0_printf("------------------------------------------------\r\n");
    delay_ms(50);
#endif
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
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( I2C_SENSOR_BUS ) );

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_ACCEL_INT_GPIO, ENABLE );

        /* Configure INT interrupt Pin */
        GPIO_InitStructure.GPIO_Pin   = ACCEL_INT;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; //Don't care for input mode
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //Don't care for input mode
        GPIO_Init (ACCEL_INT_GPIO_GRP, &GPIO_InitStructure);

        /* Enable SYSCFG clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        /* Connect the EXTI Line to the port pin */
        SYSCFG_EXTILineConfig(EXTI_PORT_SRC_ACCEL_INT, EXTI_PIN_SRC_ACCEL_INT);

        EXTI_ClearFlag(EXTI_LINE_ACCEL_INT);

        EXTI_InitStructure.EXTI_Line = EXTI_LINE_ACCEL_INT;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(ACCEL_INT_IRQChannel);
        NVIC_InitStructure.NVIC_IRQChannel = ACCEL_INT_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ACCEL_INT_PREEMPT_PRIO;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = ACCEL_INT_SUB_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        /* Int enabled via Accel_ConfigDataInt() */
    }
    else //TODO: disable interrupts and free GPIOs
    {
        //TODO!!
    }

}



/****************************************************************************************************
 * @fn      Accel_ReadData
 *          Reads data from Accelerometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Accel_ReadData( AccelData_t *pxyzData )
{
    static uint32_t lastRtc = 0;
    bma2x2acc_t acc;                //create object of type bma250acc_t
    bma2x2_read_accel_xyz(&acc);    //get acceleration data from sensor

    pxyzData->X = acc.x;
    pxyzData->Y = acc.y;
    pxyzData->Z = acc.z;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
    if (pxyzData->timeStamp < lastRtc)
    {
        AccelTimeExtend++; //Rollover counter
    }
    lastRtc = pxyzData->timeStamp;
}


/****************************************************************************************************
 * @fn      Accel_ClearDataInt
 *          Resets the New Data interrupt on the Accelerometer.
 *
 ***************************************************************************************************/
void Accel_ClearDataInt( void )
{
}


/****************************************************************************************************
 * @fn      Accel_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the Accelerometer.
 *
 ***************************************************************************************************/
void Accel_ConfigDataInt( osp_bool_t enInt )
{
    if (enInt)
    {
        bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 1 );
    }
    else
    {
        bma2x2_set_Int_Enable( BMA2x2_DATA_EN, 0 );
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
    bma2x2_soft_reset();  // Problem with I2C at 1.8V OK for YS-SH
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
