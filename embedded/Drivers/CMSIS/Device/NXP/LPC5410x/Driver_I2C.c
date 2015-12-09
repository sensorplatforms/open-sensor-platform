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
/*----------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*----------------------------------------------------------------------------*/

#include "Driver_I2C.h"
#include "Common.h"
#include "Driver_I2C_prv.h"


/*----------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*----------------------------------------------------------------------------*/
extern AsfTaskHandle asfTaskHandleTable[];

/*----------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*----------------------------------------------------------------------------*/
#define    I2C_TXRX_STATUS_ACTIVE               0
#define    I2C_TXRX_STATUS_PASSED               1
#define    I2C_TXRX_STATUS_FAILED               2
#define    MX_I2C0_IRQHandler                   I2C0_IRQHandler
/*----------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*----------------------------------------------------------------------------*/
/** \brief I2C asynchronous transfer descriptor.*/
typedef struct _AsyncXfer {
    /* Slave Addr */
    uint32_t i2c_slave_addr;
    /** Asynchronous transfer status. */
    uint8_t i2c_txrx_status;
    /** Pointer to the data buffer.*/
    uint8_t *pData;
    /** Total number of bytes to transfer.*/
    uint32_t num;
    /** Index of current receive/transmit buffer.*/
    uint32_t byte_index;
} AsyncXfer_t;

/*----------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*----------------------------------------------------------------------------*/
static volatile AsyncXfer_t asyncXfer;
/* Mode of transfer */
static I2C_SendMode_t i2c_mode;

/* I2C0 Resources Register base, IRQ type, control struct */
I2C_CTRL i2c0_ctrl = {0};

static I2C_RESOURCES I2C0_Resources = {
    LPC_I2C0,
    I2C0_IRQn,
    NULL,
    NULL,
    NULL,
    0,
    &i2c0_ctrl
};

/* I2C2 Resources Register base, IRQ type, control struct */
I2C_CTRL i2c2_ctrl = {0};

static I2C_RESOURCES I2C2_Resources = {
    LPC_I2C2,
    I2C2_IRQn,
    NULL,
    NULL,
    NULL,
    0,
    &i2c2_ctrl
};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_I2C_API_VERSION,
    ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
    0  /* does not supports 10-bit addressing */
};

static i2c_t i2c_slave_obj;

/*----------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*----------------------------------------------------------------------------*/
static void i2c_slave_start( uint8_t addr );
static void i2c_slave_done( void );
static uint8_t i2c_slave_tx( uint8_t *data );
static uint8_t i2c_slave_rx( uint8_t data );
osp_bool_t I2C_Master_HardwareSetup( I2C_TypeDef *busId );
void I2C_Wait_Completion( void );
uint8_t I2C_Master_Start_Transfer( uint8_t slaveAddr, uint8_t *pData, uint8_t dataSize,
                            I2C_SendMode_t sendMode );


/*----------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*----------------------------------------------------------------------------*/

/* I2C slave callback functions */
static const I2CS_XFER_T i2cs_HostIfCallBacks = {
    &i2c_slave_start,
    &i2c_slave_tx,
    &i2c_slave_rx,
    &i2c_slave_done
};

/*----------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*----------------------------------------------------------------------------*/

/******************************************************************************
 * @fn      i2c_slave_start
 *          This function is start I2C handler, current unused
 *
 * @param   addr - I2C Slave address
 *
 * @return  none
 *
 ******************************************************************************/
static void i2c_slave_start( uint8_t addr )
{
    i2c_slave_obj.pXfer.rxSz         = 0;
    return;
}


/******************************************************************************
 * @fn      i2c_slave_tx
 *          This function is I2C  tx handler
 *
 * @param   data - Pointer to byte to be transmitted
 *
 * @return  Slave continue bit to continue transmission.
 *
 ******************************************************************************/
static uint8_t i2c_slave_tx( uint8_t *data )
{
    uint8_t *p8;
    i2c_t *obj = &i2c_slave_obj; /* get the record*/

    p8 = (uint8_t *)obj->pXfer.txBuff;
    if ( p8 != NULL )
    {
        *data = p8[obj->pXfer.bytesSent]; /* fetch next byte to be sent */
        obj->pXfer.bytesSent++;
    }
    else
    {
        *data = 0; /* Have to send something, so NAK with 0 */
    }

    if ( obj->pXfer.bytesSent > obj->pXfer.txSz )
    {
        obj->i2c_operation      = 0x2; /* write operation */
        obj->pXfer.bytesSent    = 0;
    }

    return I2C_SLVCTL_SLVCONTINUE;
}


/******************************************************************************
 * @fn      i2c_slave_rx
 *          This function is I2C  rx handler
 *
 * @param   data - data received from the master
 *
 * @return  Slave continue bit to continue transmission.
 *
 ******************************************************************************/
static uint8_t i2c_slave_rx( uint8_t data )
{
    uint8_t *p8;
    i2c_t *obj = &i2c_slave_obj; /* get the record*/

    obj->i2c_operation = 0x1; /* read operation */

    p8                          = obj->pXfer.rxBuff;
    p8[obj->pXfer.bytesRecv++]  = (uint8_t)data;

    if ( obj->pXfer.bytesRecv >= obj->pXfer.rxSz )
    {
        obj->pXfer.rxBuff = (uint8_t *)obj->pXfer.rxBuff + obj->pXfer.rxSz;
        obj->pXfer.rxSz++;
        obj->pXfer.bytesRecv = 0;
    }

    return I2C_SLVCTL_SLVCONTINUE;
}


/******************************************************************************
 * @fn      i2c_slave_done
 *          This function is I2C complete handler
 *
 * @param   data - data received from the master
 *
 * @return  Slave continue bit to continue transmission.
 *
 ******************************************************************************/
static void i2c_slave_done( void )
{
    i2c_t *obj = &i2c_slave_obj; /* get the record*/

    obj->pXfer.rxBuff       = obj->rxBuff;
    obj->pXfer.status       = ERR_I2C_BUSY;
    obj->pXfer.bytesSent    = 0;
    obj->pXfer.bytesRecv    = 0;
}


/******************************************************************************
 * @fn      i2c_slave_setup_slave
 *          Sets up the slace interface and enables  it.
 *
 * @param  None
 *
 * @return  None
 *
 ******************************************************************************/
static void i2c_slave_setup_slave( void )
{
    uint32_t optimalDev;

    /* Limit usable slave address indexes to the maximum the controller can support */
    {
        Chip_I2CS_SetSlaveAddr( I2C_HOSTIF, 
                                0, 
                                (I2C_HOSTIF_ADDR & I2C_SLV_ADDR_MSK));
        /* Clear interrupt status */
        Chip_I2CS_ClearStatus( I2C_HOSTIF, I2C_STAT_SLVDESEL );

        Chip_I2C_EnableInt( I2C_HOSTIF, 
                            I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL );
    }

    optimalDev = Chip_Clock_GetAsyncSyscon_ClockRate() / I2C_SLV_PCLK_FREQ;
    Chip_I2C_SetClockDiv( I2C_HOSTIF, optimalDev );
    /* Enable I2C slave interface */
    Chip_I2CS_Enable( I2C_HOSTIF );
}


/******************************************************************************
 * @fn      I2C_Master_HardwareSetup
 *          Configures the GPIOs and h/w interface for the I2C bus
 *
 * @param   busId - I2C bus identifier in case multiple buses are supported
 *
 * @return  true if successful, false otherwise.
 *
 ******************************************************************************/
osp_bool_t I2C_Master_HardwareSetup( I2C_TypeDef *busId )
{
    /* setup speed and config. as Master */
    Chip_I2C_SetClockDiv( busId, I2C_MASTER_CLOCK_DIV );
    Chip_I2CM_SetBusSpeed( busId, I2C_MCLOCK_SPEED );
    /* Reset master state machine */
    Chip_I2CM_Disable( busId );
    Chip_I2CM_Enable( busId );

    /* Enable interrupt for pending status */
    Chip_I2C_EnableInt( busId, I2C_INTENSET_MSTPENDING );

    asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;

    /* Configure TWI interrupts */
    NVIC_SetPriority( I2C_SENSOR_BUS_IRQn, I2C_SENSOR_BUS_INT_PRIORITY );
    NVIC_EnableIRQ( I2C_SENSOR_BUS_IRQn );           //enable I2C isr

    return true;
}


/******************************************************************************
 * @fn      I2C_Wait_Completion
 *          This function allows application to pend on completion
 *
 * @param   none
 *
 * @return  none
 *
 ******************************************************************************/
void I2C_Wait_Completion( void )
{
    OS_RESULT result;

    result = os_evt_wait_or( I2C_TXRX_STATUS_FAILED | I2C_TXRX_STATUS_PASSED, 
                             MSEC_TO_TICS( 10 ));
    if ( result == OS_R_TMO )
    {
        D0_printf( "### WARNING - Timedout on I2C completion [%02X] ###\r\n", 
                   asyncXfer.i2c_slave_addr );
    }
}


/******************************************************************************
 * @fn      I2C_Master_Start_Transfer
 *          Call this function to send a prepared data. Also include how many bytes that should be
 *          sent/read including the address byte. The function will initiate the transfer and return
 *          immediately (or return with error if previous transfer pending). User must wait for
 *          transfer to complete by calling I2C_Wait_Completion
 *
 * @param   slaveAddr - I2C slave address to tx the data
 *          pData - data to write to/read from
 *          dataSize - data size to write/read
 *          sendMode - Operation to be performed (write/read)
 *
 * @return  none
 *
 *******************************************************************************/
uint8_t I2C_Master_Start_Transfer( uint8_t          slaveAddr,
                                   uint8_t          *pData,
                                   uint8_t          dataSize,
                                   I2C_SendMode_t   sendMode )
{
    /* Check that no transfer is already pending*/
    if ( asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE )
    {
        D0_printf( "I2C_Master_Start_Transfer: A transfer is already pending\n\r" );
        return I2C_ERR_BUSY;
    }

    /* Update the transfer descriptor */
    asyncXfer.i2c_slave_addr    = slaveAddr << 1;
    asyncXfer.i2c_txrx_status   = I2C_TXRX_STATUS_ACTIVE;
    asyncXfer.pData             = pData;
    asyncXfer.num               = dataSize;
    asyncXfer.byte_index        = 0;
    i2c_mode                    = sendMode;

    /* Enable read interrupt and start the transfer */
    I2C_SENSOR_BUS->MSTDAT      = asyncXfer.i2c_slave_addr; //SLA+W
    /* generate a START before enabling the interrupt */
    I2C_SENSOR_BUS->MSTCTL      = I2C_MSTCTL_MSTSTART;
    I2C_SENSOR_BUS->INTENSET    = I2C_STAT_MSTPENDING;

    return I2C_ERR_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_GetVersion
 *          This function returns the Driver,API version
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Driver versions - API, driver versions
 *
 ******************************************************************************/
ARM_DRIVER_VERSION OSP_I2C_GetVersion( I2C_RESOURCES *i2c )
{
    return DriverVersion;
}


/******************************************************************************
 * @fn      OSP_I2C_GetCapabilities
 *          This function returns the I2C capabilities
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Driver caps (does not support 10 bit addressing)
 *
 ******************************************************************************/
ARM_I2C_CAPABILITIES OSP_I2C_GetCapabilities( I2C_RESOURCES *i2c )
{
    return DriverCapabilities;
}


/******************************************************************************
 * @fn      OSP_I2C_Initialize
 *          Initializes and the I/O resources for the I2C interface
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Returns ARM_DRIVER_OK on completion of init
 *
 ******************************************************************************/
int32_t OSP_I2C_Initialize( ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c )
{
    if ( LPC_I2C0 == i2c->reg )
    {
        /* Configure the I2C interface in Master mode with the given speed */
        Chip_IOCON_PinMuxSet( LPC_IOCON, I2C_SENSOR_BUS_SCL_PIN );
        Chip_IOCON_PinMuxSet( LPC_IOCON, I2C_SENSOR_BUS_SDA_PIN );

        Chip_I2C_Init( LPC_I2C0 ); /* Enables clock and resets the peripheral */
    }
    else if ( LPC_I2C2 == i2c->reg )
    {
        /* Enable I2C clock and reset I2C peripheral */
        Chip_I2C_Init( I2C_HOSTIF );

        /* Setup I2C pin mux */
        Chip_IOCON_PinMuxSet( LPC_IOCON, HOSTIF_SCL_PIN );
        Chip_IOCON_PinMuxSet( LPC_IOCON, HOSTIF_SDA_PIN );
    }
    i2c->ctrl->cb_event = cb_event;
    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_Uninitialize
 *          uninitializes and the I/O resources for the I2C interface (does nothing)
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Returns ARM_DRIVER_OK
 *
 ******************************************************************************/
int32_t OSP_I2C_Uninitialize( I2C_RESOURCES *i2c )
{
    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_PowerControl
 *          Control I2C Interface Power.
 *
 * @param  state - Power state to set
 *               i2c - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ******************************************************************************/
int32_t OSP_I2C_PowerControl( ARM_POWER_STATE state, I2C_RESOURCES *i2c )
{
    switch ( state )
    {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        if ( i2c->reg == LPC_I2C0 )
        {
            ASF_assert( true == I2C_Master_HardwareSetup( LPC_I2C0 ));
        }
        else if ( i2c->reg == LPC_I2C2 )
        {
            /* Setup slave address to respond to */
            i2c_slave_setup_slave();

            /* Init transfer parameters  */
            i2c_slave_obj.pXfer.rxBuff      = i2c_slave_obj.rxBuff;
            i2c_slave_obj.pXfer.status      = ERR_I2C_BUSY;
            i2c_slave_obj.pXfer.bytesSent   = 0;
            i2c_slave_obj.pXfer.bytesRecv   = 0;

            Chip_GPIO_SetPinDIROutput( LPC_GPIO_PORT,
                                       HOSTIF_IRQ_PORT,
                                       HOSTIF_IRQ_PIN );
            /* de-assert interrupt line to high to indicate Host/AP that
                          * there is no data to receive */
            Chip_GPIO_SetPinState( LPC_GPIO_PORT,
                                   HOSTIF_IRQ_PORT,
                                   HOSTIF_IRQ_PIN,
                                   0 );

            /* Enable the interrupt for the I2C */
            NVIC_SetPriority( I2C_HOSTIF_IRQn, HOSTIF_IRQ_PRIORITY );
            NVIC_EnableIRQ( I2C_HOSTIF_IRQn );

            /* enable I2C hostif to wake-up the sensor hub */
            Chip_SYSCON_EnableWakeup( I2C_HOSTIF_WAKE );
        }
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    } /* switch */

    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_MasterTransmit
 *          Start transmitting data as I2C Master.
 *
 * @param  addr - Slave address
 *               data - Pointer to buffer with data to transmit to I2C Slave
 *               num - Number of data bytes to transmit
 *               xfer_pending - Transfer operation is pending(unused)
 *               i2c   - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ******************************************************************************/
int32_t OSP_I2C_MasterTransmit( uint32_t        addr,
                                const uint8_t   *data,
                                uint32_t        num,
                                bool            xfer_pending,
                                I2C_RESOURCES   *i2c )
{
    uint8_t result;

    /* Get the transmit going. Rest is handled in the ISR */
    result = I2C_Master_Start_Transfer( addr,
                                 (uint8_t *)data,
                                 num,
                                 I2C_MASTER_WRITE );
    ASF_assert( result == I2C_ERR_OK );

    /* Wait for transfer to finish before returning */
    I2C_Wait_Completion();

    return result;
}


/******************************************************************************
 * @fn      OSP_I2C_MasterReceive
 *          Start receiving data as I2C Master.
 *
 * @param  addr - Slave address
 *               data - Pointer to buffer for data to receive from I2C Slave
 *               num - Number of data bytes to receive
 *               xfer_pending - Transfer operation is pending(unused)
 *               i2c   - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ******************************************************************************/
int32_t OSP_I2C_MasterReceive( uint32_t       addr,
                               uint8_t        *data,
                               uint32_t       num,
                               bool           xfer_pending,
                               I2C_RESOURCES  *i2c )
{
    uint8_t result;

    /* Get the transfer going. Rest is handled in the ISR */
    result = I2C_Master_Start_Transfer( addr, data, num, I2C_MASTER_READ );
    ASF_assert( result == I2C_ERR_OK );

    /* Wait for status */
    I2C_Wait_Completion();

    return result;
}


/******************************************************************************
 * @fn      OSP_I2C_SlaveTransmit
 *          Start transmitting data as I2C Slave.
 *
 * @param  data - Pointer to buffer with data to transmit to I2C Master
 *               num - Number of data bytes to transmit
 *               i2c   - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ******************************************************************************/
int32_t OSP_I2C_SlaveTransmit( const uint8_t *data,
                               uint32_t      num,
                               I2C_RESOURCES *i2c )
{
    i2c_slave_obj.pXfer.txBuff  = (void *)data;
    i2c_slave_obj.pXfer.txSz    = num;

    /* Return success now, bytes will be written when master is requesting for data */
    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_SlaveReceive
 *          Start receiving data as I2C Slave.
 *
 * @param  data - Pointer to buffer for data to receive from I2C Master
 *               num - Number of data bytes to receive
 *               i2c   - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ******************************************************************************/
int32_t OSP_I2C_SlaveReceive( uint8_t         *data,
                              uint32_t        num,
                              I2C_RESOURCES   *i2c )
{
    uint8_t *p8 = i2c_slave_obj.pXfer.rxBuff;

    if ( 0 == i2c_slave_obj.pXfer.rxSz )
    {
        if ( 0x1 == i2c_slave_obj.i2c_operation )
        {
            *data = p8[0];
        }
    }
    else
    {
        *data = p8[i2c_slave_obj.pXfer.rxSz - 1];
    }
    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_GetDataCount
 *          Get transferred data count.
 *
 * @param  i2c   - Private struct for I2C resources
 *
 * @return  number of data bytes transferred
 *
 ******************************************************************************/
int32_t OSP_I2C_GetDataCount( I2C_RESOURCES *i2c )
{
    if( i2c->reg == LPC_I2C0 )
    {
        return asyncXfer.byte_index;
    }
    else if( i2c->reg == LPC_I2C2 )
    {
        return i2c_slave_obj.pXfer.rxSz;
    }
    return 0;
}


/******************************************************************************
 * @fn      OSP_I2C_Control
 *          Get transferred data count.
 *
 * @param   control - Operation
 *                arg - Argument of operation (optional)
 *                i2c - Private struct for I2C resources
 *
 * @return  number of data bytes transferred
 *
 ******************************************************************************/
int32_t OSP_I2C_Control( uint32_t control, uint32_t arg, I2C_RESOURCES *i2c )
{
    switch ( control )
    {
    case ARM_I2C_OWN_ADDRESS:
        break;

    case ARM_I2C_BUS_SPEED:
        switch ( arg )
        {
        case ARM_I2C_BUS_SPEED_STANDARD:
            break;

        case ARM_I2C_BUS_SPEED_FAST:
            break;

        case ARM_I2C_BUS_SPEED_FAST_PLUS:
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        break;

    case ARM_I2C_BUS_CLEAR:
        break;

    case ARM_I2C_ABORT_TRANSFER:
        break;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    } /* switch */

    return ARM_DRIVER_OK;
}


/******************************************************************************
 * @fn      OSP_I2C_GetStatus
 *          Get I2C status.
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  I2C status
 *
 ******************************************************************************/
ARM_I2C_STATUS OSP_I2C_GetStatus( I2C_RESOURCES *i2c )
{
    return i2c->ctrl->status;
}


/******************************************************************************
 * @fn      OSP_I2C_SignalEvent
 *          Signal I2C Events.
 *
 * @param   event - I2C Events notification mask
 *                i2c - Private struct for I2C resources
 *
 * @return  None
 *
 ******************************************************************************/
void OSP_I2C_SignalEvent( uint32_t event, I2C_RESOURCES *i2c )
{
    // function body
    return;
}


/*******************************************************************************
 * @fn      MX_I2C0_IRQHandler
 *          ISR Handler for I2C0 interface
 *          Handler for I2C Tx/Rx related interrupt
 *
 *******************************************************************************/
void MX_I2C0_IRQHandler()
{
    LPC_I2C_T *pI2C = LPC_I2C0;

    uint32_t status         = Chip_I2CM_GetStatus( pI2C );
    /* Only check Master State */
    uint32_t i2cmststate    = Chip_I2CM_GetMasterState( pI2C );
    uint32_t mstCtrl        = I2C_MSTCTL_MSTCONTINUE;

    if ( status & I2C_STAT_MSTRARBLOSS )
    {
        /* Master Lost Arbitration */
        /* Clear Status Flags */
        Chip_I2CM_ClearStatus( pI2C, I2C_STAT_MSTRARBLOSS );
        /* Master continue */
        if ( status & I2C_STAT_MSTPENDING )
        {
            pI2C->MSTCTL = I2C_MSTCTL_MSTCONTINUE;
        }
        asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
        isr_evt_set( I2C_TXRX_STATUS_FAILED,
                     asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
        D0_printf( "I2C-ISR: Arb Loss Err!\r\n" );
    }
    else if ( status & I2C_STAT_MSTSTSTPERR )
    {
        /* Master Start Stop Error */
        /* Clear Status Flags */
        Chip_I2CM_ClearStatus( pI2C, I2C_STAT_MSTSTSTPERR );

        /* Master continue */
        if ( status & I2C_STAT_MSTPENDING )
        {
            pI2C->MSTCTL = I2C_MSTCTL_MSTCONTINUE;
        }
        asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
        isr_evt_set( I2C_TXRX_STATUS_FAILED,
                     asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
        D0_printf( "I2C-ISR: Start Stop Err!\r\n" );
    }
    else if ( status & I2C_STAT_MSTPENDING )
    {
        pI2C->INTENCLR = I2C_STAT_MSTPENDING;
        /* Below five states are for Master mode: IDLE, TX, RX, NACK_ADDR, NAC_TX.
        IDLE is not under consideration for now. */
        switch ( i2cmststate )
        {
        case I2C_STAT_MSTCODE_IDLE:
            /* Do not send the message to the waiting task until we are done with the transaction */
            if ((asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE)
                                                && (asyncXfer.num == 0))
            {
                asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;
                isr_evt_set( I2C_TXRX_STATUS_PASSED,
                             asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            }
            break;

        /* Received data. Address plus Read was previously sent and Acknowledged by slave */
        case I2C_STAT_MSTCODE_RXREADY:
            asyncXfer.pData[asyncXfer.byte_index++] = pI2C->MSTDAT;
            asyncXfer.num--;
            if ( asyncXfer.num == 0 )
            { //we are done with the read
                mstCtrl |= I2C_MSTCTL_MSTSTOP;
            }
            pI2C->MSTCTL    = mstCtrl;
            pI2C->INTENSET  = I2C_STAT_MSTPENDING;
            break;

        /* Ready to Transmit data. Address plus Write was previously 
         * sent and Acknowledged by slave */
        case I2C_STAT_MSTCODE_TXREADY:
            if ( i2c_mode == I2C_MASTER_WRITE )
            {
                if ( asyncXfer.num == 0 )
                { //Done with write transaction
                    mstCtrl |= I2C_MSTCTL_MSTSTOP;
                } else
                {
                    pI2C->MSTDAT = asyncXfer.pData[asyncXfer.byte_index++];
                    asyncXfer.num--;
                }
                pI2C->MSTCTL    = mstCtrl;
                pI2C->INTENSET  = I2C_STAT_MSTPENDING;
            } else
            { // MASTER_READ//Send Re-Start with SLA+R
                pI2C->MSTDAT    = asyncXfer.i2c_slave_addr | 1;
                mstCtrl         |= I2C_MSTCTL_MSTSTART;
            }
            pI2C->MSTCTL    = mstCtrl;
            pI2C->INTENSET  = I2C_STAT_MSTPENDING;
            break;

        case I2C_STAT_MSTCODE_NACKADR: //Slave NACKed address
            /* For now just stop the transaction */
            pI2C->MSTCTL                = I2C_MSTCTL_MSTSTOP | I2C_MSTCTL_MSTCONTINUE;
            pI2C->INTENSET              = I2C_STAT_MSTPENDING;
            asyncXfer.i2c_txrx_status   = I2C_TXRX_STATUS_FAILED;
            isr_evt_set( I2C_TXRX_STATUS_FAILED,
                         asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            D0_printf( "I2C-ISR: Slave NACKed Addr!\r\n" );
            break;

        default:
        case I2C_STAT_MSTCODE_NACKDAT: //Slave NACKed transmitted data
            /* For now just stop the transaction */
            pI2C->MSTCTL                = I2C_MSTCTL_MSTSTOP | I2C_MSTCTL_MSTCONTINUE;
            pI2C->INTENSET              = I2C_STAT_MSTPENDING;
            asyncXfer.i2c_txrx_status   = I2C_TXRX_STATUS_FAILED;
            isr_evt_set( I2C_TXRX_STATUS_FAILED,
                         asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            D0_printf( "I2C-ISR: Slave NACKed Data!\r\n" );
            break;
        } /* switch */
    }
}


/******************************************************************************
 * @fn      MX_I2C2_IRQHandler
 *          IRQ handler for I2C2 (sends start,stop sequence, sends byte buffer for tx/Received
 *          bytes from slave to a buffer. Based on event set in the callback process the received
 *          byte of set the next buffer from host ctrl
 *
 * @param   None
 *
 * @return  none
 *
 ******************************************************************************/
void MX_I2C2_IRQHandler ( void )
{
    uint32_t i2c_event = 0;

    i2c_slave_obj.i2c_operation = 0; /* reset current i2c operation status */
    Chip_I2CS_XferHandler( I2C_HOSTIF, &i2cs_HostIfCallBacks );

    if ( i2c_slave_obj.i2c_operation == 0x1 )
    {
        i2c_event = ARM_I2C_EVENT_SLAVE_RECEIVE;
    }
    else if ( i2c_slave_obj.i2c_operation == 0x2 )
    {
        i2c_event = ARM_I2C_EVENT_SLAVE_TRANSMIT;
    }
    if ( NULL != I2C2_Resources.ctrl->cb_event )
    {
        I2C2_Resources.ctrl->cb_event( i2c_event );
    }
}


OSP_BUILD_DRIVER_SENSOR( I2C2, OSP_I2C, &I2C2_Resources );

OSP_BUILD_DRIVER_SENSOR( I2C0, OSP_I2C, &I2C0_Resources );
