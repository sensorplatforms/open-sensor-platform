/* Adapted from Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2016 Knowles Corporation.
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
 #ifdef I2C_DRIVER_ENABLE
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "common.h"
#include "i2c_driver.h"


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

#define I2C_TXRX_STATUS_ACTIVE          0x0
#define I2C_TXRX_STATUS_PASSED          0x1
#define I2C_TXRX_STATUS_FAILED          0x2
#define I2C_TXRX_STATUS_NACK            0x4


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum _TransferPhase {
    W_XFER_STATE_SLA,   //Slave address transmit for write transaction
    W_XFER_STATE_REG,   //Register address transmit for write
    W_XFER_STATE_DAT,   //Data transmit for write
    R_XFER_STATE_REG,   //Register address transmit for read
    R_XFER_STATE_DAT,   //Data receive for read
} XferPhase_t;

/* I2C asynchronous transfer descriptor.*/
typedef struct _AsyncDesc {
    /* 7-bit Slave Address */
    uint32_t i2c_slave_addr;
    /* Reg addr */
    uint32_t i2c_slave_reg;
    /** Transfer status. */
    uint8_t i2c_txrx_status;
    /** Transfer phase. */
    XferPhase_t i2c_txrx_phase;
    /** Pointer to the data buffer.*/
    uint8_t *pData;
    /** Total number of bytes to transfer.*/
    uint32_t num;
    /** Index of current receive/transmit buffer.*/
    uint32_t byte_index;

} AsyncDesc_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
//static osp_bool_t _I2CInitialized = false;
static volatile AsyncDesc_t _AsyncXfer;
/* Mode of transfer */
static I2C_SendMode_t _SendMode;

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
 * @fn      I2C_HardwareSetup
 *          Configures the GPIOs and h/w interface for the I2C bus to communicate with the LED board.
 *          this is specific to LED board and I2C2 is used for this purpose.
 *
 *
 * @return  void
 *
 ***************************************************************************************************/
void I2C_HardwareSetup(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    I2C_InitTypeDef   I2C_InitStructure;
    /* init gpio struct */
    GPIO_StructInit(&GPIO_InitStructure);
    
    /* Reset the peripheral (this allows soft restarts to recover the bus in case of hangups) */
    I2C_DeInit(DBG_IF_I2C);

    /* Enable Clocks to the peripheral and GPIOs used */
    /* No need to enable GPIOB here, as it is already enabled */
    RCC_AHBPeriphClockCmd(DBG_IF_I2C_GPIO_GRP_RCC, ENABLE );
    RCC_CLKFTN_DBG_I2C(RCC_PERIPH_DBG_I2C, ENABLE);
    

    /* Enable Alternate function for I2C */
    GPIO_PinAFConfig(DBG_IF_I2C_GPIO_GRP, DBG_IF_I2C_CLK_SRC, DBG_IF_I2C_CLK_AF);
    GPIO_PinAFConfig(DBG_IF_I2C_GPIO_GRP, DBG_IF_I2C_SDA_SRC, DBG_IF_I2C_SDA_AF);

    /* NVIC/Interrupt config */
    /* Configure and enable I2C event interrupt -------------------------------*/
    NVIC_InitStructure.NVIC_IRQChannel = DBG_IF_I2C_IRQ_CHANNEL;
    NVIC_InitStructure.NVIC_IRQChannelPriority = DBG_IF_I2C_IRQ_CH_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DBG_IF_I2C_IRQ_CH_CMD;
    NVIC_Init(&NVIC_InitStructure);

    /* GPIO Configuration for CLK and SDA signals */
    GPIO_InitStructure.GPIO_Pin   = DBG_IF_I2C_CLK_PIN  | DBG_IF_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( DBG_IF_I2C_GPIO_GRP, &GPIO_InitStructure );

    //Init the I2C driver interface
    I2C_StructInit(&I2C_InitStructure);
    
    //RCC_PERIPH_RESET_CMD_I2C(RCC_PERIPH_DBG_I2C, ENABLE);
    //RCC_PERIPH_RESET_CMD_I2C(RCC_PERIPH_DBG_I2C, DISABLE);

    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_Timing = DBG_IF_I2C_TIMING;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_OwnAddress1 = DBG_IF_I2C_OWN_ADDRESS;
    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_Init(DBG_IF_I2C, &I2C_InitStructure);
    
    I2C_Cmd(DBG_IF_I2C, ENABLE);  
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
void I2C_Master_Initialise(void)
{
    _AsyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;     //initialize last comm status
    /* Note: I2C bus event and error interrupts are enabled when tx is started */
}


/****************************************************************************************************
 * @fn      I2C_Wait_Completion
 *          This function allows application to pend on completion
 *
 * @param   none
 *
 * @return  returns the even received by the I2C driver task for the current transaction
 *
 ***************************************************************************************************/
uint8_t I2C_Wait_Completion( void )
{
    uint8_t result = I2C_ERR_OK;
    uint16_t event = I2C_TXRX_STATUS_FAILED;

    
    if (OS_R_TMO == os_evt_wait_or(I2C_TXRX_STATUS_FAILED | I2C_TXRX_STATUS_PASSED | I2C_TXRX_STATUS_NACK, MSEC_TO_TICS(20)))
    {
        D0_printf("### WARNING - Timedout on I2C completion ###\r\n");
        result = I2C_ERR_FAIL;
    }
    else
    {
        event = os_evt_get();
        /* I2C driver can get any one of 3 events */
        if(event == I2C_TXRX_STATUS_FAILED)
        {
            result = I2C_ERR_FAIL;
        }
        else if(event == I2C_TXRX_STATUS_NACK)
        {
            result = I2C_ERR_ADDR_NACK;
        }
    }
    //return result
    return result;
}

/****************************************************************************************************
 * @fn      I2C_Transceiver_Busy
 *          Call this function to test if the I2C_ISR is busy transmitting.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
uint8_t I2C_Transceiver_Busy( void )
{
    if (_AsyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE)
    {
        return I2C_ERR_BUSY;
    }
    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      I2C_Start_Transfer
 *          Call this function to send a prepared data. Also include how many bytes that should be
 *          sent/read excluding the address byte. The function will initiate the transfer and return
 *          immediately (or return with error if previous transfer pending). User must wait for
 *          transfer to complete by calling I2C_Wait_Completion
 *
 * @param   slaveAddr - 7-bit slave address
 * @param   regAddr - Register address at slave device from where read/write will happen
 * @param   pData - Buffer to send/receive
 * @param   dataSize - Bytes of data to send/receive
 * @param   sendMode - Either I2C_MASTER_READ or I2C_MASTER_WRITE
 *
 * @return  Error code for success or failure
 *
 ***************************************************************************************************/
uint8_t I2C_Start_Transfer( uint8_t slaveAddr, uint16_t regAddr, uint8_t *pData, uint16_t dataSize,
    I2C_SendMode_t sendMode )
{
    /* Check that no transfer is already pending*/
    if (_AsyncXfer.i2c_txrx_status == I2C_TXRX_STATUS_ACTIVE)
    {

        D0_printf("I2C_Start_Transfer: A transfer is already pending\n\r");
        return I2C_ERR_BUSY;
    }

    if ((sendMode == I2C_MASTER_READ) || (sendMode == I2C_MASTER_WRITE))
    {
        /* Update the transfer descriptor */
        _AsyncXfer.i2c_slave_addr  = (slaveAddr << 1);
        _AsyncXfer.i2c_slave_reg   = regAddr;
        _AsyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_ACTIVE;
        _AsyncXfer.pData           = pData;
        _AsyncXfer.num             = dataSize;
        _AsyncXfer.byte_index      = 0;
        _AsyncXfer.i2c_txrx_phase  = W_XFER_STATE_SLA;
        _SendMode = sendMode;

        /* Enable interrupts and clear flags */
        I2C_ITConfig( DBG_IF_I2C, I2C_IT_ERRI | I2C_IT_TCI | I2C_IT_STOPI | I2C_IT_NACKI | I2C_IT_RXI
            | I2C_IT_TXI, ENABLE );
        I2C_ClearITPendingBit( DBG_IF_I2C, I2C_IT_ADDR | I2C_IT_NACKF | I2C_IT_STOPF | I2C_IT_BERR
            | I2C_IT_ARLO | I2C_IT_OVR | I2C_IT_PECERR | I2C_IT_TIMEOUT | I2C_IT_ALERT );

        /* Initiate start */
        if (sendMode == I2C_MASTER_WRITE)
        {
            /* If buffer size is < 255, set AUTOEND mode since we don't need to send any ReStarts */
            if (dataSize < I2C_NBYTES_MAX)
            {
                dataSize += 1; //Account for register address
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, dataSize, I2C_AutoEnd_Mode,
                    I2C_Generate_Start_Write );
            }
            else
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, I2C_NBYTES_MAX, I2C_Reload_Mode,
                    I2C_Generate_Start_Write );
            }
            
            
        }
        else
        {
            /* We start with one byte for register address and then do the restart in ISR */
            I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write );
        }
    }
    else
    {
        return I2C_ERR_REQ;
    }
    return I2C_ERR_OK;
}


/****************************************************************************************************
 * @fn      I2C_Driver_ISR_Handler
 *          This function is the Interrupt Service Routine (ISR), and called when the I2C interrupt
 *          is triggered; that is whenever a I2C event has occurred.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
void DBG_IF_I2C_IRQ_HANDLER(void)
{

    __IO uint32_t SRRegister;
    uint8_t errflag=FALSE;
    uint16_t remaining;

    /* Read the I2C interrupt status register */
    SRRegister = DBG_IF_I2C->ISR;

    /* If TXIS = 1, transmit buffer is empty */
    if (SRRegister & I2C_IT_TXIS)
    {
        switch (_AsyncXfer.i2c_txrx_phase)
        {
        case W_XFER_STATE_SLA:
            DBG_IF_I2C->TXDR = _AsyncXfer.i2c_slave_reg;
            if (_SendMode == I2C_MASTER_WRITE)
            {
                _AsyncXfer.i2c_txrx_phase = W_XFER_STATE_REG;
            }
            else
            {
                _AsyncXfer.i2c_txrx_phase = R_XFER_STATE_REG;
            }
            break;

        case W_XFER_STATE_REG:
            DBG_IF_I2C->TXDR = _AsyncXfer.pData[_AsyncXfer.byte_index++];
            _AsyncXfer.i2c_txrx_phase = W_XFER_STATE_DAT;
            break;

        case W_XFER_STATE_DAT:
            DBG_IF_I2C->TXDR = _AsyncXfer.pData[_AsyncXfer.byte_index++];
            break;

        default:
            //Do Nothing!
            break;
        }
    }

    /* RX full interrupt */
    if (SRRegister & I2C_IT_RXNE)
    {
        switch (_AsyncXfer.i2c_txrx_phase)
        {
        case R_XFER_STATE_REG:
            ASF_assert( false ); //Shouldn't happen!
//            break;

        case R_XFER_STATE_DAT:
            /* Read the data register */
            _AsyncXfer.pData[_AsyncXfer.byte_index++] = DBG_IF_I2C->RXDR;
            break;

        default:
            //Do nothing!
            break;
        }
    }

    /* Transfer Complete flag set when RELOAD=0, AUTOEND=0 & NBYTES data has been transferred.
     * Note: If AUTOEND=1 is used TC interrupt will not be generated. In this case the STOPF can be
     * used to detect end of transfer and signal software
     */
    if (SRRegister & I2C_IT_TC)
    {
        /* Need to generate restart... */
        if (_SendMode == I2C_MASTER_READ)
        {
            remaining = _AsyncXfer.num - _AsyncXfer.byte_index;

            /* If remaining buffer size is < 255, set AUTOEND mode */
            if (remaining <= I2C_NBYTES_MAX)
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr,remaining, I2C_AutoEnd_Mode,
                    I2C_Generate_Start_Read );
            }
            else
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, I2C_NBYTES_MAX, I2C_Reload_Mode,
                    I2C_Generate_Start_Read );
            }
            _AsyncXfer.i2c_txrx_phase = R_XFER_STATE_DAT;
        }
    }

    /* Transfer Complete Reload flag set when RELOAD=1 & NBYTES data has been transferred.
     * RELOAD is used when more than 255 bytes need to be transferred. When the first NBYTES are
     * transmitted, TCR is set and SCL is stretched to allow software to reprogram NBYTES & other regs
     */
    if (SRRegister & I2C_IT_TCR)
    {
        if (_SendMode == I2C_MASTER_WRITE)
        {
            remaining = _AsyncXfer.num - _AsyncXfer.byte_index;

            /* If remaining buffer size is < 255, set AUTOEND mode */
            if (remaining <= I2C_NBYTES_MAX)
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr,remaining, I2C_AutoEnd_Mode,
                    I2C_No_StartStop );
            }
            else
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, I2C_NBYTES_MAX, I2C_Reload_Mode,
                    I2C_No_StartStop );
            }
        }

        if (_SendMode == I2C_MASTER_READ)
        {
            remaining = _AsyncXfer.num - _AsyncXfer.byte_index;

            /* If remaining buffer size is < 255, set AUTOEND mode */
            if (remaining <= I2C_NBYTES_MAX)
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr,remaining, I2C_AutoEnd_Mode,
                    I2C_No_StartStop );
            }
            else
            {
                I2C_TransferHandling( DBG_IF_I2C, _AsyncXfer.i2c_slave_addr, I2C_NBYTES_MAX, I2C_Reload_Mode,
                    I2C_No_StartStop );
            }
            _AsyncXfer.i2c_txrx_phase = R_XFER_STATE_DAT;
        }

    }

    /* NACK Received */
    if (SRRegister & I2C_IT_NACKF)
    {
        if (_AsyncXfer.i2c_txrx_phase == W_XFER_STATE_SLA)
        {
            D1_printf("Error: Slave NACKED - Check address settings!\r\n");
            _AsyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_NACK;
            isr_evt_set(I2C_TXRX_STATUS_NACK, asfTaskHandleTable[I2C_DRIVER_TASK_ID].handle );
        }

        /* Clear the flag */
        DBG_IF_I2C->ICR = I2C_IT_NACKF;
    }

    /* STOP detected (master or slave mode). This is useful when AUTOEND=1 is used. It will serve to
     * indicate end of transfer.
     */
    if (SRRegister & I2C_IT_STOPF)
    {
        _AsyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_PASSED;
        isr_evt_set(I2C_TXRX_STATUS_PASSED, asfTaskHandleTable[I2C_DRIVER_TASK_ID].handle );

        /* Clear the flag */
        DBG_IF_I2C->ICR = I2C_IT_STOPF;
    }
        /* If Arbitration lost is detected */
    if (SRRegister & I2C_IT_ARLO)
    {
        DBG_IF_I2C->ICR = I2C_ICR_ARLOCF;
        errflag = TRUE;
    }

    /* If Bus error is detected , this happens only in master mode. For slave mode I2CF enters 
     * address recognition state */
    if (SRRegister & I2C_IT_BERR)
    {
        DBG_IF_I2C->ICR = I2C_ICR_BERRCF;
        errflag = TRUE;
    }

    /* If Overrun or Underrun error is detected */
    if (SRRegister & I2C_IT_OVR)
    {
        DBG_IF_I2C->ICR = I2C_ICR_OVRCF;
        errflag = TRUE;
        
    }

    if(errflag == TRUE)
    {
        _AsyncXfer.i2c_txrx_status = I2C_TXRX_STATUS_FAILED;
        isr_evt_set(I2C_TXRX_STATUS_FAILED, asfTaskHandleTable[I2C_DRIVER_TASK_ID].handle );
    }
}

#endif //I2C_DRIVER
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
