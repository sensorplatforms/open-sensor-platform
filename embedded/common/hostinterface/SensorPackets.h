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
#if !defined (SENSOR_PACKETS_H)
#define   SENSOR_PACKETS_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
 \*-------------------------------------------------------------------------------------------------*/
#define HOST_TARGET_BUILD              0
#define HUB_TARGET_BUILD               (!HOST_TARGET_BUILD)

#include <stdint.h>
#include <stddef.h>

#if (HOST_TARGET_BUILD)
#include "Common.h"
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
 \*-------------------------------------------------------------------------------------------------*/
#define INLINE __inline

/********************************************************/
/*              CONFIGURATION CONSTANTS                 */
/********************************************************/

#define PACKET_SYNC_BYTE_ENABLED             0
#define PACKET_SYNC_BYTE                     0xFA

#define DISABLE_STATE_COMMAND_VALIDATION     0

#define PRINT_QUEUE_SIZES                    1

#define BATCH_MANAGER_REPORT_PACKET_ENQUEUE  0
#define BATCH_MANAGER_REPORT_PACKET_DEQUEUE  0

/*  pack error code with file ID and line number,
 *  if USE_PACKED_ERROR_CODES is defined and error code is negative.
 *
 *  example:  #define MY_FID   FID_SENSOR_PACKETS_C
 *            int32_t errorCode = SET_ERROR( -BATCH_BAD_BUFFER );
 *
 *  packed error code, 32 bits:  11ff ffff  LLLL LLLL  LLLL LLLL  eeee eeee
 *
 *     1 = hardcoded 1's, f = File ID, L = Line Number, e = Error Code.
 */
#define  USE_PACKED_ERROR_CODES

#ifndef USE_PACKED_ERROR_CODES
#define SET_ERROR(A_error)  ((int32_t) (A_error))
#else
#define SET_ERROR(A_error)  \
    (A_error != OSP_STATUS_OK) ? \
        SetErrorCode( ((int32_t) (A_error)), ((uint8_t) (MY_FID)), ((uint16_t) (__LINE__)) ) : ((int32_t) (A_error))
#endif

#define  FID_UNDEFINED                  0
#define  FID_SENSOR_PACKETS_COMMON_C    1
#define  FID_SENSOR_PACKETS_FORMAT_C    2
#define  FID_SENSOR_PACKETS_PARSE_C     3
#define  FID_SENSOR_PACKETS_PRINT_C     4
#define  FID_CONFIG_MANAGER_C           5
#define  FID_BATCH_MANAGER_C            6
#define  NUM_FID_NAMES                  7

/*  This flag defines the CRC flag set when formatting packets.
 *
 *  (When parsing packets, CRC check is always performed (or not),
 *   according to the CRC flag field in the packets,
 *   whether FORMAT_WITH_CRC_ENABLED is set or not.)
 */
#define FORMAT_WITH_CRC_ENABLED        0

/*  Version 1 packets rejected, if Version 1 not supported.
 */
#define PACKET_VERSION_1_SUPPORTED     0

/*  Flags needed to determine if packets are to be byteswapped when parsing or formatting.
 *
 *  Three categories of packet fields are affected:  Timestamp, Payload, and CRC.
 *
 *  (Hub is little-Endian, thus !HOST_TARGET_BUILD in the macro.)
 */
#define DEBUG_SENSOR_PACKETS           1
#define HOST_IS_BIG_ENDIAN             0
#define LOCAL_IS_LITTLE_ENDIAN         ((!HOST_TARGET_BUILD) || (!HOST_IS_BIG_ENDIAN))

#if (DEBUG_SENSOR_PACKETS)
# if (HOST_TARGET_BUILD)
#  define _DEBUG_HOST_TARGET
#  define DPRINTF                       D1_printf
# else
#  define _DEBUG_SENSOR_HUB_TARGET
#  define DPRINTF(...)
# endif
#endif


#define SH_MEMCPY(A_pDest, B_pSrc, C_nBytes)   memcpy( (A_pDest), (B_pSrc), (C_nBytes) )



/********************************************************/
/*              COMMON PACKET DEFINITION                */
/********************************************************/

#define PKT_CONTROL_BYTE_OFFSET         0
#define PKT_SENSOR_ID_BYTE_OFFSET       1
#define PKT_ATTRIBUTE_BYTE1_OFFSET      2
#define PKT_TIMESTAMP_OFFSET            3  // overlayed for Control (AB2), Sensor Data (TS) packets
#define PKT_ATTRIBUTE_BYTE2_OFFSET      3  // overlayed for Control (AB2), Sensor Data (TS) packets
#define PKT_BASE_HEADER_SIZE            3  // min header shared by Control and Sensor pkts
#define CTRL_PKT_HEADER_SIZE            4
#define SENSOR_DATA_PKT_HEADER_SIZE     3

/* Packet Identifier for version 0 packets */
#define PKID_SENSOR_DATA                0x00
#define PKID_CONTROL_REQ_RD             0x01
#define PKID_CONTROL_REQ_WR             0x02
#define PKID_CONTROL_RESP               0x03
#define PKID_SENSOR_TEST_DATA           0x04
#define N_PACKET_IDENTIFIERS            5
#define N_CONTROL_PACKET_IDENTIFIERS    3

#define PKID_MASK_VER0                  0xF0
#define PKID_MASK                       0x70
#define PKID_SHIFT                      4

#define PACKET_VERSION_MASK             0x80

/** =============== CONTROL ID BYTE =============== */
#define SENSOR_TYPE_ANDROID             0x0
#define SENSOR_TYPE_PRIVATE             0x1
/* Checksum option (check sum is always 16-bit CRC if enabled) */
#define PKT_CRC_MASK                    0x08
/** =============== SENSOR ID BYTE ================ */
#define M_SensorMetaData(i)             ((uint8_t)((i << 6) & 0xC0))
#define M_ParseSensorMetaData(i)        ((uint8_t)((i >> 6) & 0x03))
#define M_SensorType(s)                 ((uint8_t)(s & 0x3F))
#define SENSOR_METADATA_MASK            0xC0
#define SENSOR_METADATA_SHIFT           6
#define SENSOR_TYPE_MASK                0x3F

/** =============== ATTRIBUTE BYTE =============== */
#define M_SensorSubType(st)             ((uint8_t)((st << 4) & 0xF0))
#define M_ParseSensorSubType(st)        ((uint8_t)((st >> 4) & 0x0F))
#define SENSOR_SUBTYPE_MASK             0xF0
#define SENSOR_SUBTYPE_SHIFT            4
#define SENSOR_SUBTYPE_UNUSED           0
#define SENSOR_DATA_FLUSH_STATUS_MASK   0x08
#define SENSOR_DATA_FLUSH_STATUS_SHIFT  3
#define CONTROL_SEQUENCE_NUMBER_MASK    0x0F
#define INVALID_SEQUENCE_NUMBER         (-1)

/********************************************************/
/*              SENSOR DATA PACKET                      */
/********************************************************/

#define SENSOR_DATA_RAW                    0  // array start
#define SENSOR_DATA_UNCALIBRATED_FIXP      1
#define SENSOR_DATA_CALIBRATED_FIXP        2
#define SENSOR_DATA_QUATERNION_FIXP        3
#define SENSOR_DATA_ORIENTATION_FIXP       4
#define SENSOR_DATA_THREE_AXIS_FIXP        5
#define SENSOR_DATA_SIGNIFICANT_MOTION     6
#define SENSOR_DATA_STEP_COUNTER           7
#define SENSOR_DATA_STEP_DETECTOR          8
#define SENSOR_DATA_Unimplemented          9
#define N_SENSOR_DATA_PACKET_TYPES        10  // array size
#define N_SENSOR_DATA_VALID_PACKET_TYPES   9

/** =============== CONTROL BYTE =============== */
/*Enumeration type of sensor*/
#define SENSOR_ANDROID_TYPE_MASK        0x01

/* Format for data values */
#define DATA_FORMAT_SENSOR_RAW          0x00    /* Raw counts (no units) from sensor values */
#define DATA_FORMAT_SENSOR_FIXPOINT     0x01    /* Fixed point format in application defined units */
#define DATA_FORMAT_SENSOR_MASK         0x04    /* mask in control byte */
#define DATA_FORMAT_SENSOR_SHIFT        2       /* shift in control byte */

/* Time Format option */
#define TIME_FORMAT_SENSOR_RAW          0x00    /* RAW count values for time stamp base */
#define TIME_FORMAT_SENSOR_FIXPOINT     0x01    /* Fixed point format specified in seconds */
#define TIME_FORMAT_SENSOR_MASK         0x02    /* mask in control byte */
#define TIME_FORMAT_SENSOR_SHIFT        1       /* shift in control byte */

/** ============ SENSOR IDENTIFIER BYTE ========== */
#define META_DATA_UNUSED                0x00    /* Meta Data Identifier  no used*/
#define META_DATA_OFFSET_CHANGE         0x01    /* Meta Data Identifier */

// if MetaData is 0x01 for this sensor packet type, payload size is doubled, for the Offset field.
#define SENSOR_DOUBLE_PAYLOAD_SIZE_FOR_METADATA( A_sensor_packet_type, B_metadata ) \
    ( ( (A_sensor_packet_type) == SENSOR_DATA_UNCALIBRATED_FIXP ) && ( (B_metadata) == META_DATA_OFFSET_CHANGE ) )

/** =============== ATTRIBUTE BYTE =============== */
/* Data size option */
#define DATA_SIZE_8_BIT                 0x00
#define DATA_SIZE_16_BIT                0x01
#define DATA_SIZE_32_BIT                0x02
#define DATA_SIZE_64_BIT                0x03
#define DATA_SIZE_SHIFT                 1
#define DATA_SIZE_MASK                  0x06

/* Time Stamp Size */
#define TIME_STAMP_32_BIT               0x00    /* Uncompressed 32-bit time stamp */
#define TIME_STAMP_64_BIT               0x01    /* Uncompressed 64-bit time stamp */
#define TIME_STAMP_SIZE_MASK            0x01
#define TIME_STAMP_OFFSET               3       /* byte offset in header, sensor data packets */
#define TIME_STAMP_32_BIT_SIZE_IN_BYTES 4
#define TIME_STAMP_64_BIT_SIZE_IN_BYTES 8

/********************************************************/
/*          SENSOR CONTROL REQ/RESP PACKET              */
/********************************************************/

#define CTRL_PKT_CONTROL_BYTE_OFFSET       0
#define CTRL_PKT_ATTRIB_BYTE_1_OFFSET      1
#define CTRL_PKT_ATTRIB_BYTE_2_OFFSET      2

#define CTRL_PKT_PARAMETER_ID_OFFSET       CTRL_PKT_ATTRIB_BYTE_2_OFFSET
#define CTRL_PKT_PAYLOAD_OFFSET            CTRL_PKT_HEADER_SIZE

/** =============== CONTROL BYTE =============== */
/* First 5 MS bits are same as Sensor Data Packet */
/* Data Format */
#define CTRL_PKT_DF_INTEGER             0x00
#define CTRL_PKT_DF_FIXPOINT            0x01
#define CTRL_PKT_DF_FLOAT               0x02
#define CTRL_PKT_DF_DOUBLE              0x03
#define CTRL_PKT_DF_MASK                0x06
#define CTRL_PKT_DF_SHIFT               1

/* A/P Definition same as Sensor Data Packet */

/** =============== ATTRIBUTE BYTE 1 =============== */
/* Sequence Number for request/response */
#define M_SequenceNum(sNum)             ((sNum) & 0x0F)

/** =============== ATTRIBUTE BYTE 2 =============== */
/* Parameter ID */
#define M_GetParamId(AttrByte2)         (AttrByte2)
#define M_SetParamId(id)                (id)

/* Parameter Identifier*/
#define PARAM_ID_ERROR_CODE_IN_DATA     0x00  // array start
#define PARAM_ID_ENABLE                 0x01
#define PARAM_ID_BATCH                  0x02
#define PARAM_ID_FLUSH                  0x03
#define PARAM_ID_RANGE_RESOLUTION       0x04
#define PARAM_ID_POWER                  0x05
#define PARAM_ID_MINMAX_DELAY           0x06
#define PARAM_ID_FIFO_EVT_CNT           0x07
#define PARAM_ID_AXIS_MAPPING           0x08
#define PARAM_ID_CONVERSION_OFFSET      0x09
#define PARAM_ID_CONVERSION_SCALE       0x0A
#define PARAM_ID_SENSOR_NOISE           0x0B
#define PARAM_ID_TIMESTAMP_OFFSET       0x0C
#define PARAM_ID_ONTIME_WAKETIME        0x0D
#define PARAM_ID_HPF_LPF_CUTOFF         0x0E
#define PARAM_ID_SENSOR_NAME            0x0F
#define PARAM_ID_XYZ_OFFSET             0x10
#define PARAM_ID_F_SKOR_MATRIX          0x11
#define PARAM_ID_F_CAL_OFFSET           0x12
#define PARAM_ID_F_NONLINEAR_EFFECTS    0x13
#define PARAM_ID_BIAS_STABILITY         0x14
#define PARAM_ID_REPEATABILITY          0x15
#define PARAM_ID_TEMP_COEFF             0x16
#define PARAM_ID_SHAKE_SUSCEPTIBILITY   0x17
#define PARAM_ID_EXPECTED_NORM          0x18
#define PARAM_ID_VERSION                0x19
#define PARAM_ID_DYNAMIC_CAL_SCALE      0x1A
#define PARAM_ID_DYNAMIC_CAL_SKEW       0x1B
#define PARAM_ID_DYNAMIC_CAL_OFFSET     0x1C
#define PARAM_ID_DYNAMIC_CAL_ROTATION   0x1D
#define PARAM_ID_DYNAMIC_CAL_QUALITY    0x1E
#define PARAM_ID_DYNAMIC_CAL_SOURCE     0x1F
#define PARAM_ID_CONFIG_DONE            0x20
#define PARAM_ID_SH_TIME_SET            0x21
#define PARAM_ID_TIME_SYNC_START        0x22
#define PARAM_ID_TIME_SYNC_FOLLOW_UP    0x23
#define PARAM_ID_TIME_SYNC_END          0x24
#define N_PARAM_ID                      0x25  // array size

/** ================== CRC FIELD =================== */
#define CRC_SIZE                        2     // size in bytes


/* Byte extraction macros */
#define BYTE7(x)                        ((uint8_t)((x >> 56) & 0xFF))
#define BYTE6(x)                        ((uint8_t)((x >> 48) & 0xFF))
#define BYTE5(x)                        ((uint8_t)((x >> 40) & 0xFF))
#define BYTE4(x)                        ((uint8_t)((x >> 32) & 0xFF))
#define BYTE3(x)                        ((uint8_t)((x >> 24) & 0xFF))
#define BYTE2(x)                        ((uint8_t)((x >> 16) & 0xFF))
#define BYTE1(x)                        ((uint8_t)((x >> 8) & 0xFF))
#define BYTE0(x)                        ((uint8_t)(x & 0xFF))

/* 16-bit, 32-bit & 64-bit data macros */
#define BYTES_TO_SHORT(b1,b0)           ((int16_t)(((int16_t)b1 << 8) | b0))

#define BYTES_TO_LONG(b3,b2,b1,b0)      \
    ((int32_t)(((int32_t)b3 << 24) | ((uint32_t)b2 << 16) | ((uint32_t)b1 << 8) | b0))

#define BYTES_TO_LONGLONG(b7,b6,b5,b4,b3,b2,b1,b0)      \
    ((uint64_t)(((uint64_t)b7 << 56) | ((uint64_t)b6 << 48) | ((uint64_t)b5 << 40) | \
    ((uint64_t)b4 << 32) | ((uint64_t)b3 << 24) | ((uint64_t)b2 << 16) | ((uint64_t)b1 << 8) | b0))

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

//! Time Stamp definition for capturing raw time counts
typedef union _TimeStamp {
    uint64_t TS64;
    uint32_t TS32[2];
    uint8_t  TS8[8];
} TimeStamp_t;

//! Generic structure definition for a 3-axis sensor raw values in 2's complement format
typedef struct TriAxisRawData_t
{
    TimeStamp_t  TStamp;
    int32_t      Axis[3];
} TriAxisRawData_t;

/* =============== Local Packet definitions ============= */

/* Definition for Sensor Data packets for internal usage */

typedef struct _QuaternionFixP {
    TimeStamp_t TimeStamp;
    int32_t     Quat[4]; //W,X,Y,Z order
} QuaternionFixP_t;

typedef struct _OrientationFixP {
    TimeStamp_t TimeStamp;
    int32_t     Pitch;
    int32_t     Roll;
    int32_t     Yaw;
} OrientationFixP_t;

typedef struct _ThreeAxisFixP {
    TimeStamp_t TimeStamp;
    int32_t     Axis[3];
    uint8_t     Accuracy;
} ThreeAxisFixP_t;

typedef struct _SignificantMotion {
    TimeStamp_t TimeStamp;
    uint8_t     MotionDetected;
} SignificantMotion_t;

typedef struct _StepCounter {
    TimeStamp_t TimeStamp;
    uint64_t    NumStepsTotal;
} StepCounter_t;

typedef struct _StepDetector {
    TimeStamp_t TimeStamp;
    uint8_t     StepDetected;
} StepDetector_t;

typedef struct _UncalibratedFixP {
    TimeStamp_t TimeStamp;
    int32_t     Axis[3];   //X,Y,Z order
    int32_t     Offset[3]; //XOFF,YOFF,ZOFF order
} UncalibratedFixP_t;

typedef struct _CalibratedFixP {
    TimeStamp_t TimeStamp;
    int32_t     Axis[3]; //X,Y,Z,
} CalibratedFixP_t;

/* Union of the structures that can be parsed out of Sensor Data packet */
typedef union _LocalSensorPktPayloadTypes_s {

    TriAxisRawData_t    RawSensor;
    UncalibratedFixP_t  UncalFixP;
    CalibratedFixP_t    CalFixP;
    QuaternionFixP_t    QuatFixP;
    OrientationFixP_t   OrientFixP;
    ThreeAxisFixP_t     ThreeAxisFixP;
    SignificantMotion_t SigMotion;
    StepCounter_t       StepCount;
    StepDetector_t      StepDetector;

} LocalSensorPktPayloadTypes_t;


typedef struct _LocalSensorPacketTypes_s {

    LocalSensorPktPayloadTypes_t P;

    uint8_t         DataFormatSensor;
    uint8_t         TimeStampFormat;
    uint8_t         Flush;
    uint8_t         DataSize;
    uint8_t         TimeStampSize;

} LocalSensorPacketTypes_t;

#define LOCAL_PACKET_TIMESTAMP_OFFSET  (offsetof( LocalSensorPacketTypes_t, P.UncalFixP.TimeStamp ))


/* Definition for Control Request/Response packets for internal usage */

typedef struct _LocalControlPktNoData_s {
    uint8_t NullData;
} LocalControlPktNoData_t;

typedef struct _LocalControlPktInt8_s {
    int8_t DataI8;
} LocalControlPktInt8_t;

typedef struct _LocalControlPktInt8x3_s {
    int8_t DataI8x3[3];
} LocalControlPktInt8x3_t;

typedef struct _LocalControlPktInt16x2_s {
    int16_t DataI16x2[2];
} LocalControlPktInt16x2_t;

typedef struct _LocalControlPktInt16x3_s {
    int16_t DataI16x3[3];
} LocalControlPktInt16x3_t;

typedef struct _LocalControlPktInt16x3x2_s {
    int16_t DataI16x3x2[3][2];
} LocalControlPktInt16x3x2_t;

typedef struct _LocalControlPktInt16x4x3_s {
    int16_t DataI16x4x3[4][3];
} LocalControlPktInt16x4x3_t;

typedef struct _LocalControlPktInt32_s {
    int32_t DataI32;
} LocalControlPktInt32_t;

typedef struct _LocalControlPktInt32x2_s {
    int32_t DataI32x2[2];
} LocalControlPktInt32x2_t;

typedef struct _LocalControlPktInt32x3_s {
    int32_t DataI32x3[3];
} LocalControlPktInt32x3_t;

typedef struct _LocalControlPktInt32x3x3_s {
    int32_t DataI32x3x3[3][3];
} LocalControlPktInt32x3x3_t;

typedef struct _LocalControlPktUint8_s {
    uint8_t DataU8;
} LocalControlPktUint8_t;

typedef struct _LocalControlPktUint8x32_s {
    uint8_t DataU8x32[32];
} LocalControlPktUint8x32_t;

typedef struct _LocalControlPktUint16x2_s {
    uint16_t DataU16x2[2];
} LocalControlPktUint16x2_t;

typedef struct _LocalControlPktUint32x2_s {
    uint32_t DataU32x2[2];
} LocalControlPktUint32x2_t;

typedef struct _LocalControlPktUint64x2_s {
    uint64_t DataU64x2[2];
} LocalControlPktUint64x2_t;

typedef struct _LocalControlPktUint64_s {
    uint64_t DataU64;
} LocalControlPktUint64_t;


typedef LocalControlPktInt32_t      LocalControlPktErrorCode_t;            // 0x00  PARAM_ID_ERROR_CODE_IN_DATA
typedef LocalControlPktUint8_t      LocalControlPktEnable_t;               // 0x01  PARAM_ID_ENABLE
typedef LocalControlPktUint64x2_t   LocalControlPktBatch_t;                // 0x02  PARAM_ID_BATCH
typedef LocalControlPktNoData_t     LocalControlPktFlush_t;                // 0x03  PARAM_ID_FLUSH
typedef LocalControlPktInt32x2_t    LocalControlPktRangeAndResolution_t;   // 0x04  PARAM_ID_RANGE_RESOLUTION
typedef LocalControlPktInt32_t      LocalControlPktPower_t;                // 0x05  PARAM_ID_POWER
typedef LocalControlPktInt32x2_t    LocalControlPktMinMaxDelay_t;          // 0x06  PARAM_ID_MINMAX_DELAY
typedef LocalControlPktUint32x2_t   LocalControlPktFifoEventCount_t;       // 0x07  PARAM_ID_FIFO_EVT_CNT

typedef LocalControlPktInt8x3_t     LocalControlPktAxisMapping_t;          // 0x08  PARAM_ID_AXIS_MAPPING
typedef LocalControlPktInt32x3_t    LocalControlPktConversionOffset_t;     // 0x09  PARAM_ID_CONVERSION_OFFSET
typedef LocalControlPktInt32x3_t    LocalControlPktConversionScale_t;      // 0x0A  PARAM_ID_CONVERSION_SCALE
typedef LocalControlPktInt32x3_t    LocalControlPktSensorNoise_t;          // 0x0B  PARAM_ID_SENSOR_NOISE
typedef LocalControlPktInt32_t      LocalControlPktTimeStampOffset_t;      // 0x0C  PARAM_ID_TIMESTAMP_OFFSET
typedef LocalControlPktUint32x2_t   LocalControlPktOnTimeWakeTime_t;       // 0x0D  PARAM_ID_ONTIME_WAKETIME
typedef LocalControlPktUint16x2_t   LocalControlPktHpfLpfCutoff_t;         // 0x0E  PARAM_ID_HPF_LPF_CUTOFF
typedef LocalControlPktUint8x32_t   LocalControlPktSensorName_t;           // 0x0F  PARAM_ID_SENSOR_NAME

typedef LocalControlPktInt32x3_t    LocalControlPktXyzOffset_t;            // 0x10  PARAM_ID_XYZ_OFFSET
typedef LocalControlPktInt32x3x3_t  LocalControlPktSkorMatrix_t;           // 0x11  PARAM_ID_F_SKOR_MATRIX
typedef LocalControlPktInt32x3_t    LocalControlPktFCalOffset_t;           // 0x12  PARAM_ID_F_CAL_OFFSET
typedef LocalControlPktInt16x4x3_t  LocalControlPktFNonlinearEffects_t;    // 0x13  PARAM_ID_F_NONLINEAR_EFFECTS
typedef LocalControlPktInt32x3_t    LocalControlPktBiasStability_t;        // 0x14  PARAM_ID_BIAS_STABILITY
typedef LocalControlPktInt32x3_t    LocalControlPktRepeatability_t;        // 0x15  PARAM_ID_REPEATABILITY
typedef LocalControlPktInt16x3x2_t  LocalControlPktTempCoeff_t;            // 0x16  PARAM_ID_TEMP_COEFF
typedef LocalControlPktInt16x3_t    LocalControlPktShakeSusceptibility_t;  // 0x17  PARAM_ID_SHAKE_SUSCEPTIBILITY

typedef LocalControlPktInt32_t      LocalControlPktExpectedNorm_t;         // 0x18  PARAM_ID_EXPECTED_NORM
typedef LocalControlPktUint8x32_t   LocalControlPktVersion_t;              // 0x19  PARAM_ID_VERSION
typedef LocalControlPktInt32x3_t    LocalControlPktDynamicCalScale_t;      // 0x1A  PARAM_ID_DYNAMIC_CAL_SCALE
typedef LocalControlPktInt32x3_t    LocalControlPktDynamicCalSkew_t;       // 0x1B  PARAM_ID_DYNAMIC_CAL_SKEW
typedef LocalControlPktInt32x3_t    LocalControlPktDynamicCalOffset_t;     // 0x1C  PARAM_ID_DYNAMIC_CAL_OFFSET
typedef LocalControlPktInt32x3_t    LocalControlPktDynamicCalRotation_t;   // 0x1D  PARAM_ID_DYNAMIC_CAL_ROTATION
typedef LocalControlPktInt32x3_t    LocalControlPktDynamicCalQuality_t;    // 0x1E  PARAM_ID_DYNAMIC_CAL_QUALITY
typedef LocalControlPktInt8_t       LocalControlPktDynamicCalSource_t;     // 0x1F  PARAM_ID_DYNAMIC_CAL_SOURCE

typedef LocalControlPktInt8_t       LocalControlPktConfigDone_t;           // 0x20  PARAM_ID_CONFIG_DONE

typedef LocalControlPktUint64_t     LocalControlPktSHTimeSet_t;            // 0x21  PARAM_ID_SH_TIME_SET
typedef LocalControlPktNoData_t     LocalControlPktTimeSyncStart_t;        // 0x22  PARAM_ID_TIME_SYNC_START
typedef LocalControlPktUint64_t     LocalControlPktTimeSyncFollowUp_t;     // 0x23  PARAM_ID_TIME_SYNC_FOLLOW_UP
typedef LocalControlPktUint64_t     LocalControlPktTimeSyncEnd_t;          // 0x24  PARAM_ID_TIME_SYNC_END

typedef union _LocalControlPktPayloadTypes_s {

    LocalControlPktErrorCode_t            ErrorCode;
    LocalControlPktEnable_t               Enable;
    LocalControlPktBatch_t                Batch;
    LocalControlPktFlush_t                Flush;
    LocalControlPktRangeAndResolution_t   RangeAndResolution;
    LocalControlPktPower_t                Power;
    LocalControlPktMinMaxDelay_t          MinMaxDelay;
    LocalControlPktFifoEventCount_t       FifoEventCount;

    LocalControlPktAxisMapping_t          AxisMapping;
    LocalControlPktConversionOffset_t     ConversionOffset;
    LocalControlPktConversionScale_t      ConversionScale;
    LocalControlPktSensorNoise_t          SensorNoise;
    LocalControlPktTimeStampOffset_t      TimeStampOffset;
    LocalControlPktOnTimeWakeTime_t       OnTimeWakeTime;
    LocalControlPktHpfLpfCutoff_t         HpfLpfCutoff;
    LocalControlPktSensorName_t           SensorName;

    LocalControlPktXyzOffset_t            XyzOffset;
    LocalControlPktSkorMatrix_t           SkorMatrix;
    LocalControlPktFCalOffset_t           FCalOffset;
    LocalControlPktFNonlinearEffects_t    FNonlinearEffects;
    LocalControlPktBiasStability_t        BiasStability;
    LocalControlPktRepeatability_t        Repeatibility;
    LocalControlPktTempCoeff_t            TempCoeff;
    LocalControlPktShakeSusceptibility_t  ShakeSusceptibility;

    LocalControlPktExpectedNorm_t         ExpectedNorm;
    LocalControlPktVersion_t              Version;
    LocalControlPktDynamicCalScale_t      DynamicCalScale;
    LocalControlPktDynamicCalSkew_t       DynamicCalSkew;
    LocalControlPktDynamicCalOffset_t     DynamicCalOffset;
    LocalControlPktDynamicCalRotation_t   DynamicCalRotation;
    LocalControlPktDynamicCalQuality_t    DynamicCalQuality;
    LocalControlPktDynamicCalSource_t     DynamicCalSource;

    LocalControlPktConfigDone_t           ConfigDone;

    LocalControlPktSHTimeSet_t            HubTimeSet;
    LocalControlPktTimeSyncStart_t        TimeSyncStart;
    LocalControlPktTimeSyncFollowUp_t     TimeSyncFUp;
    LocalControlPktTimeSyncEnd_t          TimeSyncEnd;

} LocalControlPktPayloadTypes_t;


/* Union of the structures that can be parsed out of Control packet */
typedef struct _LocalControlPacketTypes_s {

    LocalControlPktPayloadTypes_t  PL;

    uint8_t         DataFormatControl;
    uint8_t         ParameterID;
    uint8_t         SequenceNumber;

} LocalControlPacketTypes_t;


typedef struct _LocalPacketTypes_s {

    union {
        LocalSensorPacketTypes_t   SDP;
        LocalControlPacketTypes_t  CRP;
    } SCP;

    uint8_t         PacketID;
    uint8_t         Metadata;
    ASensorType_t   SType;      // if Private sensor, this has Private bit set at 0x10000
    uint8_t         SubType;
    int16_t         PayloadSize;

} LocalPacketTypes_t;

#define LOCAL_PKT_PAYLOAD_OFFSET        0



/* ========== Host Interface Packet definitions ========= */
/*
 * WARNING - Do not change structure definition without changing the handlers
 * and the document describing the Host Interface Protocol
 *
 * The structure fields in these packets must remain 8-bit types, to avoid
 * the padding the compiler would insert in between multi-byte fields
 * for type alignment.
 */

/* Sensor Data Host Interface Packet definitions */

/* Sensor Packet Type qualifier */
typedef struct _HifSensorPktQualifier {
    uint8_t ControlByte;
    uint8_t SensorIdByte;
    uint8_t AttributeByte;
} HifSnsrPktQualifier_t;

/* Basic packet: Raw 16-bit data, 32-bit integer time stamp; No checksum */
typedef struct _HifSensorDataRaw {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[4];   //32-bit Time Stamp
    uint8_t DataRaw[6];     //3-Axis Raw 16-bit sensor data
    uint8_t CRCField[CRC_SIZE];
} HifSensorDataRaw_t;

#define SENSOR_RAW_DATA_PKT_SZ          sizeof(HifSensorDataRaw_t)
#define MIN_HIF_SENSOR_DATA_PKT_SZ      (offsetof(HifSensorDataRaw_t, CRCField) - CRC_SIZE) /* Without CRC field */

/* Uncalibrated data packet: 32-bit Fixedpoint uncalibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifUncalibratedFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t Data[12];       //3-Axis Fixed point 32-bit uncalibrated data
    uint8_t Offset[12];     //3-Axis Fixed point 32-bit Offset
    uint8_t CRCField[CRC_SIZE];
} HifUncalibratedFixPoint_t;

#define UNCALIB_FIXP_DATA_PKT_SZ        (offsetof(HifUncalibratedFixPoint_t, Offset))
#define UNCALIB_FIXP_DATA_OFFSET_PKT_SZ sizeof(HifUncalibratedFixPoint_t)

/* Calibrated data packet: 32-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifCalibratedFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t Data[12];       //3-Axis Fixed point 32-bit calibrated data
    uint8_t CRCField[CRC_SIZE];
} HifCalibratedFixPoint_t;

#define CALIBRATED_FIXP_DATA_PKT_SZ     sizeof(HifCalibratedFixPoint_t)

/* Three Axis Fixed Point data packet: 32-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifThreeAxisPktFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t Data[12];       //3-Axis Fixed point 32-bit calibrated data
    uint8_t CRCField[CRC_SIZE];
} HifThreeAxisPktFixPoint_t;

#define THREEAXIS_FIXP_DATA_PKT_SZ     sizeof(HifThreeAxisPktFixPoint_t)

/* SignificantMotion Fixed Point data packet: 32-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifSignificantMotionPktFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t significantMotionDetected;
    uint8_t CRCField[CRC_SIZE];
} HifSignificantMotionPktFixPoint_t;

#define SIGNIFICANTMOTION_FIXP_DATA_PKT_SZ     sizeof(HifSignificantMotionPktFixPoint_t)

/* StepCounter data packet: 64-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifStepCounter {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t NumStepsTotal[8];
    uint8_t CRCField[CRC_SIZE];
} HifStepCounter_t;

#define STEPCOUNTER_DATA_PKT_SZ     sizeof(HifStepCounter_t)

/* StepDetector data packet: 8-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifStepDetector {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t stepDetected;
    uint8_t CRCField[CRC_SIZE];
} HifStepDetector_t;

#define STEPDETECTOR_DATA_PKT_SZ     sizeof(HifStepDetector_t)

/* Orientation Fixed Point data packet: 32-bit Fixedpoint Calibrated; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifOrientationPktFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];                           //!< Time in seconds
    uint8_t Pitch[4];                           //!< pitch in degrees
    uint8_t Roll[4];                            //!< roll in degrees
    uint8_t Yaw[4];                             //!< yaw in degrees
    uint8_t CRCField[CRC_SIZE];
} HifOrientationPktFixPoint_t;

#define ORIENTATION_FIXP_DATA_PKT_SZ     sizeof(HifOrientationPktFixPoint_t)

/* Quaternion data packet: 32-bit Fixedpoint quaternion; 64-bit Fixedpoint time stamp; No Checksum */
typedef struct _HifQuaternionFixPoint {
    HifSnsrPktQualifier_t Q;
    uint8_t TimeStamp[8];   //64-bit Time Stamp in fixed point format
    uint8_t Data[16];       //4-Axis Fixed point 32-bit quaternion data
    uint8_t CRCField[CRC_SIZE];
} HifQuaternionFixPoint_t;

#define QUATERNION_FIXP_DATA_PKT_SZ     sizeof(HifQuaternionFixPoint_t)



/* Control Request/Response Host Interface Packet definitions */

typedef struct _HifControlPktNoData_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    uint8_t CRCField[CRC_SIZE];
} HifControlPktNoData_t;

#define MIN_HIF_CONTROL_PKT_SZ      (offsetof(HifControlPktNoData_t, CRCField) - CRC_SIZE) /* Without CRC field */

typedef struct _HifControlPktInt8_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t DataI8;
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt8_t;

typedef struct _HifControlPktInt8x3_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t DataI8x3[3];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt8x3_t;

typedef struct _HifControlPktInt16x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI16x2[4];         // int16_t DataI16x2[2]
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt16x2_t;

typedef struct _HifControlPktInt16x3_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI16x3[6];         // int16_t DataI16x3[3]
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt16x3_t;

typedef struct _HifControlPktInt16x3x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI16x3x2[12];        // int16_t DataI16x3x2[3][2]
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt16x3x2_t;

typedef struct _HifControlPktInt16x4x3_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI16x4x3[24];        // int16_t DataI16x4x3[4][3]
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt16x4x3_t;

typedef struct _HifControlPktInt32_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI32[4];         // int32_t DataI32
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt32_t;

typedef struct _HifControlPktInt32x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI32x2[8];         // int32_t DataI32x2[2];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt32x2_t;

typedef struct _HifControlPktInt32x3_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI32x3[12];        // int32_t DataI32x3[3];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt32x3_t;

typedef struct _HifControlPktInt32x3x3_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataI32x3x3[36];        // int32_t DataI32x3x3[3][3];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktInt32x3x3_t;

typedef struct _HifControlPktUint8_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    uint8_t DataU8;
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint8_t;

typedef struct _HifControlPktUint8x32_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    uint8_t DataU8x32[32];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint8x32_t;

typedef struct _HifControlPktUint16x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataU16x2[4];         // uint16_t DataU16[2];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint16x2_t;

typedef struct _HifControlPktUint32x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataU32x2[8];         // uint32_t DataU32[2];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint32x2_t;

typedef struct _HifControlPktUint64x2_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataU64x2[16];         // uint64_t DataU64[2];
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint64x2_t;

typedef struct _HifControlPktUint64_s {
    HifSnsrPktQualifier_t Q;
    uint8_t AttrByte2;
    int8_t  DataU64[8];         // uint64_t DataU64;
    uint8_t CRCField[CRC_SIZE];
} HifControlPktUint64_t;

typedef HifControlPktInt32_t       HifControlPktErrorCode_t;            // 0x00  PARAM_ID_ERROR_CODE_IN_DATA
typedef HifControlPktUint8_t       HifControlPktEnable_t;               // 0x01  PARAM_ID_ENABLE
typedef HifControlPktUint64x2_t    HifControlPktBatch_t;                // 0x02  PARAM_ID_BATCH
typedef HifControlPktNoData_t      HifControlPktFlush_t;                // 0x03  PARAM_ID_FLUSH
typedef HifControlPktInt32x2_t     HifControlPktRangeAndResolution_t;   // 0x04  PARAM_ID_RANGE_RESOLUTION
typedef HifControlPktInt32_t       HifControlPktPower_t;                // 0x05  PARAM_ID_POWER
typedef HifControlPktInt32x2_t     HifControlPktMinMaxDelay_t;          // 0x06  PARAM_ID_MINMAX_DELAY
typedef HifControlPktUint32x2_t    HifControlPktFifoEventCount_t;       // 0x07  PARAM_ID_FIFO_EVT_CNT

typedef HifControlPktInt8x3_t      HifControlPktAxisMapping_t;          // 0x08  PARAM_ID_AXIS_MAPPING
typedef HifControlPktInt32x3_t     HifControlPktConversionOffset_t;     // 0x09  PARAM_ID_CONVERSION_OFFSET
typedef HifControlPktInt32x3_t     HifControlPktConversionScale_t;      // 0x0A  PARAM_ID_CONVERSION_SCALE
typedef HifControlPktInt32x3_t     HifControlPktSensorNoise_t;          // 0x0B  PARAM_ID_SENSOR_NOISE
typedef HifControlPktInt32_t       HifControlPktTimeStampOffset_t;      // 0x0C  PARAM_ID_TIMESTAMP_OFFSET
typedef HifControlPktUint32x2_t    HifControlPktOnTimeWakeTime_t;       // 0x0D  PARAM_ID_ONTIME_WAKETIME
typedef HifControlPktUint16x2_t    HifControlPktHpfLpfCutoff_t;         // 0x0E  PARAM_ID_HPF_LPF_CUTOFF
typedef HifControlPktUint8x32_t    HifControlPktSensorName_t;           // 0x0F  PARAM_ID_SENSOR_NAME

typedef HifControlPktInt32x3_t     HifControlPktXyzOffset_t;            // 0x10  PARAM_ID_XYZ_OFFSET
typedef HifControlPktInt32x3x3_t   HifControlPktSkorMatrix_t;           // 0x11  PARAM_ID_F_SKOR_MATRIX
typedef HifControlPktInt32x3_t     HifControlPktFCalOffset_t;           // 0x12  PARAM_ID_F_CAL_OFFSET
typedef HifControlPktInt16x4x3_t   HifControlPktFNonlinearEffects_t;    // 0x13  PARAM_ID_F_NONLINEAR_EFFECTS
typedef HifControlPktInt32x3_t     HifControlPktBiasStability_t;        // 0x14  PARAM_ID_BIAS_STABILITY
typedef HifControlPktInt32x3_t     HifControlPktRepeatability_t;        // 0x15  PARAM_ID_REPEATABILITY
typedef HifControlPktInt16x3x2_t   HifControlPktTempCoeff_t;            // 0x16  PARAM_ID_TEMP_COEFF
typedef HifControlPktInt16x3_t     HifControlPktShakeSusceptibility_t;  // 0x17  PARAM_ID_SHAKE_SUSCEPTIBILITY

typedef HifControlPktInt32_t       HifControlPktExpectedNorm_t;         // 0x18  PARAM_ID_EXPECTED_NORM
typedef HifControlPktUint8x32_t    HifControlPktVersion_t;              // 0x19  PARAM_ID_VERSION
typedef HifControlPktInt32x3_t     HifControlPktDynamicCalScale_t;      // 0x1A  PARAM_ID_DYNAMIC_CAL_SCALE
typedef HifControlPktInt32x3_t     HifControlPktDynamicCalSkew_t;       // 0x1B  PARAM_ID_DYNAMIC_CAL_SKEW
typedef HifControlPktInt32x3_t     HifControlPktDynamicCalOffset_t;     // 0x1C  PARAM_ID_DYNAMIC_CAL_OFFSET
typedef HifControlPktInt32x3_t     HifControlPktDynamicCalRotation_t;   // 0x1D  PARAM_ID_DYNAMIC_CAL_ROTATION
typedef HifControlPktInt32x3_t     HifControlPktDynamicCalQuality_t;    // 0x1E  PARAM_ID_DYNAMIC_CAL_QUALITY
typedef HifControlPktInt8_t        HifControlPktDynamicCalSource_t;     // 0x1F  PARAM_ID_DYNAMIC_CAL_SOURCE

typedef HifControlPktNoData_t      HifControlPktConfigDone_t;           // 0x20  PARAM_ID_CONFIG_DONE

typedef HifControlPktUint64_t      HifControlPktSHTimeSet_t;            // 0x21  PARAM_ID_SH_TIME_SET
typedef HifControlPktNoData_t      HifControlPktTimeSyncStart_t;        // 0x22  PARAM_ID_TIME_SYNC_START
typedef HifControlPktUint64_t      HifControlPktTimeSyncFollowUp_t;     // 0x23  PARAM_ID_TIME_SYNC_FOLLOW_UP
typedef HifControlPktUint64_t      HifControlPktTimeSyncEnd_t;          // 0x24  PARAM_ID_TIME_SYNC_END



/* Define union for all the host interface packet types */

typedef union _HostIFPackets {

    HifSensorDataRaw_t                  SensPktRaw;
    HifUncalibratedFixPoint_t           UncalPktFixP;
    HifCalibratedFixPoint_t             CalPktFixP;
    HifQuaternionFixPoint_t             QuatPktFixP;
    HifOrientationPktFixPoint_t         OrientationFixP;
    HifThreeAxisPktFixPoint_t           ThreeAxisFixp;
    HifSignificantMotionPktFixPoint_t   SignificantMotion;
    HifStepCounter_t                    StepCounter;
    HifStepDetector_t                   StepDetector;

    HifControlPktErrorCode_t            ErrorCode;
    HifControlPktEnable_t               Enable;
    HifControlPktBatch_t                Batch;
    HifControlPktFlush_t                Flush;
    HifControlPktRangeAndResolution_t   RangeAndResolution;
    HifControlPktPower_t                Power;
    HifControlPktMinMaxDelay_t          MinMaxDelay;
    HifControlPktFifoEventCount_t       FifoEventCount;

    HifControlPktAxisMapping_t          AxisMapping;
    HifControlPktConversionOffset_t     ConversionOffset;
    HifControlPktConversionScale_t      ConversionScale;
    HifControlPktSensorNoise_t          SensorNoise;
    HifControlPktTimeStampOffset_t      TimeStampOffset;
    HifControlPktOnTimeWakeTime_t       OnTimeWakeTime;
    HifControlPktHpfLpfCutoff_t         HpfLpfCutoff;
    HifControlPktSensorName_t           SensorName;

    HifControlPktXyzOffset_t            XyzOffset;
    HifControlPktSkorMatrix_t           SkorMatrix;
    HifControlPktFCalOffset_t           FCalOffset;
    HifControlPktFNonlinearEffects_t    FNonlinearEffects;
    HifControlPktBiasStability_t        BiasStability;
    HifControlPktRepeatability_t        Repeatibility;
    HifControlPktTempCoeff_t            TempCoeff;
    HifControlPktShakeSusceptibility_t  ShakeSusceptibility;

    HifControlPktExpectedNorm_t         ExpectedNorm;
    HifControlPktVersion_t              Version;
    HifControlPktDynamicCalScale_t      DynamicCalScale;
    HifControlPktDynamicCalSkew_t       DynamicCalSkew;
    HifControlPktDynamicCalOffset_t     DynamicCalOffset;
    HifControlPktDynamicCalRotation_t   DynamicCalRotation;
    HifControlPktDynamicCalQuality_t    DynamicCalQuality;
    HifControlPktDynamicCalSource_t     DynamicCalSource;

    HifControlPktConfigDone_t           ConfigDone;

    HifControlPktSHTimeSet_t            HubTimeSet;
    HifControlPktTimeSyncStart_t        TimeSyncStart;
    HifControlPktTimeSyncFollowUp_t     TimeSyncFUp;
    HifControlPktTimeSyncEnd_t          TimeSyncEnd;

    HifControlPktNoData_t               GenericControlPkt;

} HostIFPackets_t;



/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

void SetResponsePacketToErrorPacket( LocalPacketTypes_t *pLocalPacket, int32_t errorCode );

/*  CRC routines  */

// returns CRC16-CCITT of the given buffer
uint16_t Crc16_CCITT(const unsigned char *buf, uint16_t len);

//  set CRC flag in packet, calculate CRC16-CCITT of resulting packet, and append CRC to packet.
void FormatPacketCRC( HostIFPackets_t *pDest, uint16_t packetSizeIncludingCRC );


/* Packet characterization routines
 * Various packet sizes in bytes.  on error, each routine returns a negative error code.
 */
int32_t GetSensorPacketPayloadSize(  uint8_t sensorPacketType, uint8_t metadata );
int32_t GetControlPacketPayloadSize( uint8_t packetID, uint8_t paramID          );
int32_t GetPacketPayloadSize(        const uint8_t *pPacket                     );

//  returns negative error code if sensor type out of range.
//  Otherwise returns 0 for Nonwakeup, 1 for Wakeup sensor types.
int32_t IsWakeupSensorType( uint32_t sensorType );

static INLINE uint8_t IsPrivateSensorType( uint32_t sensorType )
{
    return (sensorType & SENSOR_DEVICE_PRIVATE_BASE) ? (uint8_t) 1 : (uint8_t) 0;
};


/*******************************************************************************************
 *  Packet parsing routines
 *
 *  Convert bitpacked/Big-Endian Host Interface Packet types to local unpacked packet types.
 *
 *  If local processor is little-endian, byte swapping will be performed where needed.
 *******************************************************************************************/

int32_t ParseHostInterfacePkt( LocalPacketTypes_t *pOut, const uint8_t *pPacket, uint16_t *pPktSizeByType, uint16_t pktBufferSize );



/*******************************************************************************************
 *  Packet formatting routines
 *
 *  Convert local unpacked packet types to bitpacked/Big-Endian Host Interface Packet types
 *  for bus transfer between processors.
 *
 *  If local processor is little-endian, byte swapping will be performed where needed.
 *******************************************************************************************/

/*  format whatever packet type is represented in the source packet. */

int16_t FormatGenericPacket( HostIFPackets_t *pDest, const LocalPacketTypes_t *pSrc );

/*  Sensor Data packet formatting routines */

int32_t FormatSensorDataPacket(
    HostIFPackets_t *pDest, const uint8_t *pSrc,
    uint8_t sensorPacketType, uint8_t metaData,
    ASensorType_t sType, uint8_t subType );

int32_t FormatSensorDataPktRaw(
    HostIFPackets_t *pDest, const TriAxisRawData_t *pSensData,
    uint8_t metaData, ASensorType_t sType, uint8_t subType );

int32_t FormatQuaternionPktFixP(
    HostIFPackets_t *pDest, const QuaternionFixP_t *pQuatData,
    ASensorType_t sType );

int32_t FormatUncalibratedPktFixP(
    HostIFPackets_t *pDest, const UncalibratedFixP_t *pUncalData,
    uint8_t metaData, ASensorType_t sType );

int32_t FormatCalibratedPktFixP(
    HostIFPackets_t *pDest, const CalibratedFixP_t *pCalData,
    ASensorType_t sType );

int32_t FormatStepDetectorPkt(
    HostIFPackets_t *pDest, const StepDetector_t *pStepDetectorData,
    ASensorType_t sType );

// TODO from FormatStepCounterPkt body:  //TODO : Find out when to use the preciseData
int32_t FormatStepCounterPkt(
    HostIFPackets_t *pDest, const StepCounter_t *pStepCounterData,
    ASensorType_t sType );

// TODO from FormatThreeAxisPktFixP body:  //TODO : Find out when to use the preciseData
// TODO from FormatThreeAxisPktFixP body:  TODO:  how to handle accuracy?
int32_t FormatThreeAxisPktFixP(
    HostIFPackets_t *pDest, const ThreeAxisFixP_t *p3AxisData,
    ASensorType_t sType );

int32_t FormatSignificantMotionPktFixP(
    HostIFPackets_t *pDest, const SignificantMotion_t *pSignificantMotionData,
    ASensorType_t sType );

int32_t FormatOrientationFixP(
    HostIFPackets_t *pDest, const OrientationFixP_t *pOrientationData,
    ASensorType_t sType );

int32_t FormatFlushCompletePacket( HostIFPackets_t *pDestPacket, ASensorType_t sensorType );


/*  Control Request/Response packet formatting routines, ordered by Parameter ID */

int32_t FormatControlPacket(
        HostIFPackets_t *pDest, const LocalPacketTypes_t *pLocalPacket,
        uint16_t *pDestPacketSize );

//   PARAM_ID_ERROR_CODE_IN_DATA     0x00    R_: Int32
//
int32_t FormatControlResp_ErrorCodeInData(
    HostIFPackets_t *pDest, int32_t errorCodeInData,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_ENABLE                 0x01    _W: Bool
//
int32_t FormatControlReqWrite_Enable(
    HostIFPackets_t *pDest, uint8_t enable,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_BATCH                  0x02    _W: Uint64 x 2
//
int32_t FormatControlReqWrite_Batch(
    HostIFPackets_t *pDest, const uint64_t *pUint64x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_FLUSH                  0x03    _W: no payload
//
int32_t FormatControlReqWrite_Flush(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_RANGE_RESOLUTION       0x04    R_: FixP32 x 2
//
int32_t FormatControlReqRead_RangeResolution(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_RangeResolution(
    HostIFPackets_t *pDest, const int32_t *pInt32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_POWER                  0x05    R_: FixP32
//
int32_t FormatControlReqRead_Power(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_Power(
    HostIFPackets_t *pDest, int32_t iPower,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_MINMAX_DELAY           0x06    R_: Int32 x 2
//
int32_t FormatControlReqRead_MinMaxDelay(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_MinMaxDelay(
    HostIFPackets_t *pDest, const int32_t *pInt32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_FIFO_EVT_CNT           0x07    R_: Uint32 x 2
//
int32_t FormatControlReqRead_FifoEventCounts(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_FifoEventCounts(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_AXIS_MAPPING           0x08    RW: Int8 x 3
//
int32_t FormatControlReqRead_AxisMapping(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_AxisMapping(
    HostIFPackets_t *pDest, const int8_t *pInt8x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_AxisMapping(
    HostIFPackets_t *pDest, const int8_t *pInt8x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_CONVERSION_OFFSET      0x09    RW: Int32 x 3
//
int32_t FormatControlReqRead_ConversionOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_ConversionOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_ConversionOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_CONVERSION_SCALE       0x0A    RW: FixP32 x 3
//
int32_t FormatControlReqRead_ConversionScale(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_ConversionScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_ConversionScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_SENSOR_NOISE           0x0B    RW: FixP32 x 3
//
int32_t FormatControlReqRead_SensorNoise(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_SensorNoise(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_SensorNoise(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_TIMESTAMP_OFFSET       0x0C    RW: FixP32
//
int32_t FormatControlReqRead_TimeStampOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_TimeStampOffset(
    HostIFPackets_t *pDest, int32_t iTimeStampOffset,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_TimeStampOffset(
    HostIFPackets_t *pDest, int32_t iTimeStampOffset,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_ONTIME_WAKETIME        0x0D    RW: Uint32 x 2
//
int32_t FormatControlReqRead_OnTimeWakeTime(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_OnTimeWakeTime(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_OnTimeWakeTime(
    HostIFPackets_t *pDest, const uint32_t *pUint32x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_HPF_LPF_CUTOFF         0x0E    RW: Uint16 x 2
//
int32_t FormatControlReqRead_HpfLpfCutoff(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_HpfLpfCutoff(
    HostIFPackets_t *pDest, const uint16_t *pUint16x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_HpfLpfCutoff(
    HostIFPackets_t *pDest, const uint16_t *pUint16x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_SENSOR_NAME            0x0F    R_: Char x 32
//
int32_t FormatControlReqRead_SensorName(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_SensorName(
    HostIFPackets_t *pDest, const uint32_t *pUint8x32,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_XYZ_OFFSET             0x10    RW: FixP32 x 3
//
int32_t FormatControlReqRead_XyzOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_XyzOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_XyzOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_F_SKOR_MATRIX          0x11    RW: FixP32 x 3 x 3
//
int32_t FormatControlReqRead_SkorMatrix(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_SkorMatrix(
    HostIFPackets_t *pDest, const int32_t *pInt32x3x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_SkorMatrix(
    HostIFPackets_t *pDest, const int32_t *pInt32x3x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_F_CAL_OFFSET           0x12    RW: FixP32 x 3
//
int32_t FormatControlReqRead_FCalOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_FCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_FCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_F_NONLINEAR_EFFECTS    0x13    RW: FixP16 x 4 x 3
//
int32_t FormatControlReqRead_FNonlinearEffects(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_FNonlinearEffects(
    HostIFPackets_t *pDest, const int16_t *pInt16x4x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_FNonlinearEffects(
    HostIFPackets_t *pDest, const int16_t *pInt16x4x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_BIAS_STABILITY         0x14    RW: FixP32 x 3
//
int32_t FormatControlReqRead_BiasStability(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_BiasStability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_BiasStability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_REPEATABILITY          0x15    RW: FixP32 x 3
//
int32_t FormatControlReqRead_Repeatability(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_Repeatability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_Repeatability(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_TEMP_COEFF             0x16    RW: FixP16 x 3 x 2
//
int32_t FormatControlReqRead_TempCoeff(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_TempCoeff(
    HostIFPackets_t *pDest, const int16_t *pInt16x3x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_TempCoeff(
    HostIFPackets_t *pDest, const int16_t *pInt16x3x2,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_SHAKE_SUSCEPTIBILITY   0x17    RW: FixP16 x 3
//
int32_t FormatControlReqRead_ShakeSusceptibility(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_ShakeSusceptibility(
    HostIFPackets_t *pDest, const int16_t *pInt16x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_ShakeSusceptibility(
    HostIFPackets_t *pDest, const int16_t *pInt16x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_EXPECTED_NORM          0x18    RW: FixP32
//
int32_t FormatControlReqRead_ExpectedNorm(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_ExpectedNorm(
    HostIFPackets_t *pDest, int32_t iExpectedNorm,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_ExpectedNorm(
    HostIFPackets_t *pDest, int32_t iExpectedNorm,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_VERSION                0x19    R_: Char x 32
//
int32_t FormatControlReqRead_Version(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_Version(
    HostIFPackets_t *pDest, const uint32_t *pUint8x32,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_SCALE      0x1A    RW: FixP32 x 3
//
int32_t FormatControlReqRead_DynamicCalScale(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalScale(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_SKEW       0x1B    RW: FixP32 x 3
//
int32_t FormatControlReqRead_DynamicCalSkew(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalSkew(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalSkew(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_OFFSET     0x1C    RW: FixP32 x 3
//
int32_t FormatControlReqRead_DynamicCalOffset(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalOffset(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_ROTATION   0x1D    RW: FixP32 x 3
//
int32_t FormatControlReqRead_DynamicCalRotation(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalRotation(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalRotation(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_QUALITY    0x1E    RW: FixP32 x 3
//
int32_t FormatControlReqRead_DynamicCalQuality(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalQuality(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalQuality(
    HostIFPackets_t *pDest, const int32_t *pInt32x3,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_DYNAMIC_CAL_SOURCE     0x1F    RW: Int8
//
int32_t FormatControlReqRead_DynamicCalSource(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlReqWrite_DynamicCalSource(
    HostIFPackets_t *pDest, int8_t iDynamicCalSource,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

int32_t FormatControlResp_DynamicCalSource(
    HostIFPackets_t *pDest, int8_t iDynamicCalSource,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_CONFIG_DONE            0x20        _W: no payload
//
int32_t FormatControlReqWrite_ConfigDone(
    HostIFPackets_t *pDest,
    ASensorType_t sType, uint8_t subType,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_SH_TIME_SET            0x21        _W
//
int32_t FormatControlReqWrite_ShTimeSet(
    HostIFPackets_t *pDest, uint64_t nsTime,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_TIME_SYNC_START        0x22        _W
//
int32_t FormatControlReqWrite_TimeSyncStart(
    HostIFPackets_t *pDest,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_TIME_SYNC_FOLLOW_UP    0x23        _W
//
int32_t FormatControlReqWrite_TimeSyncFUp(
    HostIFPackets_t *pDest, uint64_t nsTime,
    uint8_t seqNum, uint8_t crcFlag );

//   PARAM_ID_TIME_SYNC_END          0x24        _W
//
int32_t FormatControlReqWrite_TimeSyncEnd(
    HostIFPackets_t *pDest, uint64_t nsTime,
    uint8_t seqNum, uint8_t crcFlag );


/*******************************************************************************************
 *  Packet field unpacking/packing routines
 ******************************************************************************************/

//  Control Byte
//    Packet Version
//    Packet ID
//    CRC flag
//    Data Format (Sensor  packets, 1 bit )
//    Data Format (Control packets, 2 bits)
//    Timestamp Format (Sensor packets)
//    Android/Private Sensor flag
//
static INLINE uint8_t GetPacketVersion(const uint8_t *pPacket) {

    return ((pPacket[PKT_CONTROL_BYTE_OFFSET]) & PACKET_VERSION_MASK) != 0;
};

static INLINE uint8_t GetPacketID(const uint8_t *pPacket) {

    return ((pPacket[PKT_CONTROL_BYTE_OFFSET]) & PKID_MASK) >> PKID_SHIFT;
};

static INLINE void SetPacketID(uint8_t *pPacket, uint8_t packetID) {

    *pPacket = ((*pPacket) & ~PKID_MASK) | ((packetID << PKID_SHIFT) & PKID_MASK);
};

static INLINE uint8_t ValidPacketID(uint8_t packetID)
{
    return (packetID <= PKID_SENSOR_TEST_DATA);  // no lower-bound check because uint
};

static INLINE uint8_t ValidSensorPacketID(uint8_t packetID)
{
    return packetID == PKID_SENSOR_DATA || packetID == PKID_SENSOR_TEST_DATA;
};

static INLINE uint8_t ValidControlPacketID(uint8_t packetID)
{
    return packetID >= PKID_CONTROL_REQ_RD && packetID <= PKID_CONTROL_RESP;
};

#define IsSensorPacketType  ValidSensorPacketID
#define IsControlPacketType ValidControlPacketID

uint8_t IsWriteConfigCommand( uint8_t packetID, uint8_t parameterID );


static INLINE uint8_t GetCRCFlag(const uint8_t *pPacket)
{
    return pPacket[PKT_CONTROL_BYTE_OFFSET] & PKT_CRC_MASK ? 1 : 0;
};

static INLINE void SetCRCFlag(uint8_t *pPacket)
{
    pPacket[PKT_CONTROL_BYTE_OFFSET] |= PKT_CRC_MASK;
};

static INLINE uint8_t GetDataFormatSensor(const uint8_t *pPacket)
{
    return ((pPacket[PKT_CONTROL_BYTE_OFFSET] & DATA_FORMAT_SENSOR_MASK) != 0);
};

static INLINE void SetDataFormatSensor(uint8_t *pPacket, uint8_t dataFormat)
{
    pPacket[PKT_CONTROL_BYTE_OFFSET] |= (dataFormat & DATA_FORMAT_SENSOR_MASK);
};

static INLINE uint8_t GetTimeFormatSensor(const uint8_t *pPacket)
{
    return ((pPacket[PKT_CONTROL_BYTE_OFFSET] & TIME_FORMAT_SENSOR_MASK) != 0);
};

static INLINE void SetTimeFormatSensor(uint8_t *pPacket, uint8_t tsFormat)
{
    pPacket[PKT_CONTROL_BYTE_OFFSET] |= (tsFormat & TIME_FORMAT_SENSOR_MASK);
};

static INLINE uint8_t GetDataFormatControl(const uint8_t *pPacket)
{
    return ((pPacket[PKT_CONTROL_BYTE_OFFSET] & CTRL_PKT_DF_MASK) >> CTRL_PKT_DF_SHIFT);
};

static INLINE void SetDataFormatControl(uint8_t *pPacket, uint8_t dataFormat)
{
    pPacket[PKT_CONTROL_BYTE_OFFSET] |= (dataFormat << CTRL_PKT_DF_SHIFT) & CTRL_PKT_DF_MASK;
};

static INLINE uint8_t GetAndroidOrPrivateField(const uint8_t *pPacket)
{
    return ((pPacket[PKT_CONTROL_BYTE_OFFSET] & SENSOR_TYPE_PRIVATE) != 0);
};

static INLINE void SetPrivateField(uint8_t *pPacket)
{
    pPacket[PKT_CONTROL_BYTE_OFFSET] |= SENSOR_TYPE_PRIVATE;
};


//  Sensor Identifier Byte
//    Metadata
//    Sensor Type
//
static INLINE uint8_t GetMetadata(const uint8_t *pPacket)
{
    return ((pPacket[PKT_SENSOR_ID_BYTE_OFFSET] & SENSOR_METADATA_MASK) >> SENSOR_METADATA_SHIFT);
};

static INLINE void SetMetadata(uint8_t *pPacket, uint8_t metaData)
{
    pPacket[PKT_SENSOR_ID_BYTE_OFFSET] |= (metaData << SENSOR_METADATA_SHIFT) & SENSOR_METADATA_MASK;
};

static INLINE uint32_t GetSensorTypeField(const uint8_t *pPacket)
{
    return (uint32_t) (pPacket[PKT_SENSOR_ID_BYTE_OFFSET] & SENSOR_TYPE_MASK);
};

static INLINE void SetSensorTypeField(uint8_t *pPacket, uint8_t sensorType) // Sensor Data packets only
{
    pPacket[PKT_SENSOR_ID_BYTE_OFFSET] |= sensorType & SENSOR_TYPE_MASK;
};



//  Attribute Byte 1
//    Sensor Subtype
//    Flush flag      (Sensor packets)
//    Data Size       (Sensor packets)
//    Timestamp Size  (Sensor packets)
//    Sequence Number (Control packets)
//
static INLINE uint8_t GetSensorSubType(const uint8_t *pPacket)  // Sensor Data or Control packets
{
    return ((pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] & SENSOR_SUBTYPE_MASK) >> SENSOR_SUBTYPE_SHIFT);
};

static INLINE void SetSensorSubType(uint8_t *pPacket, uint8_t subType) // Sensor Data packets only
{
    pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] |= (subType << SENSOR_SUBTYPE_SHIFT) & SENSOR_SUBTYPE_MASK;
};

static INLINE uint8_t GetSensorDataFlushStatus(const uint8_t *pPacket) // Sensor Data packets only
{
    return ((pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] & SENSOR_DATA_FLUSH_STATUS_MASK) >> SENSOR_DATA_FLUSH_STATUS_SHIFT);
};

static INLINE void SetSensorDataFlushStatus(uint8_t *pPacket, uint8_t dataFlush) // Sensor Data packets only
{
    pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] |= (dataFlush << SENSOR_DATA_FLUSH_STATUS_SHIFT) & SENSOR_DATA_FLUSH_STATUS_MASK;
};

static INLINE uint8_t GetSensorDataDataSize(const uint8_t *pPacket) // Sensor Data packets only
{
    return ((pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] & DATA_SIZE_MASK) >> DATA_SIZE_SHIFT);
};

static INLINE void SetSensorDataDataSize(uint8_t *pPacket, uint8_t dataSize) // Sensor Data packets only
{
    pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] |= (dataSize & DATA_SIZE_MASK);
};

static INLINE uint8_t GetSensorDataTimeStampSize(const uint8_t *pPacket) // Sensor Data packets only
{
    return (pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] & TIME_STAMP_SIZE_MASK);
};

static INLINE void SetSensorDataTimeStampSize(uint8_t *pPacket, uint8_t tsSize) // Sensor Data packets only
{
    pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] |= (tsSize & TIME_STAMP_SIZE_MASK);
};

static INLINE uint8_t GetControlSequenceNumber(const uint8_t *pPacket) // Control packets only
{
    return pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] & CONTROL_SEQUENCE_NUMBER_MASK;
};

static INLINE void SetControlSequenceNumber(uint8_t *pPacket, uint8_t sequenceNumber) // Control packets only
{
    pPacket[PKT_ATTRIBUTE_BYTE1_OFFSET] |= sequenceNumber & CONTROL_SEQUENCE_NUMBER_MASK;
};


//  Attribute Byte 2 (Control Packets only)
//    Parameter ID (Control packets)
//
static INLINE uint8_t GetControlParameterID(const uint8_t *pPacket)
{
    return pPacket[PKT_ATTRIBUTE_BYTE2_OFFSET];
};

static INLINE uint8_t ValidParameterID(uint8_t paramID)
{
    return (uint8_t) (paramID < N_PARAM_ID);
};


//  CRC field (16-bit trailer)
//
static INLINE uint16_t GetCRCField( const uint8_t *pPacket, uint16_t pPacketSizeWithCRC )
{
    uint16_t crcUnpacked;                               // local CRC
    uint8_t *pCRCUnpacked = (uint8_t *) &crcUnpacked;

#if(LOCAL_IS_LITTLE_ENDIAN)
    pCRCUnpacked[0] = pPacket[pPacketSizeWithCRC - 1];  // little Endian
    pCRCUnpacked[1] = pPacket[pPacketSizeWithCRC - 2];  // little Endian
#else
    pCRCUnpacked[0] = pPacket[pPacketSizeWithCRC - 2];  // big    Endian
    pCRCUnpacked[1] = pPacket[pPacketSizeWithCRC - 1];  // big    Endian
#endif

    return crcUnpacked;
};

static INLINE void SetCRCField( uint8_t *pPacket, uint16_t pPacketSizeWithCRC, uint16_t crcToWrite )
{
    const uint8_t *pCRCToWrite = (uint8_t *) &crcToWrite;  // local CRC

#if(LOCAL_IS_LITTLE_ENDIAN)
    pPacket[pPacketSizeWithCRC - 2] = pCRCToWrite[1];   // little Endian
    pPacket[pPacketSizeWithCRC - 1] = pCRCToWrite[0];   // little Endian
#else
    pPacket[pPacketSizeWithCRC - 2] = pCRCToWrite[0];   // big    Endian
    pPacket[pPacketSizeWithCRC - 1] = pCRCToWrite[1];   // big    Endian
#endif
};


static INLINE ASensorType_t AndroidSensorSetPrivateBase(   ASensorType_t androidSensor )
{
    return (ASensorType_t) ( (uint32_t) androidSensor | SENSOR_DEVICE_PRIVATE_BASE );
};

static INLINE ASensorType_t AndroidSensorClearPrivateBase( ASensorType_t androidSensor )
{
    return (ASensorType_t) ( (uint32_t) androidSensor & (~SENSOR_DEVICE_PRIVATE_BASE) );
};

static INLINE uint8_t IsPrivateNotAndroid( ASensorType_t androidSensor )
{
    return ( (uint32_t) androidSensor & SENSOR_DEVICE_PRIVATE_BASE ) ? (uint8_t) 1 : (uint8_t) 0;
};

static INLINE uint8_t IsSensorDataPacket( uint8_t packetID )
{
        return ( ( packetID == PKID_SENSOR_DATA || packetID == PKID_SENSOR_TEST_DATA ) );
};

static INLINE uint8_t IsControlPacket( uint8_t packetID )
{
        return ( ( packetID == PKID_CONTROL_REQ_RD || packetID == PKID_CONTROL_REQ_WR || packetID == PKID_CONTROL_RESP ) );
};

static INLINE ASensorType_t GetSensorType( const uint8_t *pPacket )
{
    const uint8_t       isPrivate  =                 GetAndroidOrPrivateField( pPacket );
          ASensorType_t sensor     = (ASensorType_t) GetSensorTypeField(       pPacket );

    if ( isPrivate )
    {
        sensor  = AndroidSensorSetPrivateBase( sensor );
    }

    return sensor;
};


#ifdef USE_PACKED_ERROR_CODES

static INLINE int32_t SetErrorCode( int32_t errorCode, uint8_t fileID, uint16_t lineNum )
{
    if ( errorCode >= 0 )
    {
        return errorCode;
    }
    else
    {
        //  packed error code, 32 bits:  11ff ffff  LLLL LLLL  LLLL LLLL  eeee eeee
        //    1 = hardcoded 1's, f = File ID, L = Line Number, e = Error Code.
        return (  (   (( int32_t) errorCode)        & 0x000000FF )
                | ( ( ((uint32_t) fileID   ) << 24) & 0x3F000000 )
                | ( ( ((uint32_t) lineNum  ) <<  8) & 0x00FFFF00 ) ) | 0xC0000000;
    }
};

static INLINE uint8_t ErrorCodeAlreadyLined( int32_t errorCode )
{
    return (errorCode < 0xFFFFFF00) ? (uint8_t) 1 : (uint8_t) 0;
};

static INLINE int8_t ExtractErrorCode( int32_t errorCode )
{
    if ( errorCode < 0)
    {
        return (int8_t) ((errorCode & 0x000000FF) | 0xFFFFFF00);
    }
    else
    {
        return OSP_STATUS_OK;
    }
};

static INLINE uint8_t ErrorCodeFileID( int32_t errorCode )
{
    if ( errorCode < 0 )
    {
        return (uint8_t)  ( ( errorCode & 0x3F000000 ) >> 24 );
    }
    else
    {
        return 0;
    }
};

static INLINE uint16_t ErrorCodeLineNumber( int32_t errorCode )
{
    if ( errorCode < 0 )
    {
        return (uint16_t)  ( ( errorCode & 0x00FFFF00 ) >> 8 );
    }
    else
    {
        return 0;
    }
};

void PrintErrorCode( int32_t errorCode );

#else
#define ErrorCodeAlreadyLined(A)  (TRUE)
#define SetErrorCode(A)
#define PrintErrorCode(A)
#endif

/* Print routines
 */
void PrintLocalPacketFields(  const LocalPacketTypes_t *pPacket );
void PrintLocalPacketPayload( const LocalPacketTypes_t *pPacket );
void PrintPacket( uint8_t *pPkt, uint16_t len );
void PrintPacketParameters(   void                              );
void PrintBuffer(       const uint8_t *pBuffer, uint16_t nCount );

#endif /* SENSOR_PACKETS_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/

