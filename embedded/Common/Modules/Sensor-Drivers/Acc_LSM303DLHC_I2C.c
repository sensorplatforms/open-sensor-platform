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
#include "Acc_LSM303DLHC_I2C.h"
#include "I2C_Driver.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

//#define DEBUG_XL


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      WriteAccReg
 *          Sends data to digital accelerometer's specified register address
 *
 ***************************************************************************************************/
static void WriteAccReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_A_I2C_ADDRESS_7BIT, regAddr, &data, 1, I2C_MASTER_WRITE );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


#ifdef DEBUG_XL
/****************************************************************************************************
 * @fn      ReadAccReg
 *          Reads data from digital accelerometer's specified register address
 *
 ***************************************************************************************************/
static uint8_t ReadAccReg( uint8_t regAddr )
{
    uint8_t result;
    uint8_t retVal;

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_A_I2C_ADDRESS_7BIT, regAddr, &retVal, 1, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();

    return retVal;
}
#endif


/****************************************************************************************************
 * @fn      ReadAccMultiByte
 *          Reads data > 1 byte from digital accelerometer's specified register start address
 *
 ***************************************************************************************************/
static void ReadAccMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
{
    uint8_t result;
    regAddr = regAddr | 0x80;     /* For multi-byte read MSB of sub-addr field should be 1 */

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_A_I2C_ADDRESS_7BIT, regAddr, pBuffer, count, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      Accel_DumpRegisters
 *          Dumps the registers of the chosen Accelerometer device
 *
 ***************************************************************************************************/
static void Accel_DumpRegisters( void )
{
#ifdef DEBUG_XL
    D0_printf("\n------- LSM303DLHC-Accel Register Dump -------\r\n");
    D0_printf("\t CTRL_REG1           (0x20) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG1          ));
    D0_printf("\t CTRL_REG2           (0x21) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG2          ));
    D0_printf("\t CTRL_REG3           (0x22) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG3          ));
    D0_printf("\t CTRL_REG4           (0x23) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG4          ));
    D0_printf("\t CTRL_REG5           (0x24) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG5          ));
    D0_printf("\t CTRL_REG6           (0x25) %02X\r\n", ReadAccReg( LSM_A_CTRL_REG6          ));
    D0_printf("\t REFERENCE_REG       (0x26) %02X\r\n", ReadAccReg( LSM_A_REFERENCE_REG      ));
    D0_printf("\t STATUS_REG          (0x27) %02X\r\n", ReadAccReg( LSM_A_STATUS_REG         ));
    D0_printf("\t OUT_X_L             (0x28) %02X\r\n", ReadAccReg( LSM_A_OUT_X_L            ));
    D0_printf("\t OUT_X_H             (0x29) %02X\r\n", ReadAccReg( LSM_A_OUT_X_H            ));
    D0_printf("\t OUT_Y_L             (0x2A) %02X\r\n", ReadAccReg( LSM_A_OUT_Y_L            ));
    D0_printf("\t OUT_Y_H             (0x2B) %02X\r\n", ReadAccReg( LSM_A_OUT_Y_H            ));
    D0_printf("\t OUT_Z_L             (0x2C) %02X\r\n", ReadAccReg( LSM_A_OUT_Z_L            ));
    D0_printf("\t OUT_Z_H             (0x2D) %02X\r\n", ReadAccReg( LSM_A_OUT_Z_H            ));
    delay_ms(50);
    D0_printf("\t FIFO_CTRL_REG       (0x2E) %02X\r\n", ReadAccReg( LSM_A_FIFO_CTRL_REG      ));
    D0_printf("\t FIFO_SRC_REG        (0x2F) %02X\r\n", ReadAccReg( LSM_A_FIFO_SRC_REG       ));
    D0_printf("\t INT1_CFG_REG        (0x30) %02X\r\n", ReadAccReg( LSM_A_INT1_CFG_REG       ));
    D0_printf("\t INT1_SRC_REG        (0x31) %02X\r\n", ReadAccReg( LSM_A_INT1_SRC_REG       ));
    D0_printf("\t INT1_THS_REG        (0x32) %02X\r\n", ReadAccReg( LSM_A_INT1_THS_REG       ));
    D0_printf("\t INT1_DURATION_REG   (0x33) %02X\r\n", ReadAccReg( LSM_A_INT1_DURATION_REG  ));
    D0_printf("\t INT2_CFG_REG        (0x34) %02X\r\n", ReadAccReg( LSM_A_INT2_CFG_REG       ));
    D0_printf("\t INT2_SCR_REG        (0x35) %02X\r\n", ReadAccReg( LSM_A_INT2_SCR_REG       ));
    D0_printf("\t INT2_THS_REG        (0x36) %02X\r\n", ReadAccReg( LSM_A_INT2_THS_REG       ));
    D0_printf("\t INT2_DURATION_REG   (0x37) %02X\r\n", ReadAccReg( LSM_A_INT2_DURATION_REG  ));
    D0_printf("\t CLICK_CFG_REG       (0x38) %02X\r\n", ReadAccReg( LSM_A_CLICK_CFG_REG      ));
    D0_printf("\t CLICK_SRC_REG       (0x39) %02X\r\n", ReadAccReg( LSM_A_CLICK_SRC_REG      ));
    D0_printf("\t CLICK_THS_REG       (0x3A) %02X\r\n", ReadAccReg( LSM_A_CLICK_THS_REG      ));
    D0_printf("\t TIME_LIMIT          (0x3B) %02X\r\n", ReadAccReg( LSM_A_TIME_LIMIT         ));
    D0_printf("\t TIME_LATENCY        (0x3C) %02X\r\n", ReadAccReg( LSM_A_TIME_LATENCY       ));
    D0_printf("\t TIME_WINDOW         (0x3D) %02X\r\n", ReadAccReg( LSM_A_TIME_WINDOW        ));
    D0_printf("----------------------------------------------\r\n");
    delay_ms(50);
#endif
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Accel_Initialize
 *          Configures the accelerometer device params & registers for the application.
 *
 * @param   option Normal/Motion sense or Sleep modes for the accels.
 *
 * @return  none
 *
 ***************************************************************************************************/
void Accel_Initialize( AccelInitOption option )
{
    uint8_t ctrl1;
    uint8_t ctrl4;

    Accel_DumpRegisters();
    /* Set key registers to default on startup */
    Accel_ResetDevice();

    /* Clear any existing interrupts */
    Accel_ClearDataInt();

    /* Enable All axis, Output Data rate = 200Hz, Normal Operating mode */
    ctrl1 = (uint8_t)( LSM_Acc_ODR_100 | LSM_Acc_XYZEN );
    WriteAccReg( LSM_A_CTRL_REG1, ctrl1 );

    /* Disable all Filtering */
    WriteAccReg( LSM_A_CTRL_REG2, 0 );

    /* Enable DRDY1 on INT1 for enabling interrupt based accel data reads */
    //ctrl3 = LSM_Acc_DRDY1_onINT1; --> done in Accel_ConfigDataInt()
    Accel_ConfigDataInt( false ); //Important: Causes stuck int to get cleared from prev inits

    /* FS = +/- 4g, Block Data Update disable, Enable High Resolution Mode (needed for normal mode) */
    ctrl4 = (uint8_t) ( LSM_Acc_FS_4g | LSM_Acc_HighRes_Out_Mode );
    WriteAccReg( LSM_A_CTRL_REG4, ctrl4 );

    /* Latch interrupt request on INT1, FIFO disabled */
    //ctrl5 = LSM_Acc_Latch_INT1;
    //WriteAccReg( LSM_A_CTRL_REG5, ctrl5 );

    WriteAccReg( LSM_A_CTRL_REG6, 0 );

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
void Accel_HardwareSetup( Bool enable )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( ACCEL_BUS ) ); //TBD - can be made bus agnostic!

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_ACCEL_INT_GPIO, ENABLE );

        /* Configure INT interrupt Pin */
        GPIO_InitStructure.GPIO_Pin = ACCEL_INT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init (ACCEL_INT_GPIO_GRP, &GPIO_InitStructure);

        GPIO_EXTILineConfig(GPIO_PORT_SRC_ACCEL_INT, GPIO_PIN_SRC_ACCEL_INT);

        EXTI_ClearFlag(EXTI_LINE_ACCEL_INT);

        EXTI_InitStructure.EXTI_Line = EXTI_LINE_ACCEL_INT;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(ACCEL_INT_IRQChannel);
        NVIC_InitStructure.NVIC_IRQChannel = ACCEL_INT_IRQChannel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ACCEL_A_INT_PREEMPT_PRIO;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = ACCEL_A_INT_SUB_PRIORITY;
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
 *          Reads data from accelerometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Accel_ReadData( AccelData_t *pxyzData )
{
    int16_t xyzData[3];

    ReadAccMultiByte( LSM_A_OUT_X_L, (uint8_t *)xyzData, 6 );

    /* Assign axis values based on device orientation. */
    pxyzData->X = xyzData[0] >> 4; //12-bit resolution, lower 4 bits are don't care
    pxyzData->Y = xyzData[1] >> 4;
    pxyzData->Z = xyzData[2] >> 4;

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Accel_ClearDataInt
 *          Resets the New Data interrupt on the accel.
 *
 ***************************************************************************************************/
void Accel_ClearDataInt( void )
{
    /* Reading of INT1 source register will clear the IA bit and the interrupt signal on the INT1 pin */
    //volatile uint8_t intSrc = ReadAccReg( LSM_A_INT1_SRC_REG );
    /* DRDY interrupt is only cleared by reading data registers */
    volatile int16_t xyzData[3];

    ReadAccMultiByte( LSM_A_OUT_X_L, (uint8_t *)xyzData, 6 );
    (void)xyzData; //avoid compiler warning
}


/****************************************************************************************************
 * @fn      Accel_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the accel.
 *
 ***************************************************************************************************/
void Accel_ConfigDataInt( Bool enInt )
{
    uint8_t ctrl3;

    /* Enable DRDY1 on INT1 for enabling interrupt based accel data reads */
    if (enInt)
    {
        ctrl3 = LSM_Acc_DRDY1_onINT1;
    }
    else
    {
        ctrl3 = 0;
        Accel_ClearDataInt();
    }
    WriteAccReg( LSM_A_CTRL_REG3, ctrl3 );
}


/****************************************************************************************************
 * @fn      Accel_ResetDevice
 *          Soft resets the accelerometer.
 *
 ***************************************************************************************************/
void Accel_ResetDevice( void )
{
    /* Set control registers to default values  */
    WriteAccReg( LSM_A_CTRL_REG1, 0 );
    WriteAccReg( LSM_A_CTRL_REG2, 0 );
    WriteAccReg( LSM_A_CTRL_REG3, 0 );
    WriteAccReg( LSM_A_CTRL_REG4, 0 );
    WriteAccReg( LSM_A_CTRL_REG5, 0 );
}


/****************************************************************************************************
 * @fn      Accel_SelfTest
 *          Runs self test on the accelerometer. Returns true if passed.
 *
 ***************************************************************************************************/
Bool Accel_SelfTest( void )
{
    return true; //Not supported
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
