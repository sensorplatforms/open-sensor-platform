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

#define XYZ_FROM_ORIGIN                 TOFIX_PRECISE(0.0f)
#define ACCEL_INPUT_RATES               TOFIX_EXTENDED(12.5f),TOFIX_EXTENDED(25.0f),TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(100.0f)
#define MAG_INPUT_RATES                 TOFIX_EXTENDED(12.5f),TOFIX_EXTENDED(25.0f),TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(100.0f)
#define GYRO_INPUT_RATES                TOFIX_EXTENDED(12.5f),TOFIX_EXTENDED(25.0f),TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(100.0f)
#define STEP_COUNT_OUTPUT_RATE          TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f)
#define ACCEL_OUTPUT_RATES              TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f)
#define MAG_OUTPUT_RATES                TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f)
#define GYRO_OUTPUT_RATES               TOFIX_EXTENDED(50.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f),TOFIX_EXTENDED(0.0f)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static InputSensorSpecificData_t _AccInputSensor =
{
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR,ACCEL_SCALING_FACTOR, //scale factor (raw to dimensional units)
    ACCEL_RANGE_MAX,                        // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -ACCEL_RANGE_MAX,                       // min value that is valid
    ACCEL_NOISE,ACCEL_NOISE,ACCEL_NOISE,    // noise
    (void *) NULL,                          // calibration data structure
    "Acc LSM303DLHC",                       // Sensor name
    0,                                      // Sensor Vendor ID
    0,                                      // Sensor Product ID (defined by vendor)
    0,                                      // Sensor version (defined by vendor)
    0,                                      // Platform id  (defined by vendor)
    XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN, // X,Y,Z position from origin (in meters)
};

static SensorDescriptor_t _AccSensDesc =
{
    SENSOR_ACCELEROMETER,
    DATA_CONVENTION_RAW,
    OSP_NO_OUTPUT_READY_CALLBACK,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    ACCEL_INPUT_RATES,
    OSP_FLAGS_INPUT_SENSOR,
    &_AccInputSensor
};

static InputSensorSpecificData_t _MagInputSensor =
{
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    MAG_SCALING_XY,MAG_SCALING_XY,MAG_SCALING_Z, //scale factor (raw to dimensional units)
    MAG_RANGE_MAX,                          // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -MAG_RANGE_MAX,                         // min value that is valid
    MAG_NOISE,MAG_NOISE,MAG_NOISE,          // noise
    (void *) NULL,                          // calibration data structure
    "Mag LSM303DLHC",                       // Sensor name
    0,                                      // Sensor Vendor ID
    0,                                      // Sensor Product ID (defined by vendor)
    0,                                      // Sensor version (defined by vendor)
    0,                                      // Platform id  (defined by vendor)
    XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN, // X,Y,Z position from origin (in meters)
};

static SensorDescriptor_t _MagSensDesc =
{
    SENSOR_MAGNETIC_FIELD,
    DATA_CONVENTION_RAW,
    OSP_NO_OUTPUT_READY_CALLBACK,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    MAG_INPUT_RATES,
    OSP_FLAGS_INPUT_SENSOR,
    &_MagInputSensor
};

static InputSensorSpecificData_t _GyroInputSensor =
{
    0xffffffff,                             // 32 bits of raw data are significant
    AXIS_MAP_POSITIVE_X,AXIS_MAP_POSITIVE_Y,AXIS_MAP_POSITIVE_Z, // X,Y,Z  sensor orientation
    0,0,0,                                  // raw data offset (where is zero?)
    GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR,GYRO_SCALING_FACTOR, //scale factor (raw to dimensional units)
    GYRO_RANGE_MAX,                         // max value that is valid (e.g. +/- 4G sensor in M/Sec*Sec)
    -GYRO_RANGE_MAX,                        // min value that is valid
    GYRO_NOISE,GYRO_NOISE,GYRO_NOISE,       // noise
    (void *) NULL,                          // calibration data structure
    "Gyro L3GD20",                          // Sensor name
    0,                                      // Sensor Vendor ID
    0,                                      // Sensor Product ID (defined by vendor)
    0,                                      // Sensor version (defined by vendor)
    0,                                      // Platform id  (defined by vendor)
    XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN,XYZ_FROM_ORIGIN, // X,Y,Z position from origin (in meters)
};

static SensorDescriptor_t _GyroSensDesc =
{
    SENSOR_GYROSCOPE,
    DATA_CONVENTION_RAW,
    OSP_NO_OUTPUT_READY_CALLBACK,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    GYRO_INPUT_RATES,
    OSP_FLAGS_INPUT_SENSOR,
    &_GyroInputSensor
};

static void stepCounterOutputCallback(OutputSensorHandle_t outputHandle,
    Android_StepCounterOutputData_t* pOutput);

static void UnCalAccelDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedAccelOutputData_t* pOutput);

static void UnCalMagDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedMagOutputData_t* pOutput);

static void UnCalGyroDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedGyroOutputData_t* pOutput);

/* Output result descriptor for subscribing to step counts */
static SensorDescriptor_t  stepCounterRequest = {
    SENSOR_STEP_COUNTER,
    DATA_CONVENTION_ANDROID,
    (OSP_OutputReadyCallback_t)stepCounterOutputCallback,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    STEP_COUNT_OUTPUT_RATE,
    OSP_NO_FLAGS,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated accelerometer data */
static SensorDescriptor_t UnCalAccelRequest = {
    SENSOR_ACCELEROMETER,
    DATA_CONVENTION_ANDROID,
    (OSP_OutputReadyCallback_t)UnCalAccelDataResultCallback,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    ACCEL_OUTPUT_RATES,
    OSP_FLAGS_UNCALIBRATED,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated magnetometer data */
static SensorDescriptor_t UnCalMagRequest = {
    SENSOR_MAGNETIC_FIELD,
    DATA_CONVENTION_ANDROID,
    (OSP_OutputReadyCallback_t)UnCalMagDataResultCallback,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    MAG_OUTPUT_RATES,
    OSP_FLAGS_UNCALIBRATED,
    OSP_NO_OPTIONAL_DATA
};

/* Output result descriptor for subscribing to uncalibrated gyroscope data */
static SensorDescriptor_t UnCalGyroRequest = {
    SENSOR_GYROSCOPE,
    DATA_CONVENTION_ANDROID,
    (OSP_OutputReadyCallback_t)UnCalGyroDataResultCallback,
    OSP_NO_NVM_WRITE_CALLBACK,
    OSP_NO_SENSOR_CONTROL_CALLBACK,
    GYRO_OUTPUT_RATES,
    OSP_FLAGS_UNCALIBRATED,
    OSP_NO_OPTIONAL_DATA
};

static InputSensorHandle_t _AccHandle;
static InputSensorHandle_t _MagHandle;
static InputSensorHandle_t _GyroHandle;
static const OSP_Library_Version_t* version;
static OutputSensorHandle_t _stepCounterHandle;
static OutputSensorHandle_t _unCalAccelHandle;
static OutputSensorHandle_t _unCalMagHandle;
static OutputSensorHandle_t _unCalGyroHandle;

static OS_MUT mutexCritSection;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

static void EnterCriticalSection(void);
static void ExitCriticalSection(void);

SystemDescriptor_t gSystemDesc =
{
    TOFIX_TIMECOEFFICIENT(US_PER_RTC_TICK * 0.000001f),        // timestamp conversion factor = 1us / count
    (OSP_CriticalSectionCallback_t) EnterCriticalSection,
    (OSP_CriticalSectionCallback_t) ExitCriticalSection
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
static void UnCalAccelDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedAccelOutputData_t* pOutput)
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
static void UnCalMagDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedMagOutputData_t* pOutput)
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
static void UnCalGyroDataResultCallback(OutputSensorHandle_t outputHandle,
    Android_UncalibratedGyroOutputData_t* pOutput)
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
static void stepCounterOutputCallback(OutputSensorHandle_t OutputHandle,
    Android_StepCounterOutputData_t* pOutput)
{
    Print_LIPS("STC, %+03.2f, %d,0", TOFLT_TIME(pOutput->TimeStamp), pOutput->StepCount);
}


/****************************************************************************************************
 * @fn      HandleSensorData
 *          Handles sensor input data and feeds it to the sensor algorithms
 *
 ***************************************************************************************************/
static void HandleSensorData( MessageBuffer *pRcvMsg )
{
    osp_status_t status;
    TriAxisSensorRawData_t sensorData;

    switch(pRcvMsg->msgId)
    {
    case MSG_ACC_DATA:
        sensorData.Data[0] = pRcvMsg->msg.msgAccelData.X;
        sensorData.Data[1] = pRcvMsg->msg.msgAccelData.Y;
        sensorData.Data[2] = pRcvMsg->msg.msgAccelData.Z;
        sensorData.TimeStamp = pRcvMsg->msg.msgAccelData.timeStamp;
        status = OSP_SetData(_AccHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);
        break;

    case MSG_MAG_DATA:
        sensorData.Data[0] = pRcvMsg->msg.msgMagData.X;
        sensorData.Data[1] = pRcvMsg->msg.msgMagData.Y;
        sensorData.Data[2] = pRcvMsg->msg.msgMagData.Z;
        sensorData.TimeStamp = pRcvMsg->msg.msgMagData.timeStamp;
        status = OSP_SetData(_MagHandle, &sensorData);
        ASF_assert(status == OSP_STATUS_OK);
        break;

    case MSG_GYRO_DATA:
        sensorData.Data[0] = pRcvMsg->msg.msgGyroData.X;
        sensorData.Data[1] = pRcvMsg->msg.msgGyroData.Y;
        sensorData.Data[2] = pRcvMsg->msg.msgGyroData.Z;
        sensorData.TimeStamp = pRcvMsg->msg.msgGyroData.timeStamp;
        status = OSP_SetData(_GyroHandle, &sensorData);
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
    osp_status_t OSP_Status;

    OSP_GetVersion(&version);
    D1_printf("OSP Version: %s\r\n", version->VersionString);

    OSP_Status = OSP_Initialize(&gSystemDesc);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_Initialize Failed");

    // Register the input sensors
    OSP_Status = OSP_RegisterInputSensor(&_AccSensDesc, &_AccHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_RegisterSensor (accel) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_MagSensDesc, &_MagHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_RegisterSensor (mag) Failed");

    OSP_Status = OSP_RegisterInputSensor(&_GyroSensDesc, &_GyroHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_RegisterSensor (gyro) Failed");

    // Register output sensors/results
    OSP_Status =  OSP_SubscribeOutputSensor(&stepCounterRequest, &_stepCounterHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_STEP_COUNTER) Failed");

    OSP_Status =  OSP_SubscribeOutputSensor(&UnCalAccelRequest, &_unCalAccelHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_ACCELEROMETER) Failed");

    OSP_Status =  OSP_SubscribeOutputSensor(&UnCalMagRequest, &_unCalMagHandle);
    ASF_assert_msg(OSP_STATUS_OK == OSP_Status, "SensorManager: OSP_SubscribeResult (SENSOR_MAGNETIC_FIELD) Failed");

    OSP_Status =  OSP_SubscribeOutputSensor(&UnCalGyroRequest, &_unCalGyroHandle);
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
                ASF_assert(OSP_Status != OSP_STATUS_ERROR);
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
