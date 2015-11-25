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
#include "common.h"
#include "SensorPackets.h"

#define SENSOR_PACKETS_PARSE_C
#include "SensorPackets_Internal.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef _DEBUG_SENSOR_HUB_TARGET
extern uint32_t g_logging;
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define MY_FID  FID_SENSOR_PACKETS_PARSE_C

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static int32_t CheckPacketSanity( const uint8_t *pPacket );
static void ParsePacketHeader( LocalPacketTypes_t *pOut, const uint8_t *pPacket, uint8_t isSensorDataPacket );
static int32_t ParseSensorDataPacket( LocalPacketTypes_t *pOut,  const uint8_t *pSrc, uint16_t *pPktSizeByType, uint16_t pktBufferSize );
static int32_t ParseControlPacket(    LocalPacketTypes_t *pDest, const uint8_t *pSrc, uint16_t *pPktSizeByType, uint16_t pktBufferSize );

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      CheckPacketSanity
 *          Initial correctness checks for Host Interface Packet.
 *
 *          Returns OSP_STATUS_OK if sanity checks pass, otherwise returns negative error code.
 ***************************************************************************************************/
static int32_t CheckPacketSanity( const uint8_t *pPacket )
{
    /* Buffer check */
    if (pPacket == NULL)
    {
        return SET_ERROR( OSP_STATUS_NULL_POINTER );
    }

    /* Version check */
    if (!(PACKET_VERSION_1_SUPPORTED) && GetPacketVersion(pPacket))
    {
        return SET_ERROR( OSP_STATUS_UNSUPPORTED_FEATURE );
    }

    return SET_ERROR( OSP_STATUS_OK );
}


/****************************************************************************************************
 * @fn      ParsePacketHeader
 *          Expands bit-packed fields in Host Interface Packet to integral fields in Local Packet.
 ***************************************************************************************************/
static void ParsePacketHeader( LocalPacketTypes_t *pOut, const uint8_t *pSrc, uint8_t isSensorDataPacket )
{
    const uint8_t isPrivateNotAndroid   = GetAndroidOrPrivateField( pSrc );

    /* Copy header fields. Note that we already have Packet ID */
    pOut->Metadata = GetMetadata( pSrc );
    pOut->SType    = (ASensorType_t) GetSensorTypeField( pSrc );
    pOut->SubType  = GetSensorSubType( pSrc );

    if ( isPrivateNotAndroid )
    {
        pOut->SType  = AndroidSensorSetPrivateBase( pOut->SType );
    }

    if (isSensorDataPacket)
    {
        pOut->SCP.SDP.DataFormatSensor  = GetDataFormatSensor(        pSrc );
        pOut->SCP.SDP.TimeStampFormat   = GetTimeFormatSensor(        pSrc );
        pOut->SCP.SDP.Flush             = GetSensorDataFlushStatus(   pSrc );
        pOut->SCP.SDP.DataSize          = GetSensorDataDataSize(      pSrc );
        pOut->SCP.SDP.TimeStampSize     = GetSensorDataTimeStampSize( pSrc );
    }
    else  // Control packet
    {
        pOut->SCP.CRP.DataFormatControl = GetDataFormatControl(     pSrc );
        pOut->SCP.CRP.ParameterID       = GetControlParameterID(    pSrc );
        pOut->SCP.CRP.SequenceNumber    = GetControlSequenceNumber( pSrc );
    }
}


/*=================================================================================================*\
 |  Packet parsing routines
 |
 |  Convert bit-packed/Big-Endian Host Interface Packet types to local unpacked packet types.
 |
 |  If local processor is little-endian, byte-swapping will be performed where needed.
\*=================================================================================================*/

/*=================================================================================================*\
 |    Sensor Data packet parsing routines
\*=================================================================================================*/

/****************************************************************************************************
 * @fn      ParseSensorDataPacket
 *          Top level parser for Sensor Data Packets.
 *
 * @param   [OUT]pOut - Sensor structure that will return the parsed values
 * @param   [IN]pSrc - Packet buffer containing the packet to parse
 * @param   [OUT]pPktSizeByType - Size of the packet as determined by its type and other parameters
 * @param   [IN]pktBufferSize - Size of the packet buffer provided. Sensor Data packet buffer can
 *              have multiple HIF packets.
 *
 * @return  OSP_STATUS_OK or negative Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
static int32_t ParseSensorDataPacket( LocalPacketTypes_t *pOut, const uint8_t *pSrc, uint16_t *pPktSizeByType, uint16_t pktBufferSize )
{
    /* Aliases. */
    uint8_t dSize;
    uint8_t dFormat;
    uint8_t timeFormat;
    uint8_t tSize;
    uint8_t sensSubType;
    HostIFPackets_t *pHif = (HostIFPackets_t*) pSrc;
    int32_t errCode = OSP_STATUS_UNSPECIFIED_ERROR;
    ASensorType_t sensorTypeRaw;

    /* Get header fields */
    ParsePacketHeader( pOut, pSrc, TRUE );
    dSize       = pOut->SCP.SDP.DataSize;
    dFormat     = pOut->SCP.SDP.DataFormatSensor;
    timeFormat  = pOut->SCP.SDP.TimeStampFormat;
    tSize       = pOut->SCP.SDP.TimeStampSize;
    sensSubType = pOut->SubType;
    *pPktSizeByType = 0; /* Init to 0 in case we return error */

    LocalSensorPacketTypes_t *pSDPOut  = &(pOut->SCP.SDP);

    sensorTypeRaw = AndroidSensorClearPrivateBase( pOut->SType );

    switch ( (ASensorType_t) sensorTypeRaw )
    {
    case SENSOR_ACCELEROMETER:
    case SENSOR_MAGNETIC_FIELD:
    case SENSOR_GYROSCOPE:

        if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_SENSOR_FIXPOINT) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract sensor data from packet */
            SwapEndianBufferToInt32X3(pSDPOut->P.CalFixP.Axis, pHif->CalPktFixP.Data);

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.CalFixP.TimeStamp.TS64), pHif->CalPktFixP.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_CALIBRATED_FIXP].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_ROTATION_VECTOR:
    case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
    case SENSOR_GAME_ROTATION_VECTOR:

        if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_SENSOR_FIXPOINT) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract Quaternion data from packet */
            SwapEndianBufferToInt32X4(pSDPOut->P.QuatFixP.Quat, pHif->QuatPktFixP.Data);

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.QuatFixP.TimeStamp.TS64), pHif->QuatPktFixP.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_QUATERNION_FIXP].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
    case SENSOR_GYROSCOPE_UNCALIBRATED:
    case PSENSOR_ACCELEROMETER_UNCALIBRATED:

        if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_SENSOR_FIXPOINT) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract Quaternion data from packet */
            SwapEndianBufferToInt32X3(pSDPOut->P.UncalFixP.Axis, pHif->UncalPktFixP.Data);

            /* Check if META_DATA is set to 0x01 then read offset */
            if (pOut->Metadata == META_DATA_OFFSET_CHANGE)
            {
                SwapEndianBufferToInt32X3(pSDPOut->P.UncalFixP.Offset, pHif->UncalPktFixP.Offset);
                *pPktSizeByType = sizeof(pHif->UncalPktFixP.Offset);
            }

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.UncalFixP.TimeStamp.TS64), pHif->UncalPktFixP.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType += sensorPacketDescriptions[SENSOR_DATA_UNCALIBRATED_FIXP].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_SIGNIFICANT_MOTION:

        if ((dSize == DATA_SIZE_8_BIT) && (dFormat == DATA_FORMAT_SENSOR_RAW) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract SignificantMotion data from packet */
            pSDPOut->P.SigMotion.MotionDetected = pHif->SignificantMotion.significantMotionDetected;

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.SigMotion.TimeStamp.TS64), pHif->SignificantMotion.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_SIGNIFICANT_MOTION].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_STEP_DETECTOR:

        if ((dSize == DATA_SIZE_8_BIT) && (dFormat == DATA_FORMAT_SENSOR_RAW) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract StepDetector data from packet */
            pSDPOut->P.StepDetector.StepDetected = pHif->StepDetector.stepDetected;

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.StepDetector.TimeStamp.TS64), pHif->StepDetector.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_STEP_DETECTOR].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_STEP_COUNTER:

        if ((dSize == DATA_SIZE_64_BIT) && (dFormat == DATA_FORMAT_SENSOR_RAW) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract StepCounter data from packet */
            SwapEndian( &(pSDPOut->P.StepCount.NumStepsTotal),
                        (const void *) &(pHif->StepCounter.NumStepsTotal), sizeof(uint64_t));

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.StepCount.TimeStamp.TS64), pHif->StepCounter.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_STEP_COUNTER].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_ORIENTATION:

        if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_SENSOR_FIXPOINT) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract OrientationFixP data from packet */
            SwapEndianBufferToInt32(&(pSDPOut->P.OrientFixP.Pitch), pHif->OrientationFixP.Pitch);
            SwapEndianBufferToInt32(&(pSDPOut->P.OrientFixP.Roll), pHif->OrientationFixP.Roll);
            SwapEndianBufferToInt32(&(pSDPOut->P.OrientFixP.Yaw), pHif->OrientationFixP.Yaw);

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.OrientFixP.TimeStamp.TS64), pHif->OrientationFixP.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_ORIENTATION_FIXP].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SENSOR_LINEAR_ACCELERATION:
    case SENSOR_GRAVITY:

        if ((dSize == DATA_SIZE_32_BIT) && (dFormat == DATA_FORMAT_SENSOR_FIXPOINT) &&
            (timeFormat == TIME_FORMAT_SENSOR_FIXPOINT) && (tSize == TIME_STAMP_64_BIT))
        {
            /* Extract ThreeAxisFixp data from packet */
            SwapEndianBufferToInt32X3(pSDPOut->P.ThreeAxisFixP.Axis, pHif->ThreeAxisFixp.Data);

            //TODO: How to handle the Accuracy here ? Leave it in the Metadeta ?

            /* Extract fixed point time stamp */
            SwapEndianBufferToTimeStamp64(&(pSDPOut->P.ThreeAxisFixP.TimeStamp.TS64), pHif->ThreeAxisFixp.TimeStamp);

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_THREE_AXIS_FIXP].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case PSENSOR_ACCELEROMETER_RAW:
    case PSENSOR_MAGNETIC_FIELD_RAW:
    case PSENSOR_GYROSCOPE_RAW:

        if ((sensSubType == SENSOR_SUBTYPE_UNUSED) && (dSize == DATA_SIZE_16_BIT) &&
            (dFormat == DATA_FORMAT_SENSOR_RAW) && (timeFormat == TIME_FORMAT_SENSOR_RAW))
        {
            /* Extract Raw sensor data from packet */
            pSDPOut->P.RawSensor.Axis[0] = (int32_t)BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[0], pHif->SensPktRaw.DataRaw[1]);
            pSDPOut->P.RawSensor.Axis[1] = (int32_t)BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[2], pHif->SensPktRaw.DataRaw[3]);
            pSDPOut->P.RawSensor.Axis[2] = (int32_t)BYTES_TO_SHORT(pHif->SensPktRaw.DataRaw[4], pHif->SensPktRaw.DataRaw[5]);

#ifdef _DEBUG_SENSOR_HUB_TARGET
            if (g_logging & 0x10)
            {
                DPRINTF("HIF: %d, %02X, %02X, %04X\r\n", sizeof(HostIFPackets_t),
                    pHif->SensPktRaw.DataRaw[0], pHif->SensPktRaw.DataRaw[1], pSDPOut->P.RawSensor.Axis[0]);
            }
#endif

            /* Extract raw 32-bit time stamp. Timestamp extension maybe done if needed by higher layer */
            SwapEndianBufferToTimeStamp32(&(pSDPOut->P.RawSensor.TStamp), (void *) &(pHif->SensPktRaw.TimeStamp));

            /* Get packet size for this known type from our lookup table */
            *pPktSizeByType = sensorPacketDescriptions[SENSOR_DATA_RAW].PktSzSansCRC;

            errCode = OSP_STATUS_OK;
        }
        else
        {
            errCode = OSP_STATUS_SENSOR_UNSUPPORTED;
        }
        break;

    case SYSTEM_REAL_TIME_CLOCK:
        //  TODO:  to be implement!
        errCode = OSP_STATUS_UNSUPPORTED_FEATURE;
        break;

    default:
        errCode = OSP_STATUS_UNSUPPORTED_FEATURE;
        break;
    }

    /* Check for CRC option and do CRC check now that we know the packet size */
    if ( *pPktSizeByType != 0 )
    {
        if (GetCRCFlag(pSrc))
        {
            /* Note that pPktSizeByType does not include CRC size at this point */
            const uint16_t crcCalc = Crc16_CCITT( pSrc, *pPktSizeByType );

            const uint16_t crcPacket = BYTES_TO_SHORT( pSrc[*pPktSizeByType], pSrc[*pPktSizeByType + 1] );

            if (crcCalc != crcPacket)
            {
                errCode = OSP_STATUS_INVALID_CRC;
            }
            else
            {
                errCode = OSP_STATUS_OK;
            }

            /* Increase packet size to account for CRC. We do that even if CRC failed so that application
               layer can skip over this packet */
            *pPktSizeByType += CRC_SIZE;
        }
    }

    /* Check if we are exceeding buffer size provided */
    if (*pPktSizeByType > pktBufferSize)
    {
        errCode = OSP_STATUS_BUFFER_TOO_SMALL;
        /* Note: purposely not resetting the pPktSizeByType to zero here */
    }

    return SET_ERROR( errCode );
}


/*=================================================================================================*\
 |    Control Request/Response packet parsing routines
\*=================================================================================================*/

/****************************************************************************************************
 * @fn      ParseControlPacket
 *          Top level parser for Control Request Packets.
 *
 * @param   [OUT]pDest - Structure that will return the parsed values
 * @param   [IN]pSrc - Packet buffer containing the packet to parse
 * @param   [OUT]pPktSizeByType - Size of the packet as determined by its type and other parameters
 * @param   [IN]pktBufferSize - Size of the packet buffer provided.  In case of Sensor Control
 *              packets, the buffer is expected to contain only one packet.
 *
 * @return  OSP_STATUS_OK, or negative Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
static int32_t ParseControlPacket( LocalPacketTypes_t *pDest, const uint8_t *pSrc, uint16_t *pPktSizeByType, uint16_t pktBufferSize )
{
    uint8_t *pDestPayload;
    int32_t errCode = SET_ERROR( OSP_STATUS_OK );
    uint8_t parameterID;

    /* Copy header fields */
    ParsePacketHeader( pDest, pSrc, FALSE );
    *pPktSizeByType = 0; /* Init to 0 in case we return error */

    parameterID = pDest->SCP.CRP.ParameterID;

    if ( !ValidParameterID(parameterID) )
    {
        return SET_ERROR( OSP_STATUS_INVALID_PARAMETER );
    }

    *pPktSizeByType = CTRL_PKT_HEADER_SIZE;

    /* Copy payload */
    pDestPayload = GetControlPayloadAddress( pDest, parameterID );
    /* Note: pDestPayLoad == NULL means that no data copy is required */
    if (pDestPayload != NULL)
    {
        *pPktSizeByType += CopyControlPacketPayload( pDestPayload, pSrc + CTRL_PKT_PAYLOAD_OFFSET, parameterID );
    }

    /* Check for CRC option and do CRC check now that we know the packet size */
    if (GetCRCFlag(pSrc))
    {
        /* Note that pPktSizeByType does not include CRC size at this point */
        const uint16_t crcCalc = Crc16_CCITT( pSrc, *pPktSizeByType );

        const uint16_t crcPacket = BYTES_TO_SHORT( pSrc[*pPktSizeByType], pSrc[*pPktSizeByType + 1] );

        if (crcCalc != crcPacket)
        {
            errCode = SET_ERROR( OSP_STATUS_INVALID_CRC );
        }

        /* Increase packet size to account for CRC. We do that even if CRC failed so that application
            layer can skip over this packet */
        *pPktSizeByType += CRC_SIZE;
    }

    /* Check if we are exceeding buffer size provided */
    if (*pPktSizeByType > pktBufferSize)
    {
        errCode = SET_ERROR( OSP_STATUS_BUFFER_TOO_SMALL );
        /* Note: purposely not resetting the pPktSizeByType to zero here */
    }

    return errCode;
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      ParseHostInterfacePkt
 *          Top level parser for Host interface Packets.
 *          Depending on packet type & data sizes, the individual parsers are called.
 *
 * @param   [OUT]pOut - Sensor structure that will return the parsed values
 * @param   [IN]pPacket - Packet buffer containing the packet to parse
 * @param   [OUT]pPktSizeByType - Size of the packet as determined by its type and other parameters
 * @param   [IN]pktBufferSize - Size of the packet buffer provided. Sensor Data packet buffer can
 *              have multiple HIF packets. In case of Sensor Control packets, the buffer is expected
 *              to contain only one packet.
 *
 * @return  OSP_STATUS_OK or Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t ParseHostInterfacePkt( LocalPacketTypes_t *pOut, const uint8_t *pPacket, uint16_t *pPktSizeByType, uint16_t pktBufferSize )
{
    uint8_t pktID;
    int32_t errCode;

    /* Sanity checks - CRC, Buffer, Version */
    if ((pOut == NULL) || (pPktSizeByType == NULL))
    {
        return SET_ERROR( OSP_STATUS_NULL_POINTER );
    }

    errCode = CheckPacketSanity( pPacket );
    if (errCode != OSP_STATUS_OK)
    {
        return errCode;
    }

    /* ========== Packet handling ========== */
    /* 1. Identify Packet type */
    pktID = GetPacketID(pPacket);
    pOut->PacketID = pktID;

    switch (pktID)
    {
    case PKID_SENSOR_DATA:
    case PKID_SENSOR_TEST_DATA:
        /* Parse a single HIF packet from the given packet buffer and return the size of the packet parsed
         * pktBufferSize must be >= HIF packet to be parsed
         */
        /* Incoming buffer size check */
        if (pktBufferSize < MIN_HIF_SENSOR_DATA_PKT_SZ)
        {
            return SET_ERROR( OSP_STATUS_BUFFER_TOO_SMALL );
        }

        errCode = ParseSensorDataPacket( pOut, pPacket, pPktSizeByType, pktBufferSize );
        break;

    case PKID_CONTROL_REQ_RD:
    case PKID_CONTROL_REQ_WR:
    case PKID_CONTROL_RESP:
        /* Incoming buffer size check */
        if (pktBufferSize < MIN_HIF_CONTROL_PKT_SZ)
        {
            return SET_ERROR( OSP_STATUS_BUFFER_TOO_SMALL );
        }

        errCode = ParseControlPacket( pOut, pPacket, pPktSizeByType, pktBufferSize );
        break;

    default:
        errCode = SET_ERROR( OSP_STATUS_INVALID_PACKETID);
        break;
    }

    return errCode;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
