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
#include <string.h>
#include "common.h"
#include "SensorPackets.h"

#define SENSOR_PACKETS_COMMON_C
#include "SensorPackets_Internal.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define MY_FID  FID_SENSOR_PACKETS_COMMON_C

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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      SwapEndian routines
 *
 *          Each SwapEndian routine copies source buffer to destination buffer,
 *          swapping endianness only if the LOCAL_IS_LITTLE_ENDIAN preprocessor define is set.
 ***************************************************************************************************/
void SwapEndian(void *pDest, const void *pSrc, size_t nBytes)
{
#if LOCAL_IS_LITTLE_ENDIAN
    int idxWrite;
    int idxRead = nBytes - 1;
    unsigned char       *pWrite = (unsigned char *)       pDest;
    unsigned char const *pRead  = (const unsigned char *) pSrc;

    // decrement/increment indices instead of points to avoid out-of-bounds pointers at
    // end of loop.

    for (idxWrite = 0; idxWrite < nBytes; idxWrite++)
    {
        pWrite[idxWrite] = pRead[idxRead];
        idxRead--;
    }
#else
    SH_MEMCPY(pDest, pSrc, nBytes);
#endif
}


/****************************************************************************************************
 * @fn      GetControlPayloadAddress
 *          Gets address of payload in Local Packet, using offset determined by
 *          control packet type (Parameter ID).
 *
 *          Returns payload address in given packet, or NULL if no payload or if bad parameterID.
 ***************************************************************************************************/
uint8_t *GetControlPayloadAddress( LocalPacketTypes_t *pLocalPacket, uint8_t parameterID )
{
    if ((parameterID == PARAM_ID_FLUSH) || (parameterID == PARAM_ID_CONFIG_DONE))
    {
        return NULL;
    }
    else
    {
        return ((uint8_t*)pLocalPacket) + LOCAL_PKT_PAYLOAD_OFFSET;
    }
}


/****************************************************************************************************
 * @fn      CopyControlPacketPayload
 *          Copies a Control Packet payload between two buffers, swapping endianness of the elements
 *          if needed, with element number and size determined by control packet type (Parameter ID).
 *
 *          Returns number of bytes copied, or 0 if no bytes copied.
 ***************************************************************************************************/
uint16_t CopyControlPacketPayload( uint8_t *pDest, const uint8_t *pSrc, uint8_t parameterID )
{
    const CtrlPktDesc_t *pCPD = &(controlPacketDescriptions[parameterID]);

    const uint8_t  elemSize    = pCPD->ElementSz;
    const uint16_t nElem       = pCPD->NumElements;
    const uint8_t  isBigEndian = pCPD->IsBigEndian;

    uint16_t nBytesToCopy = 0;

    if (elemSize != 0)
    {
        /* Calc data bytes to copy */
        nBytesToCopy = nElem * (uint16_t) elemSize;

        if (isBigEndian)
        {
            SwapEndianX(pDest, pSrc, elemSize, nElem);
        }
        else
        {
            SH_MEMCPY(pDest, pSrc, nBytesToCopy);
        }
    }

    return nBytesToCopy;
}


/****************************************************************************************************
 * @fn      GetSensorPacketType
 *          Determines Sensor Data packet type from base sensor type and Android/Private flag.
 *
 *          Returns sensor packet type if given a valid sensor type, otherwise a negative error code.
 ***************************************************************************************************/
int32_t GetSensorPacketType( ASensorType_t SensorType )
{
    const ASensorType_t rawSensorType        = AndroidSensorClearPrivateBase( SensorType );

    int32_t sensorPacketTypeOrErrorCode;

    if ( rawSensorType < NUM_SENSOR_TYPE )
    {
        sensorPacketTypeOrErrorCode = (int32_t) SensorTypesToSensorPacketTypes[ (uint8_t) rawSensorType];
    }
    else
    {
        sensorPacketTypeOrErrorCode = SET_ERROR( OSP_STATUS_INVALID_PARAMETER );
    }

    if ( sensorPacketTypeOrErrorCode == SENSOR_DATA_Unimplemented )
    {
        sensorPacketTypeOrErrorCode = SET_ERROR( OSP_STATUS_UNSUPPORTED_FEATURE );
    }

    return sensorPacketTypeOrErrorCode;
}


/****************************************************************************************************
 * @fn      IsWakeupSensorType
 *          Determines if Sensor Type is for a NonWakeup or a Wakeup sensor.
 *
 *          Returns negative error code if sensor type out of range (Android range or Private range,
 *          depending on sensor).  Otherwise returns 0 for Nonwakeup, 1 for Wakeup sensor types.
 ***************************************************************************************************/
int32_t IsWakeupSensorType( ASensorType_t sensorType )
{
    const ASensorType_t rawSensorType        = AndroidSensorClearPrivateBase( sensorType );

    int32_t errorCodeOrResult;

    if ( sensorType >= NUM_SENSOR_TYPE )
    {
        errorCodeOrResult = SET_ERROR( OSP_STATUS_INVALID_PARAMETER );
    }
    else
    {
        errorCodeOrResult = (int32_t) SensorTypeIsWakeup[rawSensorType];
    }

    return errorCodeOrResult;
}


/*=================================================================================================*\
 |  CRC routines
\*=================================================================================================*/

/****************************************************************************************************
 * @fn      Crc16_CCITT
 *          Calculates a 16 bit CRC with the polynomial of X^16 + X^12 + X^5 + 1 (0x1021), seed = 0
 *          and no post processing.
 *
 * @param   [IN]buf - Buffer containing data to check
 * @param   [IN]len - Size of data in buffer
 *
 * @return  16-bit CRC for the data in the buffer, in native Endian format.
 *
 ***************************************************************************************************/
uint16_t Crc16_CCITT(const uint8_t *buf, uint16_t len)
{
    uint16_t x;
    uint16_t crc = 0;

    while( len-- ) {
        x = ((crc>>8) ^ (uint16_t)*buf++) & 0xff;
        x ^= x>>4;
        crc = (crc << 8) ^ (x << 12) ^ (x <<5) ^ x;
    }
    return crc;
}

#if 0
/****************************************************************************************************
 * @fn      CheckPacketCRC
 *          If CRC flag is set in packet, check CRC16-CCITT for the packet and return Pass (0) if CRC
 *          matches the CRC in the packet, or an error code if the CRC doesn't match.
 *          Otherwise if CRC flag is zeroed in the packet, return Pass (0).
 *
 * @param   [IN]pPacket - Buffer containing an existing packet
 * @param   [IN]pktSize - Size of the packet in bytes
 *
 * @return  MQ_SUCCESS if CRC flag in packet is 0 or if CRC check passes, else a negative error code.
 *
 ***************************************************************************************************/
int32_t CheckPacketCRC(const uint8_t *pPacket, uint16_t pktSize)
{
    if (GetCRCFlag(pPacket))
    {
        const uint16_t crcCalc = Crc16_CCITT(pPacket, pktSize - CRC_SIZE);

        const uint16_t crcPacket = BYTES_TO_SHORT( pPacket[pktSize - CRC_SIZE], pPacket[pktSize - CRC_SIZE + 1]);

        if (crcCalc != crcPacket)
        {
            return SET_ERROR( -MQ_CRC_FAILED );
        }
        else
        {
            return MQ_SUCCESS;
        }
    }
    else
    {
        return MQ_SUCCESS;
    }
}
#endif

/****************************************************************************************************
 * @fn      FormatPacketCRC
 *          Set the CRC flag in the packet header, then calculate and append to the packet the
 *          CRC16-CCITT value for the packet exclusive of the appended CRC bytes.
 *
 * @param   [IN]pDest                   - Buffer containing an existing packet
 * @param   [IN]packetSizeIncludingCRC  - Size of the packet in bytes
 *
 * @return  nothing.
 *
 ***************************************************************************************************/
void FormatPacketCRC( HostIFPackets_t *pDest, uint16_t packetSizeIncludingCRC )
{
    uint8_t *pDestUint8 = (uint8_t *) pDest;

    SetCRCFlag( pDestUint8 );  // must be set before calculating CRC

    const uint16_t crcCalc = Crc16_CCITT( pDestUint8, packetSizeIncludingCRC - CRC_SIZE );

    SetCRCField( pDestUint8, packetSizeIncludingCRC, crcCalc );
}


/*=================================================================================================*\
 |  Packet characterization routines
\*=================================================================================================*/
#if 0
/****************************************************************************************************
 * @fn      GetPacketSize
 *          Packet size calculated from packet type and fields as read from packet.
 *
 * @param   [IN]pPacket - Buffer containing an existing packet
 *
 * @return  Size of the packet or if negative -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
void GetPacketSize( const uint8_t *pPacket, uint16_t *pPacketSize, int32_t *pErrorCode )
{
    const uint8_t packetID = GetPacketID(pPacket);

    int32_t errCode = SET_ERROR( -MQ_UNSET_ERROR_CODE );
    uint16_t packetSize;

    packetSize = PKT_BASE_HEADER_SIZE;  //  Control Byte + Sensor ID Byte + Attribute1 Byte

    if (GetCRCFlag(pPacket))
    {
        packetSize += CRC_SIZE;  // CRC, 2 bytes
    }

    switch(packetID)
    {
    case PKID_SENSOR_DATA:
    case PKID_SENSOR_TEST_DATA:
        {
            ASensorType_t sensorType = (ASensorType_t)GetSensorTypeField( pPacket );
            const uint8_t isPrivateNotAndroid  = GetAndroidOrPrivateField( pPacket );
            const uint8_t metadata             = GetMetadata( pPacket );
            const int32_t sensorPacketType     = GetSensorPacketType( sensorType );;

            if ( isPrivateNotAndroid )
            {
                sensorType  = AndroidSensorSetPrivateBase( sensorType );
            }

            if (sensorPacketType < 0)
            {
                errCode = SET_ERROR( sensorPacketType );
            }
            else
            {
                int32_t  payloadSize;

                payloadSize = GetSensorPacketPayloadSize( sensorPacketType, metadata );

                if (payloadSize < 0)
                {
                    errCode  = SET_ERROR( payloadSize );
                }
                else
                {
                    uint8_t  ts_size;

                    ts_size = sensorPacketDescriptions[sensorPacketType].TStampSz == _TS32 ?
                              TIME_STAMP_32_BIT_SIZE_IN_BYTES : TIME_STAMP_64_BIT_SIZE_IN_BYTES;

                    packetSize  += (uint16_t) payloadSize + (uint16_t) ts_size;

                    errCode  = MQ_SUCCESS;
                }
            }
        }
        break;

    case PKID_CONTROL_REQ_RD:
    case PKID_CONTROL_REQ_WR:
    case PKID_CONTROL_RESP:
        {
            int32_t payloadSize = 0;

            uint8_t paramID = GetControlParameterID(pPacket);

            packetSize++;  //  Attribute2 Byte

            payloadSize = GetControlPacketPayloadSize(packetID, paramID);

            if (payloadSize < 0)
            {
                errCode = SET_ERROR( payloadSize );
            }
            else
            {
                packetSize += (uint16_t) payloadSize;

                errCode = MQ_SUCCESS;
            }
        }
        break;

    default:
        errCode = SET_ERROR( -MQ_DEFAULT_IN_SWITCH );
        break;
    }

    if (errCode != MQ_SUCCESS)
    {
        packetSize = 0;
    }

    *pErrorCode   = errCode;
    *pPacketSize  = packetSize;
}


/****************************************************************************************************
 * @fn      GetSensorPacketPayloadSize
 *          Sensor packet payload size calculated from sensor packet type and metadata field.
 *
 * @param   [IN]sensorPacketType - sensor type enum (SENSOR_DATA_RAW, etc.)
 * @param   [IN]metadata         - Metadata field from the packet
 *
 * @return  Size of the packet or if negative -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t GetSensorPacketPayloadSize( uint8_t sensorPacketType, uint8_t metadata ) {

    int32_t payloadSizeOrErrorCode = SET_ERROR( -MQ_UNSET_ERROR_CODE );

    if (sensorPacketType < N_SENSOR_DATA_VALID_PACKET_TYPES)
    {
        const uint8_t  elemSize = sensorPacketDescriptions[sensorPacketType].ElementSz;

        const uint16_t nElem    = sensorPacketDescriptions[sensorPacketType].NumElements;

        uint16_t payloadSize    = nElem * (uint16_t) elemSize;

        if ( SENSOR_DOUBLE_PAYLOAD_SIZE_FOR_METADATA( sensorPacketType, metadata ) )
        {
            /* Special handling for SENSOR_DATA_UNCALIBRATED_FIXP packet */
            /* Check if metadata is available  if it is available then add it to HIF packet */
            /* Add it by doubling the payload size. */

            payloadSize *= 2;  // double the payload size by doubling number of elements.
        }

        payloadSizeOrErrorCode = (int32_t) payloadSize;
    }
    else
    {
        payloadSizeOrErrorCode = SET_ERROR( -MQ_INVALID_PARAMETER );
    }

    return payloadSizeOrErrorCode;
}


/****************************************************************************************************
 * @fn      GetPacketPayloadSize
 *          Packet payload size calculated from fields in the packet.
 *
 * @param   [IN]pPacket - binary packet address
 *
 * @return  Size of the packet or if negative -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t GetPacketPayloadSize( const uint8_t *pPacket )
{
    const uint8_t packetID  = GetPacketID( pPacket );

    if ( ValidSensorPacketID( packetID ) )
    {
        const uint8_t        metaData          = GetMetadata(         pPacket    );
        const ASensorType_t  sensorType        = GetSensorType(       pPacket    );
        const int32_t        sensorPacketType  = GetSensorPacketType( sensorType );

        if ( sensorPacketType < 0 )
        {
            return sensorPacketType;
        }
        else
        {
            return  GetSensorPacketPayloadSize( sensorPacketType, metaData );
        }
    }
    else if ( ValidControlPacketID( packetID ) )
    {
        const uint8_t parameterID  = GetControlParameterID( pPacket );

        return  GetControlPacketPayloadSize( packetID, parameterID );
    }
    else
    {
        return (-MQ_INVALID_PARAMETER);
    }
}


/****************************************************************************************************
 * @fn      GetLocalPacketPayloadElementSizeAndCount
 *          Packet payload size calculated from fields in the packet.
 *
 * @param   [IN]pPacket - binary packet address
 * @param   [OUT]pElementSize - element size
 * @param   [OUT]pElementCount - element count
 *
 * @return  Size of the packet or if negative -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t GetLocalPacketPayloadElementSizeAndCount( const LocalPacketTypes_t *pPacket, uint16_t *pElementSize, uint16_t *pElementCount )
{
    int32_t errorCode = (-MQ_UNSET_ERROR_CODE);

    *pElementSize  = 0;
    *pElementCount = 0;

    if ( IsSensorPacketType( pPacket->PacketID ) )
    {
        const int32_t sensorPacketType = GetSensorPacketType( pPacket->SType );

        if ( sensorPacketType < 0 )
        {
            return sensorPacketType;
        }

        *pElementSize   = sensorPacketDescriptions[sensorPacketType].ElementSz;
        *pElementCount  = sensorPacketDescriptions[sensorPacketType].NumElements;
        errorCode       = MQ_SUCCESS;
    }
    else if ( IsControlPacketType( pPacket->PacketID ) )
    {
        const uint8_t parameterID  = pPacket->SCP.CRP.ParameterID;

        if ( ValidParameterID( parameterID ) )
        {
            *pElementSize   = controlPacketDescriptions[parameterID].ElementSz;
            *pElementCount  = controlPacketDescriptions[parameterID].NumElements;
            errorCode       = MQ_SUCCESS;
        }
        else
        {
            errorCode = (-MQ_INVALID_PARAMETER);
        }
    }
    else
    {
        errorCode = (-MQ_INVALID_PACKETID);
    }

    return errorCode;
}
#endif

/****************************************************************************************************
 * @fn      GetControlPacketPayloadSize
 *          Control packet payload size calculated from packet type (packet ID) and parameter ID.
 *
 * @param   [IN]packetID  - packet ID enum (PKID_CONTROL_REQ_RD, etc.)
 * @param   [IN]paramID   - Parameter ID field from the packet (PARAM_ID_POWER, etc.)
 *
 * @return  Size of the packet or if negative -Error code enum corresponding to the error encountered
 *
 ***************************************************************************************************/
int32_t GetControlPacketPayloadSize( uint8_t packetID, uint8_t paramID )
{
    int32_t packetSizeOrErrorCode = SET_ERROR( OSP_STATUS_INVALID_PARAMETER );

    if ( ValidControlPacketID(packetID) && ValidParameterID(paramID) )
    {
        const uint8_t rW = controlPacketDescriptions[paramID].RWAccess;

        const uint8_t lookupKind = controlPacketSizeKinds[packetID][rW];

        if (lookupKind == PSIZE_ZERO)
        {
            packetSizeOrErrorCode = 0;
        }
        else if (lookupKind == PSIZE_FROM_TABLE)
        {
            const uint8_t  elemSize = controlPacketDescriptions[paramID].ElementSz;

            const uint16_t nElem = controlPacketDescriptions[paramID].NumElements;

            uint16_t payloadSize = nElem * (uint16_t)elemSize;

            packetSizeOrErrorCode = (int32_t)payloadSize;
        }
    }
    // else PSIZE_ILLEGAL, leave error code in place for return value.

    return packetSizeOrErrorCode;
}


/****************************************************************************************************
 * @fn      SetResponsePacketToErrorPacket
 *          Convert Response packet to Error_Code_In_Data Response packet.
 *
 * @param   [IN/OUT]pLocalPacket  - packet to change.
 * @param   [IN]errorCode         - error code to set.
 *
 * @return  nothing.
 *
 ***************************************************************************************************/
void SetResponsePacketToErrorPacket( LocalPacketTypes_t *pLocalPacket, int32_t errorCode )
{
    uint8_t *pPacketPayload;

    pLocalPacket->SCP.CRP.ParameterID = PARAM_ID_ERROR_CODE_IN_DATA;
    pLocalPacket->PayloadOffset = LOCAL_PKT_PAYLOAD_OFFSET;

    pLocalPacket->PayloadSize = controlPacketDescriptions[PARAM_ID_ERROR_CODE_IN_DATA].ElementSz
        * controlPacketDescriptions[PARAM_ID_ERROR_CODE_IN_DATA].NumElements;

    pPacketPayload = pLocalPacket->PayloadOffset + (uint8_t *) &(pLocalPacket);

    SH_MEMCPY( pPacketPayload, (const uint8_t *) &(errorCode), pLocalPacket->PayloadSize );
}


/****************************************************************************************************
 * @fn      IsWriteConfigCommand
 *          Report whether packet is a valid Write Request packet.
 *
 * @param   [IN]packetID  - packet ID enum (PKID_CONTROL_REQ_RD, etc.)
 * @param   [IN]paramID   - Parameter ID field from the packet (PARAM_ID_POWER, etc.)
 *
 * @return  True if valid Write Request, False otherwise.
 *
 ***************************************************************************************************/
uint8_t IsWriteConfigCommand( uint8_t packetID, uint8_t parameterID )
{
    uint8_t isWrite;

    if ( packetID == PKID_CONTROL_REQ_WR && ValidParameterID(parameterID) )
    {
        const uint8_t rW = controlPacketDescriptions[parameterID].RWAccess;

        isWrite  = ( rW == PA_W || rW == PA_RW ) ? TRUE : FALSE;
    }
    else
    {
        isWrite  = FALSE;
    }

    return isWrite;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
