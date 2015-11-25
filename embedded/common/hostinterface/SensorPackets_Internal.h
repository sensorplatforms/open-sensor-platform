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
#if !defined (SENSOR_PACKETS_INTERNAL_H)
#define   SENSOR_PACKETS_INTERNAL_H

#if ( defined(SENSOR_PACKETS_COMMON_C) || defined(SENSOR_PACKETS_FORMAT_C) || \
      defined(SENSOR_PACKETS_PARSE_C)  || defined(SENSOR_PACKETS_PRINT_C) )

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
 \*-------------------------------------------------------------------------------------------------*/
#include "SensorPackets.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
 \*-------------------------------------------------------------------------------------------------*/
#define _DS8                        (DATA_SIZE_8_BIT << DATA_SIZE_SHIFT)
#define _DS16                       (DATA_SIZE_16_BIT << DATA_SIZE_SHIFT)
#define _DS32                       (DATA_SIZE_32_BIT << DATA_SIZE_SHIFT)
#define _DS64                       (DATA_SIZE_64_BIT << DATA_SIZE_SHIFT)
#define _TS32                       TIME_STAMP_32_BIT
#define _TS64                       TIME_STAMP_64_BIT
#define _DF_RAW                     (DATA_FORMAT_SENSOR_RAW << DATA_FORMAT_SENSOR_SHIFT)
#define _DF_FP                      (DATA_FORMAT_SENSOR_FIXPOINT << DATA_FORMAT_SENSOR_SHIFT)
#define _TF_RAW                     (TIME_FORMAT_SENSOR_RAW << TIME_FORMAT_SENSOR_SHIFT)
#define _TF_FP                      (TIME_FORMAT_SENSOR_FIXPOINT << TIME_FORMAT_SENSOR_SHIFT)

/*
 * Fields for controlPacketDescriptions[].
 */

#define PA_R                        0  // R/W direction allowed (R/W from Host's perspective)
#define PA_W                        1
#define PA_RW                       2
#define N_PA_RWS                    3  // array size

//  Control Packet Size Kinds
//
//  T = payload size determined from controlPacketDescriptions[], else payload size zero
//  0 = payload size zero
//  N = illegal (N/A)
//
//          PA_R  PA_W  PA_RW
//  REQ_R      0     N      0
//  REQ_W      N     T      T
//  RESP       T     0      T
//
#define PSIZE_FROM_TABLE   0x80
#define PSIZE_ZERO         0x81
#define PSIZE_ILLEGAL      0x82
#define N_PSIZES           3

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Sensor Packet Descriptor for lookup table to get pkt field values corresponding to the sensor type */
typedef struct _SensorPktDescriptor {
    uint8_t IsBigEndian;
    uint8_t ElementSz;
    uint8_t NumElements;
    uint8_t DataFormat;
    uint8_t TimeFormat;
    uint8_t DataSz;
    uint8_t TStampSz;
    uint8_t PktSzSansCRC;
} SensorPktDesc_t;

/* Control Packet Descriptor for lookup table to get pkt field values corresponding to the param type */
typedef struct _CtrlPktDescriptor {
    uint8_t IsBigEndian;
    uint8_t ElementSz;
    uint8_t NumElements;
    uint8_t RWAccess;       //R/W-Host-perspective
    //uint8_t PktSzSansCRC;
} CtrlPktDesc_t;



/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
#if defined(SENSOR_PACKETS_FORMAT_C) || defined(SENSOR_PACKETS_PARSE_C) || defined(SENSOR_PACKETS_PRINT_C)

extern const uint8_t  SensorTypesToSensorPacketTypes[NUM_SENSOR_TYPE];
extern const SensorPktDesc_t  sensorPacketDescriptions[N_SENSOR_DATA_PACKET_TYPES];
extern const uint16_t sensorPacketSizesSansCRC[N_SENSOR_DATA_PACKET_TYPES];
extern const CtrlPktDesc_t  controlPacketDescriptions[N_PARAM_ID];
extern const uint8_t  controlPacketSizeKinds[N_PACKET_IDENTIFIERS][N_PA_RWS];

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
#elif defined(SENSOR_PACKETS_COMMON_C)

/*  Constant parameter arrays
 */

/* Map Android and Private sensor types to sensor packet types */

const uint8_t SensorTypesToSensorPacketTypes[NUM_SENSOR_TYPE] =
{
    [SENSOR_META_DATA]                    =   SENSOR_DATA_Unimplemented,       //   0
    [SENSOR_ACCELEROMETER]                = SENSOR_DATA_CALIBRATED_FIXP,       //   1
    [SENSOR_GEOMAGNETIC_FIELD]            = SENSOR_DATA_CALIBRATED_FIXP,       //   2
      //  SENSOR_MAGNETIC_FIELD = SENSOR_GEOMAGNETIC_FIELD
    [SENSOR_ORIENTATION]                  = SENSOR_DATA_ORIENTATION_FIXP,      //   3
    [SENSOR_GYROSCOPE]                    = SENSOR_DATA_CALIBRATED_FIXP,       //   4
    [SENSOR_LIGHT]                        =   SENSOR_DATA_Unimplemented,       //   5
    [SENSOR_PRESSURE]                     =   SENSOR_DATA_Unimplemented,       //   6
    [SENSOR_TEMPERATURE]                  =   SENSOR_DATA_Unimplemented,       //   7
    [SENSOR_PROXIMITY]                    =   SENSOR_DATA_Unimplemented,       //   8
    [SENSOR_GRAVITY]                      = SENSOR_DATA_THREE_AXIS_FIXP,       //   9
    [SENSOR_LINEAR_ACCELERATION]          = SENSOR_DATA_THREE_AXIS_FIXP,       //  10
    [SENSOR_ROTATION_VECTOR]              = SENSOR_DATA_QUATERNION_FIXP,       //  11
    [SENSOR_RELATIVE_HUMIDITY]            =   SENSOR_DATA_Unimplemented,       //  12
    [SENSOR_AMBIENT_TEMPERATURE]          =   SENSOR_DATA_Unimplemented,       //  13
    [SENSOR_MAGNETIC_FIELD_UNCALIBRATED]  = SENSOR_DATA_UNCALIBRATED_FIXP,     //  14
    [SENSOR_GAME_ROTATION_VECTOR]         = SENSOR_DATA_QUATERNION_FIXP,       //  15
    [SENSOR_GYROSCOPE_UNCALIBRATED]       = SENSOR_DATA_UNCALIBRATED_FIXP,     //  16
    [SENSOR_SIGNIFICANT_MOTION]           = SENSOR_DATA_SIGNIFICANT_MOTION,    //  17
    [SENSOR_STEP_DETECTOR]                = SENSOR_DATA_STEP_DETECTOR,         //  18
    [SENSOR_STEP_COUNTER]                 = SENSOR_DATA_STEP_COUNTER,          //  19
    [SENSOR_GEOMAGNETIC_ROTATION_VECTOR]  = SENSOR_DATA_QUATERNION_FIXP,       //  20
                                                                               //  NUM_ANDROID_SENSOR_TYPE
    [PSENSOR_ENUM_FIRST_SENSOR]           =   SENSOR_DATA_Unimplemented,       //  21
      //  PSENSOR_DEBUG_TUNNEL = PSENSOR_ENUM_FIRST_SENSOR
    [PSENSOR_ACCELEROMETER_RAW]           = SENSOR_DATA_RAW,                   //  22
    [PSENSOR_MAGNETIC_FIELD_RAW]          = SENSOR_DATA_RAW,                   //  23
    [PSENSOR_GYROSCOPE_RAW]               = SENSOR_DATA_RAW,                   //  24
    [PSENSOR_LIGHT_UV]                    =   SENSOR_DATA_Unimplemented,       //  25
    [PSENSOR_LIGHT_RGB]                   =   SENSOR_DATA_Unimplemented,       //  26
    [PSENSOR_STEP]                        =   SENSOR_DATA_Unimplemented,       //  27
    [PSENSOR_ACCELEROMETER_UNCALIBRATED]  = SENSOR_DATA_UNCALIBRATED_FIXP,     //  28
    [PSENSOR_ORIENTATION]                 =   SENSOR_DATA_Unimplemented,       //  29
    [PSENSOR_CONTEXT_DEVICE_MOTION]       =   SENSOR_DATA_Unimplemented,       //  30
    [PSENSOR_CONTEXT_CARRY]               =   SENSOR_DATA_Unimplemented,       //  31
    [PSENSOR_CONTEXT_POSTURE]             =   SENSOR_DATA_Unimplemented,       //  32
    [PSENSOR_CONTEXT_TRANSPORT]           =   SENSOR_DATA_Unimplemented,       //  33
    [PSENSOR_CONTEXT_GESTURE_EVENT]               =   SENSOR_DATA_Unimplemented,       //  34
    [PSENSOR_HEART_RATE]                  =   SENSOR_DATA_Unimplemented,       //  35
    [SYSTEM_REAL_TIME_CLOCK]              =   SENSOR_DATA_Unimplemented,       //  36
    [PSENSOR_MAGNETIC_FIELD_ANOMALY]      =   SENSOR_DATA_Unimplemented        //  37
                                       //       NUM_PRIVATE_SENSOR_TYPE
};


const uint8_t SensorTypeIsWakeup[NUM_SENSOR_TYPE] =
{
    //         Mapping from ParseSensorDataPacket()
    [SENSOR_META_DATA]                    =     FALSE,       //   0
    [SENSOR_ACCELEROMETER]                =     FALSE,       //   1
    [SENSOR_GEOMAGNETIC_FIELD]            =     FALSE,       //   2
      //  SENSOR_MAGNETIC_FIELD = SENSOR_GEOMAGNETIC_FIELD
    [SENSOR_ORIENTATION]                  =     FALSE,       //   3
    [SENSOR_GYROSCOPE]                    =     FALSE,       //   4
    [SENSOR_LIGHT]                        =     FALSE,       //   5
    [SENSOR_PRESSURE]                     =     FALSE,       //   6
    [SENSOR_TEMPERATURE]                  =     FALSE,       //   7
    [SENSOR_PROXIMITY]                    =  TRUE,           //   8
    [SENSOR_GRAVITY]                      =     FALSE,       //   9
    [SENSOR_LINEAR_ACCELERATION]          =     FALSE,       //  10
    [SENSOR_ROTATION_VECTOR]              =     FALSE,       //  11
    [SENSOR_RELATIVE_HUMIDITY]            =     FALSE,       //  12
    [SENSOR_AMBIENT_TEMPERATURE]          =     FALSE,       //  13
    [SENSOR_MAGNETIC_FIELD_UNCALIBRATED]  =     FALSE,       //  14
    [SENSOR_GAME_ROTATION_VECTOR]         =     FALSE,       //  15
    [SENSOR_GYROSCOPE_UNCALIBRATED]       =     FALSE,       //  16
    [SENSOR_SIGNIFICANT_MOTION]           =     FALSE,       //  17
    [SENSOR_STEP_DETECTOR]                =     FALSE,       //  18
    [SENSOR_STEP_COUNTER]                 =     FALSE,       //  19
    [SENSOR_GEOMAGNETIC_ROTATION_VECTOR]  =     FALSE,       //  20
                                                             //  NUM_ANDROID_SENSOR_TYPE
    [PSENSOR_ENUM_FIRST_SENSOR]           =     FALSE,       //  21
      //  PSENSOR_DEBUG_TUNNEL = PSENSOR_ENUM_FIRST_SENSOR
    [PSENSOR_ACCELEROMETER_RAW]           =     FALSE,       //  22
    [PSENSOR_MAGNETIC_FIELD_RAW]          =     FALSE,       //  23
    [PSENSOR_GYROSCOPE_RAW]               =     FALSE,       //  24
    [PSENSOR_LIGHT_UV]                    =     FALSE,       //  25
    [PSENSOR_LIGHT_RGB]                   =     FALSE,       //  26
    [PSENSOR_STEP]                        =     FALSE,       //  27
    [PSENSOR_ACCELEROMETER_UNCALIBRATED]  =  TRUE,           //  28
    [PSENSOR_ORIENTATION]                 =     FALSE,       //  29
    [PSENSOR_CONTEXT_DEVICE_MOTION]       =     FALSE,       //  30
    [PSENSOR_CONTEXT_CARRY]               =     FALSE,       //  31
    [PSENSOR_CONTEXT_POSTURE]             =     FALSE,       //  32
    [PSENSOR_CONTEXT_TRANSPORT]           =     FALSE,       //  33
    [PSENSOR_CONTEXT_GESTURE_EVENT]               =     FALSE,       //  34
    [PSENSOR_HEART_RATE]                  =     FALSE,       //  35
    [SYSTEM_REAL_TIME_CLOCK]              =     FALSE,       //  36
    [PSENSOR_MAGNETIC_FIELD_ANOMALY]      =     FALSE        //  37
                                                             //  NUM_PRIVATE_SENSOR_TYPE
};

/* Sensor Packet Description lookup table to get pkt field values corresponding to the sensor type */
const  SensorPktDesc_t sensorPacketDescriptions[N_SENSOR_DATA_PACKET_TYPES] =
{                                    // BE ES NE  DataFmt   TSFmt    DataSz  TSSize  Pkt Size without CRC field
    [SENSOR_DATA_RAW]                = { 1, 2, 3, _DF_RAW,  _TF_RAW, _DS16,  _TS32, offsetof( HifSensorDataRaw_t,                 CRCField ) },
    [SENSOR_DATA_UNCALIBRATED_FIXP]  = { 1, 4, 3, _DF_FP ,  _TF_FP,  _DS32,  _TS64, offsetof( HifUncalibratedFixPoint_t, CRCField )-(4*3)    },  // !inc Offset[] sz, sz added ltr
    [SENSOR_DATA_CALIBRATED_FIXP]    = { 1, 4, 3, _DF_FP ,  _TF_FP,  _DS32,  _TS64, offsetof( HifCalibratedFixPoint_t,            CRCField ) },
    [SENSOR_DATA_QUATERNION_FIXP]    = { 1, 4, 4, _DF_FP ,  _TF_FP,  _DS32,  _TS64, offsetof( HifQuaternionFixPoint_t,            CRCField ) },
    [SENSOR_DATA_ORIENTATION_FIXP]   = { 1, 4, 3, _DF_FP ,  _TF_FP,  _DS32,  _TS64, offsetof( HifOrientationPktFixPoint_t,        CRCField ) },
    [SENSOR_DATA_THREE_AXIS_FIXP]    = { 1, 4, 3, _DF_FP ,  _TF_FP,  _DS32,  _TS64, offsetof( HifThreeAxisPktFixPoint_t,          CRCField ) },  // TODO:  how to handle accuracy?
    [SENSOR_DATA_SIGNIFICANT_MOTION] = { 0, 1, 1, _DF_RAW,  _TF_FP,  _DS8 ,  _TS64, offsetof( HifSignificantMotionPktFixPoint_t,  CRCField ) },
    [SENSOR_DATA_STEP_COUNTER]       = { 1, 8, 1, _DF_RAW,  _TF_FP,  _DS64,  _TS64, offsetof( HifStepCounter_t,                   CRCField ) },
    [SENSOR_DATA_STEP_DETECTOR]      = { 0, 1, 1, _DF_RAW,  _TF_FP,  _DS8 ,  _TS64, offsetof( HifStepDetector_t,                  CRCField ) },
    [SENSOR_DATA_Unimplemented]      = { 0, 0, 0, 0,        0,       0,      0,     0 }
};


/* Control Packet Descriptor for lookup table to get pkt field values corresponding to the param type */
const CtrlPktDesc_t controlPacketDescriptions[N_PARAM_ID] =
{                                    //Endn Esz  NE   R/W-Host-perspective
    [PARAM_ID_ERROR_CODE_IN_DATA]   = { 1,  4,  1,   PA_R  },
    [PARAM_ID_ENABLE]               = { 0,  1,  1,   PA_W  },
    [PARAM_ID_BATCH]                = { 1,  8,  2,   PA_W  },
    [PARAM_ID_FLUSH]                = { 0,  0,  0,   PA_W  },
    [PARAM_ID_RANGE_RESOLUTION]     = { 1,  4,  2,   PA_R  },
    [PARAM_ID_POWER]                = { 1,  4,  1,   PA_R  },
    [PARAM_ID_MINMAX_DELAY]         = { 1,  4,  2,   PA_R  },
    [PARAM_ID_FIFO_EVT_CNT]         = { 1,  4,  2,   PA_R  },
    [PARAM_ID_AXIS_MAPPING]         = { 0,  1,  3,   PA_RW },
    [PARAM_ID_CONVERSION_OFFSET]    = { 1,  4,  3,   PA_RW },
    [PARAM_ID_CONVERSION_SCALE]     = { 1,  4,  3,   PA_RW },
    [PARAM_ID_SENSOR_NOISE]         = { 1,  4,  3,   PA_RW },
    [PARAM_ID_TIMESTAMP_OFFSET]     = { 1,  4,  1,   PA_RW },
    [PARAM_ID_ONTIME_WAKETIME]      = { 1,  4,  2,   PA_RW },
    [PARAM_ID_HPF_LPF_CUTOFF]       = { 1,  2,  2,   PA_RW },
    [PARAM_ID_SENSOR_NAME]          = { 0,  1, 32,   PA_R  },
    [PARAM_ID_XYZ_OFFSET]           = { 1,  4,  3,   PA_RW },
    [PARAM_ID_F_SKOR_MATRIX]        = { 1,  4,  9,   PA_RW },
    [PARAM_ID_F_CAL_OFFSET]         = { 1,  4,  3,   PA_RW },
    [PARAM_ID_F_NONLINEAR_EFFECTS]  = { 1,  2, 12,   PA_RW },
    [PARAM_ID_BIAS_STABILITY]       = { 1,  4,  3,   PA_RW },
    [PARAM_ID_REPEATABILITY]        = { 1,  4,  3,   PA_RW },
    [PARAM_ID_TEMP_COEFF]           = { 1,  2,  6,   PA_RW },
    [PARAM_ID_SHAKE_SUSCEPTIBILITY] = { 1,  2,  3,   PA_RW },
    [PARAM_ID_EXPECTED_NORM]        = { 1,  4,  1,   PA_RW },
    [PARAM_ID_VERSION]              = { 0,  1, 32,   PA_R  },
    [PARAM_ID_DYNAMIC_CAL_SCALE]    = { 1,  4,  3,   PA_RW },
    [PARAM_ID_DYNAMIC_CAL_SKEW]     = { 1,  4,  3,   PA_RW },
    [PARAM_ID_DYNAMIC_CAL_OFFSET]   = { 1,  4,  3,   PA_RW },
    [PARAM_ID_DYNAMIC_CAL_ROTATION] = { 1,  4,  3,   PA_RW },
    [PARAM_ID_DYNAMIC_CAL_QUALITY]  = { 1,  4,  3,   PA_RW },
    [PARAM_ID_DYNAMIC_CAL_SOURCE]   = { 0,  1,  1,   PA_RW },
    [PARAM_ID_CONFIG_DONE]          = { 0,  0,  0,   PA_W  }
};

//  Control Packet Size Kinds
const uint8_t controlPacketSizeKinds[N_PACKET_IDENTIFIERS][N_PA_RWS] =
{
                               // PA_R                PA_W                PA_RW
    [PKID_SENSOR_DATA]       = { PSIZE_ILLEGAL,    PSIZE_ILLEGAL,    PSIZE_ILLEGAL    },
    [PKID_CONTROL_REQ_RD]    = { PSIZE_ZERO,       PSIZE_ILLEGAL,    PSIZE_ZERO       },
    [PKID_CONTROL_REQ_WR]    = { PSIZE_ILLEGAL,    PSIZE_FROM_TABLE, PSIZE_FROM_TABLE },
    [PKID_CONTROL_RESP]      = { PSIZE_FROM_TABLE, PSIZE_ZERO,       PSIZE_FROM_TABLE },
    [PKID_SENSOR_TEST_DATA]  = { PSIZE_ILLEGAL,    PSIZE_ILLEGAL,    PSIZE_ILLEGAL    }
};

#endif    //  if (SENSOR_PACKETS_HOST_C | SENSOR_PACKETS_HUB_C), else SENSOR_PACKETS_COMMON_C

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

uint16_t CopyControlPacketPayload( uint8_t *pDest, const uint8_t *pSrc, uint8_t parameterID );
int32_t GetLocalPacketPayloadElementSizeAndCount( const LocalPacketTypes_t *pPacket, uint16_t *pElementSize, uint16_t *pElementCount );
int32_t GetSensorPacketType( uint32_t SensorType );
uint8_t *GetControlPayloadAddress( LocalPacketTypes_t *pLocalPacket, uint8_t parameterID );


/****************************************************************************************************
 * @fn      SwapEndian routines
 *
 *          Each SwapEndian routine copies source buffer to destination buffer,
 *          swapping endianness only if the LOCAL_IS_LITTLE_ENDIAN preprocessor define is set.
 ***************************************************************************************************/

void SwapEndian(void *pDest, const void *pSrc, size_t nBytes);


static inline void SwapEndianX(void *pDest, const void *pSrc, size_t elementBytes, size_t nElements)
{
    int i = 0, k;

    for (k = 0; k < nElements; k++)
    {
        SwapEndian( ((uint8_t *) pDest) + i, ((const uint8_t *) (pSrc)) + i, elementBytes);

        i += elementBytes;
    }
};

static inline void SwapEndianBufferToInt32(int32_t *pDest, const void *pSrc)
{
    SwapEndian((void *) pDest, pSrc, sizeof(int32_t));
};

static inline void SwapEndianBufferToInt32X4(int32_t *pDest, const void *pSrc)
{
    SwapEndianX((void *) pDest, pSrc, sizeof(int32_t), 4);
};

static inline void SwapEndianBufferToInt32X3(int32_t *pDest, const void *pSrc)
{
    SwapEndianX((void *) pDest, pSrc, sizeof(int32_t), 3);
};

static inline void SwapEndianBufferToTimeStamp64(uint64_t *pDest, const void *pSrc)
{
    SwapEndian((void *) pDest, pSrc, sizeof(uint64_t));
};

static inline void SwapEndianBufferToTimeStamp32(TimeStamp_t *pDest, const void *pSrc)
{
    pDest->TS64 = 0; //helps clear higher 32-bit
    SwapEndian((void *) &(pDest->TS32[0]), pSrc, sizeof(uint32_t));  // four bytes only are copied
};


#endif    // (defined(SENSOR_PACKETS_COMMON_C) || defined(SENSOR_FORMAT_C) || defined(SENSOR_PARSE_C))

#endif /* SENSOR_PACKETS_INTERNAL_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
