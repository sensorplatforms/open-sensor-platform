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
#include "osp-api.h"
#include "osp-sensors.h"    // defines android and private sensor types

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D  F U N C T I O N S  D E C L A R A T I O N
\*-------------------------------------------------------------------------------------------------*/
static void GenericDataResultCallback(ResultHandle_t resultHandle,
                                        void* pOutput);
static OSP_STATUS_t SensorControlActivate( SensorControl_t *pControl);
static void CalibrationUpdateCallback( InputSensorHandle_t handle, void *cal,
                                       uint32_t size, NTTIME time);

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define M_SI_EARTH_GRAVITY          (9.805f)
#define EXPECTED_MAG_NORM           (50.0f)

// TBD:  These physical sensor parameters probably should be defined in the
// physical sensor driver file or the driver should provide an API to
// return a pointer to its SensorDescriptor_t structure. This way, there is no
// change in this algorithm_t.c file.
// The following setting is for BMI055-Accel sensor
#define ACCEL_SENSITIVITY_4G        (0.01915f)     //  unit is m/s^2 per bit. was 0.001953f    //512LSB/g
#define ACCEL_SCALING_FACTOR        TOFIX_PRECISE(ACCEL_SENSITIVITY_4G)
#define ACCEL_RANGE_MAX             TOFIX_EXTENDED(4.0f * M_SI_EARTH_GRAVITY)
#define ACCEL_NOISE                 TOFIX_PRECISE(1.0f/M_SI_EARTH_GRAVITY)

// Setting for BMC150 magnetometer
#define MAG_MAX_1_3G_XY             (1300.0f)    //uT
#define MAG_SCALING_XY              TOFIX_PRECISE(0.305f)  //uT/LSB
#define MAG_MAX_2_5G_Z              (2500.0f)    //uT
#define MAG_SCALING_Z               TOFIX_PRECISE(0.305f)  //uT/LSB
#define MAG_RANGE_MAX               TOFIX_EXTENDED(1300.0f)    //µT
#define MAG_NOISE                   TOFIX_PRECISE(0.2f)

#define GYRO_SENSITIVITY            (0.06097f)    //70mdps/digit
#define GYRO_SCALING_FACTOR         TOFIX_PRECISE(GYRO_SENSITIVITY * 0.0175f)  //rad/sec/lsb
#define GYRO_RANGE_MAX              TOFIX_EXTENDED(2000.0f * 0.017453f) //in rad/sec
#define GYRO_NOISE                  TOFIX_PRECISE(0.006f)

#define XYZ_FROM_ORIGIN             TOFIX_PRECISE(0.0f)

#define STEP_COUNT_OUTPUT_RATE      TOFIX_PRECISE(0.0f)
#define SIG_MOTION_OUTPUT_RATE      TOFIX_PRECISE(0.0f)

#define ACCEL_OUTPUT_RATES          TOFIX_PRECISE(0.02f)      // 50Hz in seconds
#define MAG_OUTPUT_RATES            TOFIX_PRECISE(0.02f)      // 50Hz in seconds
#define GYRO_OUTPUT_RATES           TOFIX_PRECISE(0.02f)      // 50Hz in seconds

#define IsPrivateAndroidSensor(sensor) (sensor & SENSOR_DEVICE_PRIVATE_BASE) ? 1:0

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static SensorDescriptor_t _AccSensDesc = {
    ACCEL_INPUT_SENSOR,                // Input sensor type
    "BMI055-ACCEL",                    // a short human readable description of sensor, Null terminated
    0xffffffff,                         // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    {0,0,0},                                      // raw sensor data offset (where is zero?)
    {ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR}, //scale factor (raw to dimensional units)
    ACCEL_RANGE_MAX,                   // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -ACCEL_RANGE_MAX,                  // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) CalibrationUpdateCallback, // callback function pointer output calibration data. NULL is not use.
    {ACCEL_NOISE,ACCEL_NOISE,ACCEL_NOISE}, // noise
    TOFIX_PRECISE(0.02f),              // sensor acquisition period in unit of second
    {{TOFIX_PRECISE(1.0f),0,0},{0,TOFIX_PRECISE(1.0f),0},{0,0,TOFIX_PRECISE(1.0f)}}, //linear factory cal factoryskr[3][3]
    {0,0,0},                           // linear factor cal offset.
    {0,0,0},                           // Bias
    {0,0,0},                           // bias repeatability
    {{0,0},{0,0},{0,0}},               // two temperature coeff for each axis
    {0,0,0},                           // susceptability to shaking
    TOFIX_EXTENDED(M_SI_EARTH_GRAVITY), // expected norminal value
    (void *) NULL                      //  used in conjunction with Flags
};


static SensorDescriptor_t _MagSensDesc = {
    MAG_INPUT_SENSOR,                // Input sensor type
    "BMM050",                          // a short human readable description of sensor, Null terminated
    0xffffffff,                         // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    0,0,0,                                      // raw sensor data offset (where is zero?)
    {MAG_SCALING_XY,MAG_SCALING_XY,MAG_SCALING_Z}, //scale factor (raw to dimensional units)
    MAG_RANGE_MAX,                     // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -MAG_RANGE_MAX,                    // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) CalibrationUpdateCallback, // callback function pointer output calibration data. NULL is not use.
    {MAG_NOISE,MAG_NOISE,MAG_NOISE},   // noise
    TOFIX_PRECISE(0.02f),              // sensor acquisition period in unit of second
    {{TOFIX_PRECISE(1.0f),0,0},{0,TOFIX_PRECISE(1.0f),0},{0,0,TOFIX_PRECISE(1.0f)}}, //linear factory cal factoryskr[3][3]
    {0,0,0},                           // linear factor cal offset.
    {0,0,0},                           // Bias
    {0,0,0},                           // bias repeatability
    {{0,0},{0,0},{0,0}},               // two temperature coeff for each axis
    {0,0,0},                           // susceptability to shaking
    TOFIX_EXTENDED(EXPECTED_MAG_NORM),  // expected norminal value
    (void *) NULL                      //  used in conjunction with Flags
};


static SensorDescriptor_t _GyroSensDesc = {
    GYRO_INPUT_SENSOR,                // Input sensor type
    "BMG160",                          // a short human readable description of sensor, Null terminated
    0xffffffff,                         // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    0,0,0,                                      // raw sensor data offset (where is zero?)
    {GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR}, //scale factor (raw to dimensional units)
    GYRO_RANGE_MAX,                    // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -GYRO_RANGE_MAX,                   // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) CalibrationUpdateCallback, // callback function pointer output calibration data. NULL is not use.
    {GYRO_NOISE,GYRO_NOISE,GYRO_NOISE},  // noise
    TOFIX_PRECISE(0.02f),              // sensor acquisition period in unit of second
    {{TOFIX_PRECISE(1.0f),0,0},{0,TOFIX_PRECISE(1.0f),0},{0,0,TOFIX_PRECISE(1.0f)}}, //linear factory cal factoryskr[3][3]
    {0,0,0},                           // linear factor cal offset.
    {0,0,0},                           // Bias
    {0,0,0},                           // bias repeatability
    {{0,0},{0,0},{0,0}},               // two temperature coeff for each axis
    {0,0,0},                           // susceptability to shaking
    TOFIX_EXTENDED(0.0f),               // expected norminal value
    (void *) NULL                      //  used in conjunction with Flags
};


#define DEF_SENSOR_REQ(sen_type, callback, rate)     \
static  ResultDescriptor_t ResultDesc_##sen_type = { \
    sen_type,                                        \
    (OSP_ResultReadyCallback_t)callback,             \
    rate,                                            \
    0,                                               \
    0,                                               \
    OSP_NO_FLAGS,                                    \
    OSP_NO_OPTIONAL_DATA                             \
}

#define SENSOR_REQ_NAME(sen_type) ResultDesc_##sen_type

// Macro for subscribe android sensor result
#define SENSOR_SUBSCRIBE(sen)                                              \
    do {                                                                   \
        OSP_STATUS_t OSP_Status;                                           \
        OSP_Status = OSP_SubscribeSensorResult(& SENSOR_REQ_NAME(sen),     \
                                                  &_outSensorHandles[sen]);\
                                                                           \
        if (OSP_Status != OSP_STATUS_OK)                                   \
            D0_printf("Subscribe failed for " #sen "\r\n");                \
        else                                                               \
            D0_printf("Subscribed sensor " #sen "successful\r\n");         \
    } while (0)


// Macro for subscribe private sensor result
#define PRIVATE_SENSOR_SUBSCRIBE(sen)                                      \
    do {                                                                   \
        OSP_STATUS_t OSP_Status;                                           \
        D0_printf("Subscribing sensor type = " #sen " \r\n");              \
        OSP_Status = OSP_SubscribeSensorResult(&SENSOR_REQ_NAME(sen),      \
                                  &_outPSensorHandles[sen & 0xff]);        \
        if (OSP_Status != OSP_STATUS_OK)                                   \
            D0_printf("\nSubscribe failed for " #sen " \r\n");             \
        else                                                               \
            D0_printf("\nSubscribed sensor " #sen " successful\r\n");      \
    } while (0)


// Instantiate result descriptors for all Android sensors.
// Standard Android sensors.

DEF_SENSOR_REQ(SENSOR_ACCELEROMETER, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_MAGNETIC_FIELD, GenericDataResultCallback, MAG_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_ORIENTATION, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_GYROSCOPE, GenericDataResultCallback, GYRO_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_GRAVITY, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_LINEAR_ACCELERATION, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_ROTATION_VECTOR, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_MAGNETIC_FIELD_UNCALIBRATED, GenericDataResultCallback, MAG_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_GAME_ROTATION_VECTOR, GenericDataResultCallback, ACCEL_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_GYROSCOPE_UNCALIBRATED, GenericDataResultCallback, GYRO_OUTPUT_RATES);
DEF_SENSOR_REQ(SENSOR_SIGNIFICANT_MOTION, GenericDataResultCallback, SIG_MOTION_OUTPUT_RATE);
DEF_SENSOR_REQ(SENSOR_STEP_DETECTOR, GenericDataResultCallback, STEP_COUNT_OUTPUT_RATE);
DEF_SENSOR_REQ(SENSOR_STEP_COUNTER, GenericDataResultCallback, STEP_COUNT_OUTPUT_RATE);
DEF_SENSOR_REQ(SENSOR_GEOMAGNETIC_ROTATION_VECTOR, GenericDataResultCallback, ACCEL_OUTPUT_RATES);


// Private sensors (use ASensorType_t)
DEF_SENSOR_REQ(AP_PSENSOR_ACCELEROMETER_UNCALIBRATED,GenericDataResultCallback,ACCEL_OUTPUT_RATES);

static InputSensorHandle_t _AccHandle;
static InputSensorHandle_t _MagHandle;
static InputSensorHandle_t _GyroHandle;

static  const OSP_Library_Version_t* version;

// Maintains two separate handles for sensor subscriptions.
static ResultHandle_t _outSensorHandles[NUM_ANDROID_SENSOR_TYPE];   // Android Sensors
static ResultHandle_t _outPSensorHandles[NUM_PRIVATE_SENSOR_TYPE];  // Private Sensors

static OS_MUT mutexCritSection;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

static void EnterCriticalSection(void);
static void ExitCriticalSection(void);

SystemDescriptor_t gSystemDesc =
{
    TOFIX_TIMECOEFFICIENT(RTC_TICK_US_FLT * 0.000001f),      // timestamp conversion factor
    (OSP_CriticalSectionCallback_t) EnterCriticalSection,
    (OSP_CriticalSectionCallback_t) ExitCriticalSection,
    (OSP_SensorControlCallback_t )  SensorControlActivate    // Sensor ON/OFF control
};


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      EnterCriticalSection/ ExitCriticalSection
 *          Helper routines for thread-safe operation of the FreeMotion Library
 *
 ***************************************************************************************************/
__inline void EnterCriticalSection(void)
{
    os_mut_wait( mutexCritSection, OS_WAIT_FOREVER );
}

__inline void ExitCriticalSection(void)
{
    os_mut_release( mutexCritSection );
}


/****************************************************************************************************
 * @fn      GenericDataResultCallback()
 *          Generic callback function for sensor result data
 *          This function should be registered as a callback function when subscribes
 *          for a sensor result from the algorithm library. When result is available
 *          from the algorithm library, it will call this callback function and providing
 *          a pointer pOutput to a structure containing the sensor result. User should
 *          cast this pointer to an appropriate result structure to extract the data.
 *          User should cast the resultHandle to ResultDescriptor_t type to obtain the
 *          sensor type value.
 *
 ***************************************************************************************************/
static void GenericDataResultCallback(ResultHandle_t resultHandle,
                                      void* pOutput)
{
    ASensorType_t sensorType;
    enum MessageIdTag msg_type;
    MessageBuffer *pSample = NULLP;
    osp_bool_t sendMessage = TRUE;
    char *lipsCode;

    // A callback with NULL handle should never happen but check it anyway
    if ( resultHandle == NULL ) {
        D0_printf("Result callback with NULL handle\r\n");
        return;
    }

    sensorType = ((ResultDescriptor_t *) resultHandle)->SensorType;

    // Android sensor result
    switch (sensorType) {
    case SENSOR_ACCELEROMETER:
    {
        Android_TriAxisPreciseData_t* pData =
            (Android_TriAxisPreciseData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_CAL_ACC_DATA,
                                    sizeof(MsgAccelData),
                                    &pSample) == ASF_OK);

        pSample->msg.msgAccelData.X = pData->X;
        pSample->msg.msgAccelData.Y = pData->Y;
        pSample->msg.msgAccelData.Z = pData->Z;
        pSample->msg.msgAccelData.timeStamp = pData->TimeStamp;

        if ( g_logging & 0x40 ) {
            Print_LIPS("A, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp), TOFLT_PRECISE(pData->X),
                TOFLT_PRECISE(pData->Y), TOFLT_PRECISE(pData->Z));
        }
        break;
    }

    case SENSOR_MAGNETIC_FIELD:
    {
        Android_TriAxisExtendedData_t* pData =
            (Android_TriAxisExtendedData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_CAL_MAG_DATA,
                                    sizeof(MsgMagData),
                                    &pSample) == ASF_OK);

        pSample->msg.msgMagData.X = pData->X;
        pSample->msg.msgMagData.Y = pData->Y;
        pSample->msg.msgMagData.Z = pData->Z;
        pSample->msg.msgMagData.timeStamp = pData->TimeStamp;

        if (g_logging & 0x40) {
            Print_LIPS("M, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp), TOFLT_EXTENDED(pData->X),
                TOFLT_EXTENDED(pData->Y), TOFLT_EXTENDED(pData->Z));
        }
        break;
    }

    case SENSOR_GYROSCOPE:
    {
        Android_TriAxisPreciseData_t* pData =
            (Android_TriAxisPreciseData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_CAL_GYRO_DATA,
                                    sizeof(MsgGyroData),
                                    &pSample) == ASF_OK);

        pSample->msg.msgGyroData.X = pData->X;
        pSample->msg.msgGyroData.Y = pData->Y;
        pSample->msg.msgGyroData.Z = pData->Z;
        pSample->msg.msgGyroData.timeStamp = pData->TimeStamp;

        if (g_logging & 0x40) {
            Print_LIPS("G, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp), TOFLT_PRECISE(pData->X),
                TOFLT_PRECISE(pData->Y), TOFLT_PRECISE(pData->Z));
        }
        break;
    }

    case SENSOR_ROTATION_VECTOR:
    case SENSOR_GAME_ROTATION_VECTOR:
    case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
    {
        Android_RotationVectorResultData_t *pRotVecOut =
            (Android_RotationVectorResultData_t *)pOutput;

        switch(sensorType) {
        case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
            msg_type = MSG_GEO_QUATERNION_DATA;
            lipsCode = "GC";
            break;

        case SENSOR_GAME_ROTATION_VECTOR:
            msg_type = MSG_GAME_QUATERNION_DATA;
            lipsCode = "GV";
            break;

        case SENSOR_ROTATION_VECTOR:
        default:
            msg_type = MSG_QUATERNION_DATA;
            lipsCode = "Q";
            break;
        }

        ASF_assert(ASFCreateMessage(msg_type,
                                    sizeof(MsgQuaternionData),
                                    &pSample) == ASF_OK);

        pSample->msg.msgQuaternionData.W = pRotVecOut->W;
        pSample->msg.msgQuaternionData.X = pRotVecOut->X;
        pSample->msg.msgQuaternionData.Y = pRotVecOut->Y;
        pSample->msg.msgQuaternionData.Z = pRotVecOut->Z;
        pSample->msg.msgQuaternionData.HeadingError = pRotVecOut->HeadingErrorEst;
        pSample->msg.msgQuaternionData.TiltError = pRotVecOut->TiltErrorEst;
        pSample->msg.msgQuaternionData.timeStamp = pRotVecOut->TimeStamp;

        if (g_logging & 0x40 ) {
            Print_LIPS("%s, %6.3f, %.6f, %.6f, %.6f, %.6f", lipsCode,
                TOFLT_TIME(pRotVecOut->TimeStamp),
                TOFLT_PRECISE(pRotVecOut->W),
                TOFLT_PRECISE(pRotVecOut->X),
                TOFLT_PRECISE(pRotVecOut->Y),
                TOFLT_PRECISE(pRotVecOut->Z));
        }
        break;
    }

    case SENSOR_ORIENTATION:
    {
        Android_OrientationResultData_t *pOrientOut =
            (Android_OrientationResultData_t *)pOutput;

        ASF_assert(ASFCreateMessage(MSG_ORIENTATION_DATA,
                                    sizeof(MsgOrientationData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgOrientationData.X = pOrientOut->Pitch;
        pSample->msg.msgOrientationData.Y = pOrientOut->Roll;
        pSample->msg.msgOrientationData.Z = pOrientOut->Yaw;
        pSample->msg.msgOrientationData.timeStamp = pOrientOut->TimeStamp;

        if ( g_logging & 0x40 ) {
            Print_LIPS("I, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pOrientOut->TimeStamp),
                TOFLT_EXTENDED(pOrientOut->Yaw),
                TOFLT_EXTENDED(pOrientOut->Pitch),
                TOFLT_EXTENDED(pOrientOut->Roll));
        }
        break;
    }

    case SENSOR_GRAVITY:
    case SENSOR_LINEAR_ACCELERATION:
    {
        Android_TriAxisPreciseData_t *pTriAxisOut =
            (Android_TriAxisPreciseData_t *)pOutput;

        if (sensorType == SENSOR_GRAVITY) {
            msg_type = MSG_GRAVITY_DATA;
            lipsCode = "GR";
        } else {
            msg_type = MSG_LINEAR_ACCELERATION_DATA;
            lipsCode = "LN";
        }

        ASF_assert(ASFCreateMessage(msg_type,
                                    sizeof(MsgGenericTriAxisData),
                                    &pSample) == ASF_OK);
        /* msgGravityData and msgLinearAcceleration are a union of the same type */
        pSample->msg.msgGravityData.X = pTriAxisOut->X;
        pSample->msg.msgGravityData.Y = pTriAxisOut->Y;
        pSample->msg.msgGravityData.Z = pTriAxisOut->Z;
        pSample->msg.msgGravityData.timeStamp = pTriAxisOut->TimeStamp;

        if ( g_logging & 0x40 ) {
            Print_LIPS("%s, %6.3f, %3.4f, %3.4f, %3.4f", lipsCode,
                TOFLT_TIME(pTriAxisOut->TimeStamp),
                TOFLT_PRECISE(pTriAxisOut->X),
                TOFLT_PRECISE(pTriAxisOut->Y),
                TOFLT_PRECISE(pTriAxisOut->Z));
        }

        break;
    }

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
    {
        Android_UncalibratedTriAxisExtendedData_t *pData =
            (Android_UncalibratedTriAxisExtendedData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_MAG_DATA,
                                    sizeof(MsgMagData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgMagData.X = pData->X;
        pSample->msg.msgMagData.Y = pData->Y;
        pSample->msg.msgMagData.Z = pData->Z;
        pSample->msg.msgMagData.timeStamp = pData->TimeStamp;

        if (g_logging & 0x40) {
            Print_LIPS("RM, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp),
                TOFLT_EXTENDED(pData->X),
                TOFLT_EXTENDED(pData->Y),
                TOFLT_EXTENDED(pData->Z));
        }
        break;
    }

    case SENSOR_GYROSCOPE_UNCALIBRATED:
    {
       Android_UncalibratedTriAxisPreciseData_t *pData =
            (Android_UncalibratedTriAxisPreciseData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_GYRO_DATA,
                                    sizeof(MsgGyroData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgGyroData.X = pData->X;
        pSample->msg.msgGyroData.Y = pData->Y;
        pSample->msg.msgGyroData.Z = pData->Z;
        pSample->msg.msgGyroData.timeStamp = pData->TimeStamp;

        if (g_logging & 0x40) {
            Print_LIPS("RG, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp),
                TOFLT_EXTENDED(pData->X),
                TOFLT_EXTENDED(pData->Y),
                TOFLT_EXTENDED(pData->Z));
        }
        break;
    }

    case SENSOR_SIGNIFICANT_MOTION:
    {
        Android_BooleanResultData_t *pData =
            (Android_BooleanResultData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_SIG_MOTION_DATA,
                                    sizeof(MsgSigMotionData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgSigMotionData.active = pData->data;
        pSample->msg.msgSigMotionData.timeStamp = pData->TimeStamp;

        Print_LIPS("SM, %6.3f, %d",
            TOFLT_TIME(pData->TimeStamp),
            pData->data);
       break;
    }

    case SENSOR_STEP_COUNTER:
    {
        Android_StepCounterResultData_t *pData =
            (Android_StepCounterResultData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_STEP_COUNT_DATA,
                                    sizeof(MsgStepData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgStepCountData.X = pData->StepCount;
        pSample->msg.msgStepCountData.Y = 0;       // not use
        pSample->msg.msgStepCountData.Z = 0;       // not use
        pSample->msg.msgStepCountData.timeStamp = pData->TimeStamp;

        Print_LIPS("SC, %6.3f, %d, 0", TOFLT_TIME(pData->TimeStamp), pData->StepCount);
        break;
    }

    case SENSOR_STEP_DETECTOR:
    {
        Android_BooleanResultData_t *pData =
            (Android_BooleanResultData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_STEP_DETECT_DATA,
                                    sizeof(MsgStepDetData),
                                    &pSample) == ASF_OK);
        pSample->msg.msgStepDetData.active = TRUE;
        pSample->msg.msgStepDetData.timeStamp = pData->TimeStamp;

        Print_LIPS("SD, %6.3f", TOFLT_TIME(pData->TimeStamp));
        break;
    }

    case AP_PSENSOR_ACCELEROMETER_UNCALIBRATED:
    {
        Android_UncalibratedTriAxisPreciseData_t *pData =
            (Android_UncalibratedTriAxisPreciseData_t *) pOutput;

        ASF_assert(ASFCreateMessage(MSG_ACC_DATA,
                    sizeof(MsgAccelData),
                    &pSample) == ASF_OK);
        pSample->msg.msgAccelData.X = pData->X;
        pSample->msg.msgAccelData.Y = pData->Y;
        pSample->msg.msgAccelData.Z = pData->Z;
        pSample->msg.msgAccelData.timeStamp = pData->TimeStamp;

        if (g_logging & 0x40) {
            Print_LIPS("RA, %6.3f, %3.4f, %3.4f, %3.4f",
                TOFLT_TIME(pData->TimeStamp),
                TOFLT_PRECISE(pData->X),
                TOFLT_PRECISE(pData->Y),
                TOFLT_PRECISE(pData->Z));
        }
        break;
    }

    default:
        D0_printf("%s not handling sensor type %i\r\n", __FUNCTION__, sensorType);
        sendMessage = FALSE;
        break;
    }

    /* Now send the created message to the I2C slave task to route to host.*/
    if ( sendMessage ) {
        ASF_assert( ASFSendMessage(HOST_INTF_TASK_ID, pSample) == ASF_OK );
    }
}


/****************************************************************************************************
 * @fn      CalibrationUpdateCallback
 *          callback function when algorithm library has update
 *          calibration coefficients that need to be stored in non-
 *          volatile memory. This function should be registered in the
 *          SensorDescriptor_t data structure for each of the sensor.
 *
 * @param   cal, is a pointer to a buffer contains the calibration coefficients.
 * @param   size, number of bytes to save from the calibration buffer.
 * @param   time, timestamp when the new calibration data was generated.
 *
 * @return  none.
 ***************************************************************************************************/
static void CalibrationUpdateCallback( InputSensorHandle_t handle, void *cal,
                                       uint32_t size, NTTIME time)
{

    // check for valid handle
    if ( handle == NULL ) return;

    if ( handle == _AccHandle ) {
        // Now save accelerometer calibration to non-volatile memory
        // D0_printf("Update accel calibration \r\n");
    }else if ( handle == _MagHandle) {
        // Now save magnetomter calibration to non-volatile memory
        //D0_printf("Update mag calibration \r\n");
    } else if ( handle == _GyroHandle ) {
        // Now save gyroscope calibration to non-volatile memory
        // D0_printf("Update gyro calibration \r\n");
    } else {
        return;    //un-recognized handle
    }
}


/****************************************************************************************************
 * @fn      HandleSensorData
 *          Handles sensor input data and feeds it to the sensor algorithms
 *
 ***************************************************************************************************/
static void HandleSensorData(MessageBuffer *pRcvMsg)
{
    osp_status_t status;
    OSP_InputSensorData_t sensorData;

    switch(pRcvMsg->msgId) {
    case MSG_ACC_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgAccelData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgAccelData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgAccelData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgAccelData.timeStamp;
        status = OSP_SetInputData(_AccHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);

        if (g_logging & 0x8)
        {
            D1_printf("Acc\t%8lu\t%+5d\t%+5d\t%+5d\r\n", sensorData.rawdata.TimeStamp, sensorData.rawdata.data[0],
                sensorData.rawdata.data[1], sensorData.rawdata.data[2]);
        }
        break;

    case MSG_MAG_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgMagData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgMagData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgMagData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgMagData.timeStamp;
        status = OSP_SetInputData(_MagHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);

        if (g_logging & 0x8)
        {
            D1_printf("Mag\t%8lu\t%+5d\t%+5d\t%+5d\r\n", sensorData.rawdata.TimeStamp, sensorData.rawdata.data[0],
                sensorData.rawdata.data[1], sensorData.rawdata.data[2]);
        }
        break;

    case MSG_GYRO_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgGyroData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgGyroData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgGyroData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgGyroData.timeStamp;
        status = OSP_SetInputData(_GyroHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);

        if (g_logging & 0x8)
        {
            D1_printf("Gyr\t%8lu\t%+5d\t%+5d\t%+5d\r\n", sensorData.rawdata.TimeStamp, sensorData.rawdata.data[0],
                sensorData.rawdata.data[1], sensorData.rawdata.data[2]);
        }
        break;

    default:
        D1_printf("ALG: Bad message: %d\r\n", pRcvMsg->msgId);
        break;
    }
}


/****************************************************************************************************
 * @fn      SendBgTrigger
 *          Sends triggers to the algorithm background task to do processing
 *
 ***************************************************************************************************/
static void SendBgTrigger( void )
{
    MessageBuffer *pSendMsg = NULLP;
    ASF_assert( ASFCreateMessage( MSG_TRIG_ALG_BG, sizeof(MsgNoData), &pSendMsg ) == ASF_OK );
    ASFSendMessage( ALG_BG_TASK_ID, pSendMsg );
}


/**********************************************************************
 * @fn      SensorControlActicate
 *          Fucntion to Control the sensor activation and deactivation.
 *          Typical usage of this function to register it as a callback
 *          function with the algorithm library and letting the algorithm
 *          library make decision when to enable and disable the sensor.
 **********************************************************************/
static OSP_STATUS_t SensorControlActivate( SensorControl_t *pControl)
{
    MessageBuffer *pSendMsg = NULLP;
    InputSensor_t sensorType;

    if ( pControl == NULL )
        return OSP_STATUS_NULL_POINTER;

    if ( pControl->Handle == _AccHandle ) {
        sensorType = _AccSensDesc.SensorType;
    } else if ( pControl->Handle == _MagHandle ) {
        sensorType = _MagSensDesc.SensorType;
    } else if ( pControl->Handle == _GyroHandle ) {
        sensorType = _GyroSensDesc.SensorType;
    } else {
        return OSP_STATUS_INVALID_HANDLE;   // unrecognize handle
    }

    switch (sensorType) {
    case ACCEL_INPUT_SENSOR:
    case MAG_INPUT_SENSOR:
    case GYRO_INPUT_SENSOR:
        /* send this request to sensor acquisition task */
        ASF_assert(ASFCreateMessage(MSG_SENSOR_CONTROL,
                                    sizeof(MsgSensorControlData),
                                    &pSendMsg) == ASF_OK);

        pSendMsg->msg.msgSensorControlData.command    = pControl->Command;
        pSendMsg->msg.msgSensorControlData.sensorType = sensorType;
        pSendMsg->msg.msgSensorControlData.data       = pControl->Data;
        ASFSendMessage(SENSOR_ACQ_TASK_ID, pSendMsg);
        break;

    default:
        break;
        // all the other sensor controls are not use by the algorithm library at this point.
    }

    return OSP_STATUS_OK;
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      AlgorithmTask
 *          This task is responsible for running the sensor algorithms on the incoming sensor
 *          data (could be raw or filtered) and processing output results
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK  void AlgorithmTask (ASF_TASK_ARG)
{
    MessageBuffer *rcvMsg = NULLP;
    OSP_STATUS_t OSP_Status;
    uint32_t mycount = 0;


    OSP_GetLibraryVersion(&version);
    D1_printf("OSP Version: %s\r\n", version->VersionString);

    OSP_Status = OSP_Initialize(&gSystemDesc);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_Initialize Failed");
    //OSP_SetCalibrationConfig( 0x1);     // disable rotational cal.

    // Register the input sensors
    OSP_Status = OSP_RegisterInputSensor(&_AccSensDesc, &_AccHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (accel) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_MagSensDesc, &_MagHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (mag) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_GyroSensDesc, &_GyroHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (gyro) Failed");

#if 0

    SENSOR_SUBSCRIBE(SENSOR_ORIENTATION);
    SENSOR_SUBSCRIBE(SENSOR_ROTATION_VECTOR);


    SENSOR_SUBSCRIBE(SENSOR_STEP_COUNTER);

    SENSOR_SUBSCRIBE(SENSOR_STEP_DETECTOR);
    SENSOR_SUBSCRIBE(SENSOR_SIGNIFICANT_MOTION);


    SENSOR_SUBSCRIBE(SENSOR_GYROSCOPE_UNCALIBRATED);
    SENSOR_SUBSCRIBE(SENSOR_MAGNETIC_FIELD_UNCALIBRATED);

    SENSOR_SUBSCRIBE(SENSOR_GYROSCOPE);
    SENSOR_SUBSCRIBE(SENSOR_ACCELEROMETER);
    SENSOR_SUBSCRIBE(SENSOR_MAGNETIC_FIELD);
    SENSOR_SUBSCRIBE(SENSOR_ORIENTATION);
    SENSOR_SUBSCRIBE(SENSOR_GRAVITY);
    SENSOR_SUBSCRIBE(SENSOR_LINEAR_ACCELERATION);
    SENSOR_SUBSCRIBE(SENSOR_ROTATION_VECTOR);
    SENSOR_SUBSCRIBE(SENSOR_GAME_ROTATION_VECTOR);
    SENSOR_SUBSCRIBE(SENSOR_GEOMAGNETIC_ROTATION_VECTOR);

    // Subscribing private sensor results
    PRIVATE_SENSOR_SUBSCRIBE(AP_PSENSOR_ACCELEROMETER_UNCALIBRATED);
#endif

    D0_printf("%s: Alg Task init done\r\n", __func__);

    while (1) {
        ASFReceiveMessage(ALGORITHM_TASK_ID, &rcvMsg);
        if (!(mycount % 64)) {
            LED_Toggle(LED_GREEN);
        }

        switch (rcvMsg->msgId) {
        case MSG_GYRO_DATA:
            SendBgTrigger();
        case MSG_ACC_DATA:
        case MSG_MAG_DATA:
            mycount++;
            HandleSensorData(rcvMsg);
            //keep doing foreground computation until its finished
            do {
                OSP_Status = OSP_DoForegroundProcessing();
                ASF_assert(OSP_Status != OSP_STATUS_UNSPECIFIED_ERROR);
            } while(OSP_Status != OSP_STATUS_IDLE); //keep doing foreground computation until its finished

            break;

        default:
            /* Unhandled messages */
            D1_printf("Alg-FG:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}


/****************************************************************************************************
 * @fn      AlgBackGndTask
 *          This task is responsible for running the background routines (e.g. calibration)
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK  void AlgBackGndTask (ASF_TASK_ARG)
{
    MessageBuffer *rcvMsg = NULLP;

    while (1)
    {
        ASFReceiveMessage( ALG_BG_TASK_ID, &rcvMsg );
        switch (rcvMsg->msgId)
        {
        case MSG_TRIG_ALG_BG:
            while(OSP_DoBackgroundProcessing() != OSP_STATUS_IDLE)
                ; //background compute. Note that it's safe to call background processing more often than needed
            break;

        default:
            /* Unhandled messages */
            D1_printf("Alg-BG:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}


/*********************************************************************
 * @fn      Algorithm_SubscribeSensor
 *          To subscribe a sensor result with the algorithm module.
 *
 *
 * @param   specify a subscribe sensor of type ASensorType_t
 *
 * @return  OSP_STATUS_OK if successsful.
*           OSP_STATUS_SENSOR_UNSUPPORTED if failure.
 *
 *********************************************************************/
OSP_STATUS_t Algorithm_SubscribeSensor( ASensorType_t sensor)
{
    ResultDescriptor_t *pResultDesc;
    OSP_STATUS_t status;
    switch( sensor ) {
    case  SENSOR_ACCELEROMETER:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_ACCELEROMETER);
        break;

    case SENSOR_MAGNETIC_FIELD:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_MAGNETIC_FIELD);
        break;

    case SENSOR_ORIENTATION:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_ORIENTATION);
        break;

    case SENSOR_GYROSCOPE:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_GYROSCOPE);
        break;

    case SENSOR_LIGHT:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_LIGHT);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_PRESSURE:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_PRESSURE);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_TEMPERATURE:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_TEMPERATURE);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_PROXIMITY:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_PROXIMITY);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_GRAVITY:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_GRAVITY);
        break;

    case SENSOR_LINEAR_ACCELERATION:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_LINEAR_ACCELERATION);
        break;

    case SENSOR_ROTATION_VECTOR:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_ROTATION_VECTOR);
        break;

    case SENSOR_RELATIVE_HUMIDITY:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_RELATIVE_HUMIDITY);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_AMBIENT_TEMPERATURE:
//        pResultDesc = & SENSOR_REQ_NAME(SENSOR_AMBIENT_TEMPERATURE);
        return OSP_STATUS_SENSOR_UNSUPPORTED;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_MAGNETIC_FIELD_UNCALIBRATED);
        break;

    case SENSOR_GAME_ROTATION_VECTOR:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_GAME_ROTATION_VECTOR);
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_GYROSCOPE_UNCALIBRATED);
        break;

    case SENSOR_SIGNIFICANT_MOTION:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_SIGNIFICANT_MOTION);
        break;

    case SENSOR_STEP_DETECTOR:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_STEP_DETECTOR);
        break;

    case SENSOR_STEP_COUNTER:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_STEP_COUNTER);
        break;

    case SENSOR_GEOMAGNETIC_ROTATION_VECTOR:
        pResultDesc = & SENSOR_REQ_NAME(SENSOR_GEOMAGNETIC_ROTATION_VECTOR);
        break;

    case  AP_PSENSOR_ACCELEROMETER_UNCALIBRATED:
        pResultDesc = & SENSOR_REQ_NAME(AP_PSENSOR_ACCELEROMETER_UNCALIBRATED);
        break;

    default:
        D0_printf("Unknown or unsupport sensor type %d \n", sensor);
        return OSP_STATUS_SENSOR_UNSUPPORTED;
    }

    // Now subscribe the sensor result
    if ( IsPrivateAndroidSensor(sensor) ) {
        ASensorType_t PSensor =  (ASensorType_t)M_ToBaseSensorEnum(sensor);
        status = OSP_SubscribeSensorResult(pResultDesc, &_outPSensorHandles[PSensor]);
    } else {
        status = OSP_SubscribeSensorResult(pResultDesc, &_outSensorHandles[sensor]);
    }

    if ( status != OSP_STATUS_OK ) {
        D0_printf("\n%s: Failed subscribe to sensor type %d\r\n", __FUNCTION__, sensor);
    }
    return status;
}


/*********************************************************************
 * @fn      Algorithm_UnsubscribeSensor
 *          To un-subscribe a sensor result that was previously
 *          subscribed with the algorithm module.
 *
 *
 * @param   specify a sensor of ASensorType_t to un-subscribe
 *
 * @return  OSP_STATUS_OK if successsful.
*           OSP_STATUS_NOT_SUBSCRIBED if failure.
 *
 *********************************************************************/
OSP_STATUS_t Algorithm_UnsubscribeSensor( ASensorType_t sensor)
{
    OSP_STATUS_t status = OSP_STATUS_NOT_SUBSCRIBED;

    if ( IsPrivateAndroidSensor(sensor) ) {
        // Private Android sensor
        ASensorType_t PSensor =  (ASensorType_t)M_ToBaseSensorEnum(sensor);
        if ( _outPSensorHandles[PSensor] != NULL ) {
           status =  OSP_UnsubscribeSensorResult(_outPSensorHandles[PSensor]);
           _outPSensorHandles[PSensor] = NULL;
        }
    } else {
        // Standard Android Sensor
        if ( _outSensorHandles[sensor] != NULL ) {
            status  = OSP_UnsubscribeSensorResult(_outSensorHandles[sensor]);
            _outSensorHandles[sensor] = NULL;
        }
    }

    if ( status != OSP_STATUS_OK) {
        D0_printf("\n%s: Failed to unsubscribed sensor 0x%x\r\n", __FUNCTION__, sensor);
    }
    return status;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
