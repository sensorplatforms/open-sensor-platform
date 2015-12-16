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
#define SENSOR_PACKETS_FORMAT_C
#include <string.h>
#include "common.h"
#include "SensorPackets.h"
#include "SensorPackets_Internal.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

#define MY_FID  FID_SENSOR_PACKETS_FORMAT_C

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

static int32_t FormatControlPacketHeader(
        HostIFPackets_t *pDest, uint8_t packetID, uint8_t parameterID,
        ASensorType_t sType, uint8_t subType, uint8_t seqNum );


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      FormatControlPacketFromFields
 *          Make a packed packet from individual field parameters.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller.
 * @param   [IN]pSrcPayloadData - Data structure carrying Write or Read control data
 * @param   [IN]packetID - Control Packet type ID
 * @param   [IN]parameterID - Parameter ID for the Control packet
 * @param   [IN]sType - Sensor Type for the Control packet
 * @param   [IN]subType - Sub type for the sensor type used (not applicable for all sensor types)
 * @param   [IN]seqNum - Sequence number to set in the Control packet
 * @param   [IN]crcFlag - CRC flag to set in the Control packet
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatControlPacketFromFields( HostIFPackets_t *pDest, const uint8_t *pSrcPayloadData,
                                       uint8_t packetID, uint8_t parameterID,
                                       ASensorType_t sType, uint8_t subType, uint8_t seqNum, uint8_t crcFlag )
{
    int16_t packetSize, packetPayloadSize;

    if (pDest == NULL)
    {
        return SET_ERROR( OSP_STATUS_NULL_POINTER );
    }

    /* write control packet header from provided fields */
    packetPayloadSize = FormatControlPacketHeader(
        pDest, packetID, parameterID, sType, subType, seqNum );

    /* return error code here, on error */
    if (packetPayloadSize < 0)
    {
        return SET_ERROR( packetPayloadSize );
    }

    packetSize = packetPayloadSize + CTRL_PKT_HEADER_SIZE;

    /* format data */
    if ( packetPayloadSize > 0 )
    {
        if (pSrcPayloadData == NULL)
        {
            return SET_ERROR( OSP_STATUS_NULL_POINTER );
        }
        else
        {
            uint8_t *pDestPayload = GetControlPacketPayloadAddress( pDest );

            CopyControlPacketPayload( pDestPayload, pSrcPayloadData, parameterID );
        }
    }

    /*  set CRC flag and append CRC, if indicated  */
    if (crcFlag)
    {
        packetSize += CRC_SIZE;
        FormatPacketCRC( pDest, packetSize );
    }

    return packetSize;
}

/****************************************************************************************************
 * @fn      FormatControlPacketHeader
 *          Fill in packed Control packet header from the parameters provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller.
 * @param   [IN]packetID - Control Packet type ID
 * @param   [IN]parameterID - Parameter ID for the Control packet
 * @param   [IN]sType - Sensor Type for the Control packet
 * @param   [IN]subType - Sub type for the sensor type used (not applicable for all sensor types)
 * @param   [IN]seqNum - Sequence number to set in the Control packet
 *
 * @return  Negative packed error code corresponding to the error encountered
 *
 ***************************************************************************************************/
static int32_t FormatControlPacketHeader( HostIFPackets_t *pDest, uint8_t packetID, uint8_t parameterID,
                                          ASensorType_t sType, uint8_t subType, uint8_t seqNum )
{
    HifControlPktNoData_t *pOut = (HifControlPktNoData_t *) pDest;

    /* Get packet payload size */
    const int16_t packetPayloadSize = GetControlPacketPayloadSize(packetID, parameterID );

    if ( packetPayloadSize < 0 )
    {
        return SET_ERROR( packetPayloadSize );
    }

    /* Check Sensor enumeration type Android or User defined */
    if (!(IsPrivateNotAndroid(sType)))
    {
        pOut->Q.ControlByte = SENSOR_TYPE_ANDROID;
    }
    else
    {
        pOut->Q.ControlByte = SENSOR_TYPE_PRIVATE;
    }

    /* Setup Control Byte */
    SetPacketID( &(pOut->Q.ControlByte), packetID );

    /* Setup Sensor ID Byte */
    pOut->Q.SensorIdByte = M_SensorType(sType);

    /* Setup Attribute Byte 1 */
    pOut->Q.AttributeByte = M_SensorSubType(subType) | M_SequenceNum(seqNum);

    /* Attribute Byte 2 */
    pOut->AttrByte2 = M_SetParamId(parameterID);

    return packetPayloadSize;
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*=================================================================================================*\
 *  Packet formatting routines
 *
 *  Convert local unpacked packet types to bitpacked/Big-Endian Host Interface Packet types
 *  for bus transfer between processors.
 *
 *  If local processor is little-endian, byteswapping will be performed where needed.
\*=================================================================================================*/

/*=================================================================================================*\
 |    Sensor Data packet formatting routines
\*=================================================================================================*/

/****************************************************************************************************
 * @fn      FormatSensorDataPacket
 *          Using the sensor sample data, this function creates the HIF Sensor Data Packet for
 *          sending to Host in the buffer provided.
 *
 * @param   [OUT]pDestPacket - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifSensorDataRaw_t) in length
 * @param   [IN]pSrc - Data structure carrying the sensor sample (typically from driver)
 * @param   [IN]sensorPacketType - Sensor Packet type ID corresponding to members in the type union
 * @param   [IN]metaData - Meta data for the sensor type used (not applicable for all sensor types)
 * @param   [IN]sType - Sensor Type for the sensor data presented
 * @param   [IN]subType - Sub type for the sensor type used (not applicable for all sensor types)
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatSensorDataPacket( HostIFPackets_t *pDestPacket, const uint8_t *pSrc,
    uint8_t sensorPacketType, uint8_t metaData,
    ASensorType_t sType, uint8_t subType )
{
    const SensorPktDesc_t *pSPD = &(sensorPacketDescriptions[sensorPacketType]);
    uint8_t *pDest = (uint8_t *) pDestPacket;
    uint8_t tsSize, elemSize;
    uint32_t i, j;           // i = dest index, j = src index.

    /* Sanity checks... */
    if (pDest == NULL || pSrc == NULL)
    {
        return SET_ERROR( OSP_STATUS_NULL_POINTER );
    }

    /* clear header before setting bits in it */
    pDestPacket->GenericControlPkt.Q.ControlByte    = 0;
    pDestPacket->GenericControlPkt.Q.SensorIdByte   = 0;
    pDestPacket->GenericControlPkt.Q.AttributeByte  = 0;

    /* Set Sensor enumeration type Android or User defined */
    if ( IsPrivateNotAndroid( sType ) )
    {
        SetPrivateField( &(pDestPacket->GenericControlPkt.Q.ControlByte) );
    }

    /* Setup Control Byte */
    SetPacketID( pDest, PKID_SENSOR_DATA );
    SetDataFormatSensor( pDest, pSPD->DataFormat );
    SetTimeFormatSensor( pDest, pSPD->TimeFormat );

    /* Setup Sensor ID Byte */
    SetMetadata( pDest, metaData );
    SetSensorTypeField( pDest, sType );

    /* Setup Attribute Byte */
    SetSensorSubType( pDest, subType );
    SetSensorDataDataSize( pDest, pSPD->DataSz );
    SetSensorDataTimeStampSize( pDest, pSPD->TStampSz );

    i = PKT_TIMESTAMP_OFFSET;           //  offset of timestamp in destination packet
    j = LOCAL_PACKET_TIMESTAMP_OFFSET;  //  offset of timestamp in source packet

    /* Copy timestamp */
    tsSize = (pSPD->TStampSz == TIME_STAMP_32_BIT ? sizeof(int32_t) : sizeof(int64_t));

    /* NOTE: TODO-BUGFIX - The logic below will fail for BE machine for 32-bit timestamp as it
       will copy the higher 32-bit of the 64-bit TS */
    SwapEndian(pDest + i, pSrc + j, tsSize);
    i += tsSize;           // dest packet TS 32 or 64 bits.
    j += sizeof(int64_t);  // src packet TS 64 bits, or TS 32 + spare 32.

    /* Copy payload */
    elemSize = pSPD->ElementSz;

    if (elemSize != 0)
    {
        /* Data bytes */
        uint16_t nElem = pSPD->NumElements;
        uint16_t nBytesToCopy;

        if ( SENSOR_DOUBLE_PAYLOAD_SIZE_FOR_METADATA( sensorPacketType, metaData ) )
        {
            /* Special handling for SENSOR_DATA_UNCALIBRATED_FIXP packet */
            /* Check if metadata is available  if it is available then add it to HIF packet */
            /* Add it by doubling the payload size before endian swap/copy. */

            nElem *= 2;  // double the payload size by doubling the number of elements.
        }

        nBytesToCopy = nElem * elemSize;

        if (pSPD->IsBigEndian)
        {
            SwapEndianX( pDest + i, pSrc + j, elemSize, nElem );
        }
        else
        {
            SH_MEMCPY( pDest + i, pSrc + j, nBytesToCopy );
        }

        i += nBytesToCopy;
    }

    /*  set CRC flag and append CRC, if indicated  */
    if (FORMAT_WITH_CRC_ENABLED)
    {
        i += CRC_SIZE;
        FormatPacketCRC( pDestPacket, i );
    }

    return i;
}

/****************************************************************************************************
 * @fn      FormatQuaternionPktFixP
 *          Using the given quaternion data, this function creates the HIF Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifQuaternionFixPoint_t) in length
 * @param   [IN]pQuatData - Data structure carrying the quaternion result from fusion
 * @param   [IN]sType - Android sensor type for the quaternion data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatQuaternionPktFixP( HostIFPackets_t *pDest, const QuaternionFixP_t *pQuatData, ASensorType_t sType )
{
        return FormatSensorDataPacket( pDest, (const uint8_t *) pQuatData,
                                       SENSOR_DATA_QUATERNION_FIXP, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
}


/****************************************************************************************************
 * @fn      FormatUncalibratedPktFixP
 *          Using the given uncalibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifUncalibratedFixPoint_t) in length
 * @param   [IN]pUncalData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]metaData - Meta data is used to indicate if the packet contains offset data
 * @param   [IN]sType - Sensor type for the uncalibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatUncalibratedPktFixP( HostIFPackets_t *pDest, const UncalibratedFixP_t *pUncalData,
    uint8_t metaData, ASensorType_t sType )
{
        return FormatSensorDataPacket( pDest, (const uint8_t *) pUncalData,
                                       SENSOR_DATA_UNCALIBRATED_FIXP, metaData, sType, SENSOR_SUBTYPE_UNUSED);
}


/****************************************************************************************************
 * @fn      FormatOrientationFixP
 *          Using the given sensor data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifOrientationPktFixPoint_t) in length
 * @param   [IN]pOrientationData - Data structure carrying the sensor data result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatOrientationFixP( HostIFPackets_t *pDest, const OrientationFixP_t *pOrientationData,
    ASensorType_t sType )
{
        return FormatSensorDataPacket( pDest, (const uint8_t *) pOrientationData,
                                       SENSOR_DATA_ORIENTATION_FIXP, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
}


/****************************************************************************************************
 * @fn      FormatSignificantMotionPktFixP
 *          Using the given sensor data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifSignificantMotionPktFixPoint_t) in length
 * @param   [IN]pSignificantMotionData - Data structure carrying the sensor result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatSignificantMotionPktFixP(
    HostIFPackets_t *pDest, const SignificantMotion_t *pSignificantMotionData, ASensorType_t sType )
{
        return FormatSensorDataPacket( pDest, (const uint8_t *) pSignificantMotionData,
                                       SENSOR_DATA_SIGNIFICANT_MOTION, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
}


/****************************************************************************************************
 * @fn      FormatThreeAxisPktFixP
 *          Using the given Calibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifThreeAxisPktFixPoint_t) in length
 * @param   [IN]p3AxisData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatThreeAxisPktFixP( HostIFPackets_t *pDest, const ThreeAxisFixP_t *p3AxisData, ASensorType_t sType )
{
    return FormatSensorDataPacket( pDest, (const uint8_t *) p3AxisData,
                                   SENSOR_DATA_THREE_AXIS_FIXP, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
        //TODO : Find out when to use the preciseData
}


/****************************************************************************************************
 * @fn      FormatStepCounterPkt
 *          Using the given data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifStepCounter_t) in length
 * @param   [IN]pStepCounterData - Data structure carrying the result from Algorithm
 * @param   [IN]sType - Sensor type for the data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatStepCounterPkt( HostIFPackets_t *pDest, const StepCounter_t *pStepCounterData, ASensorType_t sType )
{
    return FormatSensorDataPacket( pDest, (const uint8_t *) pStepCounterData,
                                   SENSOR_DATA_STEP_COUNTER, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
        //TODO : Find out when to use the preciseData
}


/****************************************************************************************************
 * @fn      FormatStepDetectorPkt
 *          Using the given data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifStepDetector_t) in length
 * @param   [IN]pStepDetectorData - Data structure carrying the result from Algorithm
 * @param   [IN]sType - Sensor type for the data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatStepDetectorPkt( HostIFPackets_t *pDest, const StepDetector_t *pStepDetectorData,
    ASensorType_t sType )
{
    return FormatSensorDataPacket( pDest, (const uint8_t *) pStepDetectorData,
                                   SENSOR_DATA_STEP_DETECTOR, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
}


/****************************************************************************************************
 * @fn      FormatCalibratedPktFixP
 *          Using the given Calibrated data, this function creates the Sensor Data Packet for sending
 *          to host in the buffer provided.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller. This buffer size must be at least
 *                  sizeof(HifCalibratedFixPoint_t) in length
 * @param   [IN]pCalData - Data structure carrying the uncalibrated result from Algorithm
 * @param   [IN]sType - Sensor type for the calibrated data
 *
 * @return  Size of the formatted packet or -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatCalibratedPktFixP( HostIFPackets_t *pDest, const CalibratedFixP_t *pCalData, ASensorType_t sType )
{
    return FormatSensorDataPacket( pDest, (const uint8_t *) pCalData,
                                   SENSOR_DATA_CALIBRATED_FIXP, META_DATA_UNUSED, sType, SENSOR_SUBTYPE_UNUSED);
}


/*=================================================================================================*\
 |    Control Request/Response packet formatting routines
\*=================================================================================================*/

/*=================================================================================================*\
 |    Generic Control Request/Response Packet formatting routines
\*=================================================================================================*/

/****************************************************************************************************
 * @fn      FormatControlPacket
 *          Make a packed Control packet from a Local (expanded) packet structure.
 *
 * @param   [OUT]pDest - Destination buffer supplied by the caller.
 * @param   [IN]pLocalPacket - Data structure carrying the fields and payload for the Control packet
 * @param   [OUT]pDestPacketSize - number of valid bytes in the packed Control packet (not buffer size)
 *
 * @return  Negative packed error code corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t FormatControlPacket( HostIFPackets_t *pDest, const LocalPacketTypes_t *pLocalPacket,
                             uint16_t *pDestPacketSize ) {

    int32_t retVal;

    if ( pDestPacketSize == NULL ) /* pDest NULL check happens in FormatControlPacketFromFields */
    {
        retVal = SET_ERROR( OSP_STATUS_NULL_POINTER );
    }
    else
    {
        retVal  = FormatControlPacketFromFields( pDest,
            pLocalPacket->PayloadOffset + (uint8_t *) pLocalPacket,
            pLocalPacket->PacketID, pLocalPacket->SCP.CRP.ParameterID,
            pLocalPacket->SType, pLocalPacket->SubType,
            pLocalPacket->SCP.CRP.SequenceNumber,
            (uint8_t) FORMAT_WITH_CRC_ENABLED );

        if ( retVal < 0 )
        {
            *pDestPacketSize  = 0;
            return retVal;
        }
    }

    /* Return value has the packet size */
    *pDestPacketSize = retVal;

    return SET_ERROR( OSP_STATUS_OK );
}

/*=================================================================================================*\
 |    Type-specific Control Request/Response Packet formatting routines
\*=================================================================================================*/

#define _FCP(PSRC, PKTID, PARAMID) \
    FormatControlPacketFromFields( \
        pDest, (const uint8_t *) (PSRC), (PKTID), (PARAMID), \
        sType, subType, seqNum, crcFlag )

//   PARAM_ID_ERROR_CODE_IN_DATA     0x00    R_: Int32
//
int32_t FormatControlResp_ErrorCodeInData( HostIFPackets_t *pDest, int32_t errorCodeInData,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &errorCodeInData, PKID_CONTROL_RESP, PARAM_ID_ERROR_CODE_IN_DATA );
}


//   PARAM_ID_ENABLE                 0x01    _W: Bool
//
int32_t FormatControlReqWrite_Enable( HostIFPackets_t *pDest, uint8_t enable,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    const uint8_t temp = (enable)? 1 : 0;

    return _FCP( &temp, PKID_CONTROL_REQ_WR, PARAM_ID_ENABLE );
}

//   PARAM_ID_BATCH                  0x02    _W: Uint64 x 2
//
int32_t FormatControlReqWrite_Batch(
    HostIFPackets_t *pDest, const uint64_t *pUint64x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint64x2, PKID_CONTROL_REQ_WR, PARAM_ID_BATCH );
}

//   PARAM_ID_FLUSH                  0x03    _W: no payload
//
int32_t FormatControlReqWrite_Flush(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_WR, PARAM_ID_FLUSH );
}

//   PARAM_ID_RANGE_RESOLUTION       0x04    R_: FixP32 x 2
//
int32_t FormatControlReqRead_RangeResolution(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_RANGE_RESOLUTION );
}

int32_t FormatControlResp_RangeResolution(
    HostIFPackets_t *pDest, const int32_t *pInt32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x2, PKID_CONTROL_RESP, PARAM_ID_RANGE_RESOLUTION );
}

//   PARAM_ID_POWER                  0x05    R_: FixP32
//
int32_t FormatControlReqRead_Power(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_POWER );
}

int32_t FormatControlResp_Power(
    HostIFPackets_t *pDest, int32_t iPower,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iPower, PKID_CONTROL_RESP, PARAM_ID_POWER );
}

//   PARAM_ID_MINMAX_DELAY           0x06    R_: Int32 x 2
//
int32_t FormatControlReqRead_MinMaxDelay(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_MINMAX_DELAY );
}

int32_t FormatControlResp_MinMaxDelay(
    HostIFPackets_t *pDest, const int32_t *pInt32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x2, PKID_CONTROL_RESP, PARAM_ID_MINMAX_DELAY );
}

//   PARAM_ID_FIFO_EVT_CNT           0x07    R_: Uint32 x 2
//
int32_t FormatControlReqRead_FifoEventCounts(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_FIFO_EVT_CNT );
}

int32_t FormatControlResp_FifoEventCounts(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint32x2, PKID_CONTROL_RESP, PARAM_ID_FIFO_EVT_CNT );
}

//   PARAM_ID_AXIS_MAPPING           0x08    RW: Int8 x 3
//
int32_t FormatControlReqRead_AxisMapping(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_AXIS_MAPPING );
}

int32_t FormatControlReqWrite_AxisMapping(
    HostIFPackets_t *pDest, const int8_t *pInt8x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt8x3, PKID_CONTROL_REQ_WR, PARAM_ID_AXIS_MAPPING );
}

int32_t FormatControlResp_AxisMapping(
    HostIFPackets_t *pDest, const int8_t *pInt8x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt8x3, PKID_CONTROL_RESP, PARAM_ID_AXIS_MAPPING );
}

//   PARAM_ID_CONVERSION_OFFSET      0x09    RW: Int32 x 3
//
int32_t FormatControlReqRead_ConversionOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_CONVERSION_OFFSET );
}

int32_t FormatControlReqWrite_ConversionOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_CONVERSION_OFFSET );
}

int32_t FormatControlResp_ConversionOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_CONVERSION_OFFSET );
}

//   PARAM_ID_CONVERSION_SCALE       0x0A    RW: FixP32 x 3
//
int32_t FormatControlReqRead_ConversionScale(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_CONVERSION_SCALE );
}

int32_t FormatControlReqWrite_ConversionScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_CONVERSION_SCALE );
}

int32_t FormatControlResp_ConversionScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_CONVERSION_SCALE );
}

//   PARAM_ID_SENSOR_NOISE           0x0B    RW: FixP32 x 3
//
int32_t FormatControlReqRead_SensorNoise(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_SENSOR_NOISE );
}

int32_t FormatControlReqWrite_SensorNoise(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_SENSOR_NOISE );
}

int32_t FormatControlResp_SensorNoise(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_SENSOR_NOISE );
}

//   PARAM_ID_TIMESTAMP_OFFSET       0x0C    RW: FixP32
//
int32_t FormatControlReqRead_TimeStampOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_TIMESTAMP_OFFSET );
}

int32_t FormatControlReqWrite_TimeStampOffset(
    HostIFPackets_t *pDest, int32_t iTimeStampOffset,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iTimeStampOffset, PKID_CONTROL_REQ_WR, PARAM_ID_TIMESTAMP_OFFSET );
}

int32_t FormatControlResp_TimeStampOffset(
    HostIFPackets_t *pDest, int32_t iTimeStampOffset,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iTimeStampOffset, PKID_CONTROL_RESP, PARAM_ID_TIMESTAMP_OFFSET );
}

//   PARAM_ID_ONTIME_WAKETIME        0x0D    RW: Uint32 x 2
//
int32_t FormatControlReqRead_OnTimeWakeTime(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_ONTIME_WAKETIME );
}

int32_t FormatControlReqWrite_OnTimeWakeTime(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint32x2, PKID_CONTROL_REQ_WR, PARAM_ID_ONTIME_WAKETIME );
}

int32_t FormatControlResp_OnTimeWakeTime(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint32x2, PKID_CONTROL_RESP, PARAM_ID_ONTIME_WAKETIME );
}

//   PARAM_ID_HPF_LPF_CUTOFF         0x0E    RW: Uint16 x 2
//

int32_t FormatControlReqRead_HpfLpfCutoff(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_HPF_LPF_CUTOFF );
}

int32_t FormatControlReqWrite_HpfLpfCutoff(
    HostIFPackets_t *pDest, const uint16_t *pUint16x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint16x2, PKID_CONTROL_REQ_WR, PARAM_ID_HPF_LPF_CUTOFF );
}

int32_t FormatControlResp_HpfLpfCutoff(
    HostIFPackets_t *pDest, const uint16_t *pUint16x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint16x2, PKID_CONTROL_RESP, PARAM_ID_HPF_LPF_CUTOFF );
}

//   PARAM_ID_SENSOR_NAME            0x0F    R_: Char x 32
//

int32_t FormatControlReqRead_SensorName(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_SENSOR_NAME );
}

int32_t FormatControlResp_SensorName(
    HostIFPackets_t *pDest, const uint32_t *pUint8x32,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint8x32, PKID_CONTROL_RESP, PARAM_ID_SENSOR_NAME );
}

//   PARAM_ID_XYZ_OFFSET             0x10    RW: FixP32 x 3
//

int32_t FormatControlReqRead_XyzOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_XYZ_OFFSET );
}

int32_t FormatControlReqWrite_XyzOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_XYZ_OFFSET );
}

int32_t FormatControlResp_XyzOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_XYZ_OFFSET );
}

//   PARAM_ID_F_SKOR_MATRIX          0x11    RW: FixP32 x 3 x 3
//

int32_t FormatControlReqRead_SkorMatrix(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_F_SKOR_MATRIX );
}

int32_t FormatControlReqWrite_SkorMatrix(
    HostIFPackets_t *pDest, const int32_t *pInt32x3x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3x3, PKID_CONTROL_REQ_WR, PARAM_ID_F_SKOR_MATRIX );
}

int32_t FormatControlResp_SkorMatrix(
    HostIFPackets_t *pDest, const int32_t *pInt32x3x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3x3, PKID_CONTROL_RESP, PARAM_ID_F_SKOR_MATRIX );
}

//   PARAM_ID_F_CAL_OFFSET           0x12    RW: FixP32 x 3
//

int32_t FormatControlReqRead_FCalOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_F_CAL_OFFSET );
}

int32_t FormatControlReqWrite_FCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_F_CAL_OFFSET );
}

int32_t FormatControlResp_FCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_F_CAL_OFFSET );
}

//   PARAM_ID_F_NONLINEAR_EFFECTS    0x13    RW: FixP16 x 4 x 3
//

int32_t FormatControlReqRead_FNonlinearEffects(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_F_NONLINEAR_EFFECTS );
}

int32_t FormatControlReqWrite_FNonlinearEffects(
    HostIFPackets_t *pDest, const int16_t *pInt16x4x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x4x3, PKID_CONTROL_REQ_WR, PARAM_ID_F_NONLINEAR_EFFECTS );
}

int32_t FormatControlResp_FNonlinearEffects(
    HostIFPackets_t *pDest, const int16_t *pInt16x4x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x4x3, PKID_CONTROL_RESP, PARAM_ID_F_NONLINEAR_EFFECTS );
}

//   PARAM_ID_BIAS_STABILITY         0x14    RW: FixP32 x 3
//

int32_t FormatControlReqRead_BiasStability(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_BIAS_STABILITY );
}

int32_t FormatControlReqWrite_BiasStability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_BIAS_STABILITY );
}

int32_t FormatControlResp_BiasStability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_BIAS_STABILITY );
}

//   PARAM_ID_REPEATABILITY          0x15    RW: FixP32 x 3
//

int32_t FormatControlReqRead_Repeatability(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_REPEATABILITY );
}

int32_t FormatControlReqWrite_Repeatability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_REPEATABILITY );
}

int32_t FormatControlResp_Repeatability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_REPEATABILITY );
}

//   PARAM_ID_TEMP_COEFF             0x16    RW: FixP16 x 3 x 2
//

int32_t FormatControlReqRead_TempCoeff(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_TEMP_COEFF );
}

int32_t FormatControlReqWrite_TempCoeff(
    HostIFPackets_t *pDest, const int16_t *pInt16x3x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x3x2, PKID_CONTROL_REQ_WR, PARAM_ID_TEMP_COEFF );
}

int32_t FormatControlResp_TempCoeff(
    HostIFPackets_t *pDest, const int16_t *pInt16x3x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x3x2, PKID_CONTROL_RESP, PARAM_ID_TEMP_COEFF );
}

//   PARAM_ID_SHAKE_SUSCEPTIBILITY   0x17    RW: FixP16 x 3
//

int32_t FormatControlReqRead_ShakeSusceptibility(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_SHAKE_SUSCEPTIBILITY );
}

int32_t FormatControlReqWrite_ShakeSusceptibility(
    HostIFPackets_t *pDest, const int16_t *pInt16x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x3, PKID_CONTROL_REQ_WR, PARAM_ID_SHAKE_SUSCEPTIBILITY );
}

int32_t FormatControlResp_ShakeSusceptibility(
    HostIFPackets_t *pDest, const int16_t *pInt16x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt16x3, PKID_CONTROL_RESP, PARAM_ID_SHAKE_SUSCEPTIBILITY );
}

//   PARAM_ID_EXPECTED_NORM          0x18    RW: FixP32
//

int32_t FormatControlReqRead_ExpectedNorm(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_EXPECTED_NORM );
}

int32_t FormatControlReqWrite_ExpectedNorm(
    HostIFPackets_t *pDest, int32_t iExpectedNorm,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iExpectedNorm, PKID_CONTROL_REQ_WR, PARAM_ID_EXPECTED_NORM );
}

int32_t FormatControlResp_ExpectedNorm(
    HostIFPackets_t *pDest, int32_t iExpectedNorm,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iExpectedNorm, PKID_CONTROL_RESP, PARAM_ID_EXPECTED_NORM );
}

//   PARAM_ID_VERSION                0x19    R_: Char x 32
//

int32_t FormatControlReqRead_Version(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_VERSION );
}

int32_t FormatControlResp_Version(
    HostIFPackets_t *pDest, const uint32_t *pUint8x32,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pUint8x32, PKID_CONTROL_RESP, PARAM_ID_VERSION );
}

//   PARAM_ID_DYNAMIC_CAL_SCALE      0x1A    RW: FixP32 x 3
//

int32_t FormatControlReqRead_DynamicCalScale(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_SCALE );
}

int32_t FormatControlReqWrite_DynamicCalScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_SCALE );
}

int32_t FormatControlResp_DynamicCalScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_SCALE );
}

//   PARAM_ID_DYNAMIC_CAL_SKEW       0x1B    RW: FixP32 x 3
//

int32_t FormatControlReqRead_DynamicCalSkew(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_SKEW );
}

int32_t FormatControlReqWrite_DynamicCalSkew(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_SKEW );
}

int32_t FormatControlResp_DynamicCalSkew(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_SKEW );
}

//   PARAM_ID_DYNAMIC_CAL_OFFSET     0x1C    RW: FixP32 x 3
//

int32_t FormatControlReqRead_DynamicCalOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_OFFSET );
}

int32_t FormatControlReqWrite_DynamicCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_OFFSET );
}

int32_t FormatControlResp_DynamicCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_OFFSET );
}

//   PARAM_ID_DYNAMIC_CAL_ROTATION   0x1D    RW: FixP32 x 3
//

int32_t FormatControlReqRead_DynamicCalRotation(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_ROTATION );
}

int32_t FormatControlReqWrite_DynamicCalRotation(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_ROTATION );
}

int32_t FormatControlResp_DynamicCalRotation(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_ROTATION );
}

//   PARAM_ID_DYNAMIC_CAL_QUALITY    0x1E    RW: FixP32 x 3
//

int32_t FormatControlReqRead_DynamicCalQuality(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_QUALITY );
}

int32_t FormatControlReqWrite_DynamicCalQuality(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_QUALITY );
}

int32_t FormatControlResp_DynamicCalQuality(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( pInt32x3, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_QUALITY );
}

//   PARAM_ID_DYNAMIC_CAL_SOURCE     0x1F    RW: Int8
//

int32_t FormatControlReqRead_DynamicCalSource(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_RD, PARAM_ID_DYNAMIC_CAL_SOURCE );
}

int32_t FormatControlReqWrite_DynamicCalSource(
    HostIFPackets_t *pDest, int8_t iDynamicCalSource,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iDynamicCalSource, PKID_CONTROL_REQ_WR, PARAM_ID_DYNAMIC_CAL_SOURCE );
}

int32_t FormatControlResp_DynamicCalSource(
    HostIFPackets_t *pDest, int8_t iDynamicCalSource,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( &iDynamicCalSource, PKID_CONTROL_RESP, PARAM_ID_DYNAMIC_CAL_SOURCE );
}

//   PARAM_ID_CONFIG_DONE            0x20    _W: no payload
//

int32_t FormatControlReqWrite_ConfigDone(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag )
{
    return _FCP( NULL, PKID_CONTROL_REQ_WR, PARAM_ID_CONFIG_DONE );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
