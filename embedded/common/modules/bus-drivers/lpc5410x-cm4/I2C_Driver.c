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
#include "I2C_Driver.h"
#include "lpc_types.h"

#ifdef I2C_DRIVER

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern AsfTaskHandle asfTaskHandleTable[];

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define    I2C_TXRX_STATUS_ACTIVE               0
#define    I2C_TXRX_STATUS_PASSED               1
#define    I2C_TXRX_STATUS_FAILED               2

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/** \brief I2C asynchronous transfer descriptor.*/
typedef struct _AsyncXfer {
    /* Slave Addr */
    uint32_t i2c_slave_addr;
    /* Reg addr */
    uint32_t i2c_slave_reg;
    /** Asynchronous transfer status. */
    uint8_t i2c_txrx_status;
    /** Asynchronous transfer phase. */
    uint8_t i2c_txrx_phase;
    /** Pointer to the data buffer.*/
    uint8_t *pData;
    /** Total number of bytes to transfer.*/
    uint32_t num;
    /** Index of current receive/transmit buffer.*/
    uint32_t byte_index;

} AsyncXfer_t;


/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static osp_bool_t sI2C_Bus1Initialized = false;
static volatile AsyncXfer_t asyncXfer;
/* Mode of transfer */
static I2C_SendMode_t i2c_mode;


/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      I2C_IRQHandler
 *          Handler for I2C Tx/Rx related interrupt
 *
 ***************************************************************************************************/
void I2C_IRQHandler(LPC_I2C_T *pI2C)
{
    uint32_t status = Chip_I2CM_GetStatus(pI2C);
    uint32_t i2cmststate = Chip_I2CM_GetMasterState(pI2C);    /* Only check Master and Slave State */
    uint32_t mstCtrl = I2C_MSTCTL_MSTCONTINUE;

    if ( status & I2C_STAT_MSTRARBLOSS )
    {
        /* Master Lost Arbitration */
        /* Clear Status Flags */
        Chip_I2CM_ClearStatus(pI2C, I2C_STAT_MSTRARBLOSS);
        /* Master continue */
        if ( status & I2C_STAT_MSTPENDING ) {
            pI2C->MSTCTL = I2C_MSTCTL_MSTCONTINUE;
        }
        asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
        isr_evt_set(I2C_TXRX_STATUS_FAILED, asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
        D0_printf("I2C-ISR: Arb Loss Err!\r\n");
    }
    else if ( status & I2C_STAT_MSTSTSTPERR )
    {
        /* Master Start Stop Error */
        /* Clear Status Flags */
        Chip_I2CM_ClearStatus(pI2C, I2C_STAT_MSTSTSTPERR);

        /* Master continue */
        if ( status & I2C_STAT_MSTPENDING ) {
            pI2C->MSTCTL = I2C_MSTCTL_MSTCONTINUE;
        }
        asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
        isr_evt_set(I2C_TXRX_STATUS_FAILED, asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
        D0_printf("I2C-ISR: Start Stop Err!\r\n");
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
            if ((asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE) && (asyncXfer.num == 0)) {
                asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;
                isr_evt_set(I2C_TXRX_STATUS_PASSED, asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            }
            break;

        /* Received data. Address plus Read was previously sent and Acknowledged by slave */
        case I2C_STAT_MSTCODE_RXREADY:
            asyncXfer.pData[asyncXfer.byte_index++] = pI2C->MSTDAT;
            asyncXfer.num--;
            if (asyncXfer.num == 0) { //we are done with the read
                mstCtrl |= I2C_MSTCTL_MSTSTOP;
            }
            pI2C->MSTCTL = mstCtrl;
            pI2C->INTENSET = I2C_STAT_MSTPENDING;
            break;

        /* Ready to Transmit data. Address plus Write was previously sent and Acknowledged by slave */
        case I2C_STAT_MSTCODE_TXREADY:
            if (i2c_mode == I2C_MASTER_WRITE) {
                if (asyncXfer.i2c_txrx_phase == 0) {
                    pI2C->MSTDAT = asyncXfer.i2c_slave_reg; //Send register addr
                    asyncXfer.i2c_txrx_phase = 1;
                } else {
                    if (asyncXfer.num == 0) { //Done with write transaction
                        mstCtrl |= I2C_MSTCTL_MSTSTOP;
                    } else {
                        pI2C->MSTDAT = asyncXfer.pData[asyncXfer.byte_index++];
                        asyncXfer.num--;
                    }
                }
                pI2C->MSTCTL = mstCtrl;
                pI2C->INTENSET = I2C_STAT_MSTPENDING;
            } else { // MASTER_READ
                if (asyncXfer.i2c_txrx_phase == 0) {
                    pI2C->MSTDAT = asyncXfer.i2c_slave_reg; //Send register addr
                    asyncXfer.i2c_txrx_phase = 1;
                } else { //Send Re-Start with SLA+R
                    pI2C->MSTDAT = asyncXfer.i2c_slave_addr | 1;
                    mstCtrl |= I2C_MSTCTL_MSTSTART;
                }
            }
            pI2C->MSTCTL = mstCtrl;
            pI2C->INTENSET = I2C_STAT_MSTPENDING;
            break;

        case I2C_STAT_MSTCODE_NACKADR: //Slave NACKed address
            /* For now just stop the transaction */
            pI2C->MSTCTL = I2C_MSTCTL_MSTSTOP | I2C_MSTCTL_MSTCONTINUE;
            pI2C->INTENSET = I2C_STAT_MSTPENDING;
            asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
            isr_evt_set(I2C_TXRX_STATUS_FAILED, asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            D0_printf("I2C-ISR: Slave NACKed Addr!\r\n");
            break;

        default:
        case I2C_STAT_MSTCODE_NACKDAT: //Slave NACKed transmitted data
            /* For now just stop the transaction */
            pI2C->MSTCTL = I2C_MSTCTL_MSTSTOP | I2C_MSTCTL_MSTCONTINUE;
            pI2C->INTENSET = I2C_STAT_MSTPENDING;
            asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
            isr_evt_set(I2C_TXRX_STATUS_FAILED, asfTaskHandleTable[SENSOR_ACQ_TASK_ID].handle );
            D0_printf("I2C-ISR: Slave NACKed Data!\r\n");
            break;
        }
    }
}


/****************************************************************************************************
 * @fn      I2C_HardwareSetup
 *          Configures the GPIOs and h/w interface for the I2C bus
 *
 * @param   busId - I2C bus identifier in case multiple buses are supported
 *
 * @return  true if successful, false otherwise.
 *
 ***************************************************************************************************/
osp_bool_t I2C_HardwareSetup( I2C_TypeDef *busId )
{
    if (busId == LPC_I2C0)
    {
        if (sI2C_Bus1Initialized)
        {
            return true;
        }
        /* Configure the I2C interface in Master mode with the given speed */
        Chip_IOCON_PinMuxSet(LPC_IOCON, I2C_SENSOR_BUS_SCL_PIN);
        Chip_IOCON_PinMuxSet(LPC_IOCON, I2C_SENSOR_BUS_SDA_PIN);

        Chip_I2C_Init(busId); /* Enables clock and resets the peripheral */

        /* setup speed and config. as Master */
        Chip_I2C_SetClockDiv( busId, I2C_MASTER_CLOCK_DIV );
        Chip_I2CM_SetBusSpeed( busId, I2C_MCLOCK_SPEED );
        /* Reset master state machine */
        Chip_I2CM_Disable( busId );
        Chip_I2CM_Enable( busId );

        /* Enable interrupt for pending status */
        Chip_I2C_EnableInt( busId, I2C_INTENSET_MSTPENDING );

        I2C_Master_Initialise();

        /* Configure TWI interrupts */
        NVIC_SetPriority( I2C_SENSOR_BUS_IRQn, I2C_SENSOR_BUS_INT_PRIORITY );
        NVIC_EnableIRQ( I2C_SENSOR_BUS_IRQn );           //enable I2C isr


        sI2C_Bus1Initialized = true;

        return true;
    }
    return false;
}


/****************************************************************************************************
 * @fn      I2C_Master_Initialise
 *          Call this function to set up the I2C master to its initial standby state.
 *          Remember to enable interrupts from the main application after initializing the I2C.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void I2C_Master_Initialise( void )
{
    asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;     //initialize last comm status
}


/****************************************************************************************************
 * @fn      I2C_Wait_Completion
 *          This function allows application to pend on completion
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void I2C_Wait_Completion( void )
{
#if 1
    OS_RESULT result;

    result = os_evt_wait_or( I2C_TXRX_STATUS_FAILED | I2C_TXRX_STATUS_PASSED, MSEC_TO_TICS(10));
    if (result == OS_R_TMO)
    {
        D0_printf("### WARNING - Timedout on I2C completion [%02X - %02X] ###\r\n", asyncXfer.i2c_slave_addr, asyncXfer.i2c_slave_reg);
    }
#else
    while( asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE );
#endif
}


/****************************************************************************************************
 * @fn      I2C_Transceiver_Busy
 *          Call this function to test if the I2C_ISR is busy transmitting.
 *
 * @param   none
 *
 * @return  interrupt enabled or busy status
 *
 ***************************************************************************************************/
uint8_t I2C_Transceiver_Busy( void )
{
    if (asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE)
    {
        return I2C_ERR_BUSY;
    }
    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      I2C_Start_Transfer
 *          Call this function to send a prepared data. Also include how many bytes that should be
 *          sent/read including the address byte. The function will initiate the transfer and return
 *          immediately (or return with error if previous transfer pending). User must wait for
 *          transfer to complete by calling I2C_Wait_Completion
 *
 * @param   //TODO
 *
 * @return  none
 *
 ***************************************************************************************************/
uint8_t I2C_Start_Transfer( uint8_t slaveAddr, uint16_t regAddr, uint8_t *pData, uint8_t dataSize, I2C_SendMode_t sendMode )
{
    /* Check that no transfer is already pending*/
    if (asyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE)
    {

        D0_printf("I2C_Start_Transfer: A transfer is already pending\n\r");
        return I2C_ERR_BUSY;
    }

    /* Update the transfer descriptor */
    asyncXfer.i2c_slave_addr  = slaveAddr << 1;
    asyncXfer.i2c_slave_reg   = regAddr;
    asyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_ACTIVE;
    asyncXfer.pData           = pData;
    asyncXfer.num             = dataSize;
    asyncXfer.byte_index      = 0;
    asyncXfer.i2c_txrx_phase  = 0;
    i2c_mode = sendMode;

    /* Enable read interrupt and start the transfer */
    I2C_SENSOR_BUS->MSTDAT = asyncXfer.i2c_slave_addr; //SLA+W
    I2C_SENSOR_BUS->MSTCTL = I2C_MSTCTL_MSTSTART;        //generate a START before enabling the interrupt
    I2C_SENSOR_BUS->INTENSET = I2C_STAT_MSTPENDING;

    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      I2C_Get_Data_From_Transceiver
 *          Call this function to read out the requested data from the I2C transceiver buffer.
 *          I.e. first call I2C_Start_Transceiver to send a request for data to the slave. Then Run
 *          this function to collect the data when they have arrived. Include a pointer to where to
 *          place the data and the number of bytes requested (including the address field) in the
 *          function call. The function will hold execution (loop) until the I2C_ISR has completed
 *          with the previous operation, before reading out the data and returning. If there was an
 *          error in the previous transmission the function will return the I2C error code.
 *
 * @param   msg Data buffer for receive
 * @param   msgSize Size of data buffer
 *
 * @return  error status
 *
 ***************************************************************************************************/
uint8_t I2C_Get_Data_From_Transceiver( uint8_t *msg, uint8_t msgSize )
{
    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      I2C_Driver_ERR_ISR_Handler
 *          This function is the I2C error interrupt handler.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void I2C_Driver_ERR_ISR_Handler(void)
{
}

#endif //I2C_DRIVER
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
