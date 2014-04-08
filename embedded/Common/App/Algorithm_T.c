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

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static SensorDescriptor_t _AccSensDesc =
{
};

static SensorDescriptor_t _MagSensDesc =
{
};

static SensorDescriptor_t _GyroSensDesc =
{
};

static ResultRequestDescriptor_t _ResDesc_CalibratedAccelData =
{
};

static ResultRequestDescriptor_t _ResDesc_CalibratedMagData =
{
};

static ResultRequestDescriptor_t _ResDesc_CalibratedGyroData =
{
};

static ResultRequestDescriptor_t _ResDesc_RotVector =
{
};

static ResultRequestDescriptor_t _ResDesc_UnCalibratedAccelData =
{
};

static ResultRequestDescriptor_t _ResDesc_UnCalibratedMagData =
{
};

static ResultRequestDescriptor_t _ResDesc_UnCalibratedGyroData =
{
};


ResultRequestDescriptor_t  changeDetectorRequest =
{
};

ResultRequestDescriptor_t  stepDetectorRequest =
{
};

static SensorHandle_t _AccHandle;
static SensorHandle_t _MagHandle;
static SensorHandle_t _GyroHandle;

static FusionResultHandle_t _AccelDataHandle;
static FusionResultHandle_t _MagDataHandle;
static FusionResultHandle_t _GyroDataHandle;
static FusionResultHandle_t _RotVectorHandle;
static FusionResultHandle_t _UnCalAccelDataHandle;
static FusionResultHandle_t _UnCalMagDataHandle;
static FusionResultHandle_t _UnCalGyroDataHandle;
static FusionResultHandle_t stepDetectorHandle;
static FusionResultHandle_t changeDetectorHandle;

static OS_MUT mutexCritSection;

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

static void SensorDataResultCallback(FusionResultHandle_t ResultHandle, SPI_CalibratedAccelResultData_t *pSensResult);
static void UnCalAccelDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedAccelResultData_t* Result);
static void UnCalMagDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedMagResultData_t* Result);
static void UnCalGyroDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedGyroResultData_t* Result);
static void rotationVectorResultCallback(FusionResultHandle_t ResultHandle, RotationVectorResultData_t* pResult);
static void EnterCriticalSection(void);
static void ExitCriticalSection(void);
static int32_t changeDetectorResultCallback(FusionResultHandle_t resultHandle, ContextChangeDetectorData_t* pResult);
static int32_t stepDetectorResultCallback(FusionResultHandle_t resultHandle, Android_StepDetectorResultData_t* pResult);


SystemDescriptor_t gSysDesc =
{
    TOFIX_TIMECOEFFICIENT(US_PER_RTC_TICK * 0.000001f),        // timestamp conversion factor = 1us / count
    (FM_CriticalSectionCallback_t) EnterCriticalSection,
    (FM_CriticalSectionCallback_t) ExitCriticalSection
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
 * @fn      SensorDataResultCallback
 *          Callback handler for calibrated sensor results
 *
 ***************************************************************************************************/
static void SensorDataResultCallback(FusionResultHandle_t ResultHandle, Win8_CalibratedAccelResultData_t *pSensResult)
{
}


/****************************************************************************************************
 * @fn      rotationVectorResultCallback
 *          Callback handler for rotation results computed by FreeMotion algorithm
 *          Data is normalized quaternion * 1000
 *
 ***************************************************************************************************/
static void rotationVectorResultCallback(FusionResultHandle_t ResultHandle, RotationVectorResultData_t *pResult)
{
}


/****************************************************************************************************
 * @fn      UnCalAccelDataResultCallback
 *          Call back for uncalibrated sensor data is used to display formatted data for visualization
 *
 ***************************************************************************************************/
static void UnCalAccelDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedAccelResultData_t *pResult)
{
}


/****************************************************************************************************
 * @fn      UnCalMagDataResultCallback
 *          Call back for uncalibrated sensor data is used to display formatted data for visualization
 *
 ***************************************************************************************************/
static void UnCalMagDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedMagResultData_t *pResult)
{
}


/****************************************************************************************************
 * @fn      UnCalGyroDataResultCallback
 *          Call back for uncalibrated sensor data is used to display formatted data for visualization
 *
 ***************************************************************************************************/
static void UnCalGyroDataResultCallback(FusionResultHandle_t ResultHandle, SPI_UnCalibratedGyroResultData_t *pResult)
{
}


/****************************************************************************************************
 * @fn      changeDetectorResultCallback
 *          Call back for Change Detector results
 *
 ***************************************************************************************************/
static int32_t changeDetectorResultCallback(FusionResultHandle_t resultHandle, ContextChangeDetectorData_t* pResult)
{
}


/****************************************************************************************************
 * @fn      stepDetectorResultCallback
 *          Call back for Step Detector results
 *
 ***************************************************************************************************/
static int32_t stepDetectorResultCallback(FusionResultHandle_t resultHandle, Android_StepDetectorResultData_t* pResult)
{
}


/****************************************************************************************************
 * @fn      DoIterationAndSend
 *          Runs mechanization on the input data and initiates sending of processed data back to
 *          sensor acquisition task.
 *
 ***************************************************************************************************/
static void DoIterationAndSend(MessageBuffer *pRcvMsg)
{
    switch(pRcvMsg->msgId)
    {
    case MSG_ACC_DATA:
        break;

    case MSG_MAG_DATA:
        break;

    case MSG_GYRO_DATA:
        break;

    default:
        D1_printf("ALG: Bad message: %d\r\n", pRcvMsg->msgId);
        break;
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      AlgorithmTask
 *          This task is responsible for running the sensor algorithms on the incoming sensor
 *          data (could be raw or filtered) and sending results to the host communication task
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK  void AlgorithmTask ( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;
#if 0
    FM_STATUS_t FM_Status;

    timestamp = (float)(RTC_GetCounter() * US_PER_RTC_TICK) * 0.000001f; //Time in seconds

    FM_Status = FM_Initialize(&gSysDesc);  // initialize the FM_Lib
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_Initialize Failed");

    // register the sensors
    FM_Status = FM_RegisterSensor(&_AccSensDesc, &_AccHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_RegisterSensor (accel) Failed");

    FM_Status = FM_RegisterSensor(&_MagSensDesc, &_MagHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_RegisterSensor (mag) Failed");

    FM_Status = FM_RegisterSensor(&_GyroSensDesc, &_GyroHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_RegisterSensor (gyro) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_CalibratedAccelData, &_AccelDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (accel) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_CalibratedMagData, &_MagDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (mag) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_CalibratedGyroData, &_GyroDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (gyro) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_RotVector, &_RotVectorHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (Rot. Vector) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_UnCalibratedAccelData, &_UnCalAccelDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (uncal-accel) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_UnCalibratedMagData, &_UnCalMagDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (uncal-mag) Failed");

    FM_Status = FM_SubscribeResult(&_ResDesc_UnCalibratedGyroData, &_UnCalGyroDataHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (uncal-gyro) Failed");

    FM_Status =  FM_SubscribeResult(&stepDetectorRequest, &stepDetectorHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (RESULT_STEP_DETECTOR) Failed");

    FM_Status =  FM_SubscribeResult(&changeDetectorRequest, &changeDetectorHandle);
    FM_ASSERT(FM_STATUS_OK == FM_Status, "SensorManager: FM_SubscribeResult (RESULT_CONTEXT_CHANGE_DETECTOR) Failed");
#endif

    while (1)
    {
        ASFReceiveMessage( ALGORITHM_TASK_ID, &rcvMsg );
        switch (rcvMsg->msgId)
        {
        case MSG_MAG_DATA:
        case MSG_ACC_DATA:
        case MSG_GYRO_DATA:
            DoIterationAndSend(rcvMsg);
            //while (FM_DoForegroundProcessing() != FM_STATUS_IDLE)
            //    ;
            break;

        default:
            /* Unhandled messages */
            D1_printf("Algo:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
