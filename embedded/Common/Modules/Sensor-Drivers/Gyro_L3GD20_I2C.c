/****************************************************************************************************
 *                                                                                                  *
 *                                    Sensor Platforms Inc.                                         *
 *                                    2860 Zanker Road, Suite 210                                   *
 *                                    San Jose, CA 95134                                            *
 *                                                                                                  *
 ****************************************************************************************************
 *                                                                                                  *
 *                                Copyright (c) 2012 Sensor Platforms Inc.                          *
 *                                        All Rights Reserved                                       *
 *                                                                                                  *
 ***************************************************************************************************/
/**
 * @file GYRO_L3GD20_I2C.c
 * This file implements functions specific to setting up and communicating with ST's 3-axis
 * L3GD20 Gyroscope connected over I2C bus.
 *
 ***************************************************************************************************/

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Gyro_Common.h"
#include "Gyro_L3GD20_I2C.h"
#include "I2C_Driver.h"

#ifndef I2C_DRIVER
# error Needs I2C_DRIVER to be defined. Check Common.h
#endif

//#define DEBUG_GYRO
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
 * @fn      WriteGyroReg
 *          Sends data to digital gyroscope's specified register address
 *
 ***************************************************************************************************/
static void WriteGyroReg( uint8_t regAddr, uint8_t data )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( L3GD20_I2C_ADDRESS_7BIT, regAddr, &data, 1, I2C_MASTER_WRITE );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      ReadGyroReg
 *          Reads data from digital gyroscope's specified register address
 *
 ***************************************************************************************************/
static uint8_t ReadGyroReg( uint8_t regAddr )
{
    uint8_t result;
    uint8_t retVal;

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( L3GD20_I2C_ADDRESS_7BIT, regAddr, &retVal, 1, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();

    return retVal;
}


/****************************************************************************************************
 * @fn      ReadGyroMultiByte
 *          Reads data > 1 byte from digital gyroscope's specified register start address
 *
 ***************************************************************************************************/
static void ReadGyroMultiByte( uint8_t regAddr, uint8_t *pBuffer, uint8_t count )
{
    uint8_t result;
    regAddr = regAddr | 0x80;     /* For multi-byte read MSB of sub-addr field should be 1 */

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Start_Transfer( L3GD20_I2C_ADDRESS_7BIT, regAddr, pBuffer, count, I2C_MASTER_READ );
    ASF_assert(result == I2C_ERR_OK);

    /* Wait for status */
    I2C_Wait_Completion();
}


/****************************************************************************************************
 * @fn      Gyro_DumpRegisters
 *          Dumps the registers of the chosen gyroscope device
 *
 ***************************************************************************************************/
static void Gyro_DumpRegisters( void )
{
#ifdef DEBUG_GYRO
    D0_printf("\n------- L3GD20 Gyroscope Register Dump -------\r\n");
    D0_printf("\t WHO_AM_I     (0x0F) %02X\r\n", ReadGyroReg( L3GD20_WHO_AM_I      ));
    D0_printf("\t CTRL_REG1    (0x20) %02X\r\n", ReadGyroReg( L3GD20_CTRL_REG1     ));
    D0_printf("\t CTRL_REG2    (0x21) %02X\r\n", ReadGyroReg( L3GD20_CTRL_REG2     ));
    D0_printf("\t CTRL_REG3    (0x22) %02X\r\n", ReadGyroReg( L3GD20_CTRL_REG3     ));
    D0_printf("\t CTRL_REG4    (0x23) %02X\r\n", ReadGyroReg( L3GD20_CTRL_REG4     ));
    D0_printf("\t CTRL_REG5    (0x24) %02X\r\n", ReadGyroReg( L3GD20_CTRL_REG5     ));
    D0_printf("\t REFERENCE    (0x25) %02X\r\n", ReadGyroReg( L3GD20_REFERENCE_REG ));
    D0_printf("\t OUT_TEMP     (0x26) %02X\r\n", ReadGyroReg( L3GD20_OUT_TEMP      ));
    D0_printf("\t STATUS_REG   (0x27) %02X\r\n", ReadGyroReg( L3GD20_STATUS_REG    ));
    D0_printf("\t OUT_X_L      (0x28) %02X\r\n", ReadGyroReg( L3GD20_OUT_X_L       ));
    D0_printf("\t OUT_X_H      (0x29) %02X\r\n", ReadGyroReg( L3GD20_OUT_X_H       ));
    D0_printf("\t OUT_Y_L      (0x2A) %02X\r\n", ReadGyroReg( L3GD20_OUT_Y_L       ));
    delay_ms(50);
    D0_printf("\t OUT_Y_H      (0x2B) %02X\r\n", ReadGyroReg( L3GD20_OUT_Y_H       ));
    D0_printf("\t OUT_Z_L      (0x2C) %02X\r\n", ReadGyroReg( L3GD20_OUT_Z_L       ));
    D0_printf("\t OUT_Z_H      (0x2D) %02X\r\n", ReadGyroReg( L3GD20_OUT_Z_H       ));
    D0_printf("\t FIFO_CTRL_REG(0x2E) %02X\r\n", ReadGyroReg( L3GD20_FIFO_CTRL_REG ));
    D0_printf("\t FIFO_SRC_REG (0x2F) %02X\r\n", ReadGyroReg( L3GD20_FIFO_SRC_REG  ));
    D0_printf("\t INT1_CFG     (0x30) %02X\r\n", ReadGyroReg( L3GD20_INT1_CFG      ));
    D0_printf("\t INT1_SRC     (0x31) %02X\r\n", ReadGyroReg( L3GD20_INT1_SRC      ));
    D0_printf("\t INT1_TSH_XH  (0x32) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_XH   ));
    D0_printf("\t INT1_TSH_XL  (0x33) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_XL   ));
    D0_printf("\t INT1_TSH_YH  (0x34) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_YH   ));
    D0_printf("\t INT1_TSH_YL  (0x35) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_YL   ));
    D0_printf("\t INT1_TSH_ZH  (0x36) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_ZH   ));
    D0_printf("\t INT1_TSH_ZL  (0x37) %02X\r\n", ReadGyroReg( L3GD20_INT1_TSH_ZL   ));
    D0_printf("\t INT1_DURATION(0x38) %02X\r\n", ReadGyroReg( L3GD20_INT1_DURATION ));
    D0_printf("----------------------------------------------\r\n");
    delay_ms(50);
#endif
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Gyro_Initialize
 *          Configures the gyroscope device params & registers for the application.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void Gyro_Initialize( void )
{
    uint8_t temp;

    Gyro_DumpRegisters();

    /* Set gyro in Normal mode and enable the 3 axis, ODR = 95Hz, BW = 12.5Hz */
    //temp = L3GD20_ODR_95HZ | L3GD20_BANDWIDTH_1 | L3GD20_MODE_ACTIVE | L3GD20_ALL_AXES_ENABLE;
    /* Set gyro in Normal mode and enable the 3 axis, ODR = 95Hz, BW = 25Hz */
    temp = L3GD20_ODR_95HZ | L3GD20_BANDWIDTH_2 | L3GD20_MODE_ACTIVE | L3GD20_ALL_AXES_ENABLE;
    WriteGyroReg( L3GD20_CTRL_REG1, temp );

    /* Disable HP Filter */
    WriteGyroReg( L3GD20_CTRL_REG2, L3GD20_HPM_NORMAL_MODE_RES );

    /* Interrupt Config - all disabled for now */
    WriteGyroReg( L3GD20_CTRL_REG3, 0x00 );

    /* Set Full scale resolution, SPI mode, little endian, Block data update */
    WriteGyroReg( L3GD20_CTRL_REG4, L3GD20_BlockDataUpdate_Single | L3GD20_FULLSCALE_2000 );

    /* Ensure that Ctrl reg 5 has default value that disables HP filtering */
    WriteGyroReg( L3GD20_CTRL_REG5, 0 );

    Gyro_DumpRegisters();
}


/****************************************************************************************************
 * @fn      Gyro_HardwareSetup
 *          Configures the GPIOs and h/w interface to talk to the gyroscope.
 *
 * @param   enable - Enables or disables communication interface
 *
 * @return  none
 *
 ***************************************************************************************************/
void Gyro_HardwareSetup( Bool enable )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;

    if (enable == true)
    {
        /* Initialize the I2C Driver interface */
        ASF_assert( true == I2C_HardwareSetup( GYRO_A_BUS ) ); //TBD - can be made bus agnostic!

        /* Enable GPIO clocks */
        RCC_APB2PeriphClockCmd( RCC_Periph_GYRO_A_RDY_GPIO, ENABLE );

        /* Configure INT/DRDY interrupt Pin */
        GPIO_InitStructure.GPIO_Pin = GYRO_A_RDY_INT_PIN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        GPIO_Init (GYRO_A_RDY_INT_GRP, &GPIO_InitStructure);

        GPIO_EXTILineConfig(GPIO_PORT_SRC_GYRO_A_RDY_INT, GPIO_PIN_SRC_GYRO_A_RDY_INT);

        EXTI_ClearFlag(GYRO_A_RDY_INT_EXTI_LINE);

        EXTI_InitStructure.EXTI_Line = GYRO_A_RDY_INT_EXTI_LINE;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* NVIC config for INT input */
        NVIC_ClearPendingIRQ(GYRO_A_RDY_IRQCHANNEL);
        NVIC_InitStructure.NVIC_IRQChannel = GYRO_A_RDY_IRQCHANNEL;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = GYRO_A_DRDY_INT_PREEMPT_PRIORITY;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = GYRO_A_DRDY_INT_SUB_PRIORITY;
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
 *          Reads data from gyroscope's X, Y, Z data registers
 *
 ***************************************************************************************************/
void Gyro_ReadData( GyroData_t *pxyzData )
{
    int16_t xyzData[3];

    ReadGyroMultiByte( L3GD20_OUT_X_L, (uint8_t *)xyzData, 6 );

    /* Assign axis values based on device orientation. */
    pxyzData->X = xyzData[0];
    pxyzData->Y = xyzData[1];
    pxyzData->Z = xyzData[2];
    //add timestamp here!
    pxyzData->timeStamp = RTC_GetCounter();
}


/****************************************************************************************************
 * @fn      Gyro_TriggerDataAcq
 *          Triggers data acquisition in the gyro sensor if such a feature exist
 *
 ***************************************************************************************************/
void Gyro_TriggerDataAcq( void )
{
    /* Not available */
}


/****************************************************************************************************
 * @fn      Gyro_WaitDataReady
 *          Waits until data is ready to be read
 *
 ***************************************************************************************************/
void Gyro_WaitDataReady( U16 timeOut )
{
    /* Not used in this manner */
}


/****************************************************************************************************
 * @fn      Gyro_ClearDataInt
 *          Resets the New Data interrupt on the gyro.
 *
 ***************************************************************************************************/
void Gyro_ClearDataInt( void )
{
    /* DRDY interrupt is only cleared by reading data registers */
    volatile int16_t xyzData[3];

    ReadGyroMultiByte( L3GD20_OUT_X_L, (uint8_t *)xyzData, 6 );
    (void)xyzData; //avoid compiler warning
}


/****************************************************************************************************
 * @fn      Gyro_ConfigDataInt
 *          Enables/Disables the New Data interrupt on the gyro.
 *
 ***************************************************************************************************/
void Gyro_ConfigDataInt( Bool enInt )
{
    if (enInt)
    {
        /* Enable data ready interrupt */
        WriteGyroReg( L3GD20_CTRL_REG3, DRDY_ON_INT2_ENABLE );
        NVIC_CH_ENABLE( GYRO_A_RDY_IRQCHANNEL );
    }
    else
    {
        uint8_t temp = ReadGyroReg( L3GD20_CTRL_REG3 );
        temp &= ~DRDY_ON_INT2_ENABLE;
        WriteGyroReg( L3GD20_CTRL_REG3, temp );
        NVIC_CH_DISABLE( GYRO_A_RDY_IRQCHANNEL );
    }
}


/****************************************************************************************************
 * @fn      Gyro_ResetDevice
 *          Soft resets the gyroscope.
 *
 ***************************************************************************************************/
void Gyro_ResetDevice( void )
{
    /* No direct reset available in L3G4200D - we can try powering down and switch to normal mode */
    WriteGyroReg( L3GD20_CTRL_REG1, L3GD20_MODE_POWERDOWN );
    Gyro_Initialize();
}


/****************************************************************************************************
 * @fn      Gyro_SelfTest
 *          Runs self test on the gyroscope. Returns true if passed.
 *
 ***************************************************************************************************/
Bool Gyro_SelfTest( void )
{
    /* Not available */
    return true;
}


/****************************************************************************************************
 * @fn      Gyro_SetLowPowerMode
 *          Configures gyro to be in power saving mode
 *
 ***************************************************************************************************/
void Gyro_SetLowPowerMode( void )
{
    WriteGyroReg( L3GD20_CTRL_REG1, L3GD20_MODE_POWERDOWN );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
