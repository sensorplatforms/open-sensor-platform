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
#if !defined (GYRO_L3GD20_I2C_H)
#define   GYRO_L3GD20_I2C_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define BIT(x) ( 1<<(x) )

/* Gyroscope I2C Slave Address */
#define L3GD20_I2C_ADDRESS              0xD7
#define L3GD20_I2C_ADDRESS_7BIT         (L3GD20_I2C_ADDRESS >> 1)
#define L3GD20_I2C_RD_ADDRESS           L3GD20_I2C_ADDRESS
#define L3GD20_I2C_WR_ADDRESS           0xD6

/* Gyroscope Register Mapping */
#define L3GD20_WHO_AM_I                 0x0F  /* device identification register */
#define L3GD20_CTRL_REG1                0x20  /* Control register 1 */
#define L3GD20_CTRL_REG2                0x21  /* Control register 2 */
#define L3GD20_CTRL_REG3                0x22  /* Control register 3 */
#define L3GD20_CTRL_REG4                0x23  /* Control register 4 */
#define L3GD20_CTRL_REG5                0x24  /* Control register 5 */
#define L3GD20_REFERENCE_REG            0x25  /* Reference register */
#define L3GD20_OUT_TEMP                 0x26  /* Out temp register (8-bit res) */
#define L3GD20_STATUS_REG               0x27  /* Status register */
#define L3GD20_OUT_X_L                  0x28  /* Output Register X */
#define L3GD20_OUT_X_H                  0x29  /* Output Register X */
#define L3GD20_OUT_Y_L                  0x2A  /* Output Register Y */
#define L3GD20_OUT_Y_H                  0x2B  /* Output Register Y */
#define L3GD20_OUT_Z_L                  0x2C  /* Output Register Z */
#define L3GD20_OUT_Z_H                  0x2D  /* Output Register Z */ 
#define L3GD20_FIFO_CTRL_REG            0x2E  /* Fifo control Register */
#define L3GD20_FIFO_SRC_REG             0x2F  /* Fifo src Register */

#define L3GD20_INT1_CFG                 0x30  /* Interrupt 1 configuration Register */
#define L3GD20_INT1_SRC                 0x31  /* Interrupt 1 source Register */
#define L3GD20_INT1_TSH_XH              0x32  /* Interrupt 1 Threshold X register */
#define L3GD20_INT1_TSH_XL              0x33  /* Interrupt 1 Threshold X register */
#define L3GD20_INT1_TSH_YH              0x34  /* Interrupt 1 Threshold Y register */
#define L3GD20_INT1_TSH_YL              0x35  /* Interrupt 1 Threshold Y register */
#define L3GD20_INT1_TSH_ZH              0x36  /* Interrupt 1 Threshold Z register */
#define L3GD20_INT1_TSH_ZL              0x37  /* Interrupt 1 Threshold Z register */
#define L3GD20_INT1_DURATION            0x38  /* Interrupt 1 DURATION register */

/* Register bit/config definitions */

/* Who AM I reg */
#define I_AM_L3GD20                     ((uint8_t)0xD4)

/* CTRL Reg 1 */
#define L3GD20_MODE_POWERDOWN           ((uint8_t)0x00)
#define L3GD20_MODE_ACTIVE              BIT(3)
#define L3GD20_ODR_95HZ                 ((uint8_t)0x00)
#define L3GD20_ODR_190HZ                ((uint8_t)0x40)
#define L3GD20_ODR_380HZ                ((uint8_t)0x80)
#define L3GD20_ODR_760HZ                ((uint8_t)0xC0)
#define L3GD20_X_ENABLE                 BIT(1)
#define L3GD20_Y_ENABLE                 BIT(0)
#define L3GD20_Z_ENABLE                 BIT(2)
#define L3GD20_ALL_AXES_ENABLE          ((uint8_t)0x07)
#define L3GD20_ALL_AXES_DISABLE         ((uint8_t)0x00)

/* Bandwidth selection depends on the ODR */
#define L3GD20_BANDWIDTH_1              ((uint8_t)0x00)
#define L3GD20_BANDWIDTH_2              ((uint8_t)0x10)
#define L3GD20_BANDWIDTH_3              ((uint8_t)0x20)
#define L3GD20_BANDWIDTH_4              ((uint8_t)0x30)

/* CTRL Reg 2 ( High Pass Filter Mode Selection ) */
#define L3GD20_HPM_NORMAL_MODE_RES      ((uint8_t)0x00)
#define L3GD20_HPM_REF_SIGNAL           ((uint8_t)0x10)
#define L3GD20_HPM_NORMAL_MODE          ((uint8_t)0x20)
#define L3GD20_HPM_AUTORESET_INT        ((uint8_t)0x30)
/*  High_Pass_CUT OFF_Frequency  */
#define L3GD20_HPFCF_0                  0x00    /* ~ODR/14  */
#define L3GD20_HPFCF_1                  0x01    /* ~ODR/28  */
#define L3GD20_HPFCF_2                  0x02    /* ~ODR/56  */
#define L3GD20_HPFCF_3                  0x03    /* ~ODR/106 */
#define L3GD20_HPFCF_4                  0x04    /* ~ODR/200 */
#define L3GD20_HPFCF_5                  0x05
#define L3GD20_HPFCF_6                  0x06
#define L3GD20_HPFCF_7                  0x07
#define L3GD20_HPFCF_8                  0x08
#define L3GD20_HPFCF_9                  0x09

/* CTRL Reg 3 ( Interrupt options ) */
#define INTR_ON_INT1_ENABLE             BIT(7)
#define BOOT_STATUS_ON_INT1             BIT(6)
#define INT1_ACTIVE_LOW                 BIT(5)
#define OPEN_DRAIN_OUTPUT_EN            BIT(4)    //?Not sure for Int1 or int2 or both?
#define DRDY_ON_INT2_ENABLE             BIT(3)    //Data ready will output on INT2/DRDY PIN
#define FIFO_WTM_INT_ON_INT2_EN         BIT(2)    //FIFO Watermark int on DRDY/INT2 pin
#define FIFO_OVR_INT_ON_INT2_EN         BIT(1)    //FIFO Overrun int on DRDY/INT2 pin
#define FIFO_EMPTY_INT_ON_INT2_EN       BIT(0)    //FIFO Empty int on DRDY/INT2 pin

/* CTRL Reg 4 ( Full Scale, BDU, Endian-ness ) */
#define L3GD20_FULLSCALE_250                ((uint8_t)0x00)
#define L3GD20_FULLSCALE_500                ((uint8_t)0x10)
#define L3GD20_FULLSCALE_2000               ((uint8_t)0x20) 
#define L3GD20_BlockDataUpdate_Continous    ((uint8_t)0x00)
#define L3GD20_BlockDataUpdate_Single       BIT(7)
#define L3GD20_BLE_Big_En                   BIT(6)

/* CTRL Reg 5 ( HP En, Reboot Mem, Int selection, Out source ) */
#define L3GD20_HIGHPASSFILTER_ENABLE        BIT(4)
#define L3GD20_FIFO_ENABLE                  BIT(6)
#define L3GD20_BOOT_REBOOTMEMORY            BIT(7)
/* Note - at 0x00, INT1 & OUT source is non-HPF data */

/* STATUS REG bit definition */
#define L3GD20_XYZ_OVR                      BIT(7)
#define L3GD20_Z_OVR                        BIT(6)
#define L3GD20_Y_OVR                        BIT(5)
#define L3GD20_X_OVR                        BIT(4)
#define L3GD20_XYZ_NewData_Avail            BIT(3)
#define L3GD20_Z_NewData_Avail              BIT(2)
#define L3GD20_Y_NewData_Avail              BIT(1)
#define L3GD20_X_NewData_Avail              BIT(0)

/* FIFO CTRL REG bit definition */
#define L3GD20_FIFO_Bypass                  0x00
#define L3GD20_FIFO_Mode                    0x20
#define L3GD20_Stream_Mode                  0x40
#define L3GD20_Stream_to_Fifo_Mode          0x60
#define L3GD20_Bypass_to_Stream_Mode        0x80

/* FIFO SRC REG bit definitions */

/* INT1 & INT2 Config bit definition */
#define L3GD20_Int_AND_OR_En                BIT(7)
#define L3GD20_Latch_Int_En                 BIT(6)
#define L3GD20_Z_High_Evt_Int_En            BIT(5)
#define L3GD20_Z_Low_Evt_Int_En             BIT(4)
#define L3GD20_Y_High_Evt_Int_En            BIT(3)
#define L3GD20_Y_Low_Evt_Int_En             BIT(2)
#define L3GD20_X_High_Evt_Int_En            BIT(1)
#define L3GD20_X_Low_Evt_Int_En             BIT(0)

/* INT1 & INT2 SRC bit definition  */
#define L3GD20_Int_Active                   BIT(6)
#define L3GD20_Z_High_Evt                   BIT(5)
#define L3GD20_Z_Low_Evt                    BIT(4)
#define L3GD20_Y_High_Evt                   BIT(3)
#define L3GD20_Y_Low_Evt                    BIT(2)
#define L3GD20_X_High_Evt                   BIT(1)
#define L3GD20_X_Low_Evt                    BIT(0)

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


#endif /* GYRO_L3GD20_I2C_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
