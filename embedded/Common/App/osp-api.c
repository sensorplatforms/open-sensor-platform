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
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

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
/****************************************************************************************************
 * @fn      StaticFunc1
 *          Helper routine for ???
 *
 ***************************************************************************************************/


/*-------------------------------------------------------------------------------------------------*\
 |    A P I     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      OSP_Initialize
 *          Does internal initializations that the library requires.
 *
 * @param   pSystemDesc - INPUT pointer to a struct that describes things like time tick conversion
 *          value
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_Initialize(const SystemDescriptor_t* pSystemDesc)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_RegisterInputSensor
 *          Tells the Open-Sensor-Platform Library what kind of sensor inputs it has to work with.
 *
 * @param   pSensorDescriptor INPUT pointer to data which describes all the details of this sensor
 *          and its current operating mode; e.g. sensor type, SI unit conversion factor
 * @param   pReturnedHandle OUTPUT a handle to use when feeding data in via OSP_SetData()
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_RegisterInputSensor(SensorDescriptor_t *pSensorDescriptor,
    InputSensorHandle_t *pReturnedHandle)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_UnregisterInputSensor
 *          Call to remove an sensor from OSP's known set of inputs.
 *
 * @param   handle INPUT a handle to the input sensor you want to unregister
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_UnregisterInputSensor(InputSensorHandle_t handle)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_SetData
 *          Queues sensor data which will be processed by OSP_DoForegroundProcessing() and
 *          OSP_DoBackgroundProcessing()
 *
 * @param   sensorHandle INPUT requires a valid handle as returned by OSP_RegisterInputSensor()
 * @param   data INPUT pointer to timestamped raw sensor data
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_SetData(InputSensorHandle_t sensorHandle, TriAxisSensorRawData_t *data)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_DoForegroundProcessing
 *          Triggers computation for primary algorithms  e.g ROTATION_VECTOR
 *
 * @param   none
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_DoForegroundProcessing(void)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_DoBackgroundProcessing
 *          Triggers computation for less time critical background algorithms, e.g. sensor calibration
 *
 * @param   none
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_DoBackgroundProcessing(void)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_SubscribeOutputSensor
 *          Call for each Open-Sensor-Platform result (STEP_COUNT, ROTATION_VECTOR, etc) you want
 *          computed and output
 *
 * @param   pSensorDescriptor INPUT pointer to data which describes the details of how the fusion
 *          should be computed: e.g output rate, sensors to use, etc.
 * @param   pOutputHandle OUTPUT a handle to be used for OSP_UnsubscribeOutputSensor()
 *
 * @return  status as specified in OSP_Types.h. OSP_UNSUPPORTED_FEATURE for results that aren't
 *          available or licensed
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_SubscribeOutputSensor(SensorDescriptor_t *pSensorDescriptor,
    OutputSensorHandle_t *pOutputHandle)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_UnsubscribeOutputSensor
 *          Stops the chain of computation for a registered result
 *
 * @param   OutputHandle INPUT OutputSensorHandle_t that was received from
 *          OSP_SubscribeOutputSensor()
 *
 * @return  status as specified in OSP_Types.h.
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_UnsubscribeOutputSensor(OutputSensorHandle_t OutputHandle)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OSP_GetVersion
 *          Provides version number and version string of the library implementation
 *
 * @param   pVersionStruct OUTPUT pointer to a pointer that will receive the version data.
 *
 * @return  status as specified in OSP_Types.h
 *
 ***************************************************************************************************/
OSP_STATUS_t OSP_GetVersion(const OSP_Library_Version_t **pVersionStruct)
{
    return OSP_STATUS_OK;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
