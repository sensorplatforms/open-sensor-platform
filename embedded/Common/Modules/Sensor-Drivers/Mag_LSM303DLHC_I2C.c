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
#include "Mag_Common.h"
#include "Mag_LSM303DLHC_I2C.h"
#include "I2C_Driver.h"

//#define DEBUG_MAG
/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern AsfTaskHandle ossTaskHandleTable[];

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define delay_ms(msec)          os_dly_wait(MSEC_TO_TICS(msec))

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Mag axis data format */
typedef union {
    int16_t word;

    struct {
        int8_t msb;
        uint8_t lsb;
    } byte;
} mag_axis_t;

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
 * @fn      mag_axis_val
 *          Construct a signed value from a raw axis sample
 *
 * The mag registers returns 2's-complement 16-bit formatted axis values stored within a
 * 16-bit word.
 *
 * @param   axis    A mag_axis_t type storing a raw sensor axis sample.
 * @return  The sign extended axis sample value.
 ***************************************************************************************************/
static __inline int16_t mag_axis_val(const mag_axis_t axis)
{
    return ((axis.byte.msb << 8) | axis.byte.lsb);
}


/****************************************************************************************************
 * @fn      WriteMagReg
 *          Sends data to digital magnetometer's specified register address
 *
 ***************************************************************************************************/
static void WriteMagReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_M_I2C_ADDRESS_7BIT, regAddr, &data, 1, I2C_MASTER_WRITE );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ReadMagReg
 *          Reads data from digital magnetometer's specified register address
 *
 ***************************************************************************************************/
#ifdef DEBUG_MAG //Only dump function using it right now... so disable it to avoid compiler warnings
static uint8_t ReadMagReg( uint8_t regAddr )
{
    uint8_t result;
    uint8_t retVal;

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_M_I2C_ADDRESS_7BIT, regAddr, &retVal, 1, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();

    return retVal;
}
#endif


/****************************************************************************************************
 * @fn      ReadMagMultiByte
 *          Reads data > 1 byte from digital magnetometer's specified register start address
 *
 ***************************************************************************************************/
static void ReadMagMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
{
    uint8_t result;
    regAddr = regAddr | 0x80;     /* For multi-byte read MSB of sub-addr field should be 1 */

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( LSM_M_I2C_ADDRESS_7BIT, regAddr, pBuffer, count, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      Mag_DumpRegisters
 *          Dumps the registers of the chosen magnetometer device
 *
 ***************************************************************************************************/
static void Mag_DumpRegisters( void )
{
#ifdef DEBUG_MAG
    D0_printf("\n------- LSM303DLHC-Mag Register Dump -------\r\n");
    D0_printf("\t CRA_REG         (0x00) %02X\r\n", ReadMagReg( LSM_M_CRA_REG       ));
    D0_printf("\t CRB_REG         (0x01) %02X\r\n", ReadMagReg( LSM_M_CRB_REG       ));
    D0_printf("\t MR_REG          (0x02) %02X\r\n", ReadMagReg( LSM_M_MR_REG        ));
    D0_printf("\t OUT_X_H         (0x03) %02X\r\n", ReadMagReg( LSM_M_OUT_X_H       ));
    D0_printf("\t OUT_X_L         (0x04) %02X\r\n", ReadMagReg( LSM_M_OUT_X_L       ));
    D0_printf("\t OUT_Z_H         (0x05) %02X\r\n", ReadMagReg( LSM_M_OUT_Z_H       ));
    D0_printf("\t OUT_Z_L         (0x06) %02X\r\n", ReadMagReg( LSM_M_OUT_Z_L       ));
    D0_printf("\t OUT_Y_H         (0x07) %02X\r\n", ReadMagReg( LSM_M_OUT_Y_H       ));
    D0_printf("\t OUT_Y_L         (0x08) %02X\r\n", ReadMagReg( LSM_M_OUT_Y_L       ));
    D0_printf("\t SR_REG          (0x09) %02X\r\n", ReadMagReg( LSM_M_SR_REG        ));
    D0_printf("\t IRA_REG         (0x0A) %02X\r\n", ReadMagReg( LSM_M_IRA_REG       ));
    D0_printf("\t IRB_REG         (0x0B) %02X\r\n", ReadMagReg( LSM_M_IRB_REG       ));
    D0_printf("\t IRC_REG         (0x0C) %02X\r\n", ReadMagReg( LSM_M_IRC_REG       ));
    D0_printf("\t TEMPERATURE_H   (0x31) %02X\r\n", ReadMagReg( LSM_M_TEMPERATURE_H ));
    D0_printf("\t TEMPERATURE_L   (0x32) %02X\r\n", ReadMagReg( LSM_M_TEMPERATURE_L ));
    D0_printf("-------------------------------------\r\n");
    delay_ms(50);
#endif
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Mag_Initialize
 *          Configures the magnetometer device params & registers for the application.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void Mag_Initialize( void )
{
    uint8_t regCRA, regCRB, regMR;

    Mag_DumpRegisters();
    /* Set output data rate, mag range/gain & conversion mode */
    regCRA = LSM_Mag_ODR_30;
    regCRB = LSM_Mag_Range_2_5G;
    regMR = LSM_Mag_MODE_CONTINUOS;

    WriteMagReg( LSM_M_CRA_REG, regCRA );
    WriteMagReg( LSM_M_CRB_REG, regCRB );
    WriteMagReg( LSM_M_MR_REG,  regMR );

    Mag_DumpRegisters();
}


/****************************************************************************************************
 * @fn      Mag_HardwareSetup
 *          Configures the GPIOs and h/w interface to talk to the magnetometer.
 *
 * @param   enable - Enables or disables communication interface
 *
 * @return  none
 *
 ***************************************************************************************************/
void Mag_HardwareSetup( Bool enable )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( MAG_BUS ) ); //TBD - can be made bus agnostic!

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_MAG_RDY_GPIO, ENABLE );

        /* Configure DRDY/INT interrupt Pin */
        GPIO_InitStructure.GPIO_Pin = MAG_RDY_INT_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init (MAG_RDY_INT_GRP, &GPIO_InitStructure);

        GPIO_EXTILineConfig(GPIO_PORT_SRC_MAG_RDY_INT, GPIO_PIN_SRC_MAG_RDY_INT);

        EXTI_ClearFlag(MAG_RDY_INT_EXTI_LINE);

        EXTI_InitStructure.EXTI_Line = MAG_RDY_INT_EXTI_LINE;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(MAG_RDY_IRQCHANNEL);
        NVIC_InitStructure.NVIC_IRQChannel = MAG_RDY_IRQCHANNEL;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MAG_DRDY_INT_PREEMPT_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = MAG_DRDY_INT_SUB_PRIORITY;
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
 *          Reads data from magnetometer's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Mag_ReadData( MagData_t *pxyzData )
{
    mag_axis_t axisData[3];

    ReadMagMultiByte( LSM_M_OUT_X_H, (uint8_t *)axisData, 6 );

    /* Assign axis values based on device orientation. */
    pxyzData->X = mag_axis_val(axisData[0]);
    pxyzData->Y = mag_axis_val(axisData[2]);
    pxyzData->Z = mag_axis_val(axisData[1]);

    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Mag_TriggerDataAcq
 *          Triggers data acquisition in the mag sensor if such a feature exist
 *
 ***************************************************************************************************/
void Mag_TriggerDataAcq( void )
{
    /* Initiate single measurement */
    WriteMagReg( LSM_M_MR_REG, LSM_Mag_MODE_SINGLE );
}


/****************************************************************************************************
 * @fn      Mag_ClearDataInt
 *          Resets the New Data interrupt on the mag.
 *
 ***************************************************************************************************/
void Mag_ClearDataInt( void )
{
#if 0 //Mag DRDY is not latched so this can be skipped
    /* DRDY interrupt is only cleared by reading data registers */
    volatile int16_t xyzData[3];

    ReadMagMultiByte( LSM_M_OUT_X_H, (uint8_t *)xyzData, 6 );
    (void)xyzData; //avoid compiler warning
#endif
}


/****************************************************************************************************
 * @fn      Mag_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the mag.
 *
 ***************************************************************************************************/
void Mag_ConfigDataInt( Bool enInt )
{
    /* Data sheet not clear about how DRDY signal is enabled */
    if (enInt)
    {
        /* Enable DRDY */
        NVIC_CH_ENABLE( MAG_RDY_IRQCHANNEL );
    }
    else
    {
        /* Disable DRDY */
        NVIC_CH_DISABLE( MAG_RDY_IRQCHANNEL );
    }
}


/****************************************************************************************************
 * @fn      Mag_ResetDevice
 *          Soft resets the magnetometer.
 *
 ***************************************************************************************************/
void Mag_ResetDevice( void )
{
    /* Not available */
}


/****************************************************************************************************
 * @fn      Mag_SelfTest
 *          Runs self test on the magnetometer. Returns true if passed.
 *
 ***************************************************************************************************/
Bool Mag_SelfTest( void )
{
    /* No self test supported */
    return true;
}


/****************************************************************************************************
 * @fn      Mag_SetLowPowerMode
 *          Configures mag to be in power saving mode
 *
 ***************************************************************************************************/
void Mag_SetLowPowerMode( void )
{
    /* Initiate Sleep Mode */
    WriteMagReg( LSM_M_MR_REG, LSM_Mag_MODE_SLEEP );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
