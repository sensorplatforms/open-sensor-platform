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
#include "hostif_i2c.h"
#include <string.h>
#include "SensorPackets.h"
#include "BlockMemory.h"
#include "BatchManager.h"
#include "BatchState.h"
#include "ConfigManager.h"

/*-------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------*/

/***************************************************************************
 * @fn      QueueSensorBoolData
 *          Packetizes in HIF format and queue data for host.
 *
 ***************************************************************************/
static void QueueSensorBoolData(ASensorType_t sensorType, MsgSensorBoolData *pMsg)
{
    HostIFPackets_t HiFPacket;
    int16_t packetLength;
    int32_t sensor_state;

    BatchManagerGetSensorState(sensorType, &sensor_state);

    if(sensor_state == DISABLE) {
        return;
    }

    /* Process sensor and format into packet */
    switch (sensorType) {
    case SENSOR_STEP_DETECTOR:
        {
            StepDetector_t stepDetectorData;

            stepDetectorData.TimeStamp.TS64 = pMsg->timeStamp;
            stepDetectorData.StepDetected   = pMsg->active;

            /* Format Packet */
            packetLength = FormatStepDetectorPkt( &HiFPacket, &stepDetectorData, sensorType );
        }
        break;

    case SENSOR_SIGNIFICANT_MOTION:
        {
            SignificantMotion_t motionData;

            motionData.TimeStamp.TS64 = pMsg->timeStamp;
            motionData.MotionDetected = pMsg->active;

            /* Format Packet */
            packetLength = FormatSignificantMotionPktFixP( &HiFPacket, &motionData, sensorType );
        }
        break;

    default:
        D0_printf("Unhandled sensor [%d] in %s!\r\n", sensorType, __FUNCTION__);
        return;
    }

    /* Enqueue packet in HIF queue */
    if (packetLength > 0) {
        packetLength = BatchManagerSensorDataEnQueue( &HiFPacket, packetLength, sensorType );
        ASF_assert(packetLength == OSP_STATUS_OK);
    } else {
        D0_printf("Packetization error [%d] for sensor %d\r\n", packetLength, sensorType);
    }
}


/***************************************************************************
 * @fn      QueueSensorData
 *          Packetize in HIF format and enqueue sensor data with Batch manager
 *
 ***************************************************************************/
static void QueueSensorData(ASensorType_t sensorType, MsgSensorData *pMsg)
{
    HostIFPackets_t HiFPacket;
    int16_t packetLength;
    int32_t sensor_state;

    BatchManagerGetSensorState(sensorType, &sensor_state);

    if(sensor_state == DISABLE) {
        return;
    }

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

            packetLength = FormatUncalibratedPktFixP(&HiFPacket,
                &UnCalFixPData, META_DATA_UNUSED, sensorType);
            ASF_assert(packetLength > 0);
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

            packetLength = FormatCalibratedPktFixP(&HiFPacket,
                &CalFixPData, sensorType);
            ASF_assert(packetLength > 0);
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

            packetLength = FormatQuaternionPktFixP(&HiFPacket,
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

            packetLength = FormatThreeAxisPktFixP(&HiFPacket,
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

            packetLength = FormatOrientationFixP(&HiFPacket,
                &fixPData, sensorType);
        }
        break;

    case SENSOR_STEP_COUNTER:
        {
            StepCounter_t stepCountData;

            stepCountData.TimeStamp.TS64 = pMsg->timeStamp;
            stepCountData.NumStepsTotal  = pMsg->X;

            /* Format Packet */
            packetLength = FormatStepCounterPkt( &HiFPacket, &stepCountData, sensorType );
        }
        break;

    case SENSOR_PRESSURE:
        //TODO - Ignore for now!
        break;

    default:
        D0_printf("Unhandled sensor [%d] in %s!\r\n", sensorType, __FUNCTION__);
        return;
    }

    /* Enqueue packet in HIF queue */
    if (packetLength > 0) {
        packetLength = BatchManagerSensorDataEnQueue( &HiFPacket, packetLength, sensorType );
        ASF_assert(packetLength == OSP_STATUS_OK);
    } else {
        D0_printf("Packetization error [%d] for sensor %d\r\n", packetLength, sensorType);
    }
}

/*------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*------------------------------------------------------------------------*/

/**********************************************************************
 * @fn      HostIntfTask
 *          This tasks primary goal is to serialize the communication
 *          request (sensor results)
 *
 * @param   none
 *
 * @return  none
 *
 **********************************************************************/
ASF_TASK void HostIntfTask(ASF_TASK_ARG)
{
    MessageBuffer *rcvMsg = NULLP;
    int16_t err;

    /* Initialize Batching Module */
    err = BatchManagerInitialize();
    ASF_assert(err == OSP_STATUS_OK);

    /* Initialize Batching State Machine */
    err = BatchStateInitialize();
    ASF_assert(err == OSP_STATUS_OK);

    /* Initialize I2C host interface */
    Hostif_I2C_Init();

    /****************************
    * TODO: Do Hardware init here
    *****************************/
    Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, HOSTIF_IRQ_PORT, HOSTIF_IRQ_PIN);

    /* Active high int, set to in active */
    SensorHubDeAssertInt();

    D0_printf("%s-> Host Interface ready\r\n", __FUNCTION__);

    while(1) {
        ASFReceiveMessage(HOST_INTF_TASK_ID, &rcvMsg );

        switch (rcvMsg->msgId) {

        case MSG_ACC_DATA:
            QueueSensorData(AP_PSENSOR_ACCELEROMETER_UNCALIBRATED,
                    &rcvMsg->msg.msgAccelData);
            break;

        case MSG_MAG_DATA:
            QueueSensorData(SENSOR_MAGNETIC_FIELD_UNCALIBRATED,
                    &rcvMsg->msg.msgMagData);
            break;

        case MSG_GYRO_DATA:
            QueueSensorData(SENSOR_GYROSCOPE_UNCALIBRATED,
                    &rcvMsg->msg.msgGyroData);
            break;

        case MSG_CAL_ACC_DATA:
            QueueSensorData(SENSOR_ACCELEROMETER,
                    &rcvMsg->msg.msgAccelData);
            break;

        case MSG_CAL_MAG_DATA:
            QueueSensorData(SENSOR_MAGNETIC_FIELD,
                    &rcvMsg->msg.msgMagData);
            break;

        case MSG_CAL_GYRO_DATA:
            QueueSensorData(SENSOR_GYROSCOPE,
                    &rcvMsg->msg.msgGyroData);
            break;

        case MSG_ORIENTATION_DATA:
            QueueSensorData(SENSOR_ORIENTATION,
                    &rcvMsg->msg.msgOrientationData);
            break;

        case MSG_QUATERNION_DATA:
            QueueSensorData(SENSOR_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;

        case MSG_GEO_QUATERNION_DATA:
            QueueSensorData(SENSOR_GEOMAGNETIC_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;

        case MSG_GAME_QUATERNION_DATA:
            QueueSensorData(SENSOR_GAME_ROTATION_VECTOR,
                    &rcvMsg->msg.msgQuaternionData);
            break;

        case MSG_STEP_COUNT_DATA:
            QueueSensorData(SENSOR_STEP_COUNTER,
                    &rcvMsg->msg.msgStepCountData);
            break;

        case MSG_CD_SEGMENT_DATA:
            break;

        case MSG_LINEAR_ACCELERATION_DATA:
            QueueSensorData(SENSOR_LINEAR_ACCELERATION,
                    &rcvMsg->msg.msgLinearAccelerationData);
            break;

        case MSG_GRAVITY_DATA:
            QueueSensorData(SENSOR_GRAVITY,
                    &rcvMsg->msg.msgGravityData);
            break;

        case MSG_STEP_DETECT_DATA:
            QueueSensorBoolData(SENSOR_STEP_DETECTOR,
                    &rcvMsg->msg.msgStepDetData);
            break;

        case MSG_SIG_MOTION_DATA:
            QueueSensorBoolData(SENSOR_SIGNIFICANT_MOTION,
                    &rcvMsg->msg.msgSigMotionData);
            D0_printf("SigMotion\n");
            break;

        case MSG_PRESS_DATA:
            QueueSensorData(SENSOR_PRESSURE,
                    &rcvMsg->msg.msgPressData);
            break;

        case MSG_PROCESS_CTRL_REQ:
            SHConfigManager_ProcessControlRequest(rcvMsg->msg.msgCtrlReq.pRequestPacket,
                    rcvMsg->msg.msgCtrlReq.length);
            break;

        default:
            D1_printf("!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}

#endif //ANDROID_COMM_TASK

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
