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
#include "Common.h"
#include "BatchManager.h"
#include "BatchState.h"
#include "i2cs_driver.h"
#include "hostif_i2c.h"
#include <string.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

#define I2C_BUF_SZ                   1024
#define CAUSE_SENSOR_DATA_READY      1  //TBD. Common definition with SPI slave driver
#define CAUSE_CONFIG_CMD_RESPONSE    6
#define GC_RESPONSE_SIZE             3
#define MAX_CONFIG_CMD_SZ            64  //TBD. Common definition with SPI slave drive


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Note: This I2C host interface driver implements the Device-Host Handshake and Transport as detailed
 * in the Host Interface Protocol Document (ver 0.9+)
 */
typedef enum _HostCmd
{
    HOST_GET_CAUSE       = 0x0,     //Read request for Get-Cause
    HOST_CONTROL_RW      = 0x3,     //Host does Configuration write to this register address
    HOST_GC_GET_DATA     = 0x48,    //Host reads Get-Cause data from this register address
    HOST_SYS_RESET       = 0x7A,    //Host writes to this register to force SensorHub soft reset
    HOST_TOGGLE_SUSPEND  = 0x7B,    //Host writes to this register to toggle its suspend indication
                                    //before suspending/after resuming.
} HostCmd;

typedef struct _HostGCResponse
{
    uint8_t Cause;    //Cause identifier for current transaction
    uint8_t szMSB;    //data size MSB
    uint8_t szLSB;    //data size LSB
} HostGCResponse_t;

typedef struct __HOSTIF_Ctrl_t {
    uint16_t rxCount;            /* Bytes so far received  */
    uint16_t txCountSet;
    uint16_t txLength;           /* Total transfer length of Tx buffer */
    uint8_t *txBuff;             /* Tx buffer pointer */
    uint8_t *oldtxBuff;
    uint8_t rxBuff[RX_LENGTH];   /* Rx buffer */
    uint8_t *txBuff_next;
    uint16_t txLength_next;
} Hostif_Ctrl_t;


/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

static uint8_t _GCBuffer[I2C_BUF_SZ];
static uint32_t _GCBufferSz = sizeof(_GCBuffer);
static HostGCResponse_t _GCResponse;
static uint8_t _CtrlReqBuf[MAX_CONFIG_CMD_SZ];

static Hostif_Ctrl_t g_hostif;
static i2c_t slave_i2c_handle;


/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      Hostif_TxNext
 *          This function get calls after a transmit has complete. This function will setup another
 *          transmit if there are data pending to be sent
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
static void Hostif_TxNext(void)
{
    int   i_tx_size = 0;

    g_hostif.txCountSet = g_hostif.txLength_next;

    if (g_hostif.txBuff_next) {
        g_hostif.txBuff = g_hostif.txBuff_next;
        g_hostif.txLength = g_hostif.txLength_next;
        g_hostif.txBuff_next = NULL;
        g_hostif.txLength_next = 0;
    } else {
        g_hostif.txBuff = NULL;
        g_hostif.txLength = 0;
    }

    i_tx_size = g_hostif.txLength > 0 ? g_hostif.txLength -1 : 0;

    /*
     * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
     * when the master is actually ready to receive.
     */
   //D0_printf("2. Startx write = %x,new data = %x\r\n",slave_i2c_handle.pXfer.txBuff,i2cXfer.txBuff);
    i2c_slave_write(&slave_i2c_handle, g_hostif.txBuff, i_tx_size);
}


/****************************************************************************************************
 * @fn      processHostCommand
 *          Processes Host Command
 *
 ***************************************************************************************************/
static int32_t processHostCommand(uint8_t *rx_buf, uint16_t length, uint8_t i2c_operation)
{
    MessageBuffer *pData = NULLP;
    uint16_t pktlen;
    uint8_t  packetID;
    int16_t  status;
    BatchStateType_t BatchState;

    switch ( rx_buf[0] )
    {

    case HOST_GET_CAUSE:
        if(i2c_operation == I2C_READ_IN_PROGRESS)
        {
            _GCBufferSz = sizeof(_GCBuffer);
            /* Dequeue Get Cause response packet into local buffer */
            BatchManagerDeQueue( _GCBuffer, &_GCBufferSz );
            packetID = GetPacketID( _GCBuffer );

            if(packetID == PKID_SENSOR_DATA)
            {
                _GCResponse.Cause = CAUSE_SENSOR_DATA_READY;
            }
            else
            {
                _GCResponse.Cause = CAUSE_CONFIG_CMD_RESPONSE;
            }

            _GCResponse.szMSB = ( ( _GCBufferSz & 0xFF00 ) >> 8 );
            _GCResponse.szLSB=  ( _GCBufferSz & 0xFF );
            Hostif_QueueTx( ( uint8_t *) &(_GCResponse), GC_RESPONSE_SIZE );
         }
        break;

    case HOST_CONTROL_RW:
        if(i2c_operation == I2C_READ_COMPLETE)
        {
            ASF_assert( ASFCreateMessage(MSG_PROCESS_CTRL_REQ,
                                sizeof(MsgCtrlReq), &pData) == ASF_OK );

            pktlen = ( ( ( uint16_t ) rx_buf[1] << 8) | ( rx_buf[2] ) );
            length -= 3;    /* first three bytes are reg offset, szMSB, sxLSB */
            ASF_assert( length == pktlen );
            memcpy( _CtrlReqBuf, &rx_buf[3], length );

            pData->msg.msgCtrlReq.pRequestPacket = _CtrlReqBuf;
            pData->msg.msgCtrlReq.length = length;

            ASF_assert( ASFSendMessage( HOST_INTF_TASK_ID, pData) == ASF_OK );
        }
        break;

    case HOST_GC_GET_DATA:
        if(i2c_operation == I2C_READ_IN_PROGRESS)
        {
            /* Dequeue Get Cause data packet into local buffer */
            Hostif_QueueTx( ( uint8_t *) &(_GCBuffer), (uint16_t)(_GCBufferSz) );
        }
        break;

    case HOST_SYS_RESET:
        if(i2c_operation == I2C_READ_COMPLETE)
        {
            NVIC_SystemReset();
        }
        break;

    case HOST_TOGGLE_SUSPEND:
        if(i2c_operation == I2C_READ_COMPLETE)
        {
            /* Get current state of Batch state machine */
            status = BatchStateGet( &BatchState );
            ASF_assert( status == OSP_STATUS_OK );

            /* toggle between Host Batch Active and Suspend states */
            if ( BatchState == BATCH_ACTIVE )
            {
                BatchStateSet( BATCH_ACTIVE_HOST_SUSPEND );
            }
            else if ( BatchState == BATCH_ACTIVE_HOST_SUSPEND )
            {
                BatchStateSet( BATCH_ACTIVE );
            }
        }
        break;

    default:
        D1_printf( "[%s] Unsupported Command: %x\r\n",__FUNCTION__, rx_buf[0] );
        break;
    }

    return ( OSP_STATUS_OK );
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      I2C_HOSTIF_IRQHandler
 *          This function is called whenever there is an I2C interrupt service request
 *          on the hostif I2C bus
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void I2C_HOSTIF_IRQHandler(void)
{

    // This transfer handler will call one of the registered callback to to service the I2C event.
    i2c_slave_receive(&slave_i2c_handle);

    if ( ( slave_i2c_handle.i2c_operation == I2C_READ_IN_PROGRESS) || (slave_i2c_handle.i2c_operation == I2C_READ_COMPLETE))
    {
        processHostCommand( slave_i2c_handle.pXfer.rxBuff, slave_i2c_handle.pXfer.bytesRecv, slave_i2c_handle.i2c_operation );
    }

    if ( slave_i2c_handle.i2c_operation == I2C_WRITE_COMPLETE )
    {
        /* Assume each transmission will transmit all the requested data
        * so no need to keep track how many been transmitted.
        * check if there is any additional data to send */
        Hostif_TxNext();
    }
}


/****************************************************************************************************
 * @fn      Hostif_QueueTx
 *          This function saves pointer to the transmit data which will be sent out when host requests
 *          for data
 * @param   [IN]pBuf  - pointer to the buffer to be transmitted
 * @param   [IN]size  - size of the buffer to be transmitted
 *
 * @return  none
 *
 ***************************************************************************************************/
void Hostif_QueueTx(uint8_t *pBuf, uint16_t size)
{
    g_hostif.txBuff = pBuf;

    if (size != 0)
        g_hostif.txCountSet = size;

    if (pBuf != NULL)
        g_hostif.oldtxBuff = pBuf;    // keep a copy of the requested transmit buffer pointer

    g_hostif.txLength = size;             // number of bytes transmitted

    /*
     * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
     * when the master is actually ready to receive.
     */
    i2c_slave_write(&slave_i2c_handle, pBuf, size);
    g_hostif.txLength_next = 0;
    g_hostif.txBuff_next = NULL;
}


/****************************************************************************************************
 * @fn      Hostif_StartTxChained
 *          This function is used to initiate a chained transmit,i.e. two buffers back to back
 *          for data
 * @param   [IN]pBuf        - pointer to the buffer to be transmitted
 * @param   [IN]size        - size of the buffer to be transmitted
 * @param   [IN]pBuf_next  - pointer to the next buffer to be transmitted
 * @param   [IN]size_next - size of the next buffer to be transmitted
 *
 * @return  none
 *
 ***************************************************************************************************/
void Hostif_StartTxChained(uint8_t *pBuf, uint16_t size, uint8_t *pBuf_next, uint16_t size_next)
{
    g_hostif.txBuff = pBuf;

    if (size != 0)
        g_hostif.txCountSet = size;

    if (pBuf != NULL)
        g_hostif.oldtxBuff = pBuf;

    g_hostif.txLength = size;
    g_hostif.txBuff_next = pBuf_next;
    g_hostif.txLength_next = size_next;

    /*
     * No actual transmission happens here, but the I2c HAL keeps the buffer for transmission
     * when the master is actually ready to receive.
     */
    i2c_slave_write(&slave_i2c_handle, pBuf, size-1);
}


/****************************************************************************************************
 * @fn      Hostif_I2C_Init
 *          This function initialize the Sensor Hub Host I2C interface
 *          for data
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void Hostif_I2C_Init(void)
{
    /* reset host IF control data structure */
    memset(&g_hostif, 0, sizeof(Hostif_Ctrl_t));

    slave_i2c_handle.pXfer.rxSz = RX_LENGTH;
    slave_i2c_handle.ui_slave_address = I2C_HOSTIF_ADDR;
    slave_i2c_handle.uc_slave_index = 0;
    slave_i2c_handle.i2c = I2C_LPC_2;
    /* I2C slave initialization */
    i2c_init(&slave_i2c_handle);

    /* Setup slave address to respond to */
    i2c_slave_mode(&slave_i2c_handle,1);

    /* init host interrupt pin */
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN);
    /* de-assert interrupt line to high to indicate Host/AP that
    * there is no data to receive */
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN, 0);

    /* Enable the interrupt for the I2C */
    NVIC_SetPriority(I2C_HOSTIF_IRQn, HOSTIF_IRQ_PRIORITY);
    NVIC_EnableIRQ(I2C_HOSTIF_IRQn);

    /* enable I2C hostif to wake-up the sensor hub */
    Chip_SYSCON_EnableWakeup(I2C_HOSTIF_WAKE);

    D0_printf("%s initialization done\r\n", __FUNCTION__);
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/

