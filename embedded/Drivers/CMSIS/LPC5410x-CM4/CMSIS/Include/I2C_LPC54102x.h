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

#if !defined (I2C_LPC54102X_H)
#define   I2C_LPC54102X_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

#define ARM_I2C_DRV_VERSION     ARM_DRIVER_VERSION_MAJOR_MINOR( 2, 0 ) /* driver version */
#define RX_LENGTH               (16)

#define I2C_SLV_ADDR_MSK        (0x7F)
#define I2C_SLV_PCLK_FREQ       (4000000) /* I2C slave peripheral clock set to 4 MHZ */

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

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* I2C Control Information */
typedef struct
{
    ARM_I2C_SignalEvent_t cb_event;           // Event callback
    ARM_I2C_STATUS        status;             // Status flags
    bool                  pending;            // Transfer pending (no STOP)
    int32_t               cnt;                // Master transfer count
    uint8_t              *data;               // Master data to transfer
    uint32_t              num;                // Number of bytes to transfer
    uint32_t              s_buf_size;         // Slave receive buffer size
    uint8_t              *sdata;              // Slave data to transfer
    uint32_t              snum;               // Number of bytes to transfer
    uint8_t              *srdata;             // Slave data to transfer
    uint32_t              srnum;              // Number of bytes received
    uint32_t              event;
} I2C_CTRL;

/* I2C Resource Configuration */
typedef struct
{
    LPC_I2C_T             *reg;                // I2C register interface
    IRQn_Type             i2c_ev_irq;         // I2C Event IRQ Number
    I2C_CTRL             *ctrl;               // Run-Time control information
} const I2C_RESOURCES;


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* I2C_LPC54102X_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/


