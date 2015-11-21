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
#include "common.h"
#include "osp-api.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define M_SI_EARTH_GRAVITY              (9.805f)
#define EXPECTED_MAG_NORM               (50.0f)

#define ACCEL_SENSITIVITY_4G            0.002f      //2mg/LSB
#define ACCEL_SCALING_FACTOR            TOFIX_PRECISE(ACCEL_SENSITIVITY_4G * M_SI_EARTH_GRAVITY)
#define ACCEL_RANGE_MAX                 TOFIX_EXTENDED(4.0f * M_SI_EARTH_GRAVITY)
#define MAG_SENSITIVITY_2_5G_XY         1.4925f     //mg/LSB
#define MAG_SCALING_XY                  TOFIX_EXTENDED(MAG_SENSITIVITY_2_5G_XY * 0.1f)  //uT/LSB
#define MAG_SENSITIVITY_2_5G_Z          1.6666f     //mg/LSB
#define MAG_SCALING_Z                   TOFIX_EXTENDED(MAG_SENSITIVITY_2_5G_Z * 0.1f)  //uT/LSB
#define MAG_RANGE_MAX                   (2.5f * 100.0f) //µT
#define GYRO_SENSITIVITY                0.07f       //70mdps/digit
#define GYRO_SCALING_FACTOR             TOFIX_PRECISE(GYRO_SENSITIVITY * 0.0175f)  //rad/sec/lsb
#define GYRO_RANGE_MAX                  (2000.0f * 0.017453f) //in rad/sec

#define ACCEL_NOISE                     TOFIX_PRECISE(1.0f/M_SI_EARTH_GRAVITY)
#define MAG_NOISE                       TOFIX_PRECISE(0.2f)
#define GYRO_NOISE                      TOFIX_PRECISE(0.006f)

#define STEP_COUNT_OUTPUT_RATE          TOFIX_PRECISE(0.0f)
#define SIG_MOTION_OUTPUT_RATE          TOFIX_PRECISE(0.0f)
#define ACCEL_OUTPUT_RATES              TOFIX_PRECISE(0.02f)    // 50Hz in seconds
#define MAG_OUTPUT_RATES                TOFIX_PRECISE(0.02f)    // 50Hz in seconds
#define GYRO_OUTPUT_RATES               TOFIX_PRECISE(0.02f)    // 50Hz in seconds

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static SensorDescriptor_t _AccSensDesc = {
    ACCEL_INPUT_SENSOR,                // Input sensor type
    "Acc LSM303DLHC",                  // a short human readable description of sensor, Null terminated
    0xffffffff,                        // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    {0,0,0},                                      // raw sensor data offset (where is zero?)
    {ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR}, //scale factor (raw to dimensional units)
    ACCEL_RANGE_MAX,                   // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -ACCEL_RANGE_MAX,                  // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) NULL, // callback function pointer output calibration data. NULL is not use.
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
    MAG_INPUT_SENSOR,                  // Input sensor type
    "Mag LSM303DLHC",                  // a short human readable description of sensor, Null terminated
    0xffffffff,                        // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    0,0,0,                                      // raw sensor data offset (where is zero?)
    {MAG_SCALING_XY,MAG_SCALING_XY,MAG_SCALING_Z}, //scale factor (raw to dimensional units)
    MAG_RANGE_MAX,                     // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -MAG_RANGE_MAX,                    // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) NULL, // callback function pointer output calibration data. NULL is not use.
    {MAG_NOISE,MAG_NOISE,MAG_NOISE},   // noise
    TOFIX_PRECISE(0.02f),              // sensor acquisition period in unit of second
    {{TOFIX_PRECISE(1.0f),0,0},{0,TOFIX_PRECISE(1.0f),0},{0,0,TOFIX_PRECISE(1.0f)}}, //linear factory cal factoryskr[3][3]
    {0,0,0},                           // linear factor cal offset.
    {0,0,0},                           // Bias
    {0,0,0},                           // bias repeatability
    {{0,0},{0,0},{0,0}},               // two temperature coeff for each axis
    {0,0,0},                           // susceptability to shaking
    TOFIX_EXTENDED(EXPECTED_MAG_NORM), // expected norminal value
    (void *) NULL                      //  used in conjunction with Flags
};


static SensorDescriptor_t _GyroSensDesc = {
    GYRO_INPUT_SENSOR,                 // Input sensor type
    "Gyro L3GD20",                     // a short human readable description of sensor, Null terminated
    0xffffffff,                        // 32 bit of raw data are significant
    {AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z}, // X,Y,Z  sensor orientation
    0,0,0,                                      // raw sensor data offset (where is zero?)
    {GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR}, //scale factor (raw to dimensional units)
    GYRO_RANGE_MAX,                    // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -GYRO_RANGE_MAX,                   // min value that is valid
    (void *) NULL,                     // pointer to calibration data structure. NULL if not available
    (OSP_WriteCalDataCallback_t) NULL, // callback function pointer output calibration data. NULL is not use.
    {GYRO_NOISE,GYRO_NOISE,GYRO_NOISE},  // noise
    TOFIX_PRECISE(0.02f),              // sensor acquisition period in unit of second
    {{TOFIX_PRECISE(1.0f),0,0},{0,TOFIX_PRECISE(1.0f),0},{0,0,TOFIX_PRECISE(1.0f)}}, //linear factory cal factoryskr[3][3]
    {0,0,0},                           // linear factor cal offset.
    {0,0,0},                           // Bias
    {0,0,0},                           // bias repeatability
    {{0,0},{0,0},{0,0}},               // two temperature coeff for each axis
    {0,0,0},                           // susceptability to shaking
    TOFIX_EXTENDED(0.0f),              // expected norminal value
    (void *) NULL                      //  used in conjunction with Flags
};


static void stepCounterOutputCallback(ResultHandle_t outputHandle,
    Android_StepCounterResultData_t* pOutput);

static void sigMotionOutputCallback(ResultHandle_t outputHandle,
    Android_BooleanResultData_t* pOutput);

static void UnCalAccelDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisPreciseData_t* pOutput);

static void UnCalMagDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisExtendedData_t* pOutput);

static void UnCalGyroDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisPreciseData_t* pOutput);

/* Output result descriptor for subscribing to step counts */
static ResultDescriptor_t  stepCounterRequest = {
    SENSOR_STEP_COUNTER,
    (OSP_ResultReadyCallback_t)stepCounterOutputCallback,
    STEP_COUNT_OUTPUT_RATE,
    0,
    0,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to significant motion */
static ResultDescriptor_t  sigMotionRequest = {
    SENSOR_SIGNIFICANT_MOTION,
    (OSP_ResultReadyCallback_t)sigMotionOutputCallback,
    SIG_MOTION_OUTPUT_RATE,
    0,
    0,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated accelerometer data */
static ResultDescriptor_t UnCalAccelRequest = {
    AP_PSENSOR_ACCELEROMETER_UNCALIBRATED,
    (OSP_ResultReadyCallback_t)UnCalAccelDataResultCallback,
    ACCEL_OUTPUT_RATES,
    0,
    0,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated magnetometer data */
static ResultDescriptor_t UnCalMagRequest = {
    SENSOR_MAGNETIC_FIELD_UNCALIBRATED,
    (OSP_ResultReadyCallback_t)UnCalMagDataResultCallback,
    MAG_OUTPUT_RATES,
    0,
    0,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated gyroscope data */
static ResultDescriptor_t UnCalGyroRequest = {
    SENSOR_GYROSCOPE_UNCALIBRATED,
    (OSP_ResultReadyCallback_t)UnCalGyroDataResultCallback,
    GYRO_OUTPUT_RATES,
    0,
    0,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

static InputSensorHandle_t _AccHandle;
static InputSensorHandle_t _MagHandle;
static InputSensorHandle_t _GyroHandle;
static const OSP_Library_Version_t* version;
static ResultHandle_t _stepCounterHandle;
static ResultHandle_t _sigMotionHandle;
static ResultHandle_t _unCalAccelHandle;
static ResultHandle_t _unCalMagHandle;
static ResultHandle_t _unCalGyroHandle;

static OS_MUT mutexCritSection;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

static void EnterCriticalSection(void);
static void ExitCriticalSection(void);

SystemDescriptor_t gSystemDesc =
{
    TOFIX_TIMECOEFFICIENT(US_PER_RTC_TICK * 0.000001f),      // timestamp conversion factor
    (OSP_CriticalSectionCallback_t) EnterCriticalSection,
    (OSP_CriticalSectionCallback_t) ExitCriticalSection,
    (OSP_SensorControlCallback_t )  NULL    // Sensor ON/OFF control
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
 * @fn      UnCalAccelDataResultCallback
 *          Call back for Uncalibrated accelerometer data
 *
 ***************************************************************************************************/
static void UnCalAccelDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisPreciseData_t* pOutput)
{
    if (g_logging & 0x40)  //Uncalibrated data, in Android conventions
    {
        Print_LIPS("RA,%.6f,%.6f,%.6f,%.6f", TOFLT_TIME(pOutput->TimeStamp), TOFLT_PRECISE(pOutput->X),
            TOFLT_PRECISE(pOutput->Y), TOFLT_PRECISE(pOutput->Z));
    }
}


/****************************************************************************************************
 * @fn      UnCalMagDataResultCallback
 *          Call back for Uncalibrated magnetometer data
 *
 ***************************************************************************************************/
static void UnCalMagDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisExtendedData_t* pOutput)
{
    if (g_logging & 0x40)  //Uncalibrated data, in Android conventions
    {
        Print_LIPS("RM,%.6f,%.6f,%.6f,%.6f", TOFLT_TIME(pOutput->TimeStamp), TOFLT_EXTENDED(pOutput->X),
            TOFLT_EXTENDED(pOutput->Y), TOFLT_EXTENDED(pOutput->Z));
    }
}


/****************************************************************************************************
 * @fn      UnCalGyroDataResultCallback
 *          Call back for Uncalibrated gyroscope data
 *
 ***************************************************************************************************/
static void UnCalGyroDataResultCallback(ResultHandle_t outputHandle,
    Android_UncalibratedTriAxisPreciseData_t* pOutput)
{
    if (g_logging & 0x40)  //Uncalibrated data, in Android conventions
    {
        Print_LIPS("RG,%.6f,%.6f,%.6f,%.6f", TOFLT_TIME(pOutput->TimeStamp), TOFLT_PRECISE(pOutput->X),
            TOFLT_PRECISE(pOutput->Y), TOFLT_PRECISE(pOutput->Z));
    }
}


/****************************************************************************************************
 * @fn      stepDetectorResultCallback
 *          Call back for Step Detector results
 *
 ***************************************************************************************************/
static void stepCounterOutputCallback(ResultHandle_t OutputHandle,
    Android_StepCounterResultData_t* pOutput)
{
    Print_LIPS("STC,%+03.4f,%d,0", TOFLT_TIME(pOutput->TimeStamp), pOutput->StepCount);
}


/****************************************************************************************************
 * @fn      sigMotionOutputCallback
 *          Call back for Significant Motion results
 *
 ***************************************************************************************************/
static void sigMotionOutputCallback(ResultHandle_t outputHandle,
    Android_BooleanResultData_t* pOutput)
{
    Print_LIPS("SM,%+03.4f,%d",TOFLT_TIME(pOutput->TimeStamp), pOutput->data);
}


/****************************************************************************************************
 * @fn      HandleSensorData
 *          Handles sensor input data and feeds it to the sensor algorithms
 *
 ***************************************************************************************************/
static void HandleSensorData( MessageBuffer *pRcvMsg )
{
    osp_status_t status;
    OSP_InputSensorData_t sensorData;

    switch(pRcvMsg->msgId)
    {
    case MSG_ACC_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgAccelData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgAccelData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgAccelData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgAccelData.timeStamp;
        status = OSP_SetInputData(_AccHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);
        break;

    case MSG_MAG_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgMagData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgMagData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgMagData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgMagData.timeStamp;
        status = OSP_SetInputData(_MagHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);
        break;

    case MSG_GYRO_DATA:
        sensorData.rawdata.data[0] = pRcvMsg->msg.msgGyroData.X;
        sensorData.rawdata.data[1] = pRcvMsg->msg.msgGyroData.Y;
        sensorData.rawdata.data[2] = pRcvMsg->msg.msgGyroData.Z;
        sensorData.rawdata.TimeStamp = pRcvMsg->msg.msgGyroData.timeStamp;
        status = OSP_SetInputData(_GyroHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);
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
ASF_TASK  void AlgorithmTask ( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;
    OSP_STATUS_t OSP_Status;

    OSP_GetLibraryVersion(&version);
    D1_printf("OSP Version: %s\r\n", version->VersionString);

    OSP_Status = OSP_Initialize(&gSystemDesc);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_Initialize Failed");

    // Register the input sensors
    OSP_Status = OSP_RegisterInputSensor(&_AccSensDesc, &_AccHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (accel) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_MagSensDesc, &_MagHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (mag) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_GyroSensDesc, &_GyroHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "OSP_RegisterInputSensor (gyro) Failed");

    // Register output sensors/results
    OSP_Status =  OSP_SubscribeSensorResult(&stepCounterRequest, &_stepCounterHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_STEP_COUNTER) Failed");

    OSP_Status =  OSP_SubscribeSensorResult(&sigMotionRequest, &_sigMotionHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_CONTEXT_DEVICE_MOTION) Failed");

    OSP_Status =  OSP_SubscribeSensorResult(&UnCalAccelRequest, &_unCalAccelHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_ACCELEROMETER) Failed");

    OSP_Status =  OSP_SubscribeSensorResult(&UnCalMagRequest, &_unCalMagHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_MAGNETIC_FIELD) Failed");

    OSP_Status =  OSP_SubscribeSensorResult(&UnCalGyroRequest, &_unCalGyroHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_GYROSCOPE) Failed");

    while (1)
    {
        ASFReceiveMessage( ALGORITHM_TASK_ID, &rcvMsg );
        switch (rcvMsg->msgId)
        {
        case MSG_MAG_DATA:
            SendBgTrigger();
        case MSG_ACC_DATA:
        case MSG_GYRO_DATA:
            HandleSensorData(rcvMsg);
            do
            {
                OSP_Status = OSP_DoForegroundProcessing();
                ASF_assert(OSP_Status != OSP_STATUS_UNSPECIFIED_ERROR);
            } while(OSP_Status != OSP_STATUS_IDLE)
                ; //keep doing foreground computation until its finished
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
ASF_TASK  void AlgBackGndTask ( ASF_TASK_ARG )
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


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
