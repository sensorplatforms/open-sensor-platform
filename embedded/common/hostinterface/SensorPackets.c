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
#include "SensorPackets.h"
#include "osp-types.h"
#include "osp-sensors.h"
/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void SendSensorEnableReq( ASensorType_t sensor );
void SendSensorDisableReq( ASensorType_t sensor );

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* if defined (MQ_UNIT_TEST) && defined (TEST_SENSOR_DATA_PKT)  */
#if 1
/****************************************************************************************************
 * @fn      ParseSensorDataPkt
 *          Top level parser for Sensor Data Packets.
 *
 * @param   [OUT]pOut - Sensor structure that will return the parsed values
 * @param   [IN]pPacket - Packet buffer containing the packet to parse
 * @param   [IN]pktSize - Size of the packet buffer provided
 *
 * @return  OSP_STATUS_OK or Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
static int16_t ParseSensorDataPkt( SensorPacketTypes_t *pOut, uint8_t *pPacket, uint16_t pktSize )
{
    HostIFPackets_t *pHif = (HostIFPackets_t*)pPacket;
    int16_t errCode = OSP_STATUS_UNSUPPORTED_FEATURE;
    uint8_t sensType;
    uint8_t sensSubType, dSize, dFormat, timeFormat, tSize, isPrivateType, hasMetaData;
    uint8_t i;
    int16_t lengthParsed;

    /* Sanity... */
    if ((pOut == NULL) || (pPacket == NULL))
        return (OSP_STATUS_NULL_POINTER);

    /* Get sensor type. */
    sensType = M_SensorType(pHif->SensPktRaw.Q.SensorIdByte);
    sensSubType = M_ParseSensorSubType(pHif->SensPktRaw.Q.AttributeByte);
    isPrivateType = pHif->SensPktRaw.Q.ControlByte & SENSOR_ANDROID_TYPE_MASK;
    hasMetaData = M_ParseSensorMetaData (pHif->SensPktRaw.Q.SensorIdByte);
    dSize = pHif->SensPktRaw.Q.AttributeByte & DATA_SIZE_MASK;
    dFormat = pHif->SensPktRaw.Q.ControlByte & DATA_FORMAT_MASK;
    timeFormat = pHif->SensPktRaw.Q.ControlByte & TIME_FORMAT_MASK;
    tSize = pHif->SensPktRaw.Q.AttributeByte & TIME_STAMP_SIZE_MASK;

    /* Check Sensor enumeration type Android or Private */
    if (!isPrivateType)
    {
        /*Sensor Enumeration type is Android*/
        switch ((ASensorType_t)sensType)
        {
        case SENSOR_ACCELEROMETER:
        case SENSOR_MAGNETIC_FIELD:
        case SENSOR_GYROSCOPE:
            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract sensor data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.CalFixP.Axis[0] = BYTES_TO_LONG(pHif->CalPktFixP.Data[0], pHif->CalPktFixP.Data[1],
                    pHif->CalPktFixP.Data[2], pHif->CalPktFixP.Data[3]);
                pOut->P.CalFixP.Axis[1] = BYTES_TO_LONG(pHif->CalPktFixP.Data[4], pHif->CalPktFixP.Data[5],
                    pHif->CalPktFixP.Data[6], pHif->CalPktFixP.Data[7]);
                pOut->P.CalFixP.Axis[2] = BYTES_TO_LONG(pHif->CalPktFixP.Data[8], pHif->CalPktFixP.Data[9],
                    pHif->CalPktFixP.Data[10], pHif->CalPktFixP.Data[11]);

                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.CalFixP.TimeStamp.TS8[i] = pHif->CalPktFixP.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = CALIBRATED_FIXP_DATA_PKT_SZ;
            }
            break;

        case SENSOR_ROTATION_VECTOR:
        case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
        case SENSOR_GAME_ROTATION_VECTOR:
            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract Quaternion data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.QuatFixP.Quat[0] = BYTES_TO_LONG(pHif->QuatPktFixP.Data[0], pHif->QuatPktFixP.Data[1],
                    pHif->QuatPktFixP.Data[2], pHif->QuatPktFixP.Data[3]);
                pOut->P.QuatFixP.Quat[1] = BYTES_TO_LONG(pHif->QuatPktFixP.Data[4], pHif->QuatPktFixP.Data[5],
                    pHif->QuatPktFixP.Data[6], pHif->QuatPktFixP.Data[7]);
                pOut->P.QuatFixP.Quat[2] = BYTES_TO_LONG(pHif->QuatPktFixP.Data[8], pHif->QuatPktFixP.Data[9],
                    pHif->QuatPktFixP.Data[10], pHif->QuatPktFixP.Data[11]);
                pOut->P.QuatFixP.Quat[3] = BYTES_TO_LONG(pHif->QuatPktFixP.Data[12], pHif->QuatPktFixP.Data[13],
                    pHif->QuatPktFixP.Data[14], pHif->QuatPktFixP.Data[15]);

                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.QuatFixP.TimeStamp.TS8[i] = pHif->QuatPktFixP.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = QUATERNION_FIXP_DATA_PKT_SZ;
            }
            break;

        case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        case SENSOR_GYROSCOPE_UNCALIBRATED:
            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract Quaternion data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.UncalFixP.Axis[0] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[0], pHif->UncalPktFixP.Data[1],
                    pHif->UncalPktFixP.Data[2], pHif->UncalPktFixP.Data[3]);
                pOut->P.UncalFixP.Axis[1] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[4], pHif->UncalPktFixP.Data[5],
                    pHif->UncalPktFixP.Data[6], pHif->UncalPktFixP.Data[7]);
                pOut->P.UncalFixP.Axis[2] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[8], pHif->UncalPktFixP.Data[9],
                    pHif->UncalPktFixP.Data[10], pHif->UncalPktFixP.Data[11]);

                /* Check if META_DATA is set to 0x01 then read offset */
                if (hasMetaData)
                {
                    pOut->P.UncalFixP.Offset[0] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[0], pHif->UncalPktFixP.Offset[1],
                        pHif->UncalPktFixP.Offset[2], pHif->UncalPktFixP.Offset[3]);
                    pOut->P.UncalFixP.Offset[1] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[4], pHif->UncalPktFixP.Offset[5],
                        pHif->UncalPktFixP.Offset[6], pHif->UncalPktFixP.Offset[7]);
                    pOut->P.UncalFixP.Offset[2] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[8], pHif->UncalPktFixP.Offset[9],
                        pHif->UncalPktFixP.Offset[10], pHif->UncalPktFixP.Offset[11]);

                    lengthParsed = UNCALIB_FIXP_DATA_OFFSET_PKT_SZ;
                }
                else
                {
                    lengthParsed = UNCALIB_FIXP_DATA_PKT_SZ;
                }
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.UncalFixP.TimeStamp.TS8[i] = pHif->UncalPktFixP.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
            }
            break;

        case SENSOR_SIGNIFICANT_MOTION:
            if ((dSize == DATA_SIZE_8_BIT) && (dFormat == DATA_FORMAT_RAW) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract SignificantMotion data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.SigMotion.MotionDetected = pHif->SignificantMotion.significantMotionDetected;
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.SigMotion.TimeStamp.TS8[i] = pHif->SignificantMotion.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = SIGNIFICANTMOTION_FIXP_DATA_PKT_SZ;
            }
            break;

        case SENSOR_STEP_DETECTOR:
            if ((dSize == DATA_SIZE_8_BIT) && (dFormat == DATA_FORMAT_RAW) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract StepDetector data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.StepDetector.StepDetected = pHif->StepDetector.stepDetected;

                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.StepDetector.TimeStamp.TS8[i] = pHif->StepDetector.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = STEPDETECTOR_DATA_PKT_SZ;
            }
            break;

        case SENSOR_STEP_COUNTER:
            if ((dSize == DATA_SIZE_64_BIT) && (dFormat == DATA_FORMAT_RAW) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract StepCounter data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.StepCount.NumStepsTotal = BYTES_TO_LONGLONG(pHif->StepCounter.NumStepsTotal[0],
                        pHif->StepCounter.NumStepsTotal[1], pHif->StepCounter.NumStepsTotal[2],
                        pHif->StepCounter.NumStepsTotal[3], pHif->StepCounter.NumStepsTotal[4],
                        pHif->StepCounter.NumStepsTotal[5], pHif->StepCounter.NumStepsTotal[6],
                        pHif->StepCounter.NumStepsTotal[7]);
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.StepCount.TimeStamp.TS8[i] = pHif->StepCounter.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = STEPCOUNTER_DATA_PKT_SZ;
            }
            break;

        case SENSOR_ORIENTATION:
            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract OrientationFixP data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.OrientFixP.Pitch = BYTES_TO_LONG(pHif->OrientationFixP.Pitch[0], pHif->OrientationFixP.Pitch[1],
                        pHif->OrientationFixP.Pitch[2], pHif->OrientationFixP.Pitch[3]);
                pOut->P.OrientFixP.Roll = BYTES_TO_LONG(pHif->OrientationFixP.Roll[0], pHif->OrientationFixP.Roll[1],
                        pHif->OrientationFixP.Roll[2], pHif->OrientationFixP.Roll[3]);
                pOut->P.OrientFixP.Yaw = BYTES_TO_LONG(pHif->OrientationFixP.Yaw[0], pHif->OrientationFixP.Yaw[1],
                        pHif->OrientationFixP.Yaw[2], pHif->OrientationFixP.Yaw[3]);
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.OrientFixP.TimeStamp.TS8[i] = pHif->OrientationFixP.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = ORIENTATION_FIXP_DATA_PKT_SZ;
            }
            break;

        case SENSOR_LINEAR_ACCELERATION:
        case SENSOR_GRAVITY:
            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract ThreeAxisFixp data from packet */
                pOut->SType = (ASensorType_t)sensType;
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.ThreeAxisFixP.Axis[0] = BYTES_TO_LONG(pHif->ThreeAxisFixp.Data[0], pHif->ThreeAxisFixp.Data[1],
                        pHif->ThreeAxisFixp.Data[2], pHif->ThreeAxisFixp.Data[3]);

                pOut->P.ThreeAxisFixP.Axis[1] = BYTES_TO_LONG(pHif->ThreeAxisFixp.Data[4], pHif->ThreeAxisFixp.Data[5],
                        pHif->ThreeAxisFixp.Data[6], pHif->ThreeAxisFixp.Data[7]);

                pOut->P.ThreeAxisFixP.Axis[2] = BYTES_TO_LONG(pHif->ThreeAxisFixp.Data[8], pHif->ThreeAxisFixp.Data[9],
                        pHif->ThreeAxisFixp.Data[10], pHif->ThreeAxisFixp.Data[11]);
                //TODO: How to handle the Accuracy here ? Leave it in the Metadeta ?
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.ThreeAxisFixP.TimeStamp.TS8[i] = pHif->ThreeAxisFixp.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
                lengthParsed = THREEAXIS_FIXP_DATA_PKT_SZ;
            }
            break;

        default:
            break;
        }

    }
    else
    {
        /*Sensor Enumeration type is Private*/
        switch ((ASensorType_t)sensType)
        {
        case PSENSOR_ACCELEROMETER_RAW:
        case PSENSOR_MAGNETIC_FIELD_RAW:
        case PSENSOR_GYROSCOPE_RAW:
            if ((sensSubType == SENSOR_SUBTYPE_UNUSED) && (dSize == DATA_SIZE_16_BIT) &&
                (dFormat == DATA_FORMAT_RAW) && (timeFormat == TIME_FORMAT_RAW))
            {
                /* Extract Raw sensor data from packet */
                pOut->SType = (ASensorType_t)M_PSensorToAndroidBase(sensType);
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.RawSensor.Axis[0] = BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[0], pHif->SensPktRaw.DataRaw[1]);
                pOut->P.RawSensor.Axis[1] = BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[2], pHif->SensPktRaw.DataRaw[3]);
                pOut->P.RawSensor.Axis[2] = BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[4], pHif->SensPktRaw.DataRaw[5]);
#if 0
                if (g_logging & 0x10)
                {
                    D1_printf("HIF: %d, %02X, %02X, %04X\r\n", sizeof(HostIFPackets_t),
                        pHif->SensPktRaw.DataRaw[0], pHif->SensPktRaw.DataRaw[1], pOut->P.RawSensor.Axis[0]);
                }
#endif
                /* Extract time stamp */
                pOut->P.RawSensor.TStamp.TS64 = 0; //helps clear higher 32-bit
                pOut->P.RawSensor.TStamp.TS8[3] = pHif->SensPktRaw.TimeStamp[0]; //MSB
                pOut->P.RawSensor.TStamp.TS8[2] = pHif->SensPktRaw.TimeStamp[1];
                pOut->P.RawSensor.TStamp.TS8[1] = pHif->SensPktRaw.TimeStamp[2];
                pOut->P.RawSensor.TStamp.TS8[0] = pHif->SensPktRaw.TimeStamp[3]; //LSB
                //TODO: 64-bit time stamp extension??
                errCode = OSP_STATUS_OK;
                lengthParsed = SENSOR_RAW_DATA_PKT_SZ;
            }
            break;

        case PSENSOR_ACCELEROMETER_UNCALIBRATED:

            if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_FIXPOINT) &&
                (timeFormat == TIME_FORMAT_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
            {
                /* Extract uncalibrated sensor data from packet */
                pOut->SType = (ASensorType_t)M_PSensorToAndroidBase(sensType);
                pOut->SubType = SENSOR_SUBTYPE_UNUSED;
                pOut->P.UncalFixP.Axis[0] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[0], pHif->UncalPktFixP.Data[1],
                    pHif->UncalPktFixP.Data[2], pHif->UncalPktFixP.Data[3]);
                pOut->P.UncalFixP.Axis[1] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[4], pHif->UncalPktFixP.Data[5],
                    pHif->UncalPktFixP.Data[6], pHif->UncalPktFixP.Data[7]);
                pOut->P.UncalFixP.Axis[2] = BYTES_TO_LONG(pHif->UncalPktFixP.Data[8], pHif->UncalPktFixP.Data[9],
                    pHif->UncalPktFixP.Data[10], pHif->UncalPktFixP.Data[11]);

                /* Check if META_DATA is set to 0x01 then read offset */
                if (hasMetaData)
                {
                    pOut->P.UncalFixP.Offset[0] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[0], pHif->UncalPktFixP.Offset[1],
                        pHif->UncalPktFixP.Offset[2], pHif->UncalPktFixP.Offset[3]);
                    pOut->P.UncalFixP.Offset[1] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[4], pHif->UncalPktFixP.Offset[5],
                        pHif->UncalPktFixP.Offset[6], pHif->UncalPktFixP.Offset[7]);
                    pOut->P.UncalFixP.Offset[2] = BYTES_TO_LONG(pHif->UncalPktFixP.Offset[8], pHif->UncalPktFixP.Offset[9],
                        pHif->UncalPktFixP.Offset[10], pHif->UncalPktFixP.Offset[11]);

                    lengthParsed = UNCALIB_FIXP_DATA_OFFSET_PKT_SZ;
                }
                else
                {
                    lengthParsed = UNCALIB_FIXP_DATA_PKT_SZ;
                }
                /* Extract fixed point time stamp */
                for (i = 0; i < sizeof(uint64_t); i++)
                {
                    /* Copy LSB to MSB data - remember that HIF packets are Big-Endian formatted */
                    pOut->P.UncalFixP.TimeStamp.TS8[i] = pHif->UncalPktFixP.TimeStamp[sizeof(uint64_t)-i-1];
                }
                errCode = OSP_STATUS_OK;
            }
            break;

        case SYSTEM_REAL_TIME_CLOCK:
            break;

        default:
            break;
        }
    }
    if (errCode == OSP_STATUS_OK)
        return lengthParsed;
    else
        return errCode;
}
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      FormatSensorDataPktRaw
 *          Using the sensor sample data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  SENSOR_DATA_PKT_RAW_SZ in length
 * @param   [IN]pSensData - Data structure carrying the raw sensor sample (typically from driver)
 * @param   [IN]sensorInstance - 0 base instance identifier if multiple sensor of same type is used
 * @param   [IN]metaData - Meta data for the sensor type used (not applicable for all sensor types)
 * @param   [IN]sType - Sensor Type for the sensor data presented
 * @param   [IN]subType - Sub type for the sensor type used (not applicable for all sensor types)
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatSensorDataPktRaw( uint8_t *pDest, const TriAxisRawData_t *pSensData, uint8_t metaData,
    ASensorType_t sType, uint8_t subType )
{
    uint8_t i = 0;
    int8_t k;
    HifSensorDataRaw_t *pOut = (HifSensorDataRaw_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;

    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;

    }
    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_RAW | TIME_FORMAT_RAW;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorMetaData(metaData) | M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = M_SensorSubType(subType) | DATA_SIZE_16_BIT | TIME_STAMP_32_BIT;

    /* Time Stamp: We pass only the lower 32-bit in this packet. Note that byte ordering is Big Endian */
    for (k = sizeof(uint32_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pSensData->TStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    for (k = 0; k < 3; k++)
    {
        pOut->DataRaw[i++] = BYTE1(pSensData->Axis[k]);  //MSB of 16-bit sensor data
        pOut->DataRaw[i++] = BYTE0(pSensData->Axis[k]);  //LSB of 16-bit sensor data
    }

    /* No Checksum... we are done! */
    return SENSOR_RAW_DATA_PKT_SZ;
}


/****************************************************************************************************
 * @fn      FormatQuaternionPktFixP
 *          Using the given quaternion data, this function creates the HIF Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  QUATERNION_PKT_FIXP_SZ in length
 * @param   [IN]pQuatData - Data structure carrying the quaternion result from fusion
 * @param   [IN]sType - Android sensor type for the quaternion data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatQuaternionPktFixP( uint8_t *pDest, const QuaternionFixP_t *pQuatData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifQuaternionFixPoint_t *pOut = (HifQuaternionFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }
    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;

    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;

    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_FIXPOINT | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte =  M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_32_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pQuatData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    for (k = 0; k < 4; k++)
    {
        pOut->Data[i++] = BYTE3(pQuatData->Quat[k]);  //MSB of 32-bit data
        pOut->Data[i++] = BYTE2(pQuatData->Quat[k]);
        pOut->Data[i++] = BYTE1(pQuatData->Quat[k]);
        pOut->Data[i++] = BYTE0(pQuatData->Quat[k]);  //LSB of 32-bit data
    }

    /* No Checksum... we are done! */
    return QUATERNION_FIXP_DATA_PKT_SZ;
}


/****************************************************************************************************
 * @fn      FormatUncalibratedPktFixP
 *          Using the given uncalibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  UNCALIB_FIXP_DATA_PKT_SZ or UNCALIB_FIXP_DATA_OFFSET_PKT_SZ in length
 * @param   [IN]pUncalData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]metaData - Meta data is used to indicate if the packet contains offset data
 * @param   [IN]sType - Sensor type for the uncalibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatUncalibratedPktFixP( uint8_t *pDest, const UncalibratedFixP_t *pSensData,
    uint8_t metaData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifUncalibratedFixPoint_t *pOut = (HifUncalibratedFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }
    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;

    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;

    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_FIXPOINT | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorMetaData(metaData) | M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_32_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pSensData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    for (k = 0; k < 3; k++)
    {
        pOut->Data[i++] = BYTE3(pSensData->Axis[k]);  //MSB of 32-bit data
        pOut->Data[i++] = BYTE2(pSensData->Axis[k]);
        pOut->Data[i++] = BYTE1(pSensData->Axis[k]);
        pOut->Data[i++] = BYTE0(pSensData->Axis[k]);  //LSB of 32-bit data
    }

    /* Check if metadata is available  if it is available then add it to HIF packet */
    if (metaData == META_DATA_OFFSET_CHANGE)
    {
        /* Data bytes (Big Endian order) */
        for (k = 0; k < 3; k++)
        {
            pOut->Data[i++] = BYTE3(pSensData->Offset[k]);  //MSB of 32-bit data
            pOut->Data[i++] = BYTE2(pSensData->Offset[k]);
            pOut->Data[i++] = BYTE1(pSensData->Offset[k]);
            pOut->Data[i++] = BYTE0(pSensData->Offset[k]);  //LSB of 32-bit data
        }
        return UNCALIB_FIXP_DATA_OFFSET_PKT_SZ;
    }

    /* No Checksum... we are done! */
    return UNCALIB_FIXP_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatOrientationFixP
 *          Using the given sensor data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  ORIENTATION_FIXP_DATA_PKT_SZ in length
 * @param   [IN]pOrientationData - Data structure carrying the sensor data result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatOrientationFixP( uint8_t *pDest, const OrientationFixP_t *pOrientationData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifOrientationPktFixPoint_t *pOut = (HifOrientationPktFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_FIXPOINT | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_32_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pOrientationData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    /*Pitch*/
    i = 0;
    pOut->Pitch[i++] = BYTE3(pOrientationData->Pitch);  //MSB of 32-bit data
    pOut->Pitch[i++] = BYTE2(pOrientationData->Pitch);
    pOut->Pitch[i++] = BYTE1(pOrientationData->Pitch);
    pOut->Pitch[i++] = BYTE0(pOrientationData->Pitch);  //LSB of 32-bit data

    /*Roll*/
    i = 0;
    pOut->Roll[i++] = BYTE3(pOrientationData->Roll);  //MSB of 32-bit data
    pOut->Roll[i++] = BYTE2(pOrientationData->Roll);
    pOut->Roll[i++] = BYTE1(pOrientationData->Roll);
    pOut->Roll[i++] = BYTE0(pOrientationData->Roll);  //LSB of 32-bit data

    /*Yaw*/
    i = 0;
    pOut->Yaw[i++] = BYTE3(pOrientationData->Yaw);  //MSB of 32-bit data
    pOut->Yaw[i++] = BYTE2(pOrientationData->Yaw);
    pOut->Yaw[i++] = BYTE1(pOrientationData->Yaw);
    pOut->Yaw[i++] = BYTE0(pOrientationData->Yaw);  //LSB of 32-bit data

    /* No Checksum... we are done! */
    return ORIENTATION_FIXP_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatSignificantMotionPktFixP
 *          Using the given sensor data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  SIGNIFICANTMOTION_FIXP_DATA_PKT_SZ in length
 * @param   [IN]pSignificantMotionData - Data structure carrying the sensor result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatSignificantMotionPktFixP( uint8_t *pDest, const SignificantMotion_t *pSignificantMotionData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifSignificantMotionPktFixPoint_t *pOut = (HifSignificantMotionPktFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_RAW | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_8_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pSignificantMotionData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    pOut->significantMotionDetected = pSignificantMotionData->MotionDetected;

    /* No Checksum... we are done! */
    return SIGNIFICANTMOTION_FIXP_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatThreeAxisPktFixP
 *          Using the given Calibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  THREEAXIS_FIXP_DATA_PKT_SZ in length
 * @param   [IN]p3AxisData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatThreeAxisPktFixP( uint8_t *pDest, const ThreeAxisFixP_t *p3AxisData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifThreeAxisPktFixPoint_t *pOut = (HifThreeAxisPktFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_FIXPOINT | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType) | M_SensorMetaData(p3AxisData->Accuracy);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_32_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = p3AxisData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    for(k = 0; k < 3; k++ )
    {
        pOut->Data[i++] = BYTE3(p3AxisData->Axis[k]);  //MSB of 32-bit data
        pOut->Data[i++] = BYTE2(p3AxisData->Axis[k]);
        pOut->Data[i++] = BYTE1(p3AxisData->Axis[k]);
        pOut->Data[i++] = BYTE0(p3AxisData->Axis[k]);  //LSB of 32-bit data
    }
    //TODO : Find out when to use the preciseData
    /* No Checksum... we are done! */
    return THREEAXIS_FIXP_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatStepCounterPkt
 *          Using the given data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  STEPCOUNTER_DATA_PKT_SZ in length
 * @param   [IN]pStepCounterData - Data structure carrying the result from Algorithm
 * @param   [IN]sType - Sensor type for the data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatStepCounterPkt( uint8_t *pDest, const StepCounter_t *pStepCounterData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifStepCounter_t *pOut = (HifStepCounter_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_RAW | TIME_FORMAT_FIXPOINT;
    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_64_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pStepCounterData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    pOut->NumStepsTotal[i++] = BYTE7(pStepCounterData->NumStepsTotal);  //MSB of 64-bit data
    pOut->NumStepsTotal[i++] = BYTE6(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE5(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE4(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE3(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE2(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE1(pStepCounterData->NumStepsTotal);
    pOut->NumStepsTotal[i++] = BYTE0(pStepCounterData->NumStepsTotal);  //LSB of 64-bit data

    //TODO : Find out when to use the preciseData
    /* No Checksum... we are done! */
    return STEPCOUNTER_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatStepDetectorPkt
 *          Using the given data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  STEPDETECTOR_DATA_PKT_SZ in length
 * @param   [IN]pStepDetectorData - Data structure carrying the result from Algorithm
 * @param   [IN]sType - Sensor type for the data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatStepDetectorPkt( uint8_t *pDest, const StepDetector_t *pStepDetectorData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifStepDetector_t *pOut = (HifStepDetector_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_RAW | TIME_FORMAT_FIXPOINT;
    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_8_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pStepDetectorData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    pOut->stepDetected = pStepDetectorData->StepDetected;

    /* No Checksum... we are done! */
    return STEPDETECTOR_DATA_PKT_SZ;
}

/****************************************************************************************************
 * @fn      FormatCalibratedPktFixP
 *          Using the given Calibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  CALIBRATED_PKT_FIXP_SZ in length
 * @param   [IN]pCalData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatCalibratedPktFixP( uint8_t *pDest, const CalibratedFixP_t *pSensData, ASensorType_t sType )
{
    uint8_t i = 0;
    int8_t k;
    HifCalibratedFixPoint_t *pOut = (HifCalibratedFixPoint_t *)pDest;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_SENSOR_DATA | DATA_FORMAT_FIXPOINT | TIME_FORMAT_FIXPOINT;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte */
    pOut->Q.AttributeByte = DATA_SIZE_32_BIT | TIME_STAMP_64_BIT;

    /* time stamp - note that byte ordering is Big Endian */
    for (k = sizeof(uint64_t)-1; k >= 0; k--)
    {
        pOut->TimeStamp[i++] = pSensData->TimeStamp.TS8[k]; //MSB to LSB copy
    }

    /* Data bytes (Big Endian order) */
    i = 0;
    for (k = 0; k < 3; k++)
    {
        pOut->Data[i++] = BYTE3(pSensData->Axis[k]);  //MSB of 32-bit data
        pOut->Data[i++] = BYTE2(pSensData->Axis[k]);
        pOut->Data[i++] = BYTE1(pSensData->Axis[k]);
        pOut->Data[i++] = BYTE0(pSensData->Axis[k]);  //LSB of 32-bit data
    }

    /* No Checksum... we are done! */
    return CALIBRATED_FIXP_DATA_PKT_SZ;
}


/****************************************************************************************************
 * @fn      FormatSensorEnableReq
 *          This function creates the Sensor Control Request Packet for enabling/disabling a sensor
 *          type in the buffer provided.  This packet is sent to hub from host.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  4 bytes in length
 * @param   [IN]enable - Boolean type specifying if sensor type must be enabled or disabled.
 * @param   [IN]sType - Sensor type for the calibrated data
 * @param   [IN]subType - Sub type for the sensor type used (not applicable for all sensor types)
 * @param   [IN]seqNum - A unique request number that might be used to distinguish between responses
 *                  Sequence number of 0 implies no response is needed.
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatSensorEnableReq( uint8_t *pDest, osp_bool_t enable, ASensorType_t sType, uint8_t subType,
    uint8_t seqNum )
{
    HifSensorParamBool_t *pOut = (HifSensorParamBool_t*)pDest;
    uint8_t temp = (enable == TRUE)? 1 : 0;

    /* Sanity checks... */
    if (pOut == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(M_GetSensorType(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    pOut->Q.ControlByte |= PKID_CONTROL_REQ_WR;

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte 1 */
    pOut->Q.AttributeByte = M_SensorSubType(subType) | M_SequenceNum(seqNum);

    /* Attribute Byte 2 */
    pOut->AttrByte2 = M_SetParamId(PARAM_ID_ENABLE);

    /* Data byte */
    pOut->Data = temp;

    /* Return the length of the packet */
    return SENSOR_ENABLE_REQ_PKT_SZ;
}


/****************************************************************************************************
 * @fn      FormatControlResponse
 *          Using the given Calibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [IN]sensType - Android sensor types (inc. private sensor types)
 * @param   [IN]seqNum - Sequence number of the corresponding request
 * @param   [IN]paramID - Sensor parameter ID
 * @param   [IN]errCode - MQ error code that was the result of the request execution
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t FormatControlResponse( uint32_t sensType, uint8_t seqNum, uint8_t paramID, int16_t errCode )
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      ParseControlRequestPkt
 *          Top level parser for Sensor Control Request Packets.
 *
 * @param   [OUT]pOut - Sensor structure that will return the parsed values
 * @param   [IN]pPacket - Packet buffer containing the packet to parse
 * @param   [IN]pktSize - Size of the packet buffer provided
 *
 * @return  OSP_STATUS_OK or Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
static int16_t ParseControlRequestPkt( SensorPacketTypes_t *pOut, uint8_t *pPacket, uint16_t pktSize )
{
    HostIFPackets_t *pHif = (HostIFPackets_t*)pPacket;
    int16_t errCode = OSP_STATUS_UNSUPPORTED_FEATURE;
    uint32_t sensType, sensSubType;
    osp_bool_t isPrivateType;
    uint8_t seqNum, paramID;

    /* Get sensor type. */
    sensType = M_SensorType(pHif->Enable.Q.SensorIdByte);
    isPrivateType = (pHif->Enable.Q.ControlByte & SENSOR_ANDROID_TYPE_MASK) ? TRUE : FALSE;
    seqNum = M_SequenceNum(pHif->Enable.Q.AttributeByte);
    sensSubType = M_ParseSensorSubType(pHif->SensPktRaw.Q.AttributeByte);
    paramID = M_GetParamId(pHif->Enable.AttrByte2);

    /* Check Sensor enumeration type Android or Private */
    if (isPrivateType)
    {
        /* Change sensor type to android base*/
        sensType = M_PSensorToAndroidBase(sensType);
    }

    /* Check parameter ID */
    switch (paramID)
    {
    case PARAM_ID_ENABLE:
        if (pHif->Enable.Data)
        {
            SendSensorEnableReq((ASensorType_t)sensType);
        }
        else
        {
            SendSensorDisableReq((ASensorType_t)sensType);
        }
        break;

    case PARAM_ID_BATCH:
        break;

    case PARAM_ID_FLUSH:
        break;

    case PARAM_ID_RANGE_RESOLUTION:
        break;

    case PARAM_ID_POWER:
        break;

    case PARAM_ID_MINMAX_DELAY:
        break;

    case PARAM_ID_FIFO_EVT_CNT:
        break;

    case PARAM_ID_AXIS_MAPPING:
        break;

    case PARAM_ID_CONVERSION_OFFSET:
        break;

    case PARAM_ID_SENSOR_NOISE:
        break;

    case PARAM_ID_TIMESTAMP_OFFSET:
        break;

    case PARAM_ID_ONTIME_WAKETIME:
        break;

    case PARAM_ID_HPF_LPF_CUTOFF:
        break;

    case PARAM_ID_SENSOR_NAME:
        break;

    case PARAM_ID_XYZ_OFFSET:
        break;

    case PARAM_ID_F_SKOR_MATRIX:
        break;

    case PARAM_ID_F_CAL_OFFSET:
        break;

    case PARAM_ID_F_NONLINEAR_EFFECTS:
        break;

    case PARAM_ID_BIAS_STABILITY:
        break;

    case PARAM_ID_REPEATABILITY:
        break;

    case PARAM_ID_TEMP_COEFF:
        break;

    case PARAM_ID_SHAKE_SUSCEPTIBILIY:
        break;

    case PARAM_ID_EXPECTED_NORM:
        break;

    case PARAM_ID_VERISON:
        break;

    default:
        errCode = (OSP_STATUS_INVALID_PARAMETER);
        break;
    }

    return errCode;
}


/****************************************************************************************************
 * @fn      ParseHostInterfacePkt
 *          Top level parser for Host interface Packets. Depending on packet type & data sizes, the
 *          individual parsers are called.
 *
 * @param   [OUT]pOut - Sensor structure that will return the parsed values
 * @param   [IN]pPacket - Packet buffer containing the packet to parse
 * @param   [IN]pktSize - Size of the packet buffer provided
 *
 * @return  OSP_STATUS_OK or Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int16_t ParseHostInterfacePkt( SensorPacketTypes_t *pOut, uint8_t *pPacket, uint16_t pktSize )
{
    uint8_t pktID;
    int16_t errCode = OSP_STATUS_UNSUPPORTED_FEATURE;

    /* Sanity... */
    if (pPacket == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* CRC Check */
    //TODO - we are not using CRC check for now!

    /* ========== Packet handling ========== */
    /* 1. Identify Packet type */
    pktID = pPacket[0] & PKID_MASK_VER0;

    switch (pktID)
    {
    case PKID_SENSOR_DATA:
        /* Invoke packet handler for Sensor Data packet */
        /* Note: This handler should be on host side. Its provided here for testing */
#if defined (MQ_UNIT_TEST) && defined (TEST_SENSOR_DATA_PKT)
        if ((pOut == NULL) || (pPacket == NULL))
        {
            return (OSP_STATUS_NULL_POINTER);
        }
        errCode = ParseSensorDataPkt( pOut, pPacket, pktSize );
#endif
        break;

    case PKID_CONTROL_REQ_RD:
    case PKID_CONTROL_REQ_WR:
        /* Invoke packet handler for Control Request packet */
        errCode = ParseControlRequestPkt( pOut, pPacket, pktSize );
        break;

    case PKID_CONTROL_RESP:
        /* To be implemented on the host side driver */
        break;

    case PKID_TEST_DATA:
        break;

    default:
        //Invalid or newer version packet
        errCode = OSP_STATUS_INVALID_PACKETID;
        break;
    }
    return errCode;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
