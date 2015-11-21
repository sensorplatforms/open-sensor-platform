/*
 ***************************************************************************************************
 *
 * (C) All rights reserved by ROBERT BOSCH GMBH
 *
 **************************************************************************************************/
/*  $Date: 2012/11/06
 *  $Revision: 1.3 $
 *
 */

/**************************************************************************************************
* Copyright (C) 2007 Bosch Sensortec GmbH
*
* bma2x2.h
*
* Usage:        BMA2x2 Sensor Driver Support Header File
*
**************************************************************************************************/
/*************************************************************************************************/
/*  Disclaimer
 *
 * Common:
 * Bosch Sensortec products are developed for the consumer goods industry. They may only be used
 * within the parameters of the respective valid product data sheet.  Bosch Sensortec products are
 * provided with the express understanding that there is no warranty of fitness for a particular purpose.
 * They are not fit for use in life-sustaining, safety or security sensitive systems or any system or device
 * that may lead to bodily harm or property damage if the system or device malfunctions. In addition,
 * Bosch Sensortec products are not fit for use in products which interact with motor vehicle systems.
 * The resale and/or use of products are at the purchaser’s own risk and his own responsibility. The
 * examination of fitness for the intended use is the sole responsibility of the Purchaser.
 *
 * The purchaser shall indemnify Bosch Sensortec from all third party claims, including any claims for
 * incidental, or consequential damages, arising from any product use not covered by the parameters of
 * the respective valid product data sheet or not approved by Bosch Sensortec and reimburse Bosch
 * Sensortec for all costs in connection with such claims.
 *
 * The purchaser must monitor the market for the purchased products, particularly with regard to
 * product safety and inform Bosch Sensortec without delay of all security relevant incidents.
 *
 * Engineering Samples are marked with an asterisk (*) or (e). Samples may vary from the valid
 * technical specifications of the product series. They are therefore not intended or fit for resale to third
 * parties or for use in end products. Their sole purpose is internal client testing. The testing of an
 * engineering sample may in no way replace the testing of a product series. Bosch Sensortec
 * assumes no liability for the use of engineering samples. By accepting the engineering samples, the
 * Purchaser agrees to indemnify Bosch Sensortec from all claims arising from the use of engineering
 * samples.
 *
 * Special:
 * This software module (hereinafter called "Software") and any information on application-sheets
 * (hereinafter called "Information") is provided free of charge for the sole purpose to support your
 * application work. The Software and Information is subject to the following terms and conditions:
 *
 * The Software is specifically designed for the exclusive use for Bosch Sensortec products by
 * personnel who have special experience and training. Do not use this Software if you do not have the
 * proper experience or training.
 *
 * This Software package is provided `` as is `` and without any expressed or implied warranties,
 * including without limitation, the implied warranties of merchantability and fitness for a particular
 * purpose.
 *
 * Bosch Sensortec and their representatives and agents deny any liability for the functional impairment
 * of this Software in terms of fitness, performance and safety. Bosch Sensortec and their
 * representatives and agents shall not be liable for any direct or indirect damages or injury, except as
 * otherwise stipulated in mandatory applicable law.
 *
 * The Information provided is believed to be accurate and reliable. Bosch Sensortec assumes no
 * responsibility for the consequences of use of such Information nor for any infringement of patents or
 * other rights of third parties which may result from its use. No license is granted by implication or
 * otherwise under any patent or patent rights of Bosch. Specifications mentioned in the Information are
 * subject to change without notice.
 *
 * It is not allowed to deliver the source code of the Software to any third party without permission of
 * Bosch Sensortec.
 */
/*************************************************************************************************/
/*! \file bma2x2.h
    \brief BMA2x2 Sensor Driver Support Header File */
/* user defined code to be added here ... */
#ifndef __BMA2x2_H__
#define __BMA2x2_H__

// Example....
// #define YOUR_H_DEFINE  /**< <Doxy Comment for YOUR_H_DEFINE> */
/** Define the calling convention of YOUR bus communication routine.
        \note This includes types of parameters. This example shows the configuration for an SPI bus link.

    If your communication function looks like this:

    write_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

    The BMA2x2_WR_FUNC_PTR would equal:

   #define     BMA2x2_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char, unsigned char *, unsigned char)

    Parameters can be mixed as needed refer to the \ref BMA2x2_BUS_WRITE_FUNC  macro.


 */
#define BMA2x2_WR_FUNC_PTR char (*bus_write)(unsigned char, unsigned char, unsigned char *, unsigned char)

/** link makro between API function calls and bus write function
        \note The bus write function can change since this is a system dependant issue.

    If the bus_write parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

    If the parameters are differently ordered or your communication function like I2C need to know the device address,
    you can change this macro accordingly.


    define BMA2x2_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
    bus_write(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the
    \ref BMA2x2_WR_FUNC_PTR definition.



 */
#define BMA2x2_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)	\
	bus_write(dev_addr, reg_addr, reg_data, wr_len)

/** Define the calling convention of YOUR bus communication routine.
        \note This includes types of parameters. This example shows the configuration for an SPI bus link.

    If your communication function looks like this:

    read_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

    The BMA2x2_RD_FUNC_PTR would equal:

   #define     BMA2x2_RD_FUNC_PTR char (* bus_read)(unsigned char, unsigned char, unsigned char *, unsigned char)

        Parameters can be mixed as needed refer to the \ref BMA2x2_BUS_READ_FUNC  macro.


 */

#define BMA2x2_SPI_RD_MASK 0x80		/* for spi read transactions on SPI the MSB has to be set */
#define BMA2x2_RD_FUNC_PTR char (*bus_read)(unsigned char, unsigned char, unsigned char *, unsigned char)
#define BMA2x2_BRD_FUNC_PTR char (*burst_read)(unsigned char, unsigned char, unsigned char *, unsigned int)

/** link makro between API function calls and bus read function
        \note The bus write function can change since this is a system dependant issue.

    If the bus_read parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

    If the parameters are differently ordered or your communication function like I2C need to know the device address,
    you can change this macro accordingly.


        define BMA2x2_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_read(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the
    \ref BMA2x2_WR_FUNC_PTR definition.

        \note: this macro also includes the "MSB='1'" for reading BMA2x2 addresses.

 */
#define BMA2x2_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len) \
	bus_read(dev_addr, reg_addr, reg_data, r_len)
#define BMA2x2_BURST_READ_FUNC(device_addr, register_addr, register_data, rd_len) \
	burst_read(device_addr, register_addr, register_data, rd_len)
/** bma2x2 I2C Address
 */

/* The LPC5102 sensor board has BMC150 and BMI055. 
 * BMC150 is an Accel/Mag combo chip. Accel I2C address is 0x10. Mag I2C address is 0x12
 * BMI055 is an Accel/Gyro combo chip. Accel I2C address is 0x19. Gyro I2C address is 0x69
 */
#define BMA2x2_I2C_ADDR1                0x10    //BMC150 accel
#define BMA2x2_I2C_ADDR2                0x19    //BMI055 accel

#define BMA2x2_I2C_ADDR                 BMA2x2_I2C_ADDR2



#define         C_BMA2x2_Zero_U8X                       (unsigned char) 0
#define         C_BMA2x2_One_U8X                        (unsigned char) 1
#define         C_BMA2x2_Two_U8X                        (unsigned char) 2
#define         C_BMA2x2_Three_U8X                      (unsigned char) 3
#define         C_BMA2x2_Four_U8X                       (unsigned char) 4
#define         C_BMA2x2_Five_U8X                       (unsigned char) 5
#define         C_BMA2x2_Six_U8X                        (unsigned char) 6
#define         C_BMA2x2_Seven_U8X                      (unsigned char) 7
#define         C_BMA2x2_Eight_U8X                      (unsigned char) 8
#define         C_BMA2x2_Nine_U8X                       (unsigned char) 9
#define         C_BMA2x2_Twelve_U8X                     (unsigned char) 12
#define         C_BMA2x2_Fifteen_U8X                    (unsigned char) 15
#define         C_BMA2x2_Sixteen_U8X                    (unsigned char) 16
#define         C_BMA2x2_ThirtyTwo_U8X                  (unsigned char) 32

/*
        SMB380 API error codes
 */

#define E_SMB_NULL_PTR          (char) -127
#define E_COMM_RES              (char) -1
#define E_OUT_OF_RANGE          (char) -2
#define E_EEPROM_BUSY           (char) -3

#define BMA2x2_RETURN_FUNCTION_TYPE        int				/**< This refers BMA2x2 return type as char */

/*
 *
 *      register definitions
 *
 */

#define BMA2x2_EEP_OFFSET                       0x16
#define BMA2x2_IMAGE_BASE                       0x38
#define BMA2x2_IMAGE_LEN                        22

#define BMA2x2_CHIP_ID_REG                      0x00
#define BMA2x2_X_AXIS_LSB_REG                   0x02
#define BMA2x2_X_AXIS_MSB_REG                   0x03
#define BMA2x2_Y_AXIS_LSB_REG                   0x04
#define BMA2x2_Y_AXIS_MSB_REG                   0x05
#define BMA2x2_Z_AXIS_LSB_REG                   0x06
#define BMA2x2_Z_AXIS_MSB_REG                   0x07
#define BMA2x2_TEMPERATURE_REG                  0x08
#define BMA2x2_STATUS1_REG                      0x09
#define BMA2x2_STATUS2_REG                      0x0A
#define BMA2x2_STATUS_TAP_SLOPE_REG             0x0B
#define BMA2x2_STATUS_ORIENT_HIGH_REG           0x0C
#define BMA2x2_STATUS_FIFO_REG                  0x0E		// ADDED
#define BMA2x2_RANGE_SEL_REG                    0x0F
#define BMA2x2_BW_SEL_REG                       0x10
#define BMA2x2_MODE_CTRL_REG                    0x11
#define BMA2x2_LOW_NOISE_CTRL_REG               0x12
#define BMA2x2_DATA_CTRL_REG                    0x13
#define BMA2x2_RESET_REG                        0x14
#define BMA2x2_INT_ENABLE1_REG                  0x16
#define BMA2x2_INT_ENABLE2_REG                  0x17
#define BMA2x2_INT_SLO_NO_MOT_REG               0x18		// ADDED
#define BMA2x2_INT1_PAD_SEL_REG                 0x19
#define BMA2x2_INT_DATA_SEL_REG                 0x1A
#define BMA2x2_INT2_PAD_SEL_REG                 0x1B
#define BMA2x2_INT_SRC_REG                      0x1E
#define BMA2x2_INT_SET_REG                      0x20
#define BMA2x2_INT_CTRL_REG                     0x21
#define BMA2x2_LOW_DURN_REG                     0x22
#define BMA2x2_LOW_THRES_REG                    0x23
#define BMA2x2_LOW_HIGH_HYST_REG                0x24
#define BMA2x2_HIGH_DURN_REG                    0x25
#define BMA2x2_HIGH_THRES_REG                   0x26
#define BMA2x2_SLOPE_DURN_REG                   0x27
#define BMA2x2_SLOPE_THRES_REG                  0x28
#define BMA2x2_SLO_NO_MOT_THRES_REG             0x29	// ADDED
#define BMA2x2_TAP_PARAM_REG                    0x2A
#define BMA2x2_TAP_THRES_REG                    0x2B
#define BMA2x2_ORIENT_PARAM_REG                 0x2C
#define BMA2x2_THETA_BLOCK_REG                  0x2D
#define BMA2x2_THETA_FLAT_REG                   0x2E
#define BMA2x2_FLAT_HOLD_TIME_REG               0x2F
#define BMA2x2_FIFO_WML_TRIG                    0x30	// ADDED
#define BMA2x2_SELF_TEST_REG                    0x32
#define BMA2x2_EEPROM_CTRL_REG                  0x33
#define BMA2x2_SERIAL_CTRL_REG                  0x34
#define BMA2x2_OFFSET_CTRL_REG                  0x36
#define BMA2x2_OFFSET_PARAMS_REG                0x37
#define BMA2x2_OFFSET_X_AXIS_REG                0x38
#define BMA2x2_OFFSET_Y_AXIS_REG                0x39
#define BMA2x2_OFFSET_Z_AXIS_REG                0x3A
#define BMA2x2_GP0_REG                          0x3B	// ADDED
#define BMA2x2_GP1_REG                          0x3C	// ADDED
#define BMA2x2_FIFO_MODE_REG                    0x3E	// ADDED
#define BMA2x2_FIFO_DATA_OUTPUT_REG             0x3F	// ADDED

#define E_BMA2x2_NULL_PTR                       (char) -127
#define BMA2x2_NULL                             0

#define BMA2x2_12_RESOLUTION                    0
#define BMA2x2_10_RESOLUTION                    1
#define BMA2x2_8_RESOLUTION                     2
#define BMA2x2_14_RESOLUTION                    3

/* register write and read delays */

#define BMA2x2_MDELAY_DATA_TYPE                 unsigned int
#define BMA2x2_EE_W_DELAY                       28						/* delay after EEP write is 28 msec */
/** bma2x2 acceleration data
        \brief Structure containing acceleration values for x,y and z-axis in signed short

 */

typedef struct {
	short x,/**< holds x-axis acceleration data sign extended. Range -512 to 511. */
								  y,/**< holds y-axis acceleration data sign extended. Range -512 to 511. */
								  z;/**< holds z-axis acceleration data sign extended. Range -512 to 511. */
} bma2x2acc_t;
typedef struct {
	short x,/**< holds x-axis acceleration data sign extended. Range -512 to 511. */
		  y,/**< holds y-axis acceleration data sign extended. Range -512 to 511. */
		  z;/**< holds z-axis acceleration data sign extended. Range -512 to 511. */
	signed char temperature;	/**< holds Temperature data sign extended. Range -128 to 127. */
} bma2x2acc_data;
/** bma2x2 image registers data structure
        \brief Register type that contains all bma2x2 image registers from address 0x38 to 0x4D
        This structure can hold the complete image data of bma2x2

 */
typedef struct {
	unsigned char
		offset_filt_x,				/**<  image address 0x38:  */
		offset_filt_y,				/**<  image address 0x39:  */
		offset_filt_z,				/**<  image address 0x3A:  */
		offset_unfilt_x,			/**<  image address 0x3B:  */
		offset_unfilt_y,			/**<  image address 0x3C:  */
		offset_unfilt_z,			/**<  image address 0x3D:  */
		spare_0,					/**<  image address 0x3E:  */
		spare_1,					/**<  image address 0x3F:  */
		crc,						/**<  image address 0x40:  */
		i2c_addr,					/**<  image address 0x41:  */
		dev_config,					/**<  image address 0x42:  */
		trim_offset_t,				/**<  image address 0x43:  */
		gain_x,						/**<  image address 0x44:  */
		offset_x,					/**<  image address 0x45:  */
		gain_y,						/**<  image address 0x46:  */
		offset_y,					/**<  image address 0x47:  */
		gain_z,						/**<  image address 0x48:  */
		offset_z,					/**<  image address 0x49:  */
		trim1,						/**<  image address 0x4A:  */
		trim2,						/**<  image address 0x4B:  */
		trim3,						/**<  image address 0x4C:  */
		trim4;						/**<  image address 0x4D:  */
} bma2x2regs_t;
/** bma2x2 typedef structure
        \brief This structure holds all relevant information about bma2x2 and links communication to the
 */
typedef struct {
	bma2x2regs_t *image;	/**< pointer to bma2x2regs_t structure not mandatory */
	unsigned char mode;		/**< save current bma2x2 operation mode */
	unsigned char chip_id,	/**< save bma2x2's chip id which has to be 0x02 after calling bma2x2_init() */
				  version;	/**< holds the bma2x2 version number */
	unsigned char dev_addr;	/**< initializes bma2x2's I2C device address 0x38 */
	unsigned char int_mask;	/**< stores the current bma2x2 API generated interrupt mask */
	BMA2x2_WR_FUNC_PTR;				/**< function pointer to the SPI/I2C write function */
	BMA2x2_RD_FUNC_PTR;				/**< function pointer to the SPI/I2C read function */
	BMA2x2_BRD_FUNC_PTR;
	void (*delay_msec)(BMA2x2_MDELAY_DATA_TYPE);	/**< function pointer to a pause in mili seconds function */
} bma2x2_t;
#define BMA2x2_CHIP_ID__POS             0
#define BMA2x2_CHIP_ID__MSK             0xFF
#define BMA2x2_CHIP_ID__LEN             8
#define BMA2x2_CHIP_ID__REG             BMA2x2_CHIP_ID_REG

/* DATA REGISTERS */

#define BMA2x2_NEW_DATA_X__POS          0
#define BMA2x2_NEW_DATA_X__LEN          1
#define BMA2x2_NEW_DATA_X__MSK          0x01
#define BMA2x2_NEW_DATA_X__REG          BMA2x2_X_AXIS_LSB_REG

#define BMA2x2_ACC_X14_LSB__POS           2
#define BMA2x2_ACC_X14_LSB__LEN           6
#define BMA2x2_ACC_X14_LSB__MSK           0xFC
#define BMA2x2_ACC_X14_LSB__REG           BMA2x2_X_AXIS_LSB_REG

#define BMA2x2_ACC_X12_LSB__POS           4
#define BMA2x2_ACC_X12_LSB__LEN           4
#define BMA2x2_ACC_X12_LSB__MSK           0xF0
#define BMA2x2_ACC_X12_LSB__REG           BMA2x2_X_AXIS_LSB_REG

#define BMA2x2_ACC_X10_LSB__POS           6
#define BMA2x2_ACC_X10_LSB__LEN           2
#define BMA2x2_ACC_X10_LSB__MSK           0xC0
#define BMA2x2_ACC_X10_LSB__REG           BMA2x2_X_AXIS_LSB_REG

#define BMA2x2_ACC_X8_LSB__POS           0
#define BMA2x2_ACC_X8_LSB__LEN           0
#define BMA2x2_ACC_X8_LSB__MSK           0x00
#define BMA2x2_ACC_X8_LSB__REG           BMA2x2_X_AXIS_LSB_REG

#define BMA2x2_ACC_X_MSB__POS           0
#define BMA2x2_ACC_X_MSB__LEN           8
#define BMA2x2_ACC_X_MSB__MSK           0xFF
#define BMA2x2_ACC_X_MSB__REG           BMA2x2_X_AXIS_MSB_REG

#define BMA2x2_NEW_DATA_Y__POS          0
#define BMA2x2_NEW_DATA_Y__LEN          1
#define BMA2x2_NEW_DATA_Y__MSK          0x01
#define BMA2x2_NEW_DATA_Y__REG          BMA2x2_Y_AXIS_LSB_REG

#define BMA2x2_ACC_Y14_LSB__POS           2
#define BMA2x2_ACC_Y14_LSB__LEN           6
#define BMA2x2_ACC_Y14_LSB__MSK           0xFC
#define BMA2x2_ACC_Y14_LSB__REG           BMA2x2_Y_AXIS_LSB_REG

#define BMA2x2_ACC_Y12_LSB__POS           4
#define BMA2x2_ACC_Y12_LSB__LEN           4
#define BMA2x2_ACC_Y12_LSB__MSK           0xF0
#define BMA2x2_ACC_Y12_LSB__REG           BMA2x2_Y_AXIS_LSB_REG

#define BMA2x2_ACC_Y10_LSB__POS           6
#define BMA2x2_ACC_Y10_LSB__LEN           2
#define BMA2x2_ACC_Y10_LSB__MSK           0xC0
#define BMA2x2_ACC_Y10_LSB__REG           BMA2x2_Y_AXIS_LSB_REG

#define BMA2x2_ACC_Y8_LSB__POS           0
#define BMA2x2_ACC_Y8_LSB__LEN           0
#define BMA2x2_ACC_Y8_LSB__MSK           0x00
#define BMA2x2_ACC_Y8_LSB__REG           BMA2x2_Y_AXIS_LSB_REG

#define BMA2x2_ACC_Y_MSB__POS           0
#define BMA2x2_ACC_Y_MSB__LEN           8
#define BMA2x2_ACC_Y_MSB__MSK           0xFF
#define BMA2x2_ACC_Y_MSB__REG           BMA2x2_Y_AXIS_MSB_REG

#define BMA2x2_NEW_DATA_Z__POS          0
#define BMA2x2_NEW_DATA_Z__LEN          1
#define BMA2x2_NEW_DATA_Z__MSK          0x01
#define BMA2x2_NEW_DATA_Z__REG          BMA2x2_Z_AXIS_LSB_REG

#define BMA2x2_ACC_Z14_LSB__POS           2
#define BMA2x2_ACC_Z14_LSB__LEN           6
#define BMA2x2_ACC_Z14_LSB__MSK           0xFC
#define BMA2x2_ACC_Z14_LSB__REG           BMA2x2_Z_AXIS_LSB_REG

#define BMA2x2_ACC_Z12_LSB__POS           4
#define BMA2x2_ACC_Z12_LSB__LEN           4
#define BMA2x2_ACC_Z12_LSB__MSK           0xF0
#define BMA2x2_ACC_Z12_LSB__REG           BMA2x2_Z_AXIS_LSB_REG

#define BMA2x2_ACC_Z10_LSB__POS           6
#define BMA2x2_ACC_Z10_LSB__LEN           2
#define BMA2x2_ACC_Z10_LSB__MSK           0xC0
#define BMA2x2_ACC_Z10_LSB__REG           BMA2x2_Z_AXIS_LSB_REG

#define BMA2x2_ACC_Z8_LSB__POS           0
#define BMA2x2_ACC_Z8_LSB__LEN           0
#define BMA2x2_ACC_Z8_LSB__MSK           0x00
#define BMA2x2_ACC_Z8_LSB__REG           BMA2x2_Z_AXIS_LSB_REG

#define BMA2x2_ACC_Z_MSB__POS           0
#define BMA2x2_ACC_Z_MSB__LEN           8
#define BMA2x2_ACC_Z_MSB__MSK           0xFF
#define BMA2x2_ACC_Z_MSB__REG           BMA2x2_Z_AXIS_MSB_REG

/*  INTERRUPT STATUS BITS  */

#define BMA2x2_LOWG_INT_S__POS          0
#define BMA2x2_LOWG_INT_S__LEN          1
#define BMA2x2_LOWG_INT_S__MSK          0x01
#define BMA2x2_LOWG_INT_S__REG          BMA2x2_STATUS1_REG

#define BMA2x2_HIGHG_INT_S__POS          1
#define BMA2x2_HIGHG_INT_S__LEN          1
#define BMA2x2_HIGHG_INT_S__MSK          0x02
#define BMA2x2_HIGHG_INT_S__REG          BMA2x2_STATUS1_REG

#define BMA2x2_SLOPE_INT_S__POS          2
#define BMA2x2_SLOPE_INT_S__LEN          1
#define BMA2x2_SLOPE_INT_S__MSK          0x04
#define BMA2x2_SLOPE_INT_S__REG          BMA2x2_STATUS1_REG

// ADDED
#define BMA2x2_SLO_NO_MOT_INT_S__POS          3
#define BMA2x2_SLO_NO_MOT_INT_S__LEN          1
#define BMA2x2_SLO_NO_MOT_INT_S__MSK          0x08
#define BMA2x2_SLO_NO_MOT_INT_S__REG          BMA2x2_STATUS1_REG

#define BMA2x2_DOUBLE_TAP_INT_S__POS     4
#define BMA2x2_DOUBLE_TAP_INT_S__LEN     1
#define BMA2x2_DOUBLE_TAP_INT_S__MSK     0x10
#define BMA2x2_DOUBLE_TAP_INT_S__REG     BMA2x2_STATUS1_REG

#define BMA2x2_SINGLE_TAP_INT_S__POS     5
#define BMA2x2_SINGLE_TAP_INT_S__LEN     1
#define BMA2x2_SINGLE_TAP_INT_S__MSK     0x20
#define BMA2x2_SINGLE_TAP_INT_S__REG     BMA2x2_STATUS1_REG

#define BMA2x2_ORIENT_INT_S__POS         6
#define BMA2x2_ORIENT_INT_S__LEN         1
#define BMA2x2_ORIENT_INT_S__MSK         0x40
#define BMA2x2_ORIENT_INT_S__REG         BMA2x2_STATUS1_REG

#define BMA2x2_FLAT_INT_S__POS           7
#define BMA2x2_FLAT_INT_S__LEN           1
#define BMA2x2_FLAT_INT_S__MSK           0x80
#define BMA2x2_FLAT_INT_S__REG           BMA2x2_STATUS1_REG

#define BMA2x2_FIFO_FULL_INT_S__POS           5
#define BMA2x2_FIFO_FULL_INT_S__LEN           1
#define BMA2x2_FIFO_FULL_INT_S__MSK           0x20
#define BMA2x2_FIFO_FULL_INT_S__REG           BMA2x2_STATUS2_REG

#define BMA2x2_FIFO_WM_INT_S__POS           6
#define BMA2x2_FIFO_WM_INT_S__LEN           1
#define BMA2x2_FIFO_WM_INT_S__MSK           0x40
#define BMA2x2_FIFO_WM_INT_S__REG           BMA2x2_STATUS2_REG

#define BMA2x2_DATA_INT_S__POS           7
#define BMA2x2_DATA_INT_S__LEN           1
#define BMA2x2_DATA_INT_S__MSK           0x80
#define BMA2x2_DATA_INT_S__REG           BMA2x2_STATUS2_REG
#define BMA2x2_SLOPE_FIRST_X__POS        0
#define BMA2x2_SLOPE_FIRST_X__LEN        1
#define BMA2x2_SLOPE_FIRST_X__MSK        0x01
#define BMA2x2_SLOPE_FIRST_X__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_SLOPE_FIRST_Y__POS        1
#define BMA2x2_SLOPE_FIRST_Y__LEN        1
#define BMA2x2_SLOPE_FIRST_Y__MSK        0x02
#define BMA2x2_SLOPE_FIRST_Y__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_SLOPE_FIRST_Z__POS        2
#define BMA2x2_SLOPE_FIRST_Z__LEN        1
#define BMA2x2_SLOPE_FIRST_Z__MSK        0x04
#define BMA2x2_SLOPE_FIRST_Z__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_SLOPE_SIGN_S__POS         3
#define BMA2x2_SLOPE_SIGN_S__LEN         1
#define BMA2x2_SLOPE_SIGN_S__MSK         0x08
#define BMA2x2_SLOPE_SIGN_S__REG         BMA2x2_STATUS_TAP_SLOPE_REG
#define BMA2x2_TAP_FIRST_X__POS        4
#define BMA2x2_TAP_FIRST_X__LEN        1
#define BMA2x2_TAP_FIRST_X__MSK        0x10
#define BMA2x2_TAP_FIRST_X__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_TAP_FIRST_Y__POS        5
#define BMA2x2_TAP_FIRST_Y__LEN        1
#define BMA2x2_TAP_FIRST_Y__MSK        0x20
#define BMA2x2_TAP_FIRST_Y__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_TAP_FIRST_Z__POS        6
#define BMA2x2_TAP_FIRST_Z__LEN        1
#define BMA2x2_TAP_FIRST_Z__MSK        0x40
#define BMA2x2_TAP_FIRST_Z__REG        BMA2x2_STATUS_TAP_SLOPE_REG

#define BMA2x2_TAP_SIGN_S__POS         7
#define BMA2x2_TAP_SIGN_S__LEN         1
#define BMA2x2_TAP_SIGN_S__MSK         0x80
#define BMA2x2_TAP_SIGN_S__REG         BMA2x2_STATUS_TAP_SLOPE_REG
#define BMA2x2_HIGHG_FIRST_X__POS        0
#define BMA2x2_HIGHG_FIRST_X__LEN        1
#define BMA2x2_HIGHG_FIRST_X__MSK        0x01
#define BMA2x2_HIGHG_FIRST_X__REG        BMA2x2_STATUS_ORIENT_HIGH_REG

#define BMA2x2_HIGHG_FIRST_Y__POS        1
#define BMA2x2_HIGHG_FIRST_Y__LEN        1
#define BMA2x2_HIGHG_FIRST_Y__MSK        0x02
#define BMA2x2_HIGHG_FIRST_Y__REG        BMA2x2_STATUS_ORIENT_HIGH_REG

#define BMA2x2_HIGHG_FIRST_Z__POS        2
#define BMA2x2_HIGHG_FIRST_Z__LEN        1
#define BMA2x2_HIGHG_FIRST_Z__MSK        0x04
#define BMA2x2_HIGHG_FIRST_Z__REG        BMA2x2_STATUS_ORIENT_HIGH_REG

#define BMA2x2_HIGHG_SIGN_S__POS         3
#define BMA2x2_HIGHG_SIGN_S__LEN         1
#define BMA2x2_HIGHG_SIGN_S__MSK         0x08
#define BMA2x2_HIGHG_SIGN_S__REG         BMA2x2_STATUS_ORIENT_HIGH_REG
#define BMA2x2_ORIENT_S__POS             4
#define BMA2x2_ORIENT_S__LEN             3
#define BMA2x2_ORIENT_S__MSK             0x70
#define BMA2x2_ORIENT_S__REG             BMA2x2_STATUS_ORIENT_HIGH_REG

#define BMA2x2_FLAT_S__POS               7
#define BMA2x2_FLAT_S__LEN               1
#define BMA2x2_FLAT_S__MSK               0x80
#define BMA2x2_FLAT_S__REG               BMA2x2_STATUS_ORIENT_HIGH_REG
// FIFO_STATUS

#define BMA2x2_FIFO_FRAME_COUNTER_S__POS             0
#define BMA2x2_FIFO_FRAME_COUNTER_S__LEN             7
#define BMA2x2_FIFO_FRAME_COUNTER_S__MSK             0x7F
#define BMA2x2_FIFO_FRAME_COUNTER_S__REG             BMA2x2_STATUS_FIFO_REG

#define BMA2x2_FIFO_OVERRUN_S__POS             7
#define BMA2x2_FIFO_OVERRUN_S__LEN             1
#define BMA2x2_FIFO_OVERRUN_S__MSK             0x80
#define BMA2x2_FIFO_OVERRUN_S__REG             BMA2x2_STATUS_FIFO_REG
#define BMA2x2_RANGE_SEL__POS             0
#define BMA2x2_RANGE_SEL__LEN             4
#define BMA2x2_RANGE_SEL__MSK             0x0F
#define BMA2x2_RANGE_SEL__REG             BMA2x2_RANGE_SEL_REG
#define BMA2x2_BANDWIDTH__POS             0
#define BMA2x2_BANDWIDTH__LEN             5
#define BMA2x2_BANDWIDTH__MSK             0x1F
#define BMA2x2_BANDWIDTH__REG             BMA2x2_BW_SEL_REG
#define BMA2x2_SLEEP_DUR__POS             1
#define BMA2x2_SLEEP_DUR__LEN             4
#define BMA2x2_SLEEP_DUR__MSK             0x1E
#define BMA2x2_SLEEP_DUR__REG             BMA2x2_MODE_CTRL_REG

#define BMA2x2_MODE_CTRL__POS             5
#define BMA2x2_MODE_CTRL__LEN             3
#define BMA2x2_MODE_CTRL__MSK             0xE0
#define BMA2x2_MODE_CTRL__REG             BMA2x2_MODE_CTRL_REG
// ADDED

#define BMA2x2_EN_LOW_POWER__POS          6
#define BMA2x2_EN_LOW_POWER__LEN          1
#define BMA2x2_EN_LOW_POWER__MSK          0x40
#define BMA2x2_EN_LOW_POWER__REG          BMA2x2_MODE_CTRL_REG

// ADDED

#define BMA2x2_SLEEP_TIMER__POS          5
#define BMA2x2_SLEEP_TIMER__LEN          1
#define BMA2x2_SLEEP_TIMER__MSK          0x20
#define BMA2x2_SLEEP_TIMER__REG          BMA2x2_LOW_NOISE_CTRL_REG

// ADDED
#define BMA2x2_LOW_POWER_MODE__POS          6
#define BMA2x2_LOW_POWER_MODE__LEN          1
#define BMA2x2_LOW_POWER_MODE__MSK          0x40
#define BMA2x2_LOW_POWER_MODE__REG          BMA2x2_LOW_NOISE_CTRL_REG

/**     DISABLE MSB SHADOWING PROCEDURE          **/

#define BMA2x2_DIS_SHADOW_PROC__POS       6
#define BMA2x2_DIS_SHADOW_PROC__LEN       1
#define BMA2x2_DIS_SHADOW_PROC__MSK       0x40
#define BMA2x2_DIS_SHADOW_PROC__REG       BMA2x2_DATA_CTRL_REG

/**     FILTERED OR UNFILTERED ACCELERATION DATA  **/

#define BMA2x2_EN_DATA_HIGH_BW__POS         7
#define BMA2x2_EN_DATA_HIGH_BW__LEN         1
#define BMA2x2_EN_DATA_HIGH_BW__MSK         0x80
#define BMA2x2_EN_DATA_HIGH_BW__REG         BMA2x2_DATA_CTRL_REG

#define BMA2x2_EN_SOFT_RESET_VALUE        0xB6
/**     INTERRUPT ENABLE REGISTER              **/

#define BMA2x2_EN_SLOPE_X_INT__POS         0
#define BMA2x2_EN_SLOPE_X_INT__LEN         1
#define BMA2x2_EN_SLOPE_X_INT__MSK         0x01
#define BMA2x2_EN_SLOPE_X_INT__REG         BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_SLOPE_Y_INT__POS         1
#define BMA2x2_EN_SLOPE_Y_INT__LEN         1
#define BMA2x2_EN_SLOPE_Y_INT__MSK         0x02
#define BMA2x2_EN_SLOPE_Y_INT__REG         BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_SLOPE_Z_INT__POS         2
#define BMA2x2_EN_SLOPE_Z_INT__LEN         1
#define BMA2x2_EN_SLOPE_Z_INT__MSK         0x04
#define BMA2x2_EN_SLOPE_Z_INT__REG         BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_DOUBLE_TAP_INT__POS      4
#define BMA2x2_EN_DOUBLE_TAP_INT__LEN      1
#define BMA2x2_EN_DOUBLE_TAP_INT__MSK      0x10
#define BMA2x2_EN_DOUBLE_TAP_INT__REG      BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_SINGLE_TAP_INT__POS      5
#define BMA2x2_EN_SINGLE_TAP_INT__LEN      1
#define BMA2x2_EN_SINGLE_TAP_INT__MSK      0x20
#define BMA2x2_EN_SINGLE_TAP_INT__REG      BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_ORIENT_INT__POS          6
#define BMA2x2_EN_ORIENT_INT__LEN          1
#define BMA2x2_EN_ORIENT_INT__MSK          0x40
#define BMA2x2_EN_ORIENT_INT__REG          BMA2x2_INT_ENABLE1_REG

#define BMA2x2_EN_FLAT_INT__POS            7
#define BMA2x2_EN_FLAT_INT__LEN            1
#define BMA2x2_EN_FLAT_INT__MSK            0x80
#define BMA2x2_EN_FLAT_INT__REG            BMA2x2_INT_ENABLE1_REG
/**     INTERRUPT ENABLE REGISTER              **/

#define BMA2x2_EN_HIGHG_X_INT__POS         0
#define BMA2x2_EN_HIGHG_X_INT__LEN         1
#define BMA2x2_EN_HIGHG_X_INT__MSK         0x01
#define BMA2x2_EN_HIGHG_X_INT__REG         BMA2x2_INT_ENABLE2_REG

#define BMA2x2_EN_HIGHG_Y_INT__POS         1
#define BMA2x2_EN_HIGHG_Y_INT__LEN         1
#define BMA2x2_EN_HIGHG_Y_INT__MSK         0x02
#define BMA2x2_EN_HIGHG_Y_INT__REG         BMA2x2_INT_ENABLE2_REG

#define BMA2x2_EN_HIGHG_Z_INT__POS         2
#define BMA2x2_EN_HIGHG_Z_INT__LEN         1
#define BMA2x2_EN_HIGHG_Z_INT__MSK         0x04
#define BMA2x2_EN_HIGHG_Z_INT__REG         BMA2x2_INT_ENABLE2_REG

#define BMA2x2_EN_LOWG_INT__POS            3
#define BMA2x2_EN_LOWG_INT__LEN            1
#define BMA2x2_EN_LOWG_INT__MSK            0x08
#define BMA2x2_EN_LOWG_INT__REG            BMA2x2_INT_ENABLE2_REG

#define BMA2x2_EN_NEW_DATA_INT__POS        4
#define BMA2x2_EN_NEW_DATA_INT__LEN        1
#define BMA2x2_EN_NEW_DATA_INT__MSK        0x10
#define BMA2x2_EN_NEW_DATA_INT__REG        BMA2x2_INT_ENABLE2_REG

// ADDED
#define BMA2x2_INT_FFULL_EN_INT__POS        5
#define BMA2x2_INT_FFULL_EN_INT__LEN        1
#define BMA2x2_INT_FFULL_EN_INT__MSK        0x20
#define BMA2x2_INT_FFULL_EN_INT__REG        BMA2x2_INT_ENABLE2_REG

#define BMA2x2_INT_FWM_EN_INT__POS        6
#define BMA2x2_INT_FWM_EN_INT__LEN        1
#define BMA2x2_INT_FWM_EN_INT__MSK        0x40
#define BMA2x2_INT_FWM_EN_INT__REG        BMA2x2_INT_ENABLE2_REG
// INT SLO NO MOT

#define BMA2x2_INT_SLO_NO_MOT_EN_X_INT__POS        0
#define BMA2x2_INT_SLO_NO_MOT_EN_X_INT__LEN        1
#define BMA2x2_INT_SLO_NO_MOT_EN_X_INT__MSK        0x01
#define BMA2x2_INT_SLO_NO_MOT_EN_X_INT__REG        BMA2x2_INT_SLO_NO_MOT_REG

#define BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__POS        1
#define BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__LEN        1
#define BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__MSK        0x02
#define BMA2x2_INT_SLO_NO_MOT_EN_Y_INT__REG        BMA2x2_INT_SLO_NO_MOT_REG

#define BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__POS        2
#define BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__LEN        1
#define BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__MSK        0x04
#define BMA2x2_INT_SLO_NO_MOT_EN_Z_INT__REG        BMA2x2_INT_SLO_NO_MOT_REG

#define BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__POS        3
#define BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__LEN        1
#define BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__MSK        0x08
#define BMA2x2_INT_SLO_NO_MOT_EN_SEL_INT__REG        BMA2x2_INT_SLO_NO_MOT_REG
#define BMA2x2_EN_INT1_PAD_LOWG__POS        0
#define BMA2x2_EN_INT1_PAD_LOWG__LEN        1
#define BMA2x2_EN_INT1_PAD_LOWG__MSK        0x01
#define BMA2x2_EN_INT1_PAD_LOWG__REG        BMA2x2_INT1_PAD_SEL_REG
#define BMA2x2_EN_INT1_PAD_HIGHG__POS       1
#define BMA2x2_EN_INT1_PAD_HIGHG__LEN       1
#define BMA2x2_EN_INT1_PAD_HIGHG__MSK       0x02
#define BMA2x2_EN_INT1_PAD_HIGHG__REG       BMA2x2_INT1_PAD_SEL_REG

#define BMA2x2_EN_INT1_PAD_SLOPE__POS       2
#define BMA2x2_EN_INT1_PAD_SLOPE__LEN       1
#define BMA2x2_EN_INT1_PAD_SLOPE__MSK       0x04
#define BMA2x2_EN_INT1_PAD_SLOPE__REG       BMA2x2_INT1_PAD_SEL_REG

// ADDED
#define BMA2x2_EN_INT1_PAD_SLO_NO_MOT__POS        3
#define BMA2x2_EN_INT1_PAD_SLO_NO_MOT__LEN        1
#define BMA2x2_EN_INT1_PAD_SLO_NO_MOT__MSK        0x08
#define BMA2x2_EN_INT1_PAD_SLO_NO_MOT__REG        BMA2x2_INT1_PAD_SEL_REG

#define BMA2x2_EN_INT1_PAD_DB_TAP__POS      4
#define BMA2x2_EN_INT1_PAD_DB_TAP__LEN      1
#define BMA2x2_EN_INT1_PAD_DB_TAP__MSK      0x10
#define BMA2x2_EN_INT1_PAD_DB_TAP__REG      BMA2x2_INT1_PAD_SEL_REG

#define BMA2x2_EN_INT1_PAD_SNG_TAP__POS     5
#define BMA2x2_EN_INT1_PAD_SNG_TAP__LEN     1
#define BMA2x2_EN_INT1_PAD_SNG_TAP__MSK     0x20
#define BMA2x2_EN_INT1_PAD_SNG_TAP__REG     BMA2x2_INT1_PAD_SEL_REG

#define BMA2x2_EN_INT1_PAD_ORIENT__POS      6
#define BMA2x2_EN_INT1_PAD_ORIENT__LEN      1
#define BMA2x2_EN_INT1_PAD_ORIENT__MSK      0x40
#define BMA2x2_EN_INT1_PAD_ORIENT__REG      BMA2x2_INT1_PAD_SEL_REG

#define BMA2x2_EN_INT1_PAD_FLAT__POS        7
#define BMA2x2_EN_INT1_PAD_FLAT__LEN        1
#define BMA2x2_EN_INT1_PAD_FLAT__MSK        0x80
#define BMA2x2_EN_INT1_PAD_FLAT__REG        BMA2x2_INT1_PAD_SEL_REG
#define BMA2x2_EN_INT2_PAD_LOWG__POS        0
#define BMA2x2_EN_INT2_PAD_LOWG__LEN        1
#define BMA2x2_EN_INT2_PAD_LOWG__MSK        0x01
#define BMA2x2_EN_INT2_PAD_LOWG__REG        BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_HIGHG__POS       1
#define BMA2x2_EN_INT2_PAD_HIGHG__LEN       1
#define BMA2x2_EN_INT2_PAD_HIGHG__MSK       0x02
#define BMA2x2_EN_INT2_PAD_HIGHG__REG       BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_SLOPE__POS       2
#define BMA2x2_EN_INT2_PAD_SLOPE__LEN       1
#define BMA2x2_EN_INT2_PAD_SLOPE__MSK       0x04
#define BMA2x2_EN_INT2_PAD_SLOPE__REG       BMA2x2_INT2_PAD_SEL_REG

// ADDED

#define BMA2x2_EN_INT2_PAD_SLO_NO_MOT__POS        3
#define BMA2x2_EN_INT2_PAD_SLO_NO_MOT__LEN        1
#define BMA2x2_EN_INT2_PAD_SLO_NO_MOT__MSK        0x08
#define BMA2x2_EN_INT2_PAD_SLO_NO_MOT__REG        BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_DB_TAP__POS      4
#define BMA2x2_EN_INT2_PAD_DB_TAP__LEN      1
#define BMA2x2_EN_INT2_PAD_DB_TAP__MSK      0x10
#define BMA2x2_EN_INT2_PAD_DB_TAP__REG      BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_SNG_TAP__POS     5
#define BMA2x2_EN_INT2_PAD_SNG_TAP__LEN     1
#define BMA2x2_EN_INT2_PAD_SNG_TAP__MSK     0x20
#define BMA2x2_EN_INT2_PAD_SNG_TAP__REG     BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_ORIENT__POS      6
#define BMA2x2_EN_INT2_PAD_ORIENT__LEN      1
#define BMA2x2_EN_INT2_PAD_ORIENT__MSK      0x40
#define BMA2x2_EN_INT2_PAD_ORIENT__REG      BMA2x2_INT2_PAD_SEL_REG

#define BMA2x2_EN_INT2_PAD_FLAT__POS        7
#define BMA2x2_EN_INT2_PAD_FLAT__LEN        1
#define BMA2x2_EN_INT2_PAD_FLAT__MSK        0x80
#define BMA2x2_EN_INT2_PAD_FLAT__REG        BMA2x2_INT2_PAD_SEL_REG
#define BMA2x2_EN_INT1_PAD_NEWDATA__POS     0
#define BMA2x2_EN_INT1_PAD_NEWDATA__LEN     1
#define BMA2x2_EN_INT1_PAD_NEWDATA__MSK     0x01
#define BMA2x2_EN_INT1_PAD_NEWDATA__REG     BMA2x2_INT_DATA_SEL_REG

// ADDED
#define BMA2x2_EN_INT1_PAD_FWM__POS     1
#define BMA2x2_EN_INT1_PAD_FWM__LEN     1
#define BMA2x2_EN_INT1_PAD_FWM__MSK     0x02
#define BMA2x2_EN_INT1_PAD_FWM__REG     BMA2x2_INT_DATA_SEL_REG

#define BMA2x2_EN_INT1_PAD_FFULL__POS     2
#define BMA2x2_EN_INT1_PAD_FFULL__LEN     1
#define BMA2x2_EN_INT1_PAD_FFULL__MSK     0x04
#define BMA2x2_EN_INT1_PAD_FFULL__REG     BMA2x2_INT_DATA_SEL_REG

#define BMA2x2_EN_INT2_PAD_FFULL__POS     5
#define BMA2x2_EN_INT2_PAD_FFULL__LEN     1
#define BMA2x2_EN_INT2_PAD_FFULL__MSK     0x20
#define BMA2x2_EN_INT2_PAD_FFULL__REG     BMA2x2_INT_DATA_SEL_REG

#define BMA2x2_EN_INT2_PAD_FWM__POS     6
#define BMA2x2_EN_INT2_PAD_FWM__LEN     1
#define BMA2x2_EN_INT2_PAD_FWM__MSK     0x40
#define BMA2x2_EN_INT2_PAD_FWM__REG     BMA2x2_INT_DATA_SEL_REG

#define BMA2x2_EN_INT2_PAD_NEWDATA__POS     7
#define BMA2x2_EN_INT2_PAD_NEWDATA__LEN     1
#define BMA2x2_EN_INT2_PAD_NEWDATA__MSK     0x80
#define BMA2x2_EN_INT2_PAD_NEWDATA__REG     BMA2x2_INT_DATA_SEL_REG
/*****          INTERRUPT SOURCE SELECTION                      *****/

#define BMA2x2_UNFILT_INT_SRC_LOWG__POS        0
#define BMA2x2_UNFILT_INT_SRC_LOWG__LEN        1
#define BMA2x2_UNFILT_INT_SRC_LOWG__MSK        0x01
#define BMA2x2_UNFILT_INT_SRC_LOWG__REG        BMA2x2_INT_SRC_REG

#define BMA2x2_UNFILT_INT_SRC_HIGHG__POS       1
#define BMA2x2_UNFILT_INT_SRC_HIGHG__LEN       1
#define BMA2x2_UNFILT_INT_SRC_HIGHG__MSK       0x02
#define BMA2x2_UNFILT_INT_SRC_HIGHG__REG       BMA2x2_INT_SRC_REG

#define BMA2x2_UNFILT_INT_SRC_SLOPE__POS       2
#define BMA2x2_UNFILT_INT_SRC_SLOPE__LEN       1
#define BMA2x2_UNFILT_INT_SRC_SLOPE__MSK       0x04
#define BMA2x2_UNFILT_INT_SRC_SLOPE__REG       BMA2x2_INT_SRC_REG

// ADDED
#define BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__POS        3
#define BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__LEN        1
#define BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__MSK        0x08
#define BMA2x2_UNFILT_INT_SRC_SLO_NO_MOT__REG        BMA2x2_INT_SRC_REG

#define BMA2x2_UNFILT_INT_SRC_TAP__POS         4
#define BMA2x2_UNFILT_INT_SRC_TAP__LEN         1
#define BMA2x2_UNFILT_INT_SRC_TAP__MSK         0x10
#define BMA2x2_UNFILT_INT_SRC_TAP__REG         BMA2x2_INT_SRC_REG

#define BMA2x2_UNFILT_INT_SRC_DATA__POS        5
#define BMA2x2_UNFILT_INT_SRC_DATA__LEN        1
#define BMA2x2_UNFILT_INT_SRC_DATA__MSK        0x20
#define BMA2x2_UNFILT_INT_SRC_DATA__REG        BMA2x2_INT_SRC_REG
/*****  INTERRUPT PAD ACTIVE LEVEL AND OUTPUT TYPE       *****/

#define BMA2x2_INT1_PAD_ACTIVE_LEVEL__POS       0
#define BMA2x2_INT1_PAD_ACTIVE_LEVEL__LEN       1
#define BMA2x2_INT1_PAD_ACTIVE_LEVEL__MSK       0x01
#define BMA2x2_INT1_PAD_ACTIVE_LEVEL__REG       BMA2x2_INT_SET_REG

#define BMA2x2_INT2_PAD_ACTIVE_LEVEL__POS       2
#define BMA2x2_INT2_PAD_ACTIVE_LEVEL__LEN       1
#define BMA2x2_INT2_PAD_ACTIVE_LEVEL__MSK       0x04
#define BMA2x2_INT2_PAD_ACTIVE_LEVEL__REG       BMA2x2_INT_SET_REG

/*****  OUTPUT TYPE IF SET TO 1 IS : OPEN DRIVE , IF NOT SET
        IT IS PUSH-PULL                                  *****/

#define BMA2x2_INT1_PAD_OUTPUT_TYPE__POS        1
#define BMA2x2_INT1_PAD_OUTPUT_TYPE__LEN        1
#define BMA2x2_INT1_PAD_OUTPUT_TYPE__MSK        0x02
#define BMA2x2_INT1_PAD_OUTPUT_TYPE__REG        BMA2x2_INT_SET_REG

#define BMA2x2_INT2_PAD_OUTPUT_TYPE__POS        3
#define BMA2x2_INT2_PAD_OUTPUT_TYPE__LEN        1
#define BMA2x2_INT2_PAD_OUTPUT_TYPE__MSK        0x08
#define BMA2x2_INT2_PAD_OUTPUT_TYPE__REG        BMA2x2_INT_SET_REG
/*****               INTERRUPT MODE SELECTION              ******/

#define BMA2x2_LATCH_INT__POS                0
#define BMA2x2_LATCH_INT__LEN                4
#define BMA2x2_LATCH_INT__MSK                0x0F
#define BMA2x2_LATCH_INT__REG                BMA2x2_INT_CTRL_REG

/*****               LATCHED INTERRUPT RESET               ******/

#define BMA2x2_RESET_INT__POS           7
#define BMA2x2_RESET_INT__LEN           1
#define BMA2x2_RESET_INT__MSK           0x80
#define BMA2x2_RESET_INT__REG           BMA2x2_INT_CTRL_REG

/*****               LOW-G HYSTERESIS                       ******/

#define BMA2x2_LOWG_HYST__POS                   0
#define BMA2x2_LOWG_HYST__LEN                   2
#define BMA2x2_LOWG_HYST__MSK                   0x03
#define BMA2x2_LOWG_HYST__REG                   BMA2x2_LOW_HIGH_HYST_REG

/*****               LOW-G INTERRUPT MODE                   ******/
/*****       IF 1 -- SUM MODE , 0 -- SINGLE MODE            ******/
#define BMA2x2_LOWG_INT_MODE__POS               2
#define BMA2x2_LOWG_INT_MODE__LEN               1
#define BMA2x2_LOWG_INT_MODE__MSK               0x04
#define BMA2x2_LOWG_INT_MODE__REG               BMA2x2_LOW_HIGH_HYST_REG

/*****               HIGH-G HYSTERESIS                       ******/

#define BMA2x2_HIGHG_HYST__POS                  6
#define BMA2x2_HIGHG_HYST__LEN                  2
#define BMA2x2_HIGHG_HYST__MSK                  0xC0
#define BMA2x2_HIGHG_HYST__REG                  BMA2x2_LOW_HIGH_HYST_REG
/*****               SLOPE DURATION                        ******/

#define BMA2x2_SLOPE_DUR__POS                    0
#define BMA2x2_SLOPE_DUR__LEN                    2
#define BMA2x2_SLOPE_DUR__MSK                    0x03
#define BMA2x2_SLOPE_DUR__REG                    BMA2x2_SLOPE_DURN_REG

// SLO_NO_MOT_DUR ADDED
#define BMA2x2_SLO_NO_MOT_DUR__POS                    2
#define BMA2x2_SLO_NO_MOT_DUR__LEN                    6
#define BMA2x2_SLO_NO_MOT_DUR__MSK                    0xFC
#define BMA2x2_SLO_NO_MOT_DUR__REG                    BMA2x2_SLOPE_DURN_REG

/*****               TAP DURATION                        ******/

#define BMA2x2_TAP_DUR__POS                    0
#define BMA2x2_TAP_DUR__LEN                    3
#define BMA2x2_TAP_DUR__MSK                    0x07
#define BMA2x2_TAP_DUR__REG                    BMA2x2_TAP_PARAM_REG

/*****               TAP SHOCK DURATION                 ******/

#define BMA2x2_TAP_SHOCK_DURN__POS             6
#define BMA2x2_TAP_SHOCK_DURN__LEN             1
#define BMA2x2_TAP_SHOCK_DURN__MSK             0x40
#define BMA2x2_TAP_SHOCK_DURN__REG             BMA2x2_TAP_PARAM_REG

/*****               ADV TAP INT                        ******/

#define BMA2x2_ADV_TAP_INT__POS                5
#define BMA2x2_ADV_TAP_INT__LEN                1
#define BMA2x2_ADV_TAP_INT__MSK                0x20
#define BMA2x2_ADV_TAP_INT__REG                BMA2x2_TAP_PARAM_REG

/*****               TAP QUIET DURATION                 ******/

#define BMA2x2_TAP_QUIET_DURN__POS             7
#define BMA2x2_TAP_QUIET_DURN__LEN             1
#define BMA2x2_TAP_QUIET_DURN__MSK             0x80
#define BMA2x2_TAP_QUIET_DURN__REG             BMA2x2_TAP_PARAM_REG
/*****               TAP THRESHOLD                       ******/

#define BMA2x2_TAP_THRES__POS                  0
#define BMA2x2_TAP_THRES__LEN                  5
#define BMA2x2_TAP_THRES__MSK                  0x1F
#define BMA2x2_TAP_THRES__REG                  BMA2x2_TAP_THRES_REG

/*****               TAP SAMPLES                         ******/

#define BMA2x2_TAP_SAMPLES__POS                6
#define BMA2x2_TAP_SAMPLES__LEN                2
#define BMA2x2_TAP_SAMPLES__MSK                0xC0
#define BMA2x2_TAP_SAMPLES__REG                BMA2x2_TAP_THRES_REG
/*****       ORIENTATION MODE                        ******/

#define BMA2x2_ORIENT_MODE__POS                  0
#define BMA2x2_ORIENT_MODE__LEN                  2
#define BMA2x2_ORIENT_MODE__MSK                  0x03
#define BMA2x2_ORIENT_MODE__REG                  BMA2x2_ORIENT_PARAM_REG

/*****       ORIENTATION BLOCKING                    ******/

#define BMA2x2_ORIENT_BLOCK__POS                 2
#define BMA2x2_ORIENT_BLOCK__LEN                 2
#define BMA2x2_ORIENT_BLOCK__MSK                 0x0C
#define BMA2x2_ORIENT_BLOCK__REG                 BMA2x2_ORIENT_PARAM_REG

/*****       ORIENTATION HYSTERESIS                  ******/

#define BMA2x2_ORIENT_HYST__POS                  4
#define BMA2x2_ORIENT_HYST__LEN                  3
#define BMA2x2_ORIENT_HYST__MSK                  0x70
#define BMA2x2_ORIENT_HYST__REG                  BMA2x2_ORIENT_PARAM_REG
/*****       ORIENTATION AXIS SELECTION              ******/
/***** IF SET TO 1 -- X AND Z ARE SWAPPED , Y IS INVERTED */

// ADDED
#define BMA2x2_ORIENT_UD_EN__POS                  6
#define BMA2x2_ORIENT_UD_EN__LEN                  1
#define BMA2x2_ORIENT_UD_EN__MSK                  0x40
#define BMA2x2_ORIENT_UD_EN__REG                  BMA2x2_THETA_BLOCK_REG

/*****       THETA BLOCKING                    ******/

#define BMA2x2_THETA_BLOCK__POS                  0
#define BMA2x2_THETA_BLOCK__LEN                  6
#define BMA2x2_THETA_BLOCK__MSK                  0x3F
#define BMA2x2_THETA_BLOCK__REG                  BMA2x2_THETA_BLOCK_REG
/*****       THETA FLAT                        ******/

#define BMA2x2_THETA_FLAT__POS                  0
#define BMA2x2_THETA_FLAT__LEN                  6
#define BMA2x2_THETA_FLAT__MSK                  0x3F
#define BMA2x2_THETA_FLAT__REG                  BMA2x2_THETA_FLAT_REG
/*****      FLAT HOLD TIME                     ******/

#define BMA2x2_FLAT_HOLD_TIME__POS              4
#define BMA2x2_FLAT_HOLD_TIME__LEN              2
#define BMA2x2_FLAT_HOLD_TIME__MSK              0x30
#define BMA2x2_FLAT_HOLD_TIME__REG              BMA2x2_FLAT_HOLD_TIME_REG
/*****      FLAT HYS                           ******/

#define BMA2x2_FLAT_HYS__POS                   0
#define BMA2x2_FLAT_HYS__LEN                   3
#define BMA2x2_FLAT_HYS__MSK                   0x07
#define BMA2x2_FLAT_HYS__REG                   BMA2x2_FLAT_HOLD_TIME_REG
/*****      FIFO WATER MARK LEVEL TRIGGER RETAIN                        ******/
// ADDED
#define BMA2x2_FIFO_WML_TRIG_RETAIN__POS                   0
#define BMA2x2_FIFO_WML_TRIG_RETAIN__LEN                   6
#define BMA2x2_FIFO_WML_TRIG_RETAIN__MSK                   0x3F
#define BMA2x2_FIFO_WML_TRIG_RETAIN__REG                   BMA2x2_FIFO_WML_TRIG
/*****      ACTIVATE SELF TEST                 ******/

#define BMA2x2_EN_SELF_TEST__POS                0
#define BMA2x2_EN_SELF_TEST__LEN                2
#define BMA2x2_EN_SELF_TEST__MSK                0x03
#define BMA2x2_EN_SELF_TEST__REG                BMA2x2_SELF_TEST_REG

/*****     SELF TEST -- NEGATIVE               ******/

#define BMA2x2_NEG_SELF_TEST__POS               2
#define BMA2x2_NEG_SELF_TEST__LEN               1
#define BMA2x2_NEG_SELF_TEST__MSK               0x04
#define BMA2x2_NEG_SELF_TEST__REG               BMA2x2_SELF_TEST_REG

/*****     EEPROM CONTROL                      ******/

/* SETTING THIS BIT  UNLOCK'S WRITING SETTING REGISTERS TO EEPROM */

#define BMA2x2_UNLOCK_EE_PROG_MODE__POS     0
#define BMA2x2_UNLOCK_EE_PROG_MODE__LEN     1
#define BMA2x2_UNLOCK_EE_PROG_MODE__MSK     0x01
#define BMA2x2_UNLOCK_EE_PROG_MODE__REG     BMA2x2_EEPROM_CTRL_REG

/* SETTING THIS BIT STARTS WRITING SETTING REGISTERS TO EEPROM */

#define BMA2x2_START_EE_PROG_TRIG__POS      1
#define BMA2x2_START_EE_PROG_TRIG__LEN      1
#define BMA2x2_START_EE_PROG_TRIG__MSK      0x02
#define BMA2x2_START_EE_PROG_TRIG__REG      BMA2x2_EEPROM_CTRL_REG

/* STATUS OF WRITING TO EEPROM */

#define BMA2x2_EE_PROG_READY__POS          2
#define BMA2x2_EE_PROG_READY__LEN          1
#define BMA2x2_EE_PROG_READY__MSK          0x04
#define BMA2x2_EE_PROG_READY__REG          BMA2x2_EEPROM_CTRL_REG

/* UPDATE IMAGE REGISTERS WRITING TO EEPROM */

#define BMA2x2_UPDATE_IMAGE__POS                3
#define BMA2x2_UPDATE_IMAGE__LEN                1
#define BMA2x2_UPDATE_IMAGE__MSK                0x08
#define BMA2x2_UPDATE_IMAGE__REG                BMA2x2_EEPROM_CTRL_REG

/* ADDED */
#define BMA2x2_EE_REMAIN__POS                4
#define BMA2x2_EE_REMAIN__LEN                4
#define BMA2x2_EE_REMAIN__MSK                0xF0
#define BMA2x2_EE_REMAIN__REG                BMA2x2_EEPROM_CTRL_REG
/* SPI INTERFACE MODE SELECTION */

#define BMA2x2_EN_SPI_MODE_3__POS              0
#define BMA2x2_EN_SPI_MODE_3__LEN              1
#define BMA2x2_EN_SPI_MODE_3__MSK              0x01
#define BMA2x2_EN_SPI_MODE_3__REG              BMA2x2_SERIAL_CTRL_REG

/* I2C WATCHDOG PERIOD SELECTION */

#define BMA2x2_I2C_WATCHDOG_PERIOD__POS        1
#define BMA2x2_I2C_WATCHDOG_PERIOD__LEN        1
#define BMA2x2_I2C_WATCHDOG_PERIOD__MSK        0x02
#define BMA2x2_I2C_WATCHDOG_PERIOD__REG        BMA2x2_SERIAL_CTRL_REG

/* I2C WATCHDOG ENABLE */

#define BMA2x2_EN_I2C_WATCHDOG__POS            2
#define BMA2x2_EN_I2C_WATCHDOG__LEN            1
#define BMA2x2_EN_I2C_WATCHDOG__MSK            0x04
#define BMA2x2_EN_I2C_WATCHDOG__REG            BMA2x2_SERIAL_CTRL_REG
/* SPI INTERFACE MODE SELECTION */
// ADDED

/* SETTING THIS BIT  UNLOCK'S WRITING TRIMMING REGISTERS TO EEPROM */

#define BMA2x2_UNLOCK_EE_WRITE_TRIM__POS        4
#define BMA2x2_UNLOCK_EE_WRITE_TRIM__LEN        4
#define BMA2x2_UNLOCK_EE_WRITE_TRIM__MSK        0xF0
#define BMA2x2_UNLOCK_EE_WRITE_TRIM__REG        BMA2x2_CTRL_UNLOCK_REG
/**    OFFSET  COMPENSATION     **/

/**    SLOW COMPENSATION FOR X,Y,Z AXIS      **/

#define BMA2x2_EN_SLOW_COMP_X__POS              0
#define BMA2x2_EN_SLOW_COMP_X__LEN              1
#define BMA2x2_EN_SLOW_COMP_X__MSK              0x01
#define BMA2x2_EN_SLOW_COMP_X__REG              BMA2x2_OFFSET_CTRL_REG

#define BMA2x2_EN_SLOW_COMP_Y__POS              1
#define BMA2x2_EN_SLOW_COMP_Y__LEN              1
#define BMA2x2_EN_SLOW_COMP_Y__MSK              0x02
#define BMA2x2_EN_SLOW_COMP_Y__REG              BMA2x2_OFFSET_CTRL_REG

#define BMA2x2_EN_SLOW_COMP_Z__POS              2
#define BMA2x2_EN_SLOW_COMP_Z__LEN              1
#define BMA2x2_EN_SLOW_COMP_Z__MSK              0x04
#define BMA2x2_EN_SLOW_COMP_Z__REG              BMA2x2_OFFSET_CTRL_REG

/**    FAST COMPENSATION READY FLAG          **/
// ADDED
#define BMA2x2_FAST_CAL_RDY_S__POS             4
#define BMA2x2_FAST_CAL_RDY_S__LEN             1
#define BMA2x2_FAST_CAL_RDY_S__MSK             0x10
#define BMA2x2_FAST_CAL_RDY_S__REG             BMA2x2_OFFSET_CTRL_REG

/**    FAST COMPENSATION FOR X,Y,Z AXIS      **/

#define BMA2x2_CAL_TRIGGER__POS                5
#define BMA2x2_CAL_TRIGGER__LEN                2
#define BMA2x2_CAL_TRIGGER__MSK                0x60
#define BMA2x2_CAL_TRIGGER__REG                BMA2x2_OFFSET_CTRL_REG

/**    RESET OFFSET REGISTERS                **/

#define BMA2x2_RESET_OFFSET_REGS__POS           7
#define BMA2x2_RESET_OFFSET_REGS__LEN           1
#define BMA2x2_RESET_OFFSET_REGS__MSK           0x80
#define BMA2x2_RESET_OFFSET_REGS__REG           BMA2x2_OFFSET_CTRL_REG
/**     SLOW COMPENSATION  CUTOFF               **/

#define BMA2x2_COMP_CUTOFF__POS                 0
#define BMA2x2_COMP_CUTOFF__LEN                 1
#define BMA2x2_COMP_CUTOFF__MSK                 0x01
#define BMA2x2_COMP_CUTOFF__REG                 BMA2x2_OFFSET_PARAMS_REG

/**     COMPENSATION TARGET                  **/

#define BMA2x2_COMP_TARGET_OFFSET_X__POS        1
#define BMA2x2_COMP_TARGET_OFFSET_X__LEN        2
#define BMA2x2_COMP_TARGET_OFFSET_X__MSK        0x06
#define BMA2x2_COMP_TARGET_OFFSET_X__REG        BMA2x2_OFFSET_PARAMS_REG

#define BMA2x2_COMP_TARGET_OFFSET_Y__POS        3
#define BMA2x2_COMP_TARGET_OFFSET_Y__LEN        2
#define BMA2x2_COMP_TARGET_OFFSET_Y__MSK        0x18
#define BMA2x2_COMP_TARGET_OFFSET_Y__REG        BMA2x2_OFFSET_PARAMS_REG

#define BMA2x2_COMP_TARGET_OFFSET_Z__POS        5
#define BMA2x2_COMP_TARGET_OFFSET_Z__LEN        2
#define BMA2x2_COMP_TARGET_OFFSET_Z__MSK        0x60
#define BMA2x2_COMP_TARGET_OFFSET_Z__REG        BMA2x2_OFFSET_PARAMS_REG
/**     FIFO DATA SELECT              **/
// ADDED

#define BMA2x2_FIFO_DATA_SELECT__POS                 0
#define BMA2x2_FIFO_DATA_SELECT__LEN                 2
#define BMA2x2_FIFO_DATA_SELECT__MSK                 0x03
#define BMA2x2_FIFO_DATA_SELECT__REG                 BMA2x2_FIFO_MODE_REG

// FIFO MODE

#define BMA2x2_FIFO_MODE__POS                 6
#define BMA2x2_FIFO_MODE__LEN                 2
#define BMA2x2_FIFO_MODE__MSK                 0xC0
#define BMA2x2_FIFO_MODE__REG                 BMA2x2_FIFO_MODE_REG
#define BMA2x2_GET_BITSLICE(regvar, bitname) \
	(regvar & bitname ## __MSK) >> bitname ## __POS

#define BMA2x2_SET_BITSLICE(regvar, bitname, val) \
	(regvar & ~bitname ## __MSK) | ((val << bitname ## __POS) & bitname ## __MSK)

/** \endcond */

/* CONSTANTS */

#define BMA2x2_STATUS1                             0				/**< It refers BMA2x2 STATUS_INT1 */
#define BMA2x2_STATUS2                             1				/**< It refers BMA2x2 STATUS_INT2 */
#define BMA2x2_STATUS3                             2				/**< It refers BMA2x2 STATUS_INT_TAP */
#define BMA2x2_STATUS4                             3				/**< It refers BMA2x2 STATUS_INT_ORIENT */
#define BMA2x2_STATUS5                             4				/**< It refers BMA2x2 STATUS_INT_FIFO */

#define E_BMA2x2_OUT_OF_RANGE                      (char) -2

/* range and bandwidth */

#define BMA2x2_RANGE_2G                 3	/**< sets range to +/- 2G mode \see BMA2x2_set_range() */
#define BMA2x2_RANGE_4G                 5	/**< sets range to +/- 4G mode \see BMA2x2_set_range() */
#define BMA2x2_RANGE_8G                 8	/**< sets range to +/- 8G mode \see BMA2x2_set_range() */
#define BMA2x2_RANGE_16G                12	/**< sets range to +/- 16G mode \see BMA2x2_set_range() */

#define BMA2x2_BW_7_81HZ        0x08		/**< sets bandwidth to LowPass 7.81  HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_15_63HZ       0x09		/**< sets bandwidth to LowPass 15.63 HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_31_25HZ       0x0A		/**< sets bandwidth to LowPass 31.25 HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_62_50HZ       0x0B		/**< sets bandwidth to LowPass 62.50 HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_125HZ         0x0C		/**< sets bandwidth to LowPass 125HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_250HZ         0x0D		/**< sets bandwidth to LowPass 250HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_500HZ         0x0E		/**< sets bandwidth to LowPass 500HZ \see BMA2x2_set_bandwidth() */
#define BMA2x2_BW_1000HZ        0x0F		/**< sets bandwidth to LowPass 1000HZ \see BMA2x2_set_bandwidth() */

/*        SLEEP DURATION              */

#define BMA2x2_SLEEP_DUR_0_5MS        0x05		/* sets sleep duration to 0.5 ms  */
#define BMA2x2_SLEEP_DUR_1MS          0x06		/* sets sleep duration to 1 ms */
#define BMA2x2_SLEEP_DUR_2MS          0x07		/* sets sleep duration to 2 ms */
#define BMA2x2_SLEEP_DUR_4MS          0x08		/* sets sleep duration to 4 ms */
#define BMA2x2_SLEEP_DUR_6MS          0x09		/* sets sleep duration to 6 ms*/
#define BMA2x2_SLEEP_DUR_10MS         0x0A		/* sets sleep duration to 10 ms */
#define BMA2x2_SLEEP_DUR_25MS         0x0B		/* sets sleep duration to 25 ms */
#define BMA2x2_SLEEP_DUR_50MS         0x0C		/* sets sleep duration to 50 ms */
#define BMA2x2_SLEEP_DUR_100MS        0x0D		/* sets sleep duration to 100 ms */
#define BMA2x2_SLEEP_DUR_500MS        0x0E			/* sets sleep duration to 500 ms */
#define BMA2x2_SLEEP_DUR_1S           0x0F			/* sets sleep duration to 1 s */

/*        LATCH DURATION              */

#define BMA2x2_LATCH_DUR_NON_LATCH    0x00		/* sets LATCH duration to NON LATCH  */
#define BMA2x2_LATCH_DUR_250MS        0x01		/* sets LATCH duration to 250 ms */
#define BMA2x2_LATCH_DUR_500MS        0x02		/* sets LATCH duration to 500 ms */
#define BMA2x2_LATCH_DUR_1S           0x03		/* sets LATCH duration to 1 s */
#define BMA2x2_LATCH_DUR_2S           0x04		/* sets LATCH duration to 2 s*/
#define BMA2x2_LATCH_DUR_4S           0x05		/* sets LATCH duration to 4 s */
#define BMA2x2_LATCH_DUR_8S           0x06		/* sets LATCH duration to 8 s */
#define BMA2x2_LATCH_DUR_LATCH        0x07		/* sets LATCH duration to LATCH */
#define BMA2x2_LATCH_DUR_NON_LATCH1   0x08		/* sets LATCH duration to NON LATCH1 */
#define BMA2x2_LATCH_DUR_250US        0x09			/* sets LATCH duration to 250 Us */
#define BMA2x2_LATCH_DUR_500US        0x0A			/* sets LATCH duration to 500 Us */
#define BMA2x2_LATCH_DUR_1MS          0x0B			/* sets LATCH duration to 1 Ms */
#define BMA2x2_LATCH_DUR_12_5MS       0x0C			/* sets LATCH duration to 12.5 Ms */
#define BMA2x2_LATCH_DUR_25MS         0x0D			/* sets LATCH duration to 25 Ms */
#define BMA2x2_LATCH_DUR_50MS         0x0E			/* sets LATCH duration to 50 Ms */
#define BMA2x2_LATCH_DUR_LATCH1       0x0F			/* sets LATCH duration to LATCH*/

/* mode settings */

#define BMA2x2_MODE_NORMAL             0
#define BMA2x2_MODE_LOWPOWER1          1
#define BMA2x2_MODE_SUSPEND            2
#define BMA2x2_MODE_DEEP_SUSPEND       3
#define BMA2x2_MODE_LOWPOWER2          4
#define BMA2x2_MODE_STANDBY            5

/* BMA2x2 AXIS      */

#define BMA2x2_X_AXIS           0							/**< It refers BMA2x2 X-axis */
#define BMA2x2_Y_AXIS           1							/**< It refers BMA2x2 Y-axis */
#define BMA2x2_Z_AXIS           2							/**< It refers BMA2x2 Z-axis */

/*  INTERRUPT TYPES    */

#define BMA2x2_Low_G_Interrupt       0
#define BMA2x2_High_G_X_Interrupt    1
#define BMA2x2_High_G_Y_Interrupt    2
#define BMA2x2_High_G_Z_Interrupt    3
#define BMA2x2_DATA_EN               4
#define BMA2x2_Slope_X_Interrupt     5
#define BMA2x2_Slope_Y_Interrupt     6
#define BMA2x2_Slope_Z_Interrupt     7
#define BMA2x2_Single_Tap_Interrupt  8
#define BMA2x2_Double_Tap_Interrupt  9
#define BMA2x2_Orient_Interrupt      10
#define BMA2x2_Flat_Interrupt        11
#define BMA2x2_FFULL_INTERRUPT       12
#define BMA2x2_FWM_INTERRUPT         13

/*  INTERRUPTS PADS  */

#define BMA2x2_INT1_LOWG         0
#define BMA2x2_INT2_LOWG         1
#define BMA2x2_INT1_HIGHG        0
#define BMA2x2_INT2_HIGHG        1
#define BMA2x2_INT1_SLOPE        0
#define BMA2x2_INT2_SLOPE        1
#define BMA2x2_INT1_SLO_NO_MOT   0
#define BMA2x2_INT2_SLO_NO_MOT   1
#define BMA2x2_INT1_DTAP         0
#define BMA2x2_INT2_DTAP         1
#define BMA2x2_INT1_STAP         0
#define BMA2x2_INT2_STAP         1
#define BMA2x2_INT1_ORIENT       0
#define BMA2x2_INT2_ORIENT       1
#define BMA2x2_INT1_FLAT         0
#define BMA2x2_INT2_FLAT         1
#define BMA2x2_INT1_NDATA        0
#define BMA2x2_INT2_NDATA        1
#define BMA2x2_INT1_FWM          0
#define BMA2x2_INT2_FWM          1
#define BMA2x2_INT1_FFULL        0
#define BMA2x2_INT2_FFULL        1

/*       SOURCE REGISTER        */

#define BMA2x2_SRC_LOWG         0
#define BMA2x2_SRC_HIGHG        1
#define BMA2x2_SRC_SLOPE        2
#define BMA2x2_SRC_SLO_NO_MOT   3
#define BMA2x2_SRC_TAP          4
#define BMA2x2_SRC_DATA         5

#define BMA2x2_INT1_OUTPUT      0
#define BMA2x2_INT2_OUTPUT      1
#define BMA2x2_INT1_LEVEL       0
#define BMA2x2_INT2_LEVEL       1

/*    DURATION         */

#define BMA2x2_LOW_DURATION            0
#define BMA2x2_HIGH_DURATION           1
#define BMA2x2_SLOPE_DURATION          2
#define BMA2x2_SLO_NO_MOT_DURATION     3

/*      THRESHOLD        */

#define BMA2x2_LOW_THRESHOLD            0
#define BMA2x2_HIGH_THRESHOLD           1
#define BMA2x2_SLOPE_THRESHOLD          2
#define BMA2x2_SLO_NO_MOT_THRESHOLD     3

#define BMA2x2_LOWG_HYST                0
#define BMA2x2_HIGHG_HYST               1

#define BMA2x2_ORIENT_THETA             0
#define BMA2x2_FLAT_THETA               1

#define BMA2x2_I2C_SELECT               0
#define BMA2x2_I2C_EN                   1

/*    COMPENSATION           */

#define BMA2x2_SLOW_COMP_X              0
#define BMA2x2_SLOW_COMP_Y              1
#define BMA2x2_SLOW_COMP_Z              2

/*       OFFSET TRIGGER          */

#define BMA2x2_CUT_OFF                  0
#define BMA2x2_OFFSET_TRIGGER_X         1
#define BMA2x2_OFFSET_TRIGGER_Y         2
#define BMA2x2_OFFSET_TRIGGER_Z         3

/*       GP REGISTERS           */

#define BMA2x2_GP0                      0
#define BMA2x2_GP1                      1

/*    SLO NO MOT REGISTER          */

#define BMA2x2_SLO_NO_MOT_EN_X          0
#define BMA2x2_SLO_NO_MOT_EN_Y          1
#define BMA2x2_SLO_NO_MOT_EN_Z          2
#define BMA2x2_SLO_NO_MOT_EN_SEL        3

/* wake up */

#define BMA2x2_WAKE_UP_DUR_20MS         0
#define BMA2x2_WAKE_UP_DUR_80MS         1
#define BMA2x2_WAKE_UP_DUR_320MS                2
#define BMA2x2_WAKE_UP_DUR_2560MS               3

/* LG/HG thresholds are in LSB and depend on RANGE setting */
/* no range check on threshold calculation */

#define BMA2x2_SELF_TEST0_ON            1
#define BMA2x2_SELF_TEST1_ON            2

#define BMA2x2_EE_W_OFF                 0
#define BMA2x2_EE_W_ON                  1

/* Resolution Settings */
#define BMA2x2_RESOLUTION_12_BIT        0
#define BMA2x2_RESOLUTION_10_BIT        1
#define BMA2x2_RESOLUTION_8_BIT         2
#define BMA2x2_RESOLUTION_14_BIT        3

#define BMA2x2           0x16
#define BMA280           0x17
#define BMA222E          0x18
#define BMA250E          0x19
/** Macro to convert floating point low-g-thresholds in G to 8-bit register values.<br>
 * Example: BMA2x2_LOW_TH_IN_G( 0.3, 2.0) generates the register value for 0.3G threshold in 2G mode.
 * \brief convert g-values to 8-bit value
 */
#define BMA2x2_LOW_TH_IN_G(gthres, range)                      ((256 * gthres ) / range)

/** Macro to convert floating point high-g-thresholds in G to 8-bit register values.<br>
 * Example: BMA2x2_HIGH_TH_IN_G( 1.4, 2.0) generates the register value for 1.4G threshold in 2G mode.
 * \brief convert g-values to 8-bit value
 */
#define BMA2x2_HIGH_TH_IN_G(gthres, range)                              ((256 * gthres ) / range)

/** Macro to convert floating point low-g-hysteresis in G to 8-bit register values.<br>
 * Example: BMA2x2_LOW_HY_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
 * \brief convert g-values to 8-bit value
 */
#define BMA2x2_LOW_HY_IN_G(ghyst, range)                              ((32 * ghyst) / range)

/** Macro to convert floating point high-g-hysteresis in G to 8-bit register values.<br>
 * Example: BMA2x2_HIGH_HY_IN_G( 0.2, 2.0) generates the register value for 0.2G threshold in 2G mode.
 * \brief convert g-values to 8-bit value
 */
#define BMA2x2_HIGH_HY_IN_G(ghyst, range)                             ((32 * ghyst) / range)

/** Macro to convert floating point G-thresholds to 8-bit register values<br>
 * Example: BMA2x2_SLOPE_TH_IN_G( 1.2, 2.0) generates the register value for 1.2G threshold in 2G mode.
 * \brief convert g-values to 8-bit value
 */

#define BMA2x2_SLOPE_TH_IN_G(gthres, range)    ((128 * gthres ) / range)
/*user defined Enums*/
// Example..
// enum {
// E_YOURDATA1, /**< <DOXY Comment for E_YOURDATA1> */
// E_YOURDATA2  /**< <DOXY Comment for E_YOURDATA2> */
// };
// Example...
// struct DUMMY_STRUCT {
// data1, /**< <DOXY Comment for data1> */
// data2  /**< <DOXY Comment for data1> */
// };
/*******************************************************************************
 * Description: *//**\brief This API reads the data from the given register
 *
 *
 *
 *
 *  \param unsigned char addr, unsigned char *data
 *                       addr -> Address of the register
 *                       data -> address of the variable, read value will be kept
 *  \return  results of bus communication function
 *
 *
 ******************************************************************************/
/* Scheduling:
 *
 *
 *
 * Usage guide:
 *
 *
 * Remarks:
 *
 ******************************************************************************/
BMA2x2_RETURN_FUNCTION_TYPE bma2x2_burst_read(unsigned char addr, unsigned char *data, unsigned int len);

int bma2x2_init(bma2x2_t *bma2x2);

int bma2x2_write_reg(unsigned char addr, unsigned char *data, unsigned char len);

int bma2x2_read_reg(unsigned char addr, unsigned char *data, unsigned char len);

int bma2x2_read_accel_x(short *a_x);

int bma2x2_read_accel_y(short *a_y);

int bma2x2_read_accel_z(short *a_z);

int bma2x2_read_accel_xyz(bma2x2acc_t *acc);

int bma2x2_get_int_tap_status(unsigned char *status_tap);

int bma2x2_get_int_orient_status(unsigned char *status_orient);

int bma2x2_get_fifo_status(unsigned char *status_fifo);

int bma2x2_get_fifo_framecount(unsigned char *framecount);

int bma2x2_get_fifo_overrun(unsigned char *overrun);

int bma2x2_get_interrupt_status(unsigned char *status);

int bma2x2_get_range(unsigned char *Range);

int bma2x2_set_range(unsigned char Range);

int bma2x2_get_bandwidth(unsigned char *bw);

int bma2x2_set_bandwidth(unsigned char bw);

int bma2x2_get_mode(unsigned char *Mode);

unsigned char bma2x2_set_mode(unsigned char Mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_sleep_dur(unsigned char *sleep_dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_sleep_dur (unsigned char sleep_dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_sleeptmr_mode(unsigned char *sleep_tmr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_sleeptmr_mode (unsigned char sleep_tmr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_high_bw (unsigned char *high_bw);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_high_bw (unsigned char high_bw);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_shadow_dis (unsigned char *shadow_dis);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_shadow_dis (unsigned char shadow_dis);

int bma2x2_soft_reset(void);

int bma2x2_update_image(void);

int bma2x2_set_Int_Enable(unsigned char InterruptType, unsigned char value);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_ffull (unsigned char *ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_ffull (unsigned char ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_fwm (unsigned char *fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_fwm (unsigned char fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_slo_no_mot (unsigned char channel, unsigned char *slo_data);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_slo_no_mot (unsigned char channel, unsigned char slo_data);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_low (unsigned char channel, unsigned char *int_low);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_low (unsigned char channel, unsigned char int_low);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_high(unsigned char channel, unsigned char *int_high);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_high(unsigned char channel, unsigned char int_high);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_slope(unsigned char channel, unsigned char *int_slope);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_slope(unsigned char channel, unsigned char int_slope);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_slo_no_mot (unsigned char channel, unsigned char *int_slo_no_mot);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_slo_no_mot (unsigned char channel, unsigned char int_slo_no_mot);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_d_tap (unsigned char channel, unsigned char *int_d_tap);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_d_tap (unsigned char channel, unsigned char int_d_tap);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_s_tap (unsigned char channel, unsigned char *int_s_tap);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_s_tap (unsigned char channel, unsigned char int_s_tap);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_orient (unsigned char channel, unsigned char *int_orient);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_orient (unsigned char channel, unsigned char int_orient);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_flat (unsigned char channel, unsigned char *int_flat);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_flat (unsigned char channel, unsigned char int_flat);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_newdata (unsigned char channel, unsigned char *int_newdata);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_newdata (unsigned char channel, unsigned char int_newdata);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int1_fwm (unsigned char *int1_fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int1_fwm (unsigned char int1_fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int2_fwm (unsigned char *int2_fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int2_fwm (unsigned char int2_fwm);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int1_ffull (unsigned char *int1_ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int1_ffull (unsigned char int1_ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int2_ffull (unsigned char *int2_ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int2_ffull (unsigned char int2_ffull);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_source (unsigned char channel, unsigned char *int_source);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_source (unsigned char channel, unsigned char int_source);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_od (unsigned char channel, unsigned char *int_od);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_od (unsigned char channel, unsigned char int_od);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_int_lvl (unsigned char channel, unsigned char *int_lvl);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_int_lvl (unsigned char channel, unsigned char int_lvl);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_reset_interrupt (unsigned char reset_int);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_latch_int (unsigned char *latch_int);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_latch_int (unsigned char latch_int);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_dur(unsigned char channel, unsigned char *dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_dur (unsigned char channel, unsigned char dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_thr(unsigned char channel, unsigned char *thr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_thr (unsigned char channel, unsigned char thr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_low_high_hyst(unsigned char channel, unsigned char *hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_low_high_hyst (unsigned char channel, unsigned char hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_low_high_hyst_mode(unsigned char *mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_low_high_hyst_mode (unsigned char mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_dur (unsigned char *tap_dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_dur (unsigned char tap_dur);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_shock (unsigned char *tap_shock);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_shock (unsigned char tap_shock);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_quiet (unsigned char *tap_quiet);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_quiet (unsigned char tap_quiet);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_thr (unsigned char *tap_thr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_thr (unsigned char tap_thr);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_tap_sample (unsigned char *tap_sample);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_tap_sample (unsigned char tap_sample);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_mode (unsigned char *orient_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_mode (unsigned char orient_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_block (unsigned char *orient_block);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_block (unsigned char orient_block);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_hyst (unsigned char *orient_hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_hyst (unsigned char orient_hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_theta(unsigned char channel, unsigned char *theta);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_theta (unsigned char channel, unsigned char theta);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_orient_en (unsigned char *orient_en);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_orient_en (unsigned char orient_en);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_flat_hyst (unsigned char *flat_hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_flat_hyst (unsigned char flat_hyst);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_flat_hold_time (unsigned char *flat_hold_time);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_flat_hold_time (unsigned char flat_hold_time);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_wml_trig (unsigned char *fifo_wml_trig);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_wml_trig (unsigned char fifo_wml_trig);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_self_test_axis (unsigned char *self_test_axis);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_self_test_axis (unsigned char self_test_axis);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_self_test_sign (unsigned char *self_test_sign);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_self_test_sign (unsigned char self_test_sign);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_nvmprog_mode (unsigned char *nvmprog_mode);

int bma2x2_set_nvmprog_mode(unsigned char prgmode);

int bma2x2_set_nvprog_trig(unsigned char trig);

int bma2x2_get_nvmprog_ready(unsigned char *ready);

int bma2x2_get_nvmprog_remain(unsigned char *remain);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_spi3 (unsigned char *spi3);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_spi3 (unsigned char spi3);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_i2c_wdt (unsigned char channel, unsigned char *prog_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_i2c_wdt (unsigned char channel, unsigned char prog_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_slow_comp(unsigned char channel, unsigned char *slow_comp);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_slow_comp (unsigned char channel, unsigned char slow_comp);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_cal_rdy (unsigned char *rdy);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_cal_rdy (unsigned char rdy);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_cal_trig (unsigned char *cal_trig);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_cal_trig (unsigned char cal_trig);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset_reset (unsigned char offset_reset);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_offset_target(unsigned char channel, unsigned char *offset);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset_target (unsigned char channel, unsigned char offset);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_offset(unsigned char channel, unsigned char *offset);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_offset (unsigned char channel, unsigned char offset);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_gp(unsigned char channel, unsigned char *gp);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_gp (unsigned char channel, unsigned char gp);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_mode (unsigned char *fifo_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_mode (unsigned char fifo_mode);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_data_sel (unsigned char *data_sel);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_data_sel (unsigned char data_sel);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_get_fifo_data_out_reg(unsigned char *out_reg);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_set_fifo_data_out_reg (unsigned char out_reg);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_read_temperature (signed char *temperature);

BMA2x2_RETURN_FUNCTION_TYPE bma2x2_read_accel_xyzt(bma2x2acc_data *acc);

BMA2x2_RETURN_FUNCTION_TYPE bmm050_set_accel_int1_drdy_interrupt \
	(unsigned char enable_disable,
	unsigned char active_low0_high1);

#endif
