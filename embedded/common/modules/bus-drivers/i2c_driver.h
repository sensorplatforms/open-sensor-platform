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
#if !defined (I2C_DRIVER_H)
#define   I2C_DRIVER_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define FLAG_LAST_TRANS_OK         0x01
#define FLAG_MAG_DATA_READY        0x02
#define I2C_BUFFER_SIZE            15    // Set this to the largest message size that will be sent
                                         // including address byte.
#define I2C_ERR_OK                                          0
#define I2C_ERR_BUSY                                        1
#define I2C_ERR_REQ                                         2

/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define I2C_READ_BIT               0     // Bit position for R/W bit in "address byte".
#define I2C_ADR_BITS               1     // Bit position for LSB of the slave address bits in the init byte.


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum _SendModeTag {
    I2C_MASTER_WRITE,
    I2C_MASTER_RESTART,
    I2C_MASTER_READ,
    I2C_SLAVE_TX,
    I2C_SLAVE_RX
} I2C_SendMode_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
osp_bool_t I2C_HardwareSetup( I2C_TypeDef *busId );
void I2C_Master_Initialise( void );
uint8_t I2C_Transceiver_Busy( void );
uint8_t I2C_Start_Transfer( uint8_t slaveAddr, uint16_t regAddr, uint8_t *pData, uint8_t dataSize,
                            I2C_SendMode_t sendMode );
uint8_t I2C_Get_Data_From_Transceiver( uint8_t *, uint8_t );
void I2C_Wait_Completion( void );


#endif /* I2C_DRIVER_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
