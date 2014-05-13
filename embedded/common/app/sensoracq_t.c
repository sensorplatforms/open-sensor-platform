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
#include "acc_common.h"
#include "mag_common.h"
#include "gyro_common.h"
#include "osp-api.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void WaitForHostSync( void );

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
#ifndef INTERRUPT_BASED_SAMPLING
 static AsfTimer sSensorTimer = NULL_TIMER;
#else
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static void SensorDataHandler( SensorType_t sensorId, uint32_t timeStamp );

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

#ifndef INTERRUPT_BASED_SAMPLING
/****************************************************************************************************
 * @fn      HandleTimers
 *          Handles the MSG_TIMER_EXPIRY messages
 *
 ***************************************************************************************************/
static void HandleTimers( MsgTimerExpiry *pTimerExp )
{
    uint32_t timeStamp;

    switch (pTimerExp->userValue)
    {
        case TIMER_REF_SENSOR_READ:
            /* Schedule the next sampling */
            ASFTimerStart( SENSOR_ACQ_TASK_ID, TIMER_REF_SENSOR_READ, SENSOR_SAMPLE_PERIOD, &sSensorTimer );

            /* Call data handler for each sensors */
            timeStamp = RTC_GetCounter();
            SensorDataHandler( SENSOR_TYPE_ACCELEROMETER, timeStamp );
            SensorDataHandler( SENSOR_TYPE_MAGNETIC_FIELD, timeStamp );
            SensorDataHandler( SENSOR_TYPE_GYROSCOPE, timeStamp );
            break;

        default:
            D1_printf("Unknown Timer! Reference %d\r\n", pTimerExp->userValue);
            break;
    }
}
#endif


#ifdef ENABLE_FLASH_STORE
/****************************************************************************************************
 * @fn      StoreCalibrationData
 *          Stores calibration data. Why in this task?. Doing it in Sensor acq makes it unlikely to
 *      interfere with the sensor data reads over I2C.
 *
 ***************************************************************************************************/
static void StoreCalibrationData( CalEvent_t event )
{
}
#endif

/****************************************************************************************************
 * @fn      SensorDataHandler
 *          Handle data ready indications from ISR for various sensors
 *
 ***************************************************************************************************/
static void SensorDataHandler( SensorType_t sensorId, uint32_t timeStamp )
{
    MsgAccelData accelData;
    MsgMagData magData;
    MsgGyroData gyroData;
    static uint32_t sMagDecimateCount = 0;
    static uint32_t gyroSampleCount = 0;
    static uint32_t accSampleCount = 0;

#if defined ALGORITHM_TASK || defined ANDROID_COMM_TASK
    MessageBuffer *pMagSample = NULLP;
    MessageBuffer *pAccSample = NULLP;
    MessageBuffer *pGyroSample = NULLP;
#endif

    switch (sensorId)
    {
    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        if ((sMagDecimateCount++ % MAG_DECIMATE_FACTOR) == 0 )
        {
            /* Read mag Data - reading would clear interrupt also */
            Mag_ReadData( &magData );
            /* Replace time stamp with that captured by interrupt handler */
            magData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_MAG_DATA, sizeof(MsgMagData), &pMagSample ) == ASF_OK );
            pMagSample->msg.msgMagData = magData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pMagSample ) == ASF_OK );
#endif
#ifdef ANDROID_COMM_TASK
            pMagSample = NULLP;
            /* Send sample to Host interface also */
            ASF_assert( ASFCreateMessage( MSG_MAG_DATA, sizeof(MsgMagData), &pMagSample ) == ASF_OK );
            pMagSample->msg.msgMagData = magData;
            ASFSendMessage( ANDROID_COMM_TASK, pMagSample ); // No error check. We are OK to drop messages
#endif
        }
        else
        {
            Mag_ClearDataInt();
        }
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        if ((gyroSampleCount++ % GYRO_SAMPLE_DECIMATE) == 0)
        {
            /* Read Gyro Data - reading typically clears interrupt as well */
            Gyro_ReadData( &gyroData ); //Reads also clears DRDY interrupt
            /* Replace time stamp with that captured by interrupt handler */
            gyroData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_GYRO_DATA, sizeof(MsgGyroData), &pGyroSample ) == ASF_OK );
            pGyroSample->msg.msgGyroData = gyroData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pGyroSample ) == ASF_OK );
#endif
#ifdef ANDROID_COMM_TASK
            pGyroSample = NULLP;
            /* Send sample to Host interface also */
            ASF_assert( ASFCreateMessage( MSG_GYRO_DATA, sizeof(MsgGyroData), &pGyroSample ) == ASF_OK );
            pGyroSample->msg.msgGyroData = gyroData;
            ASFSendMessage( ANDROID_COMM_TASK, pGyroSample ); // No error check. We are OK to drop messages
#endif
        }
        else
        {
            Gyro_ClearDataInt();
        }
        break;

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
#if defined TRIGGERED_MAG_SAMPLING
        if (accSampleCount % MAG_TRIGGER_RATE_DECIMATE == 0)
        {
            Mag_TriggerDataAcq(); //Mag is triggered relative to Accel to avoid running separate timer
        }
#endif
        if (accSampleCount++ % ACCEL_SAMPLE_DECIMATE == 0)
        {
            /* Read Accel Data - reading typically clears interrupt as well */
            Accel_ReadData( &accelData );
            /* Replace time stamp with that captured by interrupt handler */
            accelData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_ACC_DATA, sizeof(MsgAccelData), &pAccSample ) == ASF_OK );
            pAccSample->msg.msgAccelData = accelData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pAccSample ) == ASF_OK );
#endif
#ifdef ANDROID_COMM_TASK
            pAccSample = NULLP;
            /* Send sample to Host interface also */
            ASF_assert( ASFCreateMessage( MSG_ACC_DATA, sizeof(MsgAccelData), &pAccSample ) == ASF_OK );
            pAccSample->msg.msgAccelData = accelData;
            ASFSendMessage( ANDROID_COMM_TASK, pAccSample ); // No error check. We are OK to drop messages
#endif

        }
        else
        {
            Accel_ClearDataInt();
        }
        break;

    default:
        break;

    }
}

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      SendDataReadyIndication
 * @brief  This helper function sends data ready indication to Sensor Acq task. Called from ISR
 * @param  sensorId: Sensor identifier whose data is ready to be read
 * @return None
 *
 ***************************************************************************************************/
void SendDataReadyIndication( uint8_t sensorId, uint32_t timeStamp )
{
    MessageBuffer *pSendMsg = NULLP;

    ASF_assert( ASFCreateMessage( MSG_SENSOR_DATA_RDY, sizeof(MsgSensorDataRdy), &pSendMsg ) == ASF_OK );
    pSendMsg->msg.msgSensorDataRdy.sensorId = sensorId;
    pSendMsg->msg.msgSensorDataRdy.timeStamp = timeStamp;
    ASF_assert( ASFSendMessage( SENSOR_ACQ_TASK_ID, pSendMsg ) == ASF_OK );

}


/****************************************************************************************************
 * @fn      SensorAcqTask
 *          This task is responsible for acquiring data from and controlling the sensors in the
 *          system.
 *
 * @param   none
 *
 * @return  none
 *
 ***************************************************************************************************/
ASF_TASK void SensorAcqTask( ASF_TASK_ARG )
{
    MessageBuffer *rcvMsg = NULLP;
    volatile uint8_t  i;

#ifndef WAIT_FOR_HOST_SYNC
    os_dly_wait(MSEC_TO_TICS(50)); /* Allow startup time for sensors */
#else
    WaitForHostSync(); //This also allows for startup time for sensors
#endif

    /* Setup interface for the Magnetometer */
    Mag_HardwareSetup( true );
    Mag_Initialize();
    /* Setup interface for the accelerometers */
    Accel_HardwareSetup( true );
    Accel_Initialize( INIT_NORMAL );
    /* Setup Gyro */
    Gyro_HardwareSetup( true );
    Gyro_Initialize();

#ifndef INTERRUPT_BASED_SAMPLING
    /* Start sample period timer */
    ASFTimerStart( SENSOR_ACQ_TASK_ID, TIMER_REF_SENSOR_READ, SENSOR_SAMPLE_PERIOD, &sSensorTimer );
#else
    /* Enable Sensor interrupts */
    Mag_ConfigDataInt( true );
    Mag_ClearDataInt();
    Accel_ConfigDataInt( true );
    Gyro_ConfigDataInt( true );
# ifdef TRIGGERED_MAG_SAMPLING
    Mag_SetLowPowerMode(); //Low power mode until triggered
# endif
#endif

    while (1)
    {
        ASFReceiveMessage( SENSOR_ACQ_TASK_ID, &rcvMsg );
        switch (rcvMsg->msgId)
        {

            case MSG_TIMER_EXPIRY:
#ifndef INTERRUPT_BASED_SAMPLING
                HandleTimers( &rcvMsg->msg.msgTimerExpiry );
#endif
                break;

            case MSG_CAL_EVT_NOTIFY:
#ifdef ENABLE_FLASH_STORE
                StoreCalibrationData( (CalEvent_t)rcvMsg->msg.msgCalEvtNotify.byte );
#else
                D0_printf("#### WARNING - NV Storage Not Implemented! #####\r\n");
#endif
                break;

            case MSG_SENSOR_DATA_RDY:
#ifdef INTERRUPT_BASED_SAMPLING
                SensorDataHandler( (SensorType_t)rcvMsg->msg.msgSensorDataRdy.sensorId,
                    rcvMsg->msg.msgSensorDataRdy.timeStamp);
#endif
                break;

            default:
                /* Unhandled messages */
                D2_printf("Snsr:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
                break;
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
