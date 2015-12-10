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
#include "Driver_Sensor.h"
#include "osp-api.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
void WaitForHostSync( void );
extern OSP_DRIVER_SENSOR Driver_Accel;
extern OSP_DRIVER_SENSOR Driver_Mag;
extern OSP_DRIVER_SENSOR Driver_Gyro;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
// Uncomment this to enable algorithm library to turn on/off sensor based on resource usage
//#define SENSOR_CONTROL_ENABLE

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
static void SensorDataHandler(InputSensor_t sensorId, uint32_t timeStamp);

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      HandleTimers
 *          Handles the MSG_TIMER_EXPIRY messages
 *
 ***************************************************************************************************/
static void HandleTimers( MsgTimerExpiry *pTimerExp )
{
    switch (pTimerExp->userValue)
    {
#ifndef INTERRUPT_BASED_SAMPLING
        uint32_t timeStamp;

        case TIMER_REF_SENSOR_READ:
        /* Schedule the next sampling */
        ASFTimerStart(SENSOR_ACQ_TASK_ID, TIMER_REF_SENSOR_READ,
                SENSOR_SAMPLE_PERIOD, &sSensorTimer);

        /* Call data handler for each sensors */
        timeStamp = RTC_GetCounter();
        SensorDataHandler(ACCEL_INPUT_SENSOR, timeStamp);
        SensorDataHandler(MAG_INPUT_SENSOR, timeStamp);
        SensorDataHandler(GYRO_INPUT_SENSOR, timeStamp);
        break;
#endif
    default:
        D1_printf("Unknown Timer! Reference %d\r\n", pTimerExp->userValue);
        break;
    }
}


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
static void SensorDataHandler(InputSensor_t sensorId, uint32_t timeStamp)
{
    MsgAccelData accelData;
    MsgMagData magData;
    MsgGyroData gyroData;
    static uint32_t sMagDecimateCount = 0;
    static uint32_t gyroSampleCount = 0;
    static uint32_t accSampleCount = 0;

#if defined ALGORITHM_TASK
    MessageBuffer *pMagSample = NULLP;
    MessageBuffer *pAccSample = NULLP;
    MessageBuffer *pGyroSample = NULLP;
#endif

    switch (sensorId)
    {
    case MAG_INPUT_SENSOR:
        if ((sMagDecimateCount++ % MAG_DECIMATE_FACTOR) == 0 )
        {
            /* Read mag Data - reading would clear interrupt also */
            Driver_Mag.ReadData(&magData,1);
            /* Replace time stamp with that captured by interrupt handler */
            magData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_MAG_DATA, sizeof(MsgMagData), &pMagSample ) == ASF_OK );
            pMagSample->msg.msgMagData = magData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pMagSample ) == ASF_OK );
#endif
        }
        else
        {
            /* Reading would clear interrupt also */
            Driver_Mag.ReadData(&magData,1);
        }
        break;

    case GYRO_INPUT_SENSOR:
        if ((gyroSampleCount++ % GYRO_SAMPLE_DECIMATE) == 0)
        {
            /* Read Gyro Data - reading typically clears interrupt as well */
            Driver_Gyro.ReadData(&gyroData,1); //Reads also clears DRDY interrupt
            /* Replace time stamp with that captured by interrupt handler */
            gyroData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_GYRO_DATA, sizeof(MsgGyroData), &pGyroSample ) == ASF_OK );
            pGyroSample->msg.msgGyroData = gyroData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pGyroSample ) == ASF_OK );
#endif
        }
        else
        {
            /* Reading would clear interrupt also */
            Driver_Gyro.ReadData(&gyroData,1); //Reads also clears DRDY interrupt
        }
        break;

    case ACCEL_INPUT_SENSOR:
#if defined TRIGGERED_MAG_SAMPLING
        if (accSampleCount % MAG_TRIGGER_RATE_DECIMATE == 0)
        {
            /* PG: No analogous function in sensor HAL. Need to find a workaround. TBD */
            Mag_TriggerDataAcq(); //Mag is triggered relative to Accel to avoid running separate timer
        }
#endif
        if (accSampleCount++ % ACCEL_SAMPLE_DECIMATE == 0)
        {
            /* Read Accel Data - reading typically clears interrupt as well */
            Driver_Accel.ReadData(&accelData,1);
            /* Replace time stamp with that captured by interrupt handler */
            accelData.timeStamp = timeStamp;
#ifdef ALGORITHM_TASK
            ASF_assert( ASFCreateMessage( MSG_ACC_DATA, sizeof(MsgAccelData), &pAccSample ) == ASF_OK );
            pAccSample->msg.msgAccelData = accelData;
            ASF_assert( ASFSendMessage( ALGORITHM_TASK_ID, pAccSample ) == ASF_OK );
#endif

        }
        else
        {
            /* Reading would clear interrupt also */
            Driver_Accel.ReadData(&accelData,1);
        }
        break;

    default:
        break;

    }
}


/***********************************************************************
 * @fn      SensorControlCmdHandler
 *          Handle sensor parameter setting request
 *
 ***********************************************************************/
void SensorControlCmdHandler(MsgSensorControlData *pData)
{
    InputSensor_t sensorType;
    SensorControlCommand_t command;
    sensorType = (InputSensor_t)pData->sensorType;

    command = (SensorControlCommand_t) pData->command;
    switch(command){

    case SENSOR_CONTROL_SENSOR_OFF:
        // e.g. put sensor into power saving mode
        D0_printf("Sensor Control set sensor idx %d OFF\r\n", sensorType);
        switch ( sensorType){

        case ACCEL_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            // Maybe add a user configurable 'powerMode' setting and here we
            // configure the sensor hardware according to its value.
            Driver_Accel.Deactivate();         // do not disable accel so watch window input will work
#endif
            break;

        case MAG_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            Driver_Mag.Deactivate();
#endif
            break;

        case  GYRO_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            Driver_Gyro.Deactivate();
#endif
            break;

        default:
            break;
        }
        break;

    case SENSOR_CONTROL_SENSOR_SLEEP:
        // put sensor into sleep mode
        D0_printf("Sensor Control set sensor idx %d to SLEEP mode\r\n", sensorType);
        break;

    case SENSOR_CONTROL_SENSOR_ON:
        // Turn sensor into normal operating mode
        D0_printf("Sensor Control set sensor idx %d ON\r\n", sensorType);
        switch ( sensorType){

        case ACCEL_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            Driver_Accel.Activate(SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */,NULL /* unused */);
#endif
            break;

        case MAG_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            Driver_Mag.Activate(SENSOR_FLAG_ONE_SHOT_MODE,NULL /* unused */,NULL /* unused */,NULL /* unused */);
#endif
            break;

        case GYRO_INPUT_SENSOR:
#ifdef SENSOR_CONTROL_ENABLE
            Driver_Gyro.Activate(SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */,NULL /* unused */);
#endif
            break;

        default:
            break;
        }
        break;

    case SENSOR_CONTROL_SET_SAMPLE_RATE:
        // Update the sensor output rate
        D0_printf("Sensor Control set sensor idx %d sampe rate to %d\r\n",
                  sensorType, pData->data);
        break;

    case SENSOR_CONTROL_SET_LPF_FREQ:
        D0_printf("Sensor Control set sensor idx %d LPF to %d\r\n",
                  sensorType, pData->data);
        break;

    case SENSOR_CONTROL_SET_HPF_FREQ:
        D0_printf("Sensor Control set sensor idx %d HPF to %d\r\n",
                  sensorType, pData->data);
        break;

    default:
        D0_printf("Invalid sensor control command value (%d)\r\n", pData->command);
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
 *          Sensor data flow:
 *          1. Sensor interrupts on data ready.
 *          2. IRQ handler is solely expected to call SendDataReadyIndication.
 *          3. SensorAcqTask will see a message data is available.
 *          4. Data is read. Sensor driver is expected to clear
 *             the interrupt from the read function.
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
    Driver_Mag.Initialize(NULL, NULL);

    /* Setup interface for the accelerometers */
    Driver_Accel.Initialize(NULL, NULL);

    /* Setup Gyro */
    Driver_Gyro.Initialize(NULL, NULL);

#ifndef INTERRUPT_BASED_SAMPLING
    /* Start sample period timer */
    ASFTimerStart( SENSOR_ACQ_TASK_ID, TIMER_REF_SENSOR_READ, SENSOR_SAMPLE_PERIOD, &sSensorTimer );
#else
    /* Enable Sensor interrupts */
    Driver_Mag.Activate(SENSOR_FLAG_ONE_SHOT_MODE,NULL /* unused */,NULL /* unused */,NULL /* unused */);
    Driver_Accel.Activate(SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */,NULL /* unused */);
    Driver_Gyro.Activate(SENSOR_FLAG_CONTINUOUS_DATA,NULL /* unused */,NULL /* unused */,NULL /* unused */);
#ifdef TRIGGERED_MAG_SAMPLING
    Driver_Mag.PowerControl(ARM_POWER_LOW); //Low power mode until triggered
# endif
#endif

    while (1) {
        ASFReceiveMessage(SENSOR_ACQ_TASK_ID, &rcvMsg);

        switch (rcvMsg->msgId)
        {
        case MSG_TIMER_EXPIRY:
            HandleTimers(&rcvMsg->msg.msgTimerExpiry);
            break;

        case MSG_CAL_EVT_NOTIFY:
#ifdef ENABLE_FLASH_STORE
            StoreCalibrationData((CalEvent_t)rcvMsg->msg.msgCalEvtNotify.byte);
#else
            D0_printf("#### WARNING - NV Storage Not Implemented! #####\r\n");
#endif
            break;

        case MSG_SENSOR_DATA_RDY:
#ifdef INTERRUPT_BASED_SAMPLING
            SensorDataHandler((InputSensor_t)rcvMsg->msg.msgSensorDataRdy.sensorId,
                    rcvMsg->msg.msgSensorDataRdy.timeStamp);
#endif
            break;

        case MSG_SENSOR_CONTROL:
            SensorControlCmdHandler(&rcvMsg->msg.msgSensorControlData);
            break;

        default:
            /* Unhandled messages */
            D2_printf("SensorAcqTask:!!!UNHANDLED MESSAGE:%d!!!\r\n", rcvMsg->msgId);
            break;
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
