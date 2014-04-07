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
#if !defined (ACC_LSM303DLHC_I2C_H)
#define   ACC_LSM303DLHC_I2C_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define BIT(x) ( 1<<(x) )

/* Accelerometer I2C Slave Address */
#define LSM_A_I2C_ADDRESS               0x33
#define LSM_A_I2C_ADDRESS_7BIT          (LSM_A_I2C_ADDRESS >> 1)
#define LSM_A_I2C_RD_ADDRESS            LSM_A_I2C_ADDRESS
#define LSM_A_I2C_WR_ADDRESS            0x32

/* Accelerometer Register Mapping */
#define LSM_A_CTRL_REG1                 0x20
#define LSM_A_CTRL_REG2                 0x21
#define LSM_A_CTRL_REG3                 0x22
#define LSM_A_CTRL_REG4                 0x23
#define LSM_A_CTRL_REG5                 0x24
#define LSM_A_CTRL_REG6                 0x25
#define LSM_A_REFERENCE_REG             0x26
#define LSM_A_STATUS_REG                0x27
#define LSM_A_OUT_X_L                   0x28
#define LSM_A_OUT_X_H                   0x29
#define LSM_A_OUT_Y_L                   0x2A
#define LSM_A_OUT_Y_H                   0x2B
#define LSM_A_OUT_Z_L                   0x2C
#define LSM_A_OUT_Z_H                   0x2D
#define LSM_A_FIFO_CTRL_REG             0x2E
#define LSM_A_FIFO_SRC_REG              0x2F
#define LSM_A_INT1_CFG_REG              0x30
#define LSM_A_INT1_SRC_REG              0x31
#define LSM_A_INT1_THS_REG              0x32
#define LSM_A_INT1_DURATION_REG         0x33
#define LSM_A_INT2_CFG_REG              0x34
#define LSM_A_INT2_SCR_REG              0x35
#define LSM_A_INT2_THS_REG              0x36
#define LSM_A_INT2_DURATION_REG         0x37
#define LSM_A_CLICK_CFG_REG             0x38
#define LSM_A_CLICK_SRC_REG             0x39
#define LSM_A_CLICK_THS_REG             0x3A
#define LSM_A_TIME_LIMIT                0x3B
#define LSM_A_TIME_LATENCY              0x3C
#define LSM_A_TIME_WINDOW               0x3D

/* Accelerometer Configuration Defines  */

// accelerometer sensitivity (mg/LSB) with different full scale selection
#define LSM_Acc_Sensitivity_2g     1    /*!< accelerometer sensitivity with 2 g full scale [mg/LSB] */
#define LSM_Acc_Sensitivity_4g     2    /*!< accelerometer sensitivity with 4 g full scale [mg/LSB] */
#define LSM_Acc_Sensitivity_8g     4    /*!< accelerometer sensitivity with 8 g full scale [mg/LSB] */
#define LSM_Acc_Sensitivity_16g    12   /*!< accelerometer sensitivity with 16 g full scale [mg/LSB] */

/* CTRL REG 1 bit definition */
#define LSM_Acc_XEN                             BIT(0)
#define LSM_Acc_YEN                             BIT(1)
#define LSM_Acc_ZEN                             BIT(2)
#define LSM_Acc_XYZEN                           ((uint8_t)0x07)
#define LSM_Acc_Lowpower_En                     BIT(3)
#define LSM_Acc_Power_Down                      ((uint8_t)0x00)
#define LSM_Acc_ODR_1                           ((uint8_t)0x10)
#define LSM_Acc_ODR_10                          ((uint8_t)0x20)
#define LSM_Acc_ODR_25                          ((uint8_t)0x30)
#define LSM_Acc_ODR_50                          ((uint8_t)0x40)
#define LSM_Acc_ODR_100                         ((uint8_t)0x50)
#define LSM_Acc_ODR_200                         ((uint8_t)0x60)
#define LSM_Acc_ODR_400                         ((uint8_t)0x70)
#define LSM_Acc_ODR_LP_1620                     ((uint8_t)0x80)
#define LSM_Acc_ODR_NP_1344                     ((uint8_t)0x90)
#define LSM_Acc_ODR_LP_5376                     LSM_Acc_ODR_NP_1344

/* CTRL REG 2 bit definition */
#define LSM_Acc_HPCLICK_En                      BIT(2)
#define LSM_Acc_Data_Filter_En                  BIT(3)
#define LSM_Acc_All_Filter_Disable              ((uint8_t)0x00)

/* CTRL REG 3 bit definition */
#define LSM_Acc_FIFO_OVR_onINT1                 BIT(1)
#define LSM_Acc_FIFO_WTM_onINT1                 BIT(2)
#define LSM_Acc_DRDY2_onINT1                    BIT(3)
#define LSM_Acc_DRDY1_onINT1                    BIT(4)
#define LSM_Acc_AOI2_onINT1                     BIT(5)
#define LSM_Acc_AOI1_onINT1                     BIT(6)
#define LSM_Acc_CLICK_onINT1                    BIT(7)

/* CTRL REG 4 bit definition */
#define LSM_Acc_SPI_3Wire_En                    BIT(0)
#define LSM_Acc_HighRes_Out_Mode                BIT(3)      //Needed for NORMAL operating mode
#define LSM_Acc_FS_2g                           ((uint8_t)0x00)
#define LSM_Acc_FS_4g                           ((uint8_t)0x10)
#define LSM_Acc_FS_8g                           ((uint8_t)0x20)
#define LSM_Acc_FS_16g                          ((uint8_t)0x30)
#define LSM_Acc_Big_Endian_En                   BIT(6)      //Default Little Endian
#define LSM_Acc_BDU_Single                      BIT(7)      //Default Continuous update

/* CTRL REG 5 bit definition */
#define LSM_Acc_4D_Detect_onINT2                BIT(0)
#define LSM_Acc_Latch_INT2                      BIT(1)
#define LSM_Acc_4D_Detect_onINT1                BIT(2)
#define LSM_Acc_Latch_INT1                      BIT(3)
#define LSM_Acc_FIFO_En                         BIT(6)
#define LSM_Acc_Reboot_Mem                      BIT(7)

/* CTRL REG 6 bit definition */
#define LSM_Acc_Click_onPAD2                    BIT(7)
#define LSM_Acc_INT1_onPAD2                     BIT(6)
#define LSM_Acc_INT2_onPAD2                     BIT(5)
#define LSM_Acc_BOOT_onPAD2                     BIT(4)
#define LSM_Acc_PAD2_Active                     BIT(3)
#define LSM_Acc_Int_Active_Low                  BIT(1)

/* STATUS REG bit definition */
#define LSM_Acc_XYZ_OVR                         BIT(7)
#define LSM_Acc_Z_OVR                           BIT(6)
#define LSM_Acc_Y_OVR                           BIT(5)
#define LSM_Acc_X_OVR                           BIT(4)
#define LSM_Acc_XYZ_NewData_Avail               BIT(3)
#define LSM_Acc_Z_NewData_Avail                 BIT(2)
#define LSM_Acc_Y_NewData_Avail                 BIT(1)
#define LSM_Acc_X_NewData_Avail                 BIT(0)

/* FIFO CTRL REG bit definition */
#define LSM_Acc_Trigger_Select                  BIT(5)
#define LSM_Acc_FIFO_Bypass                     0x00
#define LSM_Acc_FIFO_Mode                       0x40
#define LSM_Acc_Stream_Mode                     0x80
#define LSM_Acc_Trigger_Mode                    0xC0

/* FIFO SRC REG bit definitions */

/* INT1 & INT2 Config bit definition */
#define LSM_Acc_Int_AND_OR_En                   BIT(7)
#define LSM_Acc_6D_Detect_En                    BIT(6)
#define LSM_Acc_Z_High_Evt_Int_En               BIT(5)
#define LSM_Acc_Z_Low_Evt_Int_En                BIT(4)
#define LSM_Acc_Y_High_Evt_Int_En               BIT(3)
#define LSM_Acc_Y_Low_Evt_Int_En                BIT(2)
#define LSM_Acc_X_High_Evt_Int_En               BIT(1)
#define LSM_Acc_X_Low_Evt_Int_En                BIT(0)

/* INT1 & INT2 SRC bit definition  */
#define LSM_Acc_Int_Active                      BIT(6)
#define LSM_Acc_Z_High_Evt                      BIT(5)
#define LSM_Acc_Z_Low_Evt                       BIT(4)
#define LSM_Acc_Y_High_Evt                      BIT(3)
#define LSM_Acc_Y_Low_Evt                       BIT(2)
#define LSM_Acc_X_High_Evt                      BIT(1)
#define LSM_Acc_X_Low_Evt                       BIT(0)


#define LSM_Acc_FilterMode_Normal               ((uint8_t)0x00)
#define LSM_AccH_FilterMode_Reference           ((uint8_t)0x20)

#define LSM_Acc_Filter_Enable                   ((uint8_t)0x10)
#define LSM_Acc_Filter_Disable                  ((uint8_t)0x00)

#define LSM_Acc_Filter_HPc8                     ((uint8_t)0x00)
#define LSM_Acc_Filter_HPc16                    ((uint8_t)0x01)
#define LSM_Acc_Filter_HPc32                    ((uint8_t)0x02)
#define LSM_Acc_Filter_HPc64                    ((uint8_t)0x03)


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


#endif /* ACC_LSM303DLHC_I2C_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
