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

/*-------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------*/
#include "common.h"
#ifdef ANDROID_COMM_TASK
#include "hostinterface.h"
#include "osp-sensors.h"
#include <string.h>
#include "SensorPackets.h"
#include "BlockMemory.h"
#include "Queue.h"

#define HIF_PACKET_SIZE    M_CalcBufferSize(sizeof(HostIFPackets_t))


void Hostif_Init(void);
void Hostif_StartTx(uint8_t *pBuf, uint16_t size, int magic);
void CHostif_StartTxChained(uint8_t *pBuf, uint16_t size, uint8_t *pBuf_next, uint16_t size_next, int magic);

/*-------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------*/
OSP_STATUS_t Algorithm_SubscribeSensor( ASensorType_t sensor);
OSP_STATUS_t Algorithm_UnsubscribeSensor( ASensorType_t sensor);

/*-------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------*/
#define SH_WHO_AM_I                 0x54
#define SH_VERSION0                 0x01
#define SH_VERSION1                 0x23

/* Q Size Common Definitions */
#define QUEUE_LOW_THR                               (0)
#define QUEUE_HIGH_THR                              (1)

/* Sensor Data Queue Size Definition */
/* HIF Queue size indicate number of packet a single Q can hold */
#define HIF_SENSOR_DATA_QUEUE_SIZE                  (256)

/* HIF Sensor Data Packet pool size */
#define HIF_SENSOR_DATA_PACKET_POOL_SIZE            HIF_SENSOR_DATA_QUEUE_SIZE

static uint32_t SensorState[2];

// TODO: Need to call algorithm module to subscribe the enable sensor.
static void SensorEnable(ASensorType_t sen)
{
    int v = (int)sen;
    int set;

    if (v < 32) {
        set = (1<<v);
        SensorState[0] |= set;
    } else {
        set = (1<<(v-32));
        SensorState[1] |= set;
    }
    // Why hardcode significant motion enable ?
    SensorState[0] |= 1 << SENSOR_SIGNIFICANT_MOTION;

    /* Now subscribe to the algorithm to enable this sensor */
    Algorithm_SubscribeSensor(sen);

}

// TODO: Need to call the algorithm to unsubscribe the sensor
static void SensorDisable(ASensorType_t sen)
{
    int v = (int)sen;
    int set;

    if (v < 32) {
        set = (1<<v);
        SensorState[0] &= ~set;
    } else {
        set = (1<<(v-32));
        SensorState[1] &= ~set;
    }

    // Why hardcode this sensor?
    SensorState[0] |= 1 << SENSOR_SIGNIFICANT_MOTION;

    // Now un-subscribe this sensor from the algorithm
    Algorithm_UnsubscribeSensor(sen);
}

static int GetSensorState(ASensorType_t sen)
{
    int v = (int)sen;
    int set;
    if (v < 32) {
        /* Standard Android sensor enum value */
        set = (1<<v);
        if (SensorState[0] & set) return 1;
        return 0;
    } else {
        /* Private android sensor. FIXME: Need to mask out the private mask bit before apply the bit shift operation */
        set = (1<<(v-32));
        if (SensorState[1] & set) return 1;
        return 0;
    }
}

/*-------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/
static SH_RegArea_t SlaveRegMap;
static uint8_t QueueOverFlow = 0;
Queue_t *_HiFNonWakeupQueue = NULL;

/* Memory Pool for Sensor Data Packet for NonWakeup sensor and Wakeup sensor */
DECLARE_BLOCK_POOL( SensorDataPacketPool, HIF_PACKET_SIZE, HIF_SENSOR_DATA_PACKET_POOL_SIZE );

/*-------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      QHighThresholdCallBack
 *          This Callback called when queue reach to high threshold mark
 *
 * @param  [IN] QType - Type of Queue
 *
 * @return  none
 *
 ***************************************************************************************************/
static void QHighThresholdCallBack( Q_Type_t QType )
{
    switch ( QType )
    {
    case QUEUE_NONWAKEUP_TYPE:
        /* Send indication to host */
        SensorHubAssertInt();
        break;

    default:
        break;
    }
}


/****************************************************************************************************
 * @fn      QEmptyCallBack
 *          Callback for queue when it gets empty
 *
 * @param  [IN] QType - Type of Queue
 *
 * @return  void
 *
 ***************************************************************************************************/
static void QEmptyCallBack( Q_Type_t QType )
{

    /* Mark Q empty */
    switch ( QType )
    {
    case QUEUE_NONWAKEUP_TYPE:
        /* Q is empty so clear Host interrupt pin */
        SensorHubDeAssertInt();
        break;

    default:
        break;
    }
}


/****************************************************************************************************
 * @fn      QInitialize
 *          Initialize Non-Wakeup, Wakeup and Control Response Q
 *
 * @param   none
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t QInitialize( void )
{
    int16_t errCode;

    /* Initialize Data Packet pool */
    //TODO - Opportunity for future optimization here by having different PACKET
    // size for this pool since not all sensors will be wakeup type and most likely
    // the wakeup sensors data will be smaller in size as compared to e.g. Rotation Vector
    InitBlockPool( SensorDataPacketPool, sizeof(SensorDataPacketPool), HIF_PACKET_SIZE );

    /* Create Non wakeup Queue */
    _HiFNonWakeupQueue = QueueCreate( HIF_SENSOR_DATA_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert(_HiFNonWakeupQueue != NULL);

    /* Register Call backs for High and Low Thresholds   */
    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_EMPTY_CB, (fpQueueEvtCallback_t) QEmptyCallBack, (void *)QUEUE_NONWAKEUP_TYPE );
    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_HIGH_THRESHOLD_CB, (fpQueueEvtCallback_t) QHighThresholdCallBack, (void *)QUEUE_NONWAKEUP_TYPE );

    return errCode;
}


/************************************************************************
 * @fn      DeQueueToBuffer
 *          Dequeues HIF packets from the given queue and copies to the
 *          buffer provided. pBufSz contains the buffer size on entry and
 *          returns the buffer size consumed.
 *
 ************************************************************************/
int16_t DeQueueToBuffer( uint8_t *pBuf, uint32_t *pBufSz, Queue_t *pQ )
{
    int16_t status;
    Buffer_t *pHIFPkt;
    uint32_t bufLen = *pBufSz;

    *pBufSz = 0; //Reset outgoing size consumed

    do {
        status = DeQueue( pQ, &pHIFPkt );
        if (status == OSP_STATUS_OK) {
            if (bufLen >= pHIFPkt->Header.Length) {
                memcpy( (pBuf + *pBufSz), &pHIFPkt->DataStart, pHIFPkt->Header.Length );

                /* Update length */
                *pBufSz += pHIFPkt->Header.Length;
                bufLen -= pHIFPkt->Header.Length;
            }

            /* Free packet memory */
            status = FreeBlock( SensorDataPacketPool, pHIFPkt );
            ASF_assert(status == OSP_STATUS_OK);
        }
    } while((status != OSP_STATUS_QUEUE_EMPTY) && (bufLen >= pHIFPkt->Header.Length));

    return OSP_STATUS_OK;
}


/************************************************************************
 * @fn      SH_Slave_init
 *          Initialize the Sensor Hub I2C Slave register interface
 *
 ************************************************************************/
static void SH_Slave_init(void)
{
    memset(&SlaveRegMap, 0, sizeof(SlaveRegMap));
    SlaveRegMap.version0 = SH_VERSION0;
    SlaveRegMap.version1 = SH_VERSION1;
    SlaveRegMap.whoami   = SH_WHO_AM_I;

    SlaveRegMap.irq_cause = 0;
    SlaveRegMap.read_len  = 0;
    SlaveRegMap.rd_mem[0] = 0;
}


/***************************************************************************
 * @fn      SendSensorBoolData
 *          Packetizes in HIF format and queue data for host.
 *
 ***************************************************************************/

static void SendSensorBoolData(ASensorType_t sensorType, MsgSensorBoolData *pMsg)
{
    Buffer_t    *pHifPacket;
    uint8_t     *pPayload;
    int16_t status;

    // Do not send data if host did not activate this sensor type
    if (GetSensorState(sensorType) == 0)
        return;

    /* Allocate packet buffer, packetize and place in queue */

    pHifPacket = (Buffer_t *) AllocBlock( SensorDataPacketPool );
    if (pHifPacket == NULL) {
        D0_printf("OOPS! Couldn't alloc packet [%s] for %d\r\n", __FUNCTION__, sensorType);
        return;
    }

    pPayload = M_GetBufferDataStart(pHifPacket);

    /* Process sensor and format into packet */
    switch (sensorType) {
    case SENSOR_STEP_DETECTOR:
        {
            StepDetector_t stepDetectorData;

            stepDetectorData.TimeStamp.TS64 = pMsg->timeStamp;
            stepDetectorData.StepDetected   = pMsg->active;

            /* Format Packet */
            pHifPacket->Header.Length = FormatStepDetectorPkt( pPayload, &stepDetectorData, sensorType );
        }
        break;

    case SENSOR_SIGNIFICANT_MOTION:
        {
            SignificantMotion_t motionData;

            motionData.TimeStamp.TS64 = pMsg->timeStamp;
            motionData.MotionDetected = pMsg->active;

            /* Format Packet */
            pHifPacket->Header.Length = FormatSignificantMotionPktFixP( pPayload, &motionData, sensorType );
        }
        break;

    default:
        /* Free the buffer */
        status = FreeBlock( SensorDataPacketPool, pHifPacket );
        ASF_assert( status == OSP_STATUS_OK );
        D0_printf("Unhandled sensor [%d] in %s!\r\n", sensorType, __FUNCTION__);
        return;
    }

    /* Enqueue packet in HIF queue */
    if (pHifPacket->Header.Length > 0) {
        status = EnQueue( _HiFNonWakeupQueue, pHifPacket );
        ASF_assert(status == OSP_STATUS_OK);
    } else {
        /* Free the packet */
        D0_printf("Packetization error [%d] for sensor %d\r\n", pHifPacket->Header.Length, sensorType);
        status = FreeBlock( SensorDataPacketPool, pHifPacket );
        ASF_assert( status == OSP_STATUS_OK );
    }
}


/***************************************************************************
 * @fn      SendSensorData
 *          Sends 3-axis sensor data over the I2C slave interface

 * Enqueues data only.
 *
 ***************************************************************************/
static void SendSensorData(ASensorType_t sensorType, MsgSensorData *pMsg)
{
    Buffer_t    *pHifPacket;
    uint8_t        *pPayload;
    int16_t status;

    // Do not send data if host did not activate this sensor type
    if (GetSensorState(sensorType) == 0)
        return;

    /* Allocate packet buffer, packetize and place in queue */

    pHifPacket = (Buffer_t *) AllocBlock( SensorDataPacketPool );
    if (pHifPacket == NULL) {
        D0_printf("OOPS! Couldn't alloc packet [%s] for %d\r\n", __FUNCTION__, sensorType);
        return;
    }

    pPayload = M_GetBufferDataStart(pHifPacket);

    /* Process sensor and format into packet */
    switch (sensorType) {
    case AP_PSENSOR_ACCELEROMETER_UNCALIBRATED:
    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
    case SENSOR_GYROSCOPE_UNCALIBRATED:
        {
            UncalibratedFixP_t  UnCalFixPData;

            UnCalFixPData.TimeStamp.TS64 = pMsg->timeStamp;
            UnCalFixPData.Axis[0]    = pMsg->X;
            UnCalFixPData.Axis[1]    = pMsg->Y;
            UnCalFixPData.Axis[2]    = pMsg->Z;
            UnCalFixPData.Offset[0]    = 0;
            UnCalFixPData.Offset[1]    = 0;
            UnCalFixPData.Offset[2]    = 0;

            pHifPacket->Header.Length = FormatUncalibratedPktFixP(pPayload,
                &UnCalFixPData, META_DATA_UNUSED, sensorType);
        }
        break;

    case SENSOR_ACCELEROMETER:
    case SENSOR_MAGNETIC_FIELD:
    case SENSOR_GYROSCOPE:
        {
            CalibratedFixP_t    CalFixPData;

            CalFixPData.TimeStamp.TS64 = pMsg->timeStamp;
            CalFixPData.Axis[0]    = pMsg->X;
            CalFixPData.Axis[1]    = pMsg->Y;
            CalFixPData.Axis[2]    = pMsg->Z;

            pHifPacket->Header.Length = FormatCalibratedPktFixP(pPayload,
                &CalFixPData, sensorType);
        }
        break;

    case SENSOR_ROTATION_VECTOR:
    case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
    case SENSOR_GAME_ROTATION_VECTOR:
        {
            QuaternionFixP_t    QuatFixPData;

            QuatFixPData.TimeStamp.TS64 = pMsg->timeStamp;
            QuatFixPData.Quat[0]    = pMsg->W;
            QuatFixPData.Quat[1]    = pMsg->X;
            QuatFixPData.Quat[2]    = pMsg->Y;
            QuatFixPData.Quat[3]    = pMsg->Z;

            pHifPacket->Header.Length = FormatQuaternionPktFixP(pPayload,
                &QuatFixPData, sensorType);
        }
        break;

    case SENSOR_GRAVITY:
    case SENSOR_LINEAR_ACCELERATION:
        {
            ThreeAxisFixP_t fixPData;

            fixPData.TimeStamp.TS64 = pMsg->timeStamp;
            fixPData.Axis[0]    = pMsg->X;
            fixPData.Axis[1]    = pMsg->Y;
            fixPData.Axis[2]    = pMsg->Z;

            pHifPacket->Header.Length = FormatThreeAxisPktFixP(pPayload,
                &fixPData, sensorType);
        }
        break;

    case SENSOR_ORIENTATION:
        {
            OrientationFixP_t fixPData;

            fixPData.TimeStamp.TS64 = pMsg->timeStamp;
            fixPData.Pitch  = pMsg->X;
            fixPData.Roll   = pMsg->Y;
            fixPData.Yaw    = pMsg->Z;

            pHifPacket->Header.Length = FormatOrientationFixP(pPayload,
                &fixPData, sensorType);
        }
        break;

    case SENSOR_STEP_COUNTER:
        {
            StepCounter_t stepCountData;

            stepCountData.TimeStamp.TS64 = pMsg->timeStamp;
            stepCountData.NumStepsTotal  = pMsg->X;

            /* Format Packet */
            pHifPacket->Header.Length = FormatStepCounterPkt( pPayload, &stepCountData, sensorType );
        }
        break;

    case SENSOR_PRESSURE:
        //TODO - Ignore for now!
    default:
        /* Free the buffer */
        status = FreeBlock( SensorDataPacketPool, pHifPacket );
        ASF_assert( status == OSP_STATUS_OK );
        D0_printf("Unhandled sensor [%d] in %s!\r\n", sensorType, __FUNCTION__);
        return;
    }

    /* Enqueue packet in HIF queue */
    if (pHifPacket->Header.Length > 0) {
        status = EnQueue( _HiFNonWakeupQueue, pHifPacket );
        ASF_assert(status == OSP_STATUS_OK);
    } else {
        /* Free the packet */
        D0_printf("Packetization error [%d] for sensor %d\r\n", pHifPacket->Header.Length, sensorType);
        status = FreeBlock( SensorDataPacketPool, pHifPacket );
        ASF_assert( status == OSP_STATUS_OK );
    }
}

extern volatile int i2cdoneflag;

/*------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*------------------------------------------------------------------------*/
uint8_t cmdlog[512];
int    cmdidx = 0;
uint8_t process_command(uint8_t *rx_buf, uint16_t length)
{
    uint8_t remain  = 0;
    SensorPacketTypes_t Out;
    uint32_t        pack_sz;

    if (length < 1) return 0;
    cmdlog[cmdidx] = rx_buf[0];
    cmdidx++;
    cmdidx %= 512;

    switch (rx_buf[0]) {

    case OSP_DATA_LEN_L:
        CHostif_StartTxChained((uint8_t *)&(SlaveRegMap.read_len2), sizeof(SlaveRegMap.read_len2),
                    (uint8_t *)SlaveRegMap.rd_mem, SlaveRegMap.read_len2, __LINE__);

        break;
    case OSP_DATA_LEN_H:
        CHostif_StartTxChained((uint8_t *)&(SlaveRegMap.read_len2)+1, sizeof(SlaveRegMap.read_len2)-1,
                    (uint8_t *)SlaveRegMap.rd_mem, SlaveRegMap.read_len2, __LINE__);
        break;

    case OSP_INT_LEN:
        if (QueueOverFlow) {
            SlaveRegMap.intlen = OSP_INT_OVER;
        } else {
            SlaveRegMap.intlen = OSP_INT_NONE;
        }
        pack_sz = sizeof(SlaveRegMap.rd_mem);

        /* Note: This logic does not handle situation where a buffer in not fully emptied in previous
           read attempt */
        DeQueueToBuffer( SlaveRegMap.rd_mem, &pack_sz, _HiFNonWakeupQueue );
        if (pack_sz > 0) {
            SlaveRegMap.intlen |= OSP_INT_DRDY;
            SlaveRegMap.intlen |= (pack_sz << 4); /* pack_sz is reset in DeQueueToBuffer() call */
        }

        SlaveRegMap.read_len2 = pack_sz;

        if (pack_sz == 0) {
            Hostif_StartTx((uint8_t *)&(SlaveRegMap.intlen), sizeof(SlaveRegMap.intlen), __LINE__);
        } else {

            CHostif_StartTxChained((uint8_t *)&(SlaveRegMap.intlen), sizeof(SlaveRegMap.intlen),
                (uint8_t *)SlaveRegMap.rd_mem, pack_sz, __LINE__);
        }
        break;

    case OSP_INT_REASON:
        if (QueueOverFlow) {
            SlaveRegMap.irq_cause = OSP_INT_OVER;
        } else {
            SlaveRegMap.irq_cause = OSP_INT_NONE;
        }
        pack_sz = sizeof(SlaveRegMap.rd_mem);

        D0_printf("OSP_INT_REASON\r\n");
        /* Note: This logic does not handle situation where a buffer in not fully emptied in previous
           read attempt */
        DeQueueToBuffer( SlaveRegMap.rd_mem, &pack_sz, _HiFNonWakeupQueue );
        if (pack_sz > 0) {
            SlaveRegMap.irq_cause |= OSP_INT_DRDY;
        }
        SlaveRegMap.read_len = pack_sz; /* pack_sz is reset in DeQueueToBuffer() call */
        SlaveRegMap.read_len2 = pack_sz;
        Hostif_StartTx((uint8_t *)&(SlaveRegMap.irq_cause), sizeof(SlaveRegMap.irq_cause), __LINE__);

        break;

    case OSP_WHOAMI:        /* Who am */
        Hostif_StartTx((uint8_t *)(&SlaveRegMap.whoami), sizeof(SlaveRegMap.whoami), __LINE__);
        break;

    case OSP_VERSION0:
        Hostif_StartTx((uint8_t *)(&SlaveRegMap.version0), sizeof(SlaveRegMap.version0), __LINE__);
        break;

    case OSP_VERSION1:
        Hostif_StartTx((uint8_t *)(&SlaveRegMap.version1), sizeof(SlaveRegMap.version1), __LINE__);
        break;

    case OSP_CONFIG:        /* Reset, not implemented yet */
        /* Need to flush queue somewhere */
        QueueOverFlow = 0;
        break;

    case OSP_DATA_OUT:        /* Read data */
        //D0_printf("OSP_DATA_OUT\r\n");
        if (SlaveRegMap.read_len2 == 0)
            break;
        if (SlaveRegMap.read_len2 < 3 && SlaveRegMap.read_len2 > 0) {
            //__ASM volatile("BKPT #01");
        }
        break;

    case OSP_DATA_IN:        /* Write data */
        /* Host has written a packet */
        ParseHostInterfacePkt(&Out, &rx_buf[1], length-1);
        break;

    default:
        if (rx_buf[0] >= 0x20 && rx_buf[0] < 0x50) {
            /* ENABLE */
            SensorEnable((ASensorType_t)(rx_buf[0]-0x20));
            D0_printf("Enable %i\r\n", rx_buf[0] - 0x20);
        } else if (rx_buf[0] >= 0x50 && rx_buf[0] < 0x80) {
            /* DISABLE */
            SensorDisable((ASensorType_t)(rx_buf[0]-0x50));
            D0_printf("Disable %i\r\n", rx_buf[0] - 0x50);
        }
        break;
    }
    return remain;
}

/**********************************************************************
 * @fn      I2CCommTask
 *          This tasks primary goal is to serialize the communication
 *        request (sensor results) going over I2C
 *
 * @param   none
 *
 * @return  none
 *
 **********************************************************************/
ASF_TASK void I2CCommTask(ASF_TASK_ARG)
{
    MessageBuffer *rcvMsg = NULLP;
    int16_t err;

    /* Initialize packet queue */
    err = QInitialize();
    ASF_assert(err == OSP_STATUS_OK);

    Hostif_Init();
    /* Active high int, set to in active */
    SensorHubDeAssertInt();
    /* Init register area for slave */
    SH_Slave_init();

    SensorState[0] = 0;
    SensorState[1] = 0;
    SensorState[0] = 1 << SENSOR_SIGNIFICANT_MOTION;

    D0_printf("%s-> I2C Slave ready\r\n", __FUNCTION__);

    while(1) {
        ASFReceiveMessage(I2CSLAVE_COMM_TASK_ID, &rcvMsg );

        switch (rcvMsg->msgId) {

        case MSG_SENSOR_ENABLE:
            SensorEnable((ASensorType_t)rcvMsg->msg.msgSensEnable.U.dword);
            break;

        case MSG_SENSOR_DISABLE:
            SensorDisable((ASensorType_t)rcvMsg->msg.msgSensDisable.U.dword);
            break;

        case MSG_ACC_DATA:
            SendSensorData(AP_PSENSOR_ACCELEROMETER_UNCALIBRATED,
                    &rcvMsg->msg.msgAccelData);
            break;
        case MSG_MAG_DATA:
            SendSensorData(SENSOR_MAGNETIC_FIELD_UNCALIBRATED,
                    &rcvMsg->msg.msgMagData);
            break;
        case MSG_GYRO_DATA:
            SendSensorData(SENSOR_GYROSCOPE_UNCALIBRATED,
                    &rcvMsg->msg.msgGyroData);
            break;
        case MSG_CAL_ACC_DATA:
            SendSensorData(SENSOR_ACCELEROMETER,
                    &rcvMsg->msg.msgAccelData);
            break;
        case MSG_CAL_MAG_DATA:
            SendSensorData(SENSOR_MAGNETIC_FIELD,
                    &rcvMsg->msg.msgMagData);
            break;
        case MSG_CAL_GYRO_DATA:
            SendSensorData(SENSOR_GYROSCOPE,
                    &rcvMsg->msg.msgGyroData);
            break;
        case MSG_ORIENTATION_DATA:
            SendSensorData(SENSOR_ORIENTATION,
                    &rcvMsg->msg.msgOrientationData);
            break;
        case MSG_QUATERNION_DATA:
            SendSensorData(SENSOR_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;
        case MSG_GEO_QUATERNION_DATA:
            SendSensorData(SENSOR_GEOMAGNETIC_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;
        case MSG_GAME_QUATERNION_DATA:
            SendSensorData(SENSOR_GAME_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;

        case MSG_STEP_COUNT_DATA:
            SendSensorData(SENSOR_STEP_COUNTER,
                    &rcvMsg->msg.msgStepCountData);
            break;
        case MSG_CD_SEGMENT_DATA:
            break;
        case MSG_LINEAR_ACCELERATION_DATA:
            SendSensorData(SENSOR_LINEAR_ACCELERATION,
                    &rcvMsg->msg.msgLinearAccelerationData);
            break;
        case MSG_GRAVITY_DATA:
            SendSensorData(SENSOR_GRAVITY,
                    &rcvMsg->msg.msgGravityData);
            break;
        case MSG_STEP_DETECT_DATA:
            SendSensorBoolData(SENSOR_STEP_DETECTOR,
                    &rcvMsg->msg.msgStepDetData);
            break;
        case MSG_SIG_MOTION_DATA:
            SendSensorBoolData(SENSOR_SIGNIFICANT_MOTION,
                    &rcvMsg->msg.msgSigMotionData);
            D0_printf("SigMotion\n");
            break;
        case MSG_PRESS_DATA:
            SendSensorData(SENSOR_PRESSURE,
                    &rcvMsg->msg.msgPressData);
            break;
        default:
            D1_printf("I2C:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}


/****************************************************************************************************
 * @fn      SendSensorEnableReq
 *          Sends message to the communication task to handle enable request received
 *
 * @param   [IN]sensor - Android sensor enum
 *
 * @return  none
 *
 ***************************************************************************************************/
void SendSensorEnableReq( ASensorType_t sensor )
{
    MessageBuffer *pSendMsg = NULLP;
    ASF_assert( ASFCreateMessage( MSG_SENSOR_ENABLE, sizeof(MsgGeneric), &pSendMsg ) == ASF_OK );
    pSendMsg->msg.msgSensEnable.U.dword = sensor;
    ASF_assert( ASFSendMessage( I2CSLAVE_COMM_TASK_ID, pSendMsg ) == ASF_OK );
}


/****************************************************************************************************
 * @fn      SendSensorDisableReq
 *          Sends message to the communication task to handle disable request received
 *
 * @param   [IN]sensor - Android sensor enum
 *
 * @return  none
 *
 ***************************************************************************************************/
void SendSensorDisableReq( ASensorType_t sensor )
{
    MessageBuffer *pSendMsg = NULLP;
    ASF_assert( ASFCreateMessage( MSG_SENSOR_DISABLE, sizeof(MsgGeneric), &pSendMsg ) == ASF_OK );
    pSendMsg->msg.msgSensDisable.U.dword = sensor;
    ASF_assert( ASFSendMessage( I2CSLAVE_COMM_TASK_ID, pSendMsg ) == ASF_OK );
}


#endif //ANDROID_COMM_TASK

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
