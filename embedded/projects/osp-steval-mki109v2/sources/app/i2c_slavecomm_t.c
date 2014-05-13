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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "common.h"
#ifdef ANDROID_COMM_TASK
#include "hostinterface.h"
#include "osp-sensors.h"
#include <string.h>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern AsfTaskHandle asfTaskHandleTable[];
extern uint32_t AccelTimeExtend;
extern uint32_t MagTimeExtend;
extern uint32_t GyroTimeExtend;
extern uint32_t QuatTimeExtend;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define TX_LENGTH                   32
#define RX_LENGTH                   32
#define I2C_SLAVE_ADDR_8BIT         (0x18 << 1)

#define SH_WHO_AM_I                 0x54
#define SH_VERSION0                 0x01
#define SH_VERSION1                 0x22

#define I2C_SLAVE_XFER_DONE         0x10
#define I2C_OVERREACH_VAL           0xCC //This value is sent for access beyond register area

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef struct I2CXferTag {
    const uint8_t *txBuff;  /* Pointer to array of bytes to be transmitted */
    uint32_t  txSz;         /* Number of bytes in transmit array,
                               if 0 only receive transfer will be carried on */
    uint32_t  txCnt;        /* Current transfer transmitted bytes */
    uint32_t  rxCnt;        /* Received bytes count */
} I2CXfer_t;

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static I2CXfer_t slave_xfer;
static SH_RegArea_t SlaveRegMap;

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
 * @fn      I2C_Slave_Initialise
 *          Call this function to set up the I2C slave to its initial standby state.
 *
 ***************************************************************************************************/
static void I2C_Slave_Initialise(void)
{
    I2C_InitTypeDef   I2C_InitStructure;

    /* Enable the bus */
    I2C_Cmd(I2C_SLAVE_BUS, ENABLE);

    /* Configure the I2C interface */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDR_8BIT;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SLAVE_BUS_CLOCK;
    I2C_Init(I2C_SLAVE_BUS, &I2C_InitStructure);

    /* Enable interrupts and clear flags */
    I2C_ITConfig( I2C_SLAVE_BUS, (I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR), ENABLE );
    I2C_ClearITPendingBit( I2C_SLAVE_BUS, I2C_IT_SMBALERT | I2C_IT_TIMEOUT | I2C_IT_PECERR | I2C_IT_OVR
        | I2C_IT_AF | I2C_IT_ARLO | I2C_IT_BERR );
}


/****************************************************************************************************
 * @fn      I2C_Slave_init
 *          Initialize the Sensor Hub I2C Slave hardware
 *
 ***************************************************************************************************/
static void I2C_Slave_init( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    /* Reset the peripheral (this allows soft restarts to recover the bus in case of hangups) */
    I2C_DeInit(I2C_SLAVE_BUS);

    /* Enable Clocks to the peripheral and GPIOs used */
    RCC_APB1PeriphClockCmd(RCC_Periph_I2C_SLAVE_BUS, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_Periph_I2C_SLAVE_BUS_GPIO, ENABLE ); //for I2C port GPIO

    /* NVIC/Interrupt config */
    /* Configure and enable I2C event interrupt -------------------------------*/
    NVIC_InitStructure.NVIC_IRQChannel = I2C_SLAVE_BUS_EVENT_IRQ_CH;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = I2C_SLAVE_BUS_INT_PREEMPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = I2C_SLAVE_BUS_EVENT_INT_SUB_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure and enable I2C error interrupt -------------------------------*/  
    NVIC_InitStructure.NVIC_IRQChannel = I2C_SLAVE_BUS_ERROR_IRQ_CH;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = I2C_SLAVE_BUS_ERROR_INT_SUB_PRIORITY;
    NVIC_Init(&NVIC_InitStructure);

    /* GPIO Configuration for CLK and SDA signals */
    GPIO_InitStructure.GPIO_Pin = I2C_SLAVE_BUS_CLK_PIN  | I2C_SLAVE_BUS_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( I2C_SLAVE_BUS_GPIO_GRP, &GPIO_InitStructure );

    /* Sensor Hub Host interrupt pin */
    RCC_APB2PeriphClockCmd(SH_INT_RCC, ENABLE );
    SensorHubIntLow(); //ensure output is deasserted when initialized.
    GPIO_InitStructure.GPIO_Pin = SH_INT_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //Because we are interfacing 3.3V to 1.8V
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( SH_INT_GPIO_GRP, &GPIO_InitStructure );

    /* Initialize the I2C slave interface */
    I2C_Slave_Initialise();
}

/****************************************************************************************************
 * @fn      SH_Slave_init
 *          Initialize the Sensor Hub I2C Slave register interface
 *
 ***************************************************************************************************/
static void SH_Slave_init(void)
{
    memset(&SlaveRegMap, 0, sizeof(SlaveRegMap));
    SlaveRegMap.version0 = SH_VERSION0;
    SlaveRegMap.version1 = SH_VERSION1;
    SlaveRegMap.whoami   = SH_WHO_AM_I;

    SlaveRegMap.irq_cause = SH_MSG_IRQCAUSE_NEWMSG;
    SlaveRegMap.read_len = 11;
    SlaveRegMap.rd_mem[0] = SH_MSG_TYPE_ABS_ACCEL;
}


/****************************************************************************************************
 * @fn      I2C_Slave_Wait_Completion
 *          This function allows application to pend on completion for slave transfer
 *
 ***************************************************************************************************/
static void I2C_Slave_Wait_Completion( void )
{
    OS_RESULT result;
    static uint32_t dessimate = 0;

    result = os_evt_wait_or( I2C_SLAVE_XFER_DONE, MSEC_TO_TICS(500));
    if (result == OS_R_TMO)
    {
        dessimate++;
        if ((dessimate & 0x1) == 0)
        {
            SensorHubIntLow(); //Deassert Interrupt
            //D1_printf("\t### WARNING - Timedout on I2C Slave completion (%d) ###\r\n", dessimate);
            LED_Toggle(LED_YELLOW);
        }
    }
}


/****************************************************************************************************
 * @fn      SendSensorData
 *          Sends 3-axis sensor data over the I2C slave interface
 *
 ***************************************************************************************************/
static void SendSensorData( uint8_t sensorId, MsgSensorData *pMsg )
{
    ShMotionSensor_t sensData;

    switch (sensorId)
    {
    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        SlaveRegMap.rd_mem[0] = SH_MSG_TYPE_ABS_ACCEL;
        sensData.TimeStamp.timestamp40 = AccelTimeExtend & 0xFF;
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        SlaveRegMap.rd_mem[0] = SH_MSG_TYPE_ABS_MAG;
        sensData.TimeStamp.timestamp40 = MagTimeExtend & 0xFF;
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        SlaveRegMap.rd_mem[0] = SH_MSG_TYPE_ABS_GYRO;
        sensData.TimeStamp.timestamp40 = GyroTimeExtend & 0xFF;
        break;

    default:
        return; //Unsupported sensor

    }

    sensData.Data[0] = -pMsg->X;
    sensData.Data[1] = pMsg->Y;
    sensData.Data[2] = -pMsg->Z;
    sensData.TimeStamp.timestamp32 = pMsg->timeStamp;

    SlaveRegMap.irq_cause = SH_MSG_IRQCAUSE_NEWMSG;
    SlaveRegMap.read_len = sizeof(sensData) + 1;
    memcpy( &SlaveRegMap.rd_mem[1], &sensData, sizeof(sensData) );
    /*  Assert interrupt request to Host */
    SensorHubIntHigh();
    /* Wait for I2C transfer to finish */
    I2C_Slave_Wait_Completion();
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      I2C_Slave_Handler
 *          Handles the I2C Slave communication events
 *
 ***************************************************************************************************/
void I2C_Slave_Handler(I2C_TypeDef *pI2C, uint8_t irqCh)
{
    uint32_t i2cslvstate = I2C_GetLastEvent(pI2C);
    uint8_t slvRxData;

    /* If error event - clear it */
    if ((irqCh == I2C_SLAVE_BUS_ERROR_IRQ_CH) && ((i2cslvstate & 0xFF00) != 0))
    {
        pI2C->SR1 &= 0x00FF;

        if ((i2cslvstate & 0xFF00) == I2C_EVENT_SLAVE_ACK_FAILURE)
        {
            /* Master NAKed - this was end of transaction when slave is transmitting */
            isr_evt_set(I2C_SLAVE_XFER_DONE, asfTaskHandleTable[I2CSLAVE_COMM_TASK_ID].handle );
            slave_xfer.rxCnt = 0;
            return;
        }
    }

    /* Below three states are for Slave mode: Address Received, TX, and RX. */
    switch ( i2cslvstate )
    {
    case I2C_EVENT_SLAVE_BYTE_RECEIVED:  /* 00020040 */
    case (I2C_EVENT_SLAVE_BYTE_RECEIVED | I2C_SR1_BTF):
        SensorHubIntLow(); //Deassert Interrupt
        slvRxData = (uint8_t)I2C_ReceiveData(pI2C);
        slave_xfer.rxCnt++;
        if (slave_xfer.rxCnt == 1) //First byte received must be the register address
        {
            switch (slvRxData)
            {
            case SH_REG_IRQ_CAUSE:
                slave_xfer.txBuff = &SlaveRegMap.irq_cause;
                slave_xfer.txSz = TX_LENGTH;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_RD_LEN:
                slave_xfer.txBuff = &SlaveRegMap.read_len;
                slave_xfer.txSz = TX_LENGTH;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_ACK:
                //TODO -- Need to define state machine logic that would handle ACK
                break;

            case SH_REG_REQUEST:
                //TODO
                break;

            case SH_REG_WHO_AM_I:
                slave_xfer.txBuff = &SlaveRegMap.whoami;
                slave_xfer.txSz = TX_LENGTH;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_VERSION0:
                slave_xfer.txBuff = &SlaveRegMap.version0;
                slave_xfer.txSz = TX_LENGTH;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_VERSION1:
                slave_xfer.txBuff = &SlaveRegMap.version1;
                slave_xfer.txSz = TX_LENGTH;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_RD_MEM:
                slave_xfer.txBuff = &SlaveRegMap.rd_mem[0];
                slave_xfer.txSz = SlaveRegMap.read_len;
                slave_xfer.txCnt = 0;
                break;

            case SH_REG_WR_MEM:
            default:
                //Not supported at this time so just NACK it for now
                I2C_AcknowledgeConfig(pI2C, DISABLE);
                break;

            case 0xEE: //Ack write
                //Ack is ignored for now
                break;
            }
        }
        else
        {
            //TODO Implement host command functions
        }
        break;

    case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:  /* 00060082 */
        slave_xfer.rxCnt = 0; /* I2C Read Mode - no more receive */
        /* Fall thru */
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTING: /* 00060080 */
    case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:
    case 0x00060004: //Sometimes we get this event and the driver is stuck in ISR!
        /* In transmit we will do not want to send data beyond the register set */
        if (&slave_xfer.txBuff[slave_xfer.txCnt] < &SlaveRegMap.endMarker)
        {
            I2C_SendData(pI2C, slave_xfer.txBuff[slave_xfer.txCnt++]);
        }
        else
        {
            I2C_SendData(pI2C, I2C_OVERREACH_VAL); //This value signifies read beyond allowed range
        }
        break;

    case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED: /* 00020002 */
        /* Nothing to do except continue */
        break;

    case I2C_EVENT_SLAVE_STOP_DETECTED: /* This is end of transaction when slave is receiving */
        /* if (STOPF==1) Read SR1;Write CR1 */
        I2C_GetFlagStatus(pI2C, I2C_FLAG_STOPF);
        I2C_Cmd(pI2C, ENABLE);
        isr_evt_set(I2C_SLAVE_XFER_DONE, asfTaskHandleTable[I2CSLAVE_COMM_TASK_ID].handle );
        slave_xfer.rxCnt = 0;
        /* Re-enable ACK (in case it was disabled) */
        I2C_AcknowledgeConfig(pI2C, ENABLE);
        break;

    default:
        break;
    }
}


/****************************************************************************************************
 * @fn      I2CCommTask
 *          This tasks primary goal is to serialize the communication request (sensor results) going
 *          over I2C
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK void I2CCommTask( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;

    /* I2C Slave mode initialization */
    I2C_Slave_init();

    /* Init register area for slave */
    SH_Slave_init();

    while(1)
    {
        ASFReceiveMessage(I2CSLAVE_COMM_TASK_ID, &rcvMsg );

        switch (rcvMsg->msgId)
        {
        case MSG_ACC_DATA:
            SendSensorData(SENSOR_ACCELEROMETER_UNCALIBRATED, &rcvMsg->msg.msgAccelData);
            break;

        case MSG_MAG_DATA:
            SendSensorData(SENSOR_MAGNETIC_FIELD_UNCALIBRATED, &rcvMsg->msg.msgMagData);
            break;

        case MSG_GYRO_DATA:
            SendSensorData(SENSOR_GYROSCOPE_UNCALIBRATED, &rcvMsg->msg.msgGyroData);
            break;

        default:
            D1_printf("I2C:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}
#endif //ANDROID_COMM_TASK

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
