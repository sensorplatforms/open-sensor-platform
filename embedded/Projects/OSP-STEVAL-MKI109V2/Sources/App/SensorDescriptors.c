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
#include "Common.h"
#include "Alg_Conversion.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
//extern NTTIME HostTimeOffset;
//extern int64_t HostTimeOffsetNanoSec;
extern uint32_t AccelTimeExtend;
extern uint32_t GyroTimeExtend;
extern uint32_t MagTimeExtend;

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* Sensor Descriptor constants for Bosch sensors*/
#define ACCEL_SENSITIVITY_4G            0.002f      //2mg/LSB
#define ACCEL_SCALING_FACTOR            (ACCEL_SENSITIVITY_4G * M_SI_EARTH_GRAVITY)
#define ACCEL_RANGE_MAX                 (4.0f * M_SI_EARTH_GRAVITY)
#define MAG_SENSITIVITY_2_5G_XY         1.4925f     //mg/LSB
#define MAG_SCALING_XY                  (MAG_SENSITIVITY_2_5G_XY * 0.1f)  //uT/LSB
#define MAG_SENSITIVITY_2_5G_Z          1.6666f     //mg/LSB
#define MAG_SCALING_Z                   (MAG_SENSITIVITY_2_5G_Z * 0.1f)  //uT/LSB
#define MAG_RANGE_MAX                   (2.5f * 100.0f) //µT
#define GYRO_SENSITIVITY                0.07f       //70mdps/digit
#define GYRO_SCALING_FACTOR             (GYRO_SENSITIVITY * 0.0175f)  //rad/sec/lsb
#define GYRO_RANGE_MAX                  (2000.0f * 0.017453f) //in rad/sec

/* Sensor time stamp offset (delays) */
#define GYRO_TIMESTAMP_OFFSET           0.013f        //gyro timestamp offset (sec)
#define ACCEL_TIMESTAMP_OFFSET          0.0f
#define MAG_TIMESTAMP_OFFSET            0.0f

#define GYRO_SAMPLE_PERIOD              0.010f          //in sec
#define ACCEL_SAMPLE_PERIOD             0.020f
#define MAG_SAMPLE_PERIOD               0.020f
#define XYZ_FROM_ORIGIN                 CONST_PRECISE(0.0f)

#define ANDROID_SWAP_SIGN(x)            (-(x))

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
SystemDescriptor_t gPlatformDesc =
{
    TOFIX_TIMECOEFFICIENT(US_PER_RTC_TICK * 0.000001f),  // timestamp conversion factor = 1us / count
    NULL,
    NULL
};

/* Sensor Descriptors for each of the inertial sensors */
SensorDescriptor_t _AccSensDesc =
{
    SENSOR_TYPE_ACCELEROMETER,
    SENSOR_DATA_SIGNED_TWO_COMP,
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_NEGATIVE_Y,AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    CONST_PRECISE(ACCEL_SCALING_FACTOR),CONST_PRECISE(ACCEL_SCALING_FACTOR),CONST_PRECISE(ACCEL_SCALING_FACTOR), //scale factor (sensor reports in ??)
    CONST_EXTENDED(ACCEL_RANGE_MAX),        // max value that is valid (+/- 4G sensor in M/Sec*Sec)
    CONST_EXTENDED(-ACCEL_RANGE_MAX),       // min value that is valid
    CONST_PRECISE(ACCEL_SAMPLE_PERIOD),     // nominal sample rate in seconds
    CONST_PRECISE(ACCEL_TIMESTAMP_OFFSET),  // sensor delay (acquisition to time stamp)
};

SensorDescriptor_t _MagSensDesc =
{
    SENSOR_TYPE_MAGNETIC_FIELD,
    SENSOR_DATA_SIGNED_TWO_COMP,
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_NEGATIVE_Y,AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    CONST_PRECISE(MAG_SCALING_XY),CONST_PRECISE(MAG_SCALING_XY),CONST_PRECISE(MAG_SCALING_Z), //scale factor
    CONST_EXTENDED(MAG_RANGE_MAX),          // max value that is valid (800 µTesla)
    CONST_EXTENDED(-MAG_RANGE_MAX),         // min value that is valid
    CONST_PRECISE(MAG_SAMPLE_PERIOD),       // nominal sample rate in seconds
    CONST_PRECISE(MAG_TIMESTAMP_OFFSET),    // sensor delay (acquisition to time stamp)
};

SensorDescriptor_t _GyroSensDesc =
{
    SENSOR_TYPE_GYROSCOPE,
    SENSOR_DATA_SIGNED_TWO_COMP,
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_NEGATIVE_Y,AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    CONST_PRECISE(GYRO_SCALING_FACTOR),CONST_PRECISE(GYRO_SCALING_FACTOR),CONST_PRECISE(GYRO_SCALING_FACTOR), //scale factor
    CONST_EXTENDED(GYRO_RANGE_MAX),         // max value that is valid (+/- 2000 deg/sec gyro in rad/sec)
    CONST_EXTENDED(-GYRO_RANGE_MAX),        // min value that is valid
    CONST_PRECISE(GYRO_SAMPLE_PERIOD),      // nominal sample rate in seconds
    CONST_PRECISE(GYRO_TIMESTAMP_OFFSET),   // sensor delay (acquisition to time stamp)
};


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef WAIT_FOR_HOST_SYNC
/****************************************************************************************************
 * @fn      WaitForHostSync
 *          Waits for Synchronization from host.
 *
 ***************************************************************************************************/
void WaitForHostSync( void )
{
    MessageBuffer *rcvMsg = NULLP;
    osp_bool_t syncRcvd = false;

    while(!syncRcvd)
    {
        ASFReceiveMessage( SENSOR_ACQ_TASK_ID, &rcvMsg );

        switch (rcvMsg->msgId)
        {
            case MSG_HOST_READY:
                syncRcvd = true;
                break;

            default:
                break;
        }
    }
}
#endif


/****************************************************************************************************
 * @fn      PublishUncalibratedSensorData
 *          Publishes the sensor data in LIPS (LibFM Inline Parseable Serial) format
 *
 ***************************************************************************************************/
void PublishUncalibratedSensorData( SensorDescriptor_t *pSensDesc, MsgSensorData *pSensData )
{
    if (g_logging & 0x40)
    {
#ifdef PUBLISH_RAW
        int64_t TimeStampNs;

        switch(pSensDesc->SensorType)
        {
        case SENSOR_TYPE_ACCELEROMETER:
            TimeStampNs = ((int64_t)AccelTimeExtend << 32) | (pSensData->timeStamp & 0xFFFFFFFF);
            TimeStampNs = TimeStampNs * US_PER_RTC_TICK * 1000;
            //TimeStampNs += HostTimeOffsetNanoSec;
            Print_LIPS("RA,%llX,%d,%d,%d", TimeStampNs, pSensData->X, pSensData->Y, pSensData->Z);
            break;

        case SENSOR_TYPE_MAGNETIC_FIELD:
            TimeStampNs = ((int64_t)MagTimeExtend << 32) | (pSensData->timeStamp & 0xFFFFFFFF);
            TimeStampNs = TimeStampNs * US_PER_RTC_TICK * 1000;
            //TimeStampNs += HostTimeOffsetNanoSec;
            Print_LIPS("RM,%llX,%d,%d,%d", TimeStampNs, pSensData->X, pSensData->Y, pSensData->Z);
            //D1_printf("RM,%llX,%d,%d,%d\r\n", TimeStampNs, pSensData->X, pSensData->Y, pSensData->Z);
            break;

        case SENSOR_TYPE_GYROSCOPE:
            TimeStampNs = ((int64_t)GyroTimeExtend << 32) | (pSensData->timeStamp & 0xFFFFFFFF);
            TimeStampNs = TimeStampNs * US_PER_RTC_TICK * 1000;
            //TimeStampNs += HostTimeOffsetNanoSec;
            Print_LIPS("RG,%llX,%d,%d,%d", TimeStampNs, pSensData->X, pSensData->Y, pSensData->Z);
            break;

        default:
            break;
        }
#else
        SPI_3AxisResult_t result;
        TriAxisSensorRawData_t rawData;
        rawData.TimeStamp = pSensData->timeStamp;
        rawData.Data[0] = pSensData->X;
        rawData.Data[1] = pSensData->Y;
        rawData.Data[2] = pSensData->Z;

        ConvertSensorData(pSensDesc, &rawData, &result);
        /* Adjust time for sensors based on host time received */
        //result.TimeStamp += HostTimeOffset;

        /* Print uncalibrated data in Android conventions
         * Note that the floating data is scaled by 4 decimal placed in the SensorHubd so no point in sending higher resolution
         */
        switch(pSensDesc->SensorType)
        {
        case SENSOR_TYPE_ACCELEROMETER:
            Print_LIPS("RA,%.6f,%.4f,%.4f,%.4f", TOFLT_TIME(result.TimeStamp), ANDROID_SWAP_SIGN(TOFLT_EXTENDED(result.Data[1])),
                TOFLT_EXTENDED(result.Data[0]), TOFLT_EXTENDED(result.Data[2]));
            break;

        case SENSOR_TYPE_MAGNETIC_FIELD:
            Print_LIPS("RM,%.6f,%.4f,%.4f,%.4f", TOFLT_TIME(result.TimeStamp), ANDROID_SWAP_SIGN(TOFLT_EXTENDED(result.Data[1])),
                TOFLT_EXTENDED(result.Data[0]), TOFLT_EXTENDED(result.Data[2]));
            break;

        case SENSOR_TYPE_GYROSCOPE:
            Print_LIPS("RG,%.6f,%.4f,%.4f,%.4f", TOFLT_TIME(result.TimeStamp), ANDROID_SWAP_SIGN(TOFLT_EXTENDED(result.Data[1])),
                TOFLT_EXTENDED(result.Data[0]), TOFLT_EXTENDED(result.Data[2]));
            break;

        default:
            break;
        }
#endif
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
