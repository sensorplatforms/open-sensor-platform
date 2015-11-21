/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2015 Audience Inc.
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
#if !defined (I2CS_DRIVER_H)
#define   I2CS_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define RX_LENGTH       16

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum {
    I2C_LPC_INVALID = -1,
    I2C_LPC_0 = 0,
    I2C_LPC_1 = 1,
    I2C_LPC_2 = 2,
    I2C_LPC_MAX
}i2c_num;

typedef struct {
    const void *txBuff;             /*!< Pointer to array of bytes to be transmitted */
    void *rxBuff;                   /*!< Pointer memory where bytes received from I2C be stored */
    volatile ErrorCode_t status;    /*!< status of the current I2C transfer (ErrorCode_t), must be 32-bits */
    uint32_t flags;                 /*!< Reserved, set to 0 */
    uint16_t txSz;                  /*!< Number of bytes in transmit array, if 0 only receive transfer will be performed */
    uint16_t rxSz;                  /*!< Number of bytes to receive, if 0 only transmission will be performed */
    uint16_t bytesSent;             /*!< Number of bytes sent */
    uint16_t bytesRecv;             /*!< Number of bytes recevied */
} i2cs_xfer_stat_t;

typedef struct
{
    i2c_num i2c;
    uint32_t ui_slave_address;
    uint8_t uc_slave_index;
    void *tx_buff;
    uint8_t rxBuff[RX_LENGTH];
    uint8_t rxCount;        /* Bytes so far received  */
    uint8_t rxLength;       /* Expected Rx buffer length */
    
    /*
    * i2c_operation indicates the I2C action to be performed as requested by host
    * 0x1 --> receive data
    * 0x2 --> tramsit data
    */
    uint8_t i2c_operation;
    /* I2C transfer structure */
    i2cs_xfer_stat_t pXfer;
} i2c_t;

enum {
    I2C_ERROR_NO_SLAVE = -1,
    I2C_ERROR_BUS_BUSY = -2
};

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
void i2c_init( i2c_t *obj );
void i2c_slave_mode( i2c_t *obj, int enable_slave );
int  i2c_slave_receive( i2c_t *obj ); // Wait for next I2C event and find out what is going on
int  i2c_slave_write( i2c_t *obj, const char *data, int length );

#ifdef __cplusplus
}
#endif

#endif /* I2CS_DRIVER_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/

