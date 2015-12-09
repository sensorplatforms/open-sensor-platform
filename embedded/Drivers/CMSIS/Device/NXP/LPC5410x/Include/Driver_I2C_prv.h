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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

#define ARM_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR( 2, 0 ) /* driver version */
#define RX_LENGTH       (16)

#define I2C_ERR_OK              (0)
#define I2C_ERR_BUSY            (1)
#define I2C_ERR_REQ             (2)
#define I2C_SLV_ADDR_NUM        (3)
#define I2C_SLV_ADDR_MSK        (0x7F)
#define I2C_SLV_PCLK_FREQ       (4000000) /* I2C slave peripheral clock set to 4 MHZ */

typedef enum _SendModeTag {
    I2C_MASTER_WRITE,
    I2C_MASTER_RESTART,
    I2C_MASTER_READ,
    I2C_SLAVE_TX,
    I2C_SLAVE_RX
} I2C_SendMode_t;

typedef enum {
    I2C_LPC_INVALID = -1,
    I2C_LPC_0       = 0,
    I2C_LPC_1       = 1,
    I2C_LPC_2       = 2,
    I2C_LPC_MAX
} i2c_num;

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
    uint32_t ui_slave_address;
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

#define OSP_BUILD_DRIVER_SENSOR( pre1, pre2, priv ) \
    static ARM_DRIVER_VERSION pre1##_GetVersion( void ) { \
        return pre2##_GetVersion( priv ); \
    } \
    static ARM_I2C_CAPABILITIES pre1##_GetCapabilities( void ) { \
        return pre2##_GetCapabilities( priv ); \
    } \
    static int32_t pre1##_Initialize( ARM_I2C_SignalEvent_t cb ) { \
        return pre2##_Initialize( cb, priv ); \
    } \
    static int32_t pre1##_Uninitialize( void ) { \
        return pre2##_Uninitialize( priv ); \
    } \
    static int32_t pre1##_PowerControl( ARM_POWER_STATE state ) { \
        return pre2##_PowerControl( state, priv ); \
    } \
    static int32_t pre1##_MasterTransmit( uint32_t addr, const uint8_t * data, uint32_t num, bool xfer_pending ) { \
        return pre2##_MasterTransmit( addr, data, num, xfer_pending, priv ); \
    } \
    static int32_t pre1##_MasterReceive( uint32_t addr, uint8_t * data, uint32_t num, bool xfer_pending ) { \
        return pre2##_MasterReceive( addr, data, num, xfer_pending, priv ); \
    } \
    static int32_t pre1##_SlaveTransmit( const uint8_t * data, uint32_t num ) { \
        return pre2##_SlaveTransmit( data, num, priv ); \
    } \
    static int32_t pre1##_SlaveReceive( uint8_t * data, uint32_t num ) { \
        return pre2##_SlaveReceive( data, num, priv ); \
    } \
    static int32_t pre1##_GetDataCount( void ) { \
        return pre2##_GetDataCount( priv ); \
    } \
    static int32_t pre1##_Control( uint32_t control, uint32_t arg ) { \
        return pre2##_Control( control, arg, priv ); \
    } \
    static ARM_I2C_STATUS pre1##_GetStatus( void ) { \
        return pre2##_GetStatus( priv ); \
    } \
    ARM_DRIVER_I2C Driver_##pre1 = { \
        pre1##_GetVersion,      \
        pre1##_GetCapabilities, \
        pre1##_Initialize,      \
        pre1##_Uninitialize,    \
        pre1##_PowerControl,    \
        pre1##_MasterTransmit,  \
        pre1##_MasterReceive,   \
        pre1##_SlaveTransmit,   \
        pre1##_SlaveReceive,    \
        pre1##_GetDataCount,    \
        pre1##_Control,         \
        pre1##_GetStatus        \
    };
