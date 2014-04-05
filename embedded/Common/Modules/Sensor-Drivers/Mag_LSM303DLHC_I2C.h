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
 * @file Mag_LSM303DLHC_I2C.h
 * Declarations for register and settings of the magnetometer part inside LSM303DLHC
 *
 ***************************************************************************************************/
#if !defined (MAG_LSM303DLHC_I2C_H)
#define   MAG_LSM303DLHC_I2C_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define BIT(x)                          ( 1<<(x) )
#define BITS(val, pos)                  ((val)<<(pos))

/* Magnetometer I2C Slave Address */
#define LSM_M_I2C_ADDRESS               0x3D
#define LSM_M_I2C_ADDRESS_7BIT          (LSM_M_I2C_ADDRESS >> 1)
#define LSM_M_I2C_RD_ADDRESS            LSM_M_I2C_ADDRESS
#define LSM_M_I2C_WR_ADDRESS            0x3C


/* Magnetometer Register Mapping */
#define LSM_M_CRA_REG                   0x00
#define LSM_M_CRB_REG                   0x01
#define LSM_M_MR_REG                    0x02
#define LSM_M_OUT_X_H                   0x03
#define LSM_M_OUT_X_L                   0x04
#define LSM_M_OUT_Z_H                   0x05 /* NOTE: Z-axis instead of Y!! */
#define LSM_M_OUT_Z_L                   0x06
#define LSM_M_OUT_Y_H                   0x07
#define LSM_M_OUT_Y_L                   0x08
#define LSM_M_SR_REG                    0x09
#define LSM_M_IRA_REG                   0x0A
#define LSM_M_IRB_REG                   0x0B
#define LSM_M_IRC_REG                   0x0C
#define LSM_M_TEMPERATURE_H             0x31
#define LSM_M_TEMPERATURE_L             0x32



/* Magnetometer Configuration Defines  */

/* Control Reg CRA - ODR settings, Temperature en */
#define LSM_Mag_ODR_0_75                0x00
#define LSM_Mag_ODR_1_5                 0x04
#define LSM_Mag_ODR_3                   0x08
#define LSM_Mag_ODR_7_5                 0x0C
#define LSM_Mag_ODR_15                  0x10
#define LSM_Mag_ODR_30                  0x14
#define LSM_Mag_ODR_75                  0x18
#define LSM_Mag_ODR_220                 0x1C
#define LSM_Mag_Temperature_En          BIT(7)

/* Control Reg CRB - Gain settings */
#define LSM_Mag_Range_1_3G              0x20
#define LSM_Mag_Range_1_9G              0x40
#define LSM_Mag_Range_2_5G              0x60
#define LSM_Mag_Range_4_0G              0x80
#define LSM_Mag_Range_4_7G              0xA0
#define LSM_Mag_Range_5_6G              0xC0
#define LSM_Mag_Range_8_1G              0xE0

/* Sensitivity settings corresponding to range */
/* NOTE: These values are from Table 3 of the data sheet. There is mismatch in values where the register
   definition is given */
#define LSM_Mag_Sensitivity_XY_1_3Ga    1100 /*!< magnetometer X Y axes sensitivity for 1.3 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_1_9Ga    855  /*!< magnetometer X Y axes sensitivity for 1.9 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_2_5Ga    670  /*!< magnetometer X Y axes sensitivity for 2.5 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_4Ga      450  /*!< magnetometer X Y axes sensitivity for 4 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_4_7Ga    400  /*!< magnetometer X Y axes sensitivity for 4.7 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_5_6Ga    330  /*!< magnetometer X Y axes sensitivity for 5.6 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_XY_8_1Ga    230  /*!< magnetometer X Y axes sensitivity for 8.1 Ga full scale [LSB/Ga] */

#define LSM_Mag_Sensitivity_Z_1_3Ga     980  /*!< magnetometer Z axes sensitivity for 1.3 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_1_9Ga     760  /*!< magnetometer Z axes sensitivity for 1.9 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_2_5Ga     600  /*!< magnetometer Z axes sensitivity for 2.5 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_4Ga       400  /*!< magnetometer Z axes sensitivity for 4 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_4_7Ga     355  /*!< magnetometer Z axes sensitivity for 4.7 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_5_6Ga     295  /*!< magnetometer Z axes sensitivity for 5.6 Ga full scale [LSB/Ga] */
#define LSM_Mag_Sensitivity_Z_8_1Ga     205  /*!< magnetometer Z axes sensitivity for 8.1 Ga full scale [LSB/Ga] */

/* Mode Reg MR - Mode select */
#define LSM_Mag_MODE_CONTINUOS          0x00
#define LSM_Mag_MODE_SINGLE             0x01
#define LSM_Mag_MODE_SLEEP              0x02

/* Status Reg SR (read only) */
#define LSM_Mag_Data_Out_Lock           BIT(1)
#define LSM_Mag_DRDY                    BIT(0)

/* Identification Reg - IRA, IRB, IRC values (same as HMC5883L) */
#define LSM_Mag_IRA_Value               0x48    //'H'
#define LSM_Mag_IRB_Value               0x34    //'4'
#define LSM_Mag_IRC_Value               0x33    //'3'

/* Temperature Reg - TEMP_OUT_H, TEMP_OUT_L - 12-bit reading - left justified into 16-bits*/
#define LSM_Mag_Temp_Out_H_Mask         0xFF    //Bits 11:4
#define LSM_Mag_Temp_Out_L_Mask         0xF0    //Bits 3:0

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* MAG_LSM303DLHC_I2C_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
