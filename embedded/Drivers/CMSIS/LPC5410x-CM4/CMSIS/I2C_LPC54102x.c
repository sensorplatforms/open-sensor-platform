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
#include "Driver_I2C.h"
#include "I2C_LPC54102x.h"


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Transfer struct used for I2C master data transfer */
static I2CM_XFER_T  i2c_master_xfer;

/* I2C0 Resources Register base, IRQ type, control struct */
static I2C_CTRL i2c0_ctrl;

static I2C_RESOURCES I2C0_Resources =
{
    LPC_I2C0,
    I2C0_IRQn,
    &i2c0_ctrl
};

/* I2C2 Resources Register base, IRQ type, control struct */
static I2C_CTRL i2c2_ctrl;

static I2C_RESOURCES I2C2_Resources =
{
    LPC_I2C2,
    I2C2_IRQn,
    &i2c2_ctrl
};

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_I2C_API_VERSION,
    ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities =
{
    0  /* does not supports 10-bit addressing */
};
/* This struct is used for storing the slave data transfer context,
 * Since multi-slave transfer wont happen simulatneously keep just a single pointer
 */
static I2C_RESOURCES *i2c_slave = NULL;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static void         i2c_slave_start( uint8_t addr );
static void         i2c_slave_done( void );
static uint8_t      i2c_slave_tx( uint8_t *data );
static uint8_t      i2c_slave_rx( uint8_t data );
static uint8_t      I2C_Master_HardwareSetup( I2C_RESOURCES *i2c );

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* I2C slave callback functions */
static const I2CS_XFER_T i2cs_HostIfCallBacks =
{
    &i2c_slave_start,
    &i2c_slave_tx,
    &i2c_slave_rx,
    &i2c_slave_done
};

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      i2c_slave_start
 *          This function is start I2C handler, reset the transferred/received byte count
 *
 * @param   addr - I2C Slave address
 *
 * @return  none
 *
 ****************************************************************************************************/
static void i2c_slave_start( uint8_t addr )
{
    i2c_slave->ctrl->cnt = 0;
    return;
}


/****************************************************************************************************
 * @fn      i2c_slave_tx
 *          This function is I2C  tx handler
 *
 * @param   data - Pointer to byte to be transmitted
 *
 * @return  Slave continue bit to continue transmission.
 *
 ****************************************************************************************************/
static uint8_t i2c_slave_tx( uint8_t *data )
{
    uint8_t *p8 = (uint8_t *)i2c_slave->ctrl->sdata;

    if ( p8 != NULL )
    {
        *data = p8[i2c_slave->ctrl->cnt]; /* fetch next byte to be sent */
        i2c_slave->ctrl->cnt++;
    }
    else
    {
        *data = 0; /* Have to send something, so NAK with 0 */
    }

    if ( i2c_slave->ctrl->cnt > i2c_slave->ctrl->snum )
    {
        i2c_slave->ctrl->event      = ARM_I2C_EVENT_SLAVE_TRANSMIT; /* write operation */
        i2c_slave->ctrl->cnt        = 0;
    }

    return I2C_SLVCTL_SLVCONTINUE;
}


/****************************************************************************************************
 * @fn      i2c_slave_rx
 *          This function is I2C  rx handler
 *
 * @param   data - data received from the master, copy to the buffer
 *
 * @return  Slave continue bit to continue transmission.
 *
 ****************************************************************************************************/
static uint8_t i2c_slave_rx( uint8_t data )
{
    uint8_t *p8;

    i2c_slave->ctrl->event      = ARM_I2C_EVENT_SLAVE_RECEIVE; /* read operation */

    p8                          = i2c_slave->ctrl->srdata;
    p8[i2c_slave->ctrl->cnt++]  = (uint8_t)data;

    ASF_assert( i2c_slave->ctrl->s_buf_size > i2c_slave->ctrl->cnt )

    return I2C_SLVCTL_SLVCONTINUE;
}


/****************************************************************************************************
 * @fn      i2c_slave_done
 *          This function is I2C complete handler
 *
 * @param   None
 *
 * @return  None
 *
 ****************************************************************************************************/
static void i2c_slave_done( void )
{
    i2c_slave->ctrl->cnt    = 0;
    i2c_slave->ctrl->event  = ARM_I2C_EVENT_TRANSFER_DONE;
}


/****************************************************************************************************
 * @fn      i2c_slave_setup_slave
 *          Sets up the slace interface and enables  it.
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  None
 *
 ****************************************************************************************************/
static void i2c_slave_setup_slave( I2C_RESOURCES *i2c )
{
    uint32_t optimalDev;

    /* Clear interrupt status */
    Chip_I2CS_ClearStatus( i2c->reg, I2C_STAT_SLVDESEL );
    Chip_I2C_EnableInt( i2c->reg, I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL );

    optimalDev = Chip_Clock_GetAsyncSyscon_ClockRate() / I2C_SLV_PCLK_FREQ;
    Chip_I2C_SetClockDiv( i2c->reg, optimalDev );
    /* Enable I2C slave interface */
    Chip_I2CS_Enable( i2c->reg );
}


/****************************************************************************************************
 * @fn      I2C_Master_HardwareSetup
 *          Configures the GPIOs and h/w interface for the I2C bus
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  0 on Success
 *
 ****************************************************************************************************/
static uint8_t I2C_Master_HardwareSetup( I2C_RESOURCES *i2c )
{
    /* Reset master state machine */
    Chip_I2CM_Disable( i2c->reg );
    Chip_I2CM_Enable( i2c->reg );
    Chip_I2CM_ClearStatus( i2c->reg, I2C_STAT_MSTPENDING | I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR );

    return 0;
}

/****************************************************************************************************
 * @fn      I2C_Master_Wait_Transfer
 *            Function to wait for I2CM transfer completion
 *
 * @param   xferRecPtr - I2C Transfer struct pointer ( to check the I2C status )
 *
 * @return  None
 *
 ****************************************************************************************************/
static void I2C_Master_Wait_Transfer( I2CM_XFER_T *xferRecPtr )
{
    /* Test for still transferring data */
    while ( xferRecPtr->status == I2CM_STATUS_BUSY )
    {
        /* Sleep until next interrupt */
        __WFI();
    }
}

/****************************************************************************************************
 * @fn      OSP_I2C_GetVersion
 *          This function returns the Driver,API version
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Driver versions - API, driver versions
 *
 ****************************************************************************************************/
static ARM_DRIVER_VERSION OSP_I2C_GetVersion( I2C_RESOURCES *i2c )
{
    return DriverVersion;
}


/****************************************************************************************************
 * @fn      OSP_I2C_GetCapabilities
 *          This function returns the I2C capabilities
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Driver caps (does not support 10 bit addressing)
 *
 ****************************************************************************************************/
static ARM_I2C_CAPABILITIES OSP_I2C_GetCapabilities( I2C_RESOURCES *i2c )
{
    return DriverCapabilities;
}


/****************************************************************************************************
 * @fn      OSP_I2C_Initialize
 *          Initializes and the I/O resources for the I2C interface
 *
 * @param   cb_event - Callback to send the event
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Returns ARM_DRIVER_OK on completion of init
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_Initialize( ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c )
{
    if ( LPC_I2C0 == i2c->reg )
    {
        /* Configure the I2C interface in Master mode with the given speed */
        Chip_IOCON_PinMuxSet( LPC_IOCON, 0, 23, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN) );
        Chip_IOCON_PinMuxSet( LPC_IOCON, 0, 24, (IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGITAL_EN | IOCON_STDI2C_EN) );

        /* Enables clock and resets the peripheral */
        Chip_I2C_Init( i2c->reg );
    }
    else if ( LPC_I2C2 == i2c->reg )
    {
        /* Setup I2C pin mux */
        Chip_IOCON_PinMuxSet( LPC_IOCON, 0, 28, (IOCON_FUNC1 | IOCON_DIGITAL_EN) );
        Chip_IOCON_PinMuxSet( LPC_IOCON, 0, 27, (IOCON_FUNC1 | IOCON_DIGITAL_EN) );

        /* Enable I2C clock and reset I2C peripheral */
        Chip_I2C_Init( i2c->reg );
    }
    else
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    i2c->ctrl->cb_event = cb_event;

    return ARM_DRIVER_OK;
}


/****************************************************************************************************
 * @fn      OSP_I2C_Uninitialize
 *          uninitializes and the I/O resources for the I2C interface (does nothing)
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  Returns ARM_DRIVER_OK
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_Uninitialize( I2C_RESOURCES *i2c )
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}


/****************************************************************************************************
 * @fn      OSP_I2C_PowerControl
 *          Control I2C Interface Power.
 *
 * @param  state - Power state to set
 *               i2c - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_PowerControl( ARM_POWER_STATE state, I2C_RESOURCES *i2c )
{
    switch ( state )
    {
    case ARM_POWER_OFF:
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        {
            ASF_assert( I2C_Master_HardwareSetup( i2c ) == 0);
            /* Setup slave address to respond to */
            if ( 0 != Chip_I2CS_GetSlaveAddr( i2c->reg, 0 ) )
            {
                i2c_slave_setup_slave( i2c );
            }

            /* Enable the interrupt for the I2C */
            NVIC_EnableIRQ( i2c->i2c_ev_irq );

        }
        return ARM_DRIVER_OK;

    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    } /* switch */

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}


/****************************************************************************************************
 * @fn      OSP_I2C_MasterTransmit
 *          Start transmitting data as I2C Master.
 *
 * @param  addr         - Slave address
 * @param  data         - Pointer to buffer with data to transmit to I2C Slave
 * @param  num          - Number of data bytes to transmit
 * @param  xfer_pending - Transfer operation is pending(unused)
 * @param  i2c          - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success, ARM_DRIVER_ERROR on driver error )
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_MasterTransmit( uint32_t        addr,
                                const uint8_t   *data,
                                uint32_t        num,
                                bool            xfer_pending,
                                I2C_RESOURCES   *i2c )
{
    i2c_master_xfer.slaveAddr   = addr;
    i2c_master_xfer.txBuff      = data;
    i2c_master_xfer.txSz        = num;
    i2c_master_xfer.status      = 0;

    /* Start the I2C data transmit */
    Chip_I2CM_Xfer( i2c->reg, &i2c_master_xfer );
    /* Enable Master Interrupts */
    Chip_I2C_EnableInt( i2c->reg, I2C_STAT_MSTPENDING | I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR );

    I2C_Master_Wait_Transfer( &i2c_master_xfer );

    if( i2c_master_xfer.status == I2CM_STATUS_OK )
    {
        return ARM_DRIVER_OK;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}


/****************************************************************************************************
 * @fn      OSP_I2C_MasterReceive
 *          Start receiving data as I2C Master.
 *
 * @param  addr         - Slave address
 * @param  data         - Pointer to buffer for data to receive from I2C Slave
 * @param  num          - Number of data bytes to receive
 * @param  xfer_pending - Transfer operation is pending(unused)
 * @param  i2c          - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success, ARM_DRIVER_ERROR on driver error )
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_MasterReceive( uint32_t       addr,
                               uint8_t        *data,
                               uint32_t       num,
                               bool           xfer_pending,
                               I2C_RESOURCES  *i2c )
{
    i2c_master_xfer.slaveAddr   = addr;
    i2c_master_xfer.rxBuff      = data;
    i2c_master_xfer.rxSz        = num;
    i2c_master_xfer.status      = 0;

    /* Start the I2C data receive */
    Chip_I2CM_Xfer( i2c->reg, &i2c_master_xfer );
    /* Enable Master Interrupts */
    Chip_I2C_EnableInt( i2c->reg, I2C_STAT_MSTPENDING | I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR );

    I2C_Master_Wait_Transfer( &i2c_master_xfer );

    if( i2c_master_xfer.status == I2CM_STATUS_OK )
    {
        return ARM_DRIVER_OK;
    }
    else
    {
        return ARM_DRIVER_ERROR;
    }
}


/****************************************************************************************************
 * @fn      OSP_I2C_SlaveTransmit
 *          Start transmitting data as I2C Slave.
 *
 * @param  data - Pointer to buffer with data to transmit to I2C Master
 * @param  num  - Number of data bytes to transmit
 * @param  i2c   - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_SlaveTransmit( const uint8_t *data,
                               uint32_t      num,
                               I2C_RESOURCES *i2c )
{
    i2c->ctrl->sdata = (void *)data;
    i2c->ctrl->snum  = num;

    i2c_slave = i2c;

    /* Return success now, bytes will be written when master is requesting for data */
    return ARM_DRIVER_OK;
}


/****************************************************************************************************
 * @fn      OSP_I2C_SlaveReceive
 *          Start receiving data as I2C Slave.
 *
 * @param  data - Pointer to buffer for data to receive from I2C Master
 * @param  num  - Number of data bytes to receive
 * @param  i2c  - Private struct for I2C resources
 *
 * @return  Error status code (ARM_DRIVER_OK on success)
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_SlaveReceive( uint8_t         *data,
                              uint32_t        num,
                              I2C_RESOURCES   *i2c )
{
    i2c->ctrl->srdata = data;
    i2c->ctrl->s_buf_size = num;
    i2c_slave = i2c;

    /* Return success now, bytes will be written when received when sends the data */
    return ARM_DRIVER_OK;
}


/****************************************************************************************************
 * @fn      OSP_I2C_GetDataCount
 *          Get transferred data count.
 *
 * @param  i2c   - Private struct for I2C resources
 *
 * @return  number of data bytes transferred
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_GetDataCount( I2C_RESOURCES *i2c )
{
    return i2c_slave->ctrl->cnt;
}


/****************************************************************************************************
 * @fn      OSP_I2C_Control
 *          Get transferred data count.
 *
 * @param   control - Operation
 * @param   arg     - Argument of operation (optional)
 * @param   i2c     - Private struct for I2C resources
 *
 * @return  number of data bytes transferred
 *
 ****************************************************************************************************/
static int32_t OSP_I2C_Control( uint32_t control, uint32_t arg, I2C_RESOURCES *i2c )
{
    switch ( control )
    {
    case ARM_I2C_OWN_ADDRESS:
        /* Set the self slave address, using only the first index */
        Chip_I2CS_SetSlaveAddr( i2c->reg,
                                0,
                                (arg & I2C_SLV_ADDR_MSK) );
        break;

    case ARM_I2C_BUS_SPEED:
        switch ( arg )
        {
        case ARM_I2C_BUS_SPEED_STANDARD:
            break;

        case ARM_I2C_BUS_SPEED_FAST:
            /* setup speed and config. as Master */
            Chip_I2C_SetClockDiv( i2c->reg, I2C_MASTER_CLOCK_DIV );
            Chip_I2CM_SetBusSpeed( i2c->reg, I2C_MCLOCK_SPEED );
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


/****************************************************************************************************
 * @fn      OSP_I2C_GetStatus
 *          Get I2C status. (unsupported)
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  I2C status
 *
 ****************************************************************************************************/
static ARM_I2C_STATUS OSP_I2C_GetStatus( I2C_RESOURCES *i2c )
{
    return i2c->ctrl->status;
}

/****************************************************************************************************
 * @fn      MX_I2C_Master_IRQHandler
 *          ISR Handler for I2C master interface
 *          Handler for I2C Master Tx/Rx related interrupt
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  None
 *
 ****************************************************************************************************/
static void MX_I2C_Master_IRQHandler( I2C_RESOURCES *i2c )
{
    uint32_t state = Chip_I2CM_GetStatus(LPC_I2C0);
    i2c->ctrl->event = ARM_I2C_EVENT_GENERAL_CALL;

    if (state & ( I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR ))
    {
        Chip_I2CM_ClearStatus( i2c->reg, I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR );
    }

    /* Call I2CM ISR function with the I2C device and transfer rec */
    if (( state & I2C_STAT_MSTPENDING ))
    {
        Chip_I2CM_XferHandler( i2c->reg, &i2c_master_xfer );
        if ( i2c_master_xfer.status == I2CM_STATUS_OK )
        {
            Chip_I2C_DisableInt( i2c->reg, I2C_STAT_MSTPENDING );
        }
    }
    Chip_I2CM_ClearStatus( i2c->reg, I2C_STAT_MSTPENDING );

    /* I2C slave related interrupt */
    while (state & ( I2C_INTENSET_SLVPENDING | I2C_INTENSET_SLVDESEL ))
    {
        Chip_I2CS_XferHandler( i2c->reg, &i2cs_HostIfCallBacks );

        /* Update state */
        state = Chip_I2C_GetPendingInt( i2c->reg );
    }
    if ( NULL != i2c->ctrl->cb_event )
    {
        i2c->ctrl->cb_event( i2c->ctrl->event );
    }
}


/****************************************************************************************************
 * @fn      MX_I2C2_IRQHandler
 *          IRQ handler for I2C2 (sends start,stop sequence, sends byte buffer for tx/Received
 *          bytes from slave to a buffer. Based on event set in the callback process the received
 *          byte of set the next buffer from host ctrl
 *
 * @param   i2c - Private struct for I2C resources
 *
 * @return  none
 *
 ****************************************************************************************************/
static void MX_I2C_Slave_IRQHandler ( I2C_RESOURCES *i2c )
{
    i2c->ctrl->event = ARM_I2C_EVENT_GENERAL_CALL; /* reset current i2c operation status */
    Chip_I2CS_XferHandler( i2c->reg, &i2cs_HostIfCallBacks );

    if ( NULL != i2c->ctrl->cb_event )
    {
        i2c->ctrl->cb_event( i2c->ctrl->event );
    }
}

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


/****************************************************************************************************
 * @fn      MX_I2C0_IRQHandler
 *          ISR Handler for I2C0 bus
 *
 * @param   None
 *
 * @return  None
 *
 ****************************************************************************************************/
void MX_I2C0_IRQHandler( void )
{
    MX_I2C_Master_IRQHandler( &I2C0_Resources );
}

/****************************************************************************************************
 * @fn      MX_I2C2_IRQHandler
 *          ISR Handler for I2C2 bus
 *
 * @param   None
 *
 * @return  None
 *
 ****************************************************************************************************/
void MX_I2C2_IRQHandler( void )
{
    MX_I2C_Slave_IRQHandler( &I2C2_Resources );
}

OSP_BUILD_DRIVER_SENSOR( I2C2, OSP_I2C, &I2C2_Resources );

OSP_BUILD_DRIVER_SENSOR( I2C0, OSP_I2C, &I2C0_Resources );

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
