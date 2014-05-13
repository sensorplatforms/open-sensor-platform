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
#include <string.h>
#include "osp_embeddedalgcalls.h"
#include "osp-version.h"


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define RESULT_FLAG_PAUSED              (1 << 0)
#define MAX_SENSOR_DESCRIPTORS          8
#define MAX_RESULT_DESCRIPTORS          5
#define SENSOR_FG_DATA_Q_SIZE           8
#define SENSOR_BG_DATA_Q_SIZE           8
#define MAX_SENSORS_PER_RESULT          5

//Sensor flags for internal use
#define SENSOR_FLAG_IN_USE              (1 << 0)
#define SENSOR_FLAG_HAVE_CTL_CALLBACK   (1 << 1)
#define SENSOR_FLAG_HAVE_CAL_CALLBACK   (1 << 2)
#define SENSOR_FLAG_NEEDS_DECIMATION    (1 << 3)

/* Result codes for local functions */
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR                        0

#ifdef ERROR
#undef ERROR
#endif
#define ERROR                           -1

#define Q32DIFF (32 - QFIXEDPOINTPRECISE)

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Local structure for keeping tab on active sensors and results */
typedef struct {
    SensorDescriptor_t *pSenDesc;
    uint16_t Flags;                 // in-use, etc
} _SenDesc_t;

typedef struct {
    SensorDescriptor_t *pResDesc;
    uint16_t Flags;                 // Paused, etc
} _ResDesc_t;

typedef struct {
    InputSensorHandle_t Handle;     // handle for this sensor
    TriAxisSensorRawData_t Data;    // raw data & time stamp from sensor
} _SensorDataBuffer_t;

typedef struct {
    SensorType_t ResultType;        // result type
    uint16_t SensorCount;           // number of sensors required
    SensorType_t Sensors[MAX_SENSORS_PER_RESULT];   // sensor types required for this result
} _ResultResourceMap_t;

typedef union {
    Android_UncalibratedAccelOutputData_t ucAccel;
    Android_UncalibratedMagOutputData_t   ucMag;
    Android_UncalibratedGyroOutputData_t  ucGyro;
} AndroidUnCalResult_t;

/* Common bridge between the different data types for base sensors (Accel/Mag/Gyro) */
typedef struct  {
    NTTIME TimeStamp;               // time stamp
    uint8_t accuracy;
    union {
        NTEXTENDED  extendedData[3];    // processed sensor data
        NTPRECISE   preciseData[3];     // processed sensor data
    } data;
} Common_3AxisResult_t;


/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static const OSP_Library_Version_t libVersion = {
    (OSP_VERSION_MAJOR << 16) | (OSP_VERSION_MINOR << 8) | (OSP_VERSION_PATCH),
    OSP_VERSION_STRING
};

static uint64_t _SubscribedResults;   // bit field of currently subscribed results, bit positions
                                      // same as SensorType_t
// pointer to platform descriptor structure
SystemDescriptor_t const *_pPlatformDesc = NULL;

// pointers to sensor data structures, and local flags
static  _SenDesc_t _SensorTable[MAX_SENSOR_DESCRIPTORS];

// pointers to result data structures, and local flags
static _ResDesc_t _ResultTable[MAX_RESULT_DESCRIPTORS];

// Raw sensor data queue for foreground processing
static _SensorDataBuffer_t _SensorFgDataQueue[SENSOR_FG_DATA_Q_SIZE];
static int16_t _SensorFgDataQCnt;          // number of data packets in the queue
static uint16_t _SensorFgDataNqPtr = SENSOR_FG_DATA_Q_SIZE - 1; // where the last data packet was put into the queue
static uint16_t _SensorFgDataDqPtr;         // where to remove next data packet from the queue


// Raw sensor data queue for background processing
static _SensorDataBuffer_t _SensorBgDataQueue[SENSOR_BG_DATA_Q_SIZE];
static int16_t _SensorBgDataQCnt;          // number of data packets in the queue
static uint16_t _SensorBgDataNqPtr = SENSOR_BG_DATA_Q_SIZE - 1; // where the last data packet was put into the queue
static uint16_t _SensorBgDataDqPtr;         // where to remove next data packet from the queue

static uint32_t _sensorLastForegroundTimeStamp = 0;             // keep the last time stamp here, we will use it to check for rollover
static uint32_t _sensorLastForegroundTimeStampExtension = 0;    // we will re-create a larger raw time stamp here

static uint32_t _sensorLastBackgroundTimeStamp = 0;             // keep the last time stamp here, we will use it to check for rollover
static uint32_t _sensorLastBackgroundTimeStampExtension = 0;    // we will re-create a larger raw time stamp here

static NTPRECISE _accel_bias[3] = {0, 0, 0};   // bias in sensor ticks
static NTPRECISE _gyro_bias[3]  = {0, 0, 0};   // bias in sensor ticks
static NTEXTENDED _mag_bias[3]  = {0, 0, 0};   // bias in sensor ticks

// copy of last data that was sent to the alg. We will
// use this for when the user _polls_ for calibrated sensor data.
static Common_3AxisResult_t _LastAccelCookedData;
static Common_3AxisResult_t _LastMagCookedData;
static Common_3AxisResult_t _LastGyroCookedData;

// table of result to resource maps. 1 entry for each result that describes which sensor types
// that it needs, which callback routine to use, etc.
static const _ResultResourceMap_t _ResultResourceMap[] = {
    {SENSOR_MAGNETIC_FIELD_UNCALIBRATED, 1, {SENSOR_MAGNETIC_FIELD_UNCALIBRATED}},
    {SENSOR_GYROSCOPE_UNCALIBRATED, 1, {SENSOR_GYROSCOPE_UNCALIBRATED}},
    {SENSOR_ACCELEROMETER_UNCALIBRATED, 1, {SENSOR_ACCELEROMETER_UNCALIBRATED}},
    {SENSOR_CONTEXT_DEVICE_MOTION, 2, {SENSOR_MAGNETIC_FIELD_UNCALIBRATED, SENSOR_ACCELEROMETER_UNCALIBRATED}},
    {SENSOR_STEP_COUNTER, 1, {SENSOR_ACCELEROMETER_UNCALIBRATED}}
};
#define RESOURCE_MAP_COUNT  (sizeof(_ResultResourceMap)/sizeof(_ResultResourceMap_t))

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static osp_status_t NullRoutine(void);
/* callback for entering/exiting a critical section of code (i.e. disable/enable task switch) */
OSP_CriticalSectionCallback_t EnterCritical = (OSP_CriticalSectionCallback_t)&NullRoutine;
OSP_CriticalSectionCallback_t ExitCritical = (OSP_CriticalSectionCallback_t)&NullRoutine;
static int16_t FindResultTableIndexByType(SensorType_t Type);

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      mult_uint16_uint16
 *          Unsigned 16-bit multiply with 32-bit result.
 *
 ***************************************************************************************************/
__inline static uint32_t mult_uint16_uint16(uint16_t a, uint16_t b)
{
    return ((uint32_t) a * (uint32_t)b);
}


/****************************************************************************************************
 * @fn      OnStepResultsReady
 *          Local callback used for Step Counter results from algorithm
 *
 ***************************************************************************************************/
static void OnStepResultsReady( StepDataOSP_t* stepData )
{
    if(_SubscribedResults & (1 << SENSOR_STEP_COUNTER)) {
        int16_t index;
        Android_StepCounterOutputData_t callbackData;

        callbackData.StepCount = stepData->numStepsTotal;
        callbackData.TimeStamp = stepData->startTime; //!TODO - Double check if start time or stop time

        index = FindResultTableIndexByType(SENSOR_STEP_COUNTER);
        _ResultTable[index].pResDesc->pOutputReadyCallback((OutputSensorHandle_t)&_ResultTable[index],
            &callbackData);
    }
}


/****************************************************************************************************
 * @fn      OnSignificantMotionResult
 *          Local callback used for Significant motion results from algorithm
 *
 ***************************************************************************************************/
static void OnSignificantMotionResult( NTTIME * eventTime )
{
    if(_SubscribedResults & (1 << SENSOR_CONTEXT_DEVICE_MOTION)) {
        int16_t index;
        Android_SignificantMotionOutputData_t callbackData;

        callbackData.significantMotionDetected = true;
        callbackData.TimeStamp = *eventTime;

        index = FindResultTableIndexByType(SENSOR_CONTEXT_DEVICE_MOTION);
        _ResultTable[index].pResDesc->pOutputReadyCallback((OutputSensorHandle_t)&_ResultTable[index],
            &callbackData);
    }
}


/****************************************************************************************************
 * @fn      NullRoutine
 *          A valid routine that does nothing. It should be used instead of a NULL function pointer
 *          for unused call backs so that we never try to execute a function pointed to by NULL
 *
 ***************************************************************************************************/
static osp_status_t NullRoutine(void)
{
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      ValidateSystemDescriptor
 *          Given a pointer to a system descriptor, validate it's contents. Return NO_ERROR if good,
 *          otherwise return ERROR
 *
 ***************************************************************************************************/
static int16_t ValidateSystemDescriptor(SystemDescriptor_t const *pSystemDescriptor)
{
    // test that the ENUMs are in range
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      FindSensorTableIndexByType
 *          Given a sensor type, return the index into the sensor table
 *
 ***************************************************************************************************/
static int16_t FindSensorTableIndexByType(SensorType_t Type)
{
    int16_t i;

    for(i = 0; i < MAX_SENSOR_DESCRIPTORS; i++) {
        if(_SensorTable[i].pSenDesc == NULL)
            continue;
        if(Type == ((SensorDescriptor_t *)(_SensorTable[i].pSenDesc))->SensorType)
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindSensorTableIndexByType
 *          Find 1st available empty sensor table slot, return the index into the sensor table
 *
 ***************************************************************************************************/
static int16_t FindEmptySensorTableIndex(void)
{
    int16_t i;

    for(i = 0; i < MAX_SENSOR_DESCRIPTORS; i++) {
        if(_SensorTable[i].pSenDesc == NULL)
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindSensorTableIndexByHandle
 *          Given a sensor handle, return the index into the sensor table
 *
 ***************************************************************************************************/
static int16_t FindSensorTableIndexByHandle(InputSensorHandle_t Handle)
{
    int16_t i;

    for(i = 0; i < MAX_SENSOR_DESCRIPTORS; i++) {
        if(Handle == (InputSensorHandle_t)&_SensorTable[i])
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindResultTableIndexByType
 *          Given a result type, return the index into the result table
 *
 ***************************************************************************************************/
static int16_t FindResultTableIndexByType(SensorType_t Type)
{
    int16_t i;

    for(i = 0; i < MAX_RESULT_DESCRIPTORS; i++) {
        if(_ResultTable[i].pResDesc == NULL)
            continue;
        if(Type == ((SensorDescriptor_t *)(_ResultTable[i].pResDesc))->SensorType)
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindEmptyResultTableIndex
 *          Find 1st available empty result table slot, return the index into the sensor table
 *
 ***************************************************************************************************/
static int16_t FindEmptyResultTableIndex(void)
{
    int16_t i;

    for(i = 0; i < MAX_RESULT_DESCRIPTORS; i++) {
        if(_ResultTable[i].pResDesc == NULL)
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindResultTableIndexByHandle
 *          Given a result handle, return the index into the result table
 *
 ***************************************************************************************************/
static int16_t FindResultTableIndexByHandle(OutputSensorHandle_t Handle)
{
    int16_t i;

    for(i = 0; i < MAX_RESULT_DESCRIPTORS; i++) {
        if(Handle == (OutputSensorHandle_t)&_ResultTable[i])
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      FindResourceMapIndexByType
 *          Given a result type, return the index into the resource map table. If not found, return
 *          ERROR
 *
 ***************************************************************************************************/
static int16_t FindResourceMapIndexByType(SensorType_t ResultType)
{
    int16_t i;

    for (i = 0; i < RESOURCE_MAP_COUNT; i++) {
        if (_ResultResourceMap[i].ResultType == ResultType)
            return i;
    }
    return ERROR;
}


/****************************************************************************************************
 * @fn      ValidateSensorDescriptor
 *          Given a pointer to a sensor descriptor, validate it's contents
 *
 ***************************************************************************************************/
static int16_t ValidateSensorDescriptor(SensorDescriptor_t *pSensorDescriptor)
{
    InputSensorSpecificData_t *pSensSpecific;
    osp_bool_t haveGoodCalData = FALSE;

    // test that the ENUMs are in range
    if ( pSensorDescriptor->SensorType >= SENSOR_ENUM_COUNT )
        return ERROR;
    if ( pSensorDescriptor->DataConvention >= DATA_CONVENTION_ENUM_COUNT )
        return ERROR;

    // if we have sensor specific data, check if its valid
    pSensSpecific = pSensorDescriptor->pSensorSpecificData;
    if (pSensSpecific != NULL) {
        if ( pSensSpecific->AxisMapping[0] >= AXIS_MAP_ENUM_COUNT )
            return ERROR;
        if ( pSensSpecific->AxisMapping[1] >= AXIS_MAP_ENUM_COUNT )
            return ERROR;
        if ( pSensSpecific->AxisMapping[2] >= AXIS_MAP_ENUM_COUNT )
            return ERROR;

        if (pSensSpecific->pCalibrationData != NULL) {
            //!TODO Validate cal data provided
        }
    }

    // Do any other validation that is required here...

    switch(pSensorDescriptor->SensorType) {

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        break;

    default:
        break;
    }

    if (haveGoodCalData != TRUE) {
        return OSP_STATUS_CAL_NOT_VALID;
    }
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      ValidateResultDescriptor
 *          Given a pointer to a result descriptor, validate it's contents
 *
 ***************************************************************************************************/
static int16_t ValidateResultDescriptor(SensorDescriptor_t *pResultDescriptor)
{
    if(pResultDescriptor->SensorType >= SENSOR_ENUM_COUNT )
        return ERROR;

    if(pResultDescriptor->DataConvention >= DATA_CONVENTION_ENUM_COUNT )
        return ERROR;

    if(pResultDescriptor->pOutputReadyCallback == NULL)
        return ERROR;

    // Do any other validation that's required
    //...

    // Add currently supported output sensor results...
    switch (pResultDescriptor->SensorType) {

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
    case SENSOR_GYROSCOPE_UNCALIBRATED:
    case SENSOR_ACCELEROMETER_UNCALIBRATED:
    case SENSOR_STEP_COUNTER:
    case SENSOR_CONTEXT_DEVICE_MOTION:
        break;

    default:
        return ERROR;

    }
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      InvalidateQueuedDataByHandle
 *          Invalidates the handles for the sensor data in the queue so that the data is discarded
 *
 ***************************************************************************************************/
void InvalidateQueuedDataByHandle(InputSensorHandle_t Handle)
{
    uint16_t i;

    EnterCritical();
    for(i = 0; i < SENSOR_FG_DATA_Q_SIZE; i++ ) {
        if(_SensorFgDataQueue[i].Handle == Handle)
            _SensorFgDataQueue[i].Handle = NULL;
    }
    for(i = 0; i < SENSOR_BG_DATA_Q_SIZE; i++ ) {
        if(_SensorBgDataQueue[i].Handle == Handle)
            _SensorBgDataQueue[i].Handle = NULL;
    }
    ExitCritical();
}


/****************************************************************************************************
 * @fn      TurnOnSensors
 *          Turns on sensors indicated by sensorsMask (bit mask based on SensorType_t bit position
 *
 ***************************************************************************************************/
static int16_t TurnOnSensors(uint32_t sensorsMask)
{
    SensorControl_t SenCtl;

    //  Check for control callback
    if(_pPlatformDesc->SensorsControl != NULL) {
        // send a sensor off command
        SenCtl.Handle = NULL;
        SenCtl.Command = SENSOR_CONTROL_SENSOR_ON;
        SenCtl.Data = sensorsMask;
        _pPlatformDesc->SensorsControl(&SenCtl);
    }
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      TurnOffSensors
 *          Turns off sensors indicated by sensorsMask (bit mask based on SensorType_t bit position
 *
 ***************************************************************************************************/
static int16_t TurnOffSensors(uint32_t sensorsMask)
{
    SensorControl_t SenCtl;

    //  does it have a control callback?
    if(_pPlatformDesc->SensorsControl != NULL) {
        // send a sensor off command
        SenCtl.Handle = NULL;
        SenCtl.Command = SENSOR_CONTROL_SENSOR_OFF;
        SenCtl.Data = sensorsMask;
        _pPlatformDesc->SensorsControl(&SenCtl);
    }
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      ActivateResultSensors
 *          Given a result type, check to be sure that all of the sensors that are needed
 *          for this result type are available. If any sensor that is needed is available and marked
 *          as "not in-use", mark it so and send a "turn on" command through the control callback if
 *          it is available.
 *
 ***************************************************************************************************/
static int16_t ActivateResultSensors(SensorType_t ResultType)
{
    int16_t i,j;
    int16_t index;
    uint32_t sensorsMask = 0;

    // use the result to sensor mapping table to see if all sensors that are needed are registered.
    // also check their "in-use" status. If any required sensor is not registered, return ERROR.

    // Find the index of the result type in the resource map table.
    for (i =  0; i < RESOURCE_MAP_COUNT; i++) {
        if (_ResultResourceMap[i].ResultType == ResultType) {
            for(j = 0; j < _ResultResourceMap[i].SensorCount; j++) {
                index = FindSensorTableIndexByType(_ResultResourceMap[i].Sensors[j]);
                if(index == ERROR)
                    return ERROR;                // sensor is not registered, exit with error
                // if this sensor is not active, mark it as such and send a command to it to go active.
                if((_SensorTable[index].Flags & SENSOR_FLAG_IN_USE) == 0) {
                    _SensorTable[index].Flags |= SENSOR_FLAG_IN_USE; // mark sensor as "in use"
                    //                  TurnOnSensor(_ResultResourceMap[i].Sensors[j]);
                    sensorsMask |= (1 << _ResultResourceMap[i].Sensors[j]);
                }
            }
            break;
        }
    }

    if (sensorsMask) TurnOnSensors(sensorsMask);

    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      DeactivateResultSensors
 *          Given a result type, deactivate (mark as "not in use") all sensors that were used by
 *          this result if they are not in use by any another active result.
 *
 ***************************************************************************************************/
static int16_t DeactivateResultSensors(SensorType_t ResultType)
{
    int16_t i,j,k,l;
    int16_t index;
    osp_bool_t NeedSensor;
    uint32_t sensorsMask = 0;


    // use the result to sensor mapping table to see if any sensors that are were used are still in use by another result.
    // We will send a control command to turn off the sensor if it supports it.

    // find our result resource table entry
    index = FindResourceMapIndexByType(ResultType);
    if (index == ERROR)
        return ERROR;

    // search active results to see if they use any of the sensors that we used
    for(i = 0; i < _ResultResourceMap[index].SensorCount; i++ ) { // for each of our sensors
        NeedSensor = FALSE;                                       // assume no other result uses this sensor
        for(k = 0; k < MAX_RESULT_DESCRIPTORS; k++) {
            if((_ResultTable[k].pResDesc != NULL) && (_ResultTable[k].pResDesc->SensorType != ResultType)) { // search active results (but not ours)
                j = FindResourceMapIndexByType(_ResultTable[k].pResDesc->SensorType);
                if(j == ERROR)
                    return ERROR;
                for(l = 0; l < _ResultResourceMap[j].SensorCount; l++) { // for each sensor in this active result
                    if(_ResultResourceMap[j].Sensors[l] == _ResultResourceMap[index].Sensors[i]) {
                        NeedSensor = TRUE; // found one, stop searching this result
                        break;
                    }
                }
                if(NeedSensor == TRUE)
                    break;  // another result is using this sensor, no need to check more results for this sensor
            }
        }
        if(NeedSensor == FALSE) {
            // if we get here, no other result uses this sensor type, mark it "not in use" and send
            // a "turn off" command to it, and mark all data in the input queues as stale
            j = FindSensorTableIndexByType(_ResultResourceMap[index].Sensors[i]);
            _SensorTable[j].Flags &= ~SENSOR_FLAG_IN_USE;   // Mark sensor "not in use"
            // mark all previously queued data for this sensor type as invalid
            InvalidateQueuedDataByHandle((InputSensorHandle_t)&_SensorTable[j]);
            sensorsMask |= (1 << _ResultResourceMap[index].Sensors[i]);
        }
    }
    if (sensorsMask) TurnOffSensors(sensorsMask);
    return NO_ERROR;
}


/****************************************************************************************************
 * @fn      UMul32
 *          Helper routine for 32-bit saturating multiply. This maybe optimized in assembly if needed
 *
 ***************************************************************************************************/
static void UMul32(uint32_t x,uint32_t y, uint32_t * pHigh, uint32_t * pLow)
{
    uint16_t xmsb;
    uint16_t ymsb;
    uint16_t xlsb;
    uint16_t ylsb;

    register uint32_t high;
    register uint32_t low;
    register uint32_t temp2;
    register uint32_t temp;


    xmsb = x >> 16;
    ymsb = y >> 16;

    xlsb = x & 0x0000FFFF;
    ylsb = y & 0x0000FFFF;

    high = mult_uint16_uint16(xmsb , ymsb);

    temp = mult_uint16_uint16(ymsb , xlsb);
    high += (temp & 0xFFFF0000) >> 16;

    low = temp << 16;

    temp = mult_uint16_uint16(xmsb , ylsb);
    high += (temp & 0xFFFF0000) >> 16;

    temp2 = low;
    low += temp << 16;

    if (low < temp2) {
        ++high;
    }

    temp = low;
    low += mult_uint16_uint16(xlsb, ylsb);

    if (low < temp) {
        ++high;
    }

    *pHigh = high;
    *pLow = low;
}


/****************************************************************************************************
 * @fn      GetTimeFromCounter
 *          Helper routine for time conversion
 *
 ***************************************************************************************************/
osp_bool_t GetTimeFromCounter(
    NTTIME * pTime,
    TIMECOEFFICIENT counterToTimeConversionFactor,
    uint32_t counterHigh,
    uint32_t counterLow)
{
    NTTIME ret = 0;
    uint32_t high1,low1;
    uint32_t high2,low2;
    const uint32_t roundfactor = (1 << (Q32DIFF-1)) ;
    if (counterToTimeConversionFactor & 0x80000000) {
        counterToTimeConversionFactor = ~counterToTimeConversionFactor + 1;
    }

    UMul32(counterToTimeConversionFactor, counterLow, &high1, &low1);
    UMul32(counterToTimeConversionFactor, counterHigh, &high2, &low2);

    low2 += high1;
    if (low2 < high1) {
        high2++;
    }

    //round things
    low1 += roundfactor;

    //check overflow
    if (low1 < roundfactor) {
        low2++;
        if (low2 ==0) {
            high2++;
        }
    }

    //right shift by Q32DIFF to make this into a Q24 number from a Q32 number
    low1 >>= Q32DIFF;
    low1 |= (low2 << (32 -  Q32DIFF) );
    low2 >>= Q32DIFF;
    low2 |= (high2 << (32 -  Q32DIFF) );
    high2 >>= Q32DIFF;

    if (high2 || low2 & 0x80000000) {
        //saturation!!!!!
        *pTime = 0x7FFFFFFFFFFFFFFFLL;
        return FALSE;
    }

    ret = low2;
    ret <<= 32;
    ret |= low1;

    *pTime = ret;

    return TRUE;
}


/****************************************************************************************************
 * @fn      ScaleSensorData
 *           Apply sign extension, offset and scaling to raw sensor data. NOTE: ScaleFactor may
 *          contain either NTPRECISE or NTEXTENDED number, base of "accuracy". Return value will also
 *          follow same logic.
 *
 ***************************************************************************************************/
static int32_t ScaleSensorData(
    int32_t Data,
    uint32_t Mask,
    int32_t Offset,
    int32_t ScaleFactor)
{
    int64_t llTemp;

    // apply offset
    Data -= Offset;

    Data &= Mask;               // mask off non-used data bits
    // sign extend (we assume that the data is in 2s complement format)
    if((Data & (~Mask >> 1)) != 0 )
        Data |= ~Mask;

    llTemp = (int64_t) Data * (int64_t) ScaleFactor; // scale the data

    if(llTemp > SATURATE_INT_MAX )
        llTemp = SATURATE_INT_MAX;   //if overflow, make max
    if(llTemp < SATURATE_INT_MIN )
        llTemp = SATURATE_INT_MIN;   //if underflow, make min
    return (int32_t)llTemp;     //return just the lower 32 bits
}


/****************************************************************************************************
 * @fn      ConvertSensorData
 *          Given a pointer to a raw sensor data packet from the input queue of type
 *          _SensorDataBuffer_t and a pointer to a sensor output data packet of type
 *          TriAxisSensorCookedData_t, apply translations and conversions into a format per Android
 *          conventions. Accuracy must be either QFIXEDPOINTPRECISE or QFIXEDPOINTEXTENDED
 *
 ***************************************************************************************************/
static int16_t ConvertSensorData(
    _SensorDataBuffer_t *pRawData,
    Common_3AxisResult_t *pCookedData,
    uint8_t accuracy,
    uint32_t *sensorTimeStamp,
    uint32_t *sensorTimeStampExtension)
{
    uint16_t i;
    unsigned char negative;
    unsigned char source;
    InputSensorSpecificData_t *pInpSensData;

    switch( ((_SenDesc_t *)(pRawData->Handle))->pSenDesc->SensorType ) {
    case SENSOR_ACCELEROMETER_UNCALIBRATED:
    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
    case SENSOR_GYROSCOPE_UNCALIBRATED:
        break;

    default:
        return ERROR;
    }

    // apply axis conversion and data width 1st, then offset, then gain (scaling), finally convert the time stamp.
    pCookedData->accuracy = accuracy;
    pInpSensData =  ((_SenDesc_t *)(pRawData->Handle))->pSenDesc->pSensorSpecificData;

    for (i = 0; i < 3; i++) {
        negative = 0;
        switch (pInpSensData->AxisMapping[i]) {

        case AXIS_MAP_UNUSED:
            switch (accuracy) {
            case QFIXEDPOINTPRECISE:
                pCookedData->data.preciseData[i] = CONST_PRECISE(0.0f);
                break;
            case QFIXEDPOINTEXTENDED:
                pCookedData->data.extendedData[i] = CONST_EXTENDED(0.0f);
                break;
            default:
                return ERROR;
            }
            continue;
        case AXIS_MAP_NEGATIVE_X:
            negative = 1;
        case AXIS_MAP_POSITIVE_X:
            source = 0;
            break;

        case AXIS_MAP_NEGATIVE_Y:
            negative = 1;
        case AXIS_MAP_POSITIVE_Y:
            source = 1;
            break;

        case AXIS_MAP_NEGATIVE_Z:
            negative = 1;
        case AXIS_MAP_POSITIVE_Z:
            source = 2;
            break;

        default:
            return ERROR;
        }

        switch (accuracy) {
        case QFIXEDPOINTPRECISE:
            // mask, sign extend (if needed), apply offset and scale factor
            pCookedData->data.preciseData[i] = ScaleSensorData(pRawData->Data.Data[source],
                pInpSensData->DataWidthMask,
                pInpSensData->ConversionOffset[i],
                pInpSensData->ConversionScale[i]);
            if (negative)
                pCookedData->data.preciseData[i] = -pCookedData->data.preciseData[i];

            break;
        case QFIXEDPOINTEXTENDED:
            // mask, sign extend (if needed), apply offset and scale factor
            pCookedData->data.extendedData[i] = ScaleSensorData(pRawData->Data.Data[source],
                pInpSensData->DataWidthMask,
                pInpSensData->ConversionOffset[i],
                pInpSensData->ConversionScale[i]);
            if (negative)
                pCookedData->data.extendedData[i] = -pCookedData->data.extendedData[i];
            break;
        default:
            return ERROR;
        }
    }

    // scale time stamp into seconds
    // check for user timestamp rollover, if so bump our timestamp extension word
    // !!WARNING!!: The time stamp extension scheme will need to be changed if timer capture is used
    // for sensor time-stamping. Current scheme will cause time jumps if two sensors are timer-captured
    // before & after rollover but the sensor that was captured after rollover is queued before the
    // sensor that was captured before timer rollover

    if( ((int32_t)(*sensorTimeStamp) < 0) && ((int32_t)pRawData->Data.TimeStamp >= 0) ) {
        (*sensorTimeStampExtension)++;
    }
    *sensorTimeStamp = pRawData->Data.TimeStamp;

    GetTimeFromCounter(&pCookedData->TimeStamp, _pPlatformDesc->TstampConversionToSeconds,
        *sensorTimeStampExtension, *sensorTimeStamp);
    return NO_ERROR;
}

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
osp_status_t OSP_Initialize(const SystemDescriptor_t* pSystemDesc)
{
    _SubscribedResults = 0;     // by definition, we are not subscribed to any results
    memset(_SensorTable, 0, sizeof(_SensorTable));   // init the sensor table
    memset(_ResultTable, 0, sizeof(_ResultTable));   // init the result table also

    if(ValidateSystemDescriptor(pSystemDesc) == ERROR)
        return (osp_status_t)OSP_STATUS_DESCRIPTOR_INVALID;
    _pPlatformDesc = pSystemDesc;
    if((pSystemDesc->EnterCritical != NULL) && (pSystemDesc->ExitCritical != NULL)) {
        EnterCritical = pSystemDesc->EnterCritical;
        ExitCritical = pSystemDesc->ExitCritical;
    }

    OSP_InitializeAlgorithms();

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
osp_status_t OSP_RegisterInputSensor(SensorDescriptor_t *pSensorDescriptor,
    InputSensorHandle_t *pReturnedHandle)
{
    int16_t status;
    int16_t index;
    osp_bool_t haveCalData = FALSE;

    // Find 1st available slot in the sensor descriptor table, insert descriptor pointer, clear flags
    // and return pointer to this table entry. If no room in the sensor table, return OSP_STATUS_NO_MORE_HANDLES
    // If this sensor type is already registered, return OSP_STATUS_ALREADY_REGISTERED.
    if((pSensorDescriptor == NULL) || (pReturnedHandle == NULL))      // just in case
        return OSP_STATUS_NULL_POINTER;

    if(FindSensorTableIndexByType(pSensorDescriptor->SensorType) != ERROR) { // is this sensor type already registered?
        *pReturnedHandle = NULL;
        return OSP_STATUS_ALREADY_REGISTERED;
    }

    // we have a new sensor, validate the sensor descriptor here before entering it into our table.
    status = ValidateSensorDescriptor(pSensorDescriptor);

    if (status == ERROR) {
        return OSP_STATUS_DESCRIPTOR_INVALID;
    } else if (status == NO_ERROR) {
        haveCalData = TRUE;
    }

    haveCalData = haveCalData; //Avoid compiler warning for now!

    // If room in the sensor table, enter it and return the handle, else return OSP_STATUS_NO_MORE_HANDLES
    index = FindEmptySensorTableIndex();
    if(index != ERROR) {
        _SensorTable[index].pSenDesc = pSensorDescriptor;
        _SensorTable[index].Flags = 0;
        *pReturnedHandle = (InputSensorHandle_t)&_SensorTable[index];
    } else {
        return OSP_STATUS_NO_MORE_HANDLES;
    }

    // setup any flags for this sensor
    if (pSensorDescriptor->pOptionalWriteCalDataCallback != NULL)  // set the flag for the optional sensor calibration changed callback
        _SensorTable[index].Flags |= SENSOR_FLAG_HAVE_CAL_CALLBACK;
    else
        _SensorTable[index].Flags &= ~SENSOR_FLAG_HAVE_CAL_CALLBACK;

    _SensorTable[index].Flags &= ~SENSOR_FLAG_IN_USE;               // by definition, this sensor isn't in use yet.

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
osp_status_t OSP_UnregisterInputSensor(InputSensorHandle_t sensorHandle)
{
    int16_t index;
    // Check the sensor table to be sure we have a valid entry, if so we need to check
    // to be sure that the sensor type is the same. Clear sensor table entry, and return a NULL
    // SensorHandle and the appropriate error code.
    // We also need to mark all data for this sensor that is in the input queue as invalid (make handle = NULL).

    index = FindSensorTableIndexByHandle(sensorHandle);
    if(index == ERROR) {                            // test for valid handle
        return OSP_STATUS_NOT_REGISTERED;
    }

    //Invalidate queued data for this sensor
    InvalidateQueuedDataByHandle(sensorHandle);

    // Invalidate the descriptor entry
    _SensorTable[index].pSenDesc = NULL;


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
osp_status_t OSP_SetData(InputSensorHandle_t sensorHandle, TriAxisSensorRawData_t *data)
{
    register osp_status_t FgStatus = 0;
    register osp_status_t BgStatus = 0;


    if (data == NULL)                                           // just in case
        return OSP_STATUS_NULL_POINTER;
    if(sensorHandle == NULL)                                    // just in case
        return OSP_STATUS_INVALID_HANDLE;
    if(FindSensorTableIndexByHandle(sensorHandle) == ERROR)
        return OSP_STATUS_INVALID_HANDLE;

    if( ((_SenDesc_t *)sensorHandle)->Flags & SENSOR_FLAG_IN_USE ) { // if this sensor is not used by a result, ignore data

        // put sensor data into the foreground queue
        FgStatus = OSP_STATUS_QUEUE_FULL;                       // assume queue full
        EnterCritical();                                        // no interrupts while we diddle the queues
        if(_SensorFgDataQCnt < SENSOR_FG_DATA_Q_SIZE) {         // check for room in the foreground queue
            _SensorFgDataQCnt++;                                //  if so, show one more in the queue
            if(++_SensorFgDataNqPtr == SENSOR_FG_DATA_Q_SIZE) { //  bump the enqueue pointer and check for pointer wrap, rewind if so
                _SensorFgDataNqPtr = 0;
            }
            FgStatus = OSP_STATUS_OK;                           // FG queue isn't full
        }
        _SensorFgDataQueue[_SensorFgDataNqPtr].Handle = sensorHandle;
        memcpy(&_SensorFgDataQueue[_SensorFgDataNqPtr].Data, data, sizeof(TriAxisSensorRawData_t)); // put data in queue (room or not)

        // put sensor data into the background queue
        BgStatus = OSP_STATUS_QUEUE_FULL;                       //    assume queue full
        if(_SensorBgDataQCnt < SENSOR_BG_DATA_Q_SIZE) {         // check for room in the background queue
            _SensorBgDataQCnt++;                                //  if so, show one more in the queue
            if(++_SensorBgDataNqPtr == SENSOR_BG_DATA_Q_SIZE) { //  bump the enqueue pointer and check for pointer wrap, rewind if so
                _SensorBgDataNqPtr = 0;
            }
            BgStatus = OSP_STATUS_OK;                           //    FG queue isn't full
        }
        _SensorBgDataQueue[_SensorBgDataNqPtr].Handle = sensorHandle;
        memcpy(&_SensorBgDataQueue[_SensorBgDataNqPtr].Data, data, sizeof(TriAxisSensorRawData_t)); // put data in queue (room or not)
        ExitCritical();
    }

    if((FgStatus == OSP_STATUS_QUEUE_FULL) || (BgStatus == OSP_STATUS_QUEUE_FULL))
        return OSP_STATUS_QUEUE_FULL;
    else
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
osp_status_t OSP_DoForegroundProcessing(void)
{
    _SensorDataBuffer_t data;
    Common_3AxisResult_t AndoidProcessedData;
    AndroidUnCalResult_t AndoidUncalProcessedData;
    int16_t index;
    Common_3AxisResult_t algConvention;

    // Get next sensor data packet from the queue. If nothing in the queue, return OSP_STATUS_IDLE.
    // If we get a data packet that has a sensor handle of NULL, we should drop it and get the next one,
    // a NULL handle is an indicator that the data is from a sensor that has been replaced or that the data is stale.

    EnterCritical();                                        // no interrupts while we diddle the queue

    // ignore any data marked as stale.
    while( (_SensorFgDataQueue[_SensorFgDataDqPtr].Handle == NULL) && (_SensorFgDataQCnt != 0) ) {
        _SensorFgDataQCnt--;                                // stale data, show one less in the queue
        if(++_SensorFgDataDqPtr == SENSOR_FG_DATA_Q_SIZE)   //  and check for pointer wrap, rewind if so
            _SensorFgDataDqPtr = 0;
    }

    // now see if there is any data to process.
    if(_SensorFgDataQCnt == 0) {                            // check for queue empty
        ExitCritical();
        return OSP_STATUS_IDLE;                 // nothing left in the queue, let the caller know that
    }

    // There is at least 1 data packet in the queue, get it.
    memcpy(&data, &_SensorFgDataQueue[_SensorFgDataDqPtr], sizeof(_SensorDataBuffer_t)); // remove data from queue
    _SensorFgDataQCnt--;                                // show one less in the queue
    if(++_SensorFgDataDqPtr == SENSOR_FG_DATA_Q_SIZE)   //  and check for pointer wrap, rewind if so
        _SensorFgDataDqPtr = 0;
    ExitCritical();

    // now send the processed data to the appropriate entry points in the alg code.
    switch( ((_SenDesc_t*)data.Handle)->pSenDesc->SensorType ) {

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        if (((_SenDesc_t*)data.Handle)->pSenDesc->DataConvention == DATA_CONVENTION_RAW) {
            ConvertSensorData(
                &data,
                &AndoidProcessedData,
                QFIXEDPOINTPRECISE,
                &_sensorLastForegroundTimeStamp,
                &_sensorLastForegroundTimeStampExtension);
        } else {
            //!TODO - Other data conventions support not implemented yet
            return OSP_STATUS_NOT_IMPLEMENTED;
        }

        // Do uncalibrated accel call back here (Android conventions)
        if (_SubscribedResults & (1LL << SENSOR_ACCELEROMETER_UNCALIBRATED)) {
            index = FindResultTableIndexByType(SENSOR_ACCELEROMETER_UNCALIBRATED);
            if (index != ERROR) {
                if (_ResultTable[index].pResDesc->DataConvention == DATA_CONVENTION_ANDROID) {
                        memcpy(&AndoidUncalProcessedData.ucAccel.X,
                            AndoidProcessedData.data.preciseData,
                            (sizeof(NTPRECISE)*3));
                        memcpy(&AndoidUncalProcessedData.ucAccel.X_offset,
                            _accel_bias, (sizeof(NTPRECISE)*3));
                        AndoidUncalProcessedData.ucAccel.TimeStamp = AndoidProcessedData.TimeStamp;

                        _ResultTable[index].pResDesc->pOutputReadyCallback(
                            (OutputSensorHandle_t)&_ResultTable[index], &AndoidUncalProcessedData.ucAccel);
                }
            } else {
                return OSP_STATUS_ERROR;
            }
        }

        // convert to algorithm convention before feeding data to algorithms.
        algConvention.accuracy = QFIXEDPOINTPRECISE;
        algConvention.data.preciseData[0] = AndoidProcessedData.data.preciseData[1];  // x (ALG) =  Y (Android)
        algConvention.data.preciseData[1] = -AndoidProcessedData.data.preciseData[0]; // y (ALG) = -X (Android)
        algConvention.data.preciseData[2] = AndoidProcessedData.data.preciseData[2];  // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        memcpy(&_LastAccelCookedData, &algConvention, sizeof(Common_3AxisResult_t));

        //OSP_SetForegroundAccelerometerMeasurement(algConvention.TimeStamp, algConvention.data.preciseData);
        // Send data on to algorithms
        OSP_SetAccelerometerMeasurement(algConvention.TimeStamp, algConvention.data.preciseData);

        // Do linear accel and gravity processing if needed
        // ... TODO
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        if (((_SenDesc_t*)data.Handle)->pSenDesc->DataConvention == DATA_CONVENTION_RAW) {
            ConvertSensorData(
                &data,
                &AndoidProcessedData,
                QFIXEDPOINTPRECISE,
                &_sensorLastForegroundTimeStamp,
                &_sensorLastForegroundTimeStampExtension);
        } else {
            //!TODO - Other data conventions support not implemented yet
            return OSP_STATUS_NOT_IMPLEMENTED;
        }

        // Do uncalibrated mag call back here (Android conventions)
        if (_SubscribedResults & (1LL << SENSOR_MAGNETIC_FIELD_UNCALIBRATED)) {
            index = FindResultTableIndexByType(SENSOR_MAGNETIC_FIELD_UNCALIBRATED);
            if (index != ERROR) {
                if (_ResultTable[index].pResDesc->DataConvention == DATA_CONVENTION_ANDROID) {
                        memcpy(&AndoidUncalProcessedData.ucMag.X,
                            AndoidProcessedData.data.extendedData,
                            (sizeof(NTEXTENDED)*3));
                        memcpy(&AndoidUncalProcessedData.ucMag.X_hardIron_offset,
                            _mag_bias, (sizeof(NTEXTENDED)*3));
                        AndoidUncalProcessedData.ucMag.TimeStamp = AndoidProcessedData.TimeStamp;

                        _ResultTable[index].pResDesc->pOutputReadyCallback(
                            (OutputSensorHandle_t)&_ResultTable[index], &AndoidUncalProcessedData.ucMag);
                } else {
                    return OSP_STATUS_ERROR;
                }
            }
        }

        // convert to algorithm convention before feeding data to algs.
        algConvention.accuracy = QFIXEDPOINTEXTENDED;
        algConvention.data.extendedData[0] = AndoidProcessedData.data.extendedData[1];  // x (ALG) =  Y (Android)
        algConvention.data.extendedData[1] = -AndoidProcessedData.data.extendedData[0]; // y (ALG) = -X (Android)
        algConvention.data.extendedData[2] = AndoidProcessedData.data.extendedData[2];  // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        memcpy(&_LastMagCookedData, &algConvention, sizeof(Common_3AxisResult_t));

        //OSP_SetForegroundMagnetometerMeasurement(AndoidProcessedData.TimeStamp, algConvention.data.extendedData);
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        if (((_SenDesc_t*)data.Handle)->pSenDesc->DataConvention == DATA_CONVENTION_RAW) {
            ConvertSensorData(
                &data,
                &AndoidProcessedData,
                QFIXEDPOINTPRECISE,
                &_sensorLastForegroundTimeStamp,
                &_sensorLastForegroundTimeStampExtension);
        } else {
            //!TODO - Other data conventions support not implemented yet
            return OSP_STATUS_NOT_IMPLEMENTED;
        }

        // Do uncalibrated mag call back here (Android conventions)
        if (_SubscribedResults & (1LL << SENSOR_GYROSCOPE_UNCALIBRATED)) {
            index = FindResultTableIndexByType(SENSOR_GYROSCOPE_UNCALIBRATED);
            if (index != ERROR) {
                if (_ResultTable[index].pResDesc->DataConvention == DATA_CONVENTION_ANDROID) {
                        memcpy(&AndoidUncalProcessedData.ucGyro.X,
                            AndoidProcessedData.data.preciseData,
                            (sizeof(NTPRECISE)*3));
                        memcpy(&AndoidUncalProcessedData.ucGyro.X_drift_offset,
                            _gyro_bias, (sizeof(NTPRECISE)*3));
                        AndoidUncalProcessedData.ucGyro.TimeStamp = AndoidProcessedData.TimeStamp;

                        _ResultTable[index].pResDesc->pOutputReadyCallback(
                            (OutputSensorHandle_t)&_ResultTable[index], &AndoidUncalProcessedData.ucGyro);
                } else {
                    return OSP_STATUS_ERROR;
                }
            }
        }

        // convert to algorithm convention before feeding data to algs.
        algConvention.accuracy = QFIXEDPOINTPRECISE;
        algConvention.data.preciseData[0] = AndoidProcessedData.data.preciseData[1];  // x (ALG) =  Y (Android)
        algConvention.data.preciseData[1] = -AndoidProcessedData.data.preciseData[0]; // y (ALG) = -X (Android)
        algConvention.data.preciseData[2] = AndoidProcessedData.data.preciseData[2];  // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        memcpy(&_LastGyroCookedData, &algConvention, sizeof(Common_3AxisResult_t));

        //OSP_SetForegroundGyroscopeMeasurement(AndoidProcessedData.TimeStamp, algConvention.data.preciseData);
        break;

    default:
        break;
    }

    // all done for now, return OSP_STATUS_IDLE if no more data in the queue, else return OSP_STATUS_OK

    if(_SensorFgDataQCnt == 0)
        return OSP_STATUS_IDLE;                 // nothing left in the queue, let the caller know that
    else
        return OSP_STATUS_OK;                   // more to process
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
osp_status_t OSP_DoBackgroundProcessing(void)
{
    _SensorDataBuffer_t data;
    Common_3AxisResult_t AndoidProcessedData;
    //Common_3AxisResult_t algConvention;

    // Get next sensor data packet from the queue. If nothing in the queue, return OSP_STATUS_IDLE.
    // If we get a data packet that has a sensor handle of NULL, we should drop it and get the next one,
    // a NULL handle is an indicator that the data is from a sensor that has been replaced and that the data is stale.

    EnterCritical();                                        // no interrupts while we diddle the queue

    // ignore any data marked as stale.
    while( (_SensorBgDataQueue[_SensorBgDataDqPtr].Handle == NULL) && (_SensorBgDataQCnt != 0) ) {
        _SensorBgDataQCnt--;                                // stale data, show one less in the queue
        if(++_SensorBgDataDqPtr == SENSOR_BG_DATA_Q_SIZE)   //  and check for pointer wrap, rewind if so
            _SensorBgDataDqPtr = 0;
    }

    // now see if there is any data to process.
    if(_SensorBgDataQCnt == 0) {                // check for queue empty
        ExitCritical();
        return OSP_STATUS_IDLE;                 // nothing left in the queue, let the caller know that
    }

    // There is at least 1 data packet in the queue, get it.
    memcpy(&data, &_SensorBgDataQueue[_SensorBgDataDqPtr], sizeof(_SensorDataBuffer_t)); // remove data from queue
    _SensorBgDataQCnt--;                                // show one less in the queue
    if(++_SensorBgDataDqPtr == SENSOR_BG_DATA_Q_SIZE)   //  and check for pointer wrap, rewind if so
        _SensorBgDataDqPtr = 0;

    ExitCritical();

    // now send the processed data to the appropriate entry points in the alg calibration code.
    switch( ((_SenDesc_t*)data.Handle)->pSenDesc->SensorType ) {

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        ConvertSensorData(
            &data,
            &AndoidProcessedData,
            QFIXEDPOINTPRECISE,
            &_sensorLastBackgroundTimeStamp,
            &_sensorLastBackgroundTimeStampExtension);

#if 0 //Nothing to be done for background processing at this time!
        // convert to algorithm convention.
        algConvention.accuracy = QFIXEDPOINTPRECISE;
        algConvention.data.preciseData[0] = AndoidProcessedData.data.preciseData[1];  // x (ALG) =  Y (Android)
        algConvention.data.preciseData[1] = -AndoidProcessedData.data.preciseData[0]; // y (ALG) = -X (Android)
        algConvention.data.preciseData[2] = AndoidProcessedData.data.preciseData[2];  // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        //OSP_SetBackgroundAccelerometerMeasurement(algConvention.TimeStamp, algConvention.data.preciseData);
#endif
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        ConvertSensorData(
            &data,
            &AndoidProcessedData,
            QFIXEDPOINTEXTENDED,
            &_sensorLastBackgroundTimeStamp,
            &_sensorLastBackgroundTimeStampExtension);

#if 0 //Nothing to be done for background processing at this time!
        // convert to algorithm convention.
        algConvention.accuracy = QFIXEDPOINTEXTENDED;
        algConvention.data.extendedData[0] = AndoidProcessedData.data.extendedData[1];   // x (ALG) =  Y (Android)
        algConvention.data.extendedData[1] = -AndoidProcessedData.data.extendedData[0];  // y (ALG) = -X (Android)
        algConvention.data.extendedData[2] = AndoidProcessedData.data.extendedData[2];   // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        //OSP_SetBackgroundMagnetometerMeasurement(algConvention.TimeStamp, algConvention.data.extendedData);
#endif
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        // Now we have a copy of the data to be processed. We need to apply any and all input conversions.
        ConvertSensorData(
            &data,
            &AndoidProcessedData,
            QFIXEDPOINTPRECISE,
            &_sensorLastBackgroundTimeStamp,
            &_sensorLastBackgroundTimeStampExtension);

#if 0 //Nothing to be done for background processing at this time!
        // convert to algorithm convention.
        algConvention.accuracy = QFIXEDPOINTPRECISE;
        algConvention.data.preciseData[0] = AndoidProcessedData.data.preciseData[1];  // x (ALG) =  Y (Android)
        algConvention.data.preciseData[1] = -AndoidProcessedData.data.preciseData[0]; // y (ALG) = -X (Android)
        algConvention.data.preciseData[2] = AndoidProcessedData.data.preciseData[2];  // z (ALG) =  Z (Android)
        algConvention.TimeStamp = AndoidProcessedData.TimeStamp;

        //OSP_SetBackgroundGyroscopeMeasurement(algConvention.TimeStamp, algConvention.data.preciseData);
#endif
        break;

    default:
        break;
    }

    // all done for now, return OSP_STATUS_IDLE if no more data in the queue, else return OSP_STATUS_OK
    if(_SensorBgDataQCnt == 0)
        return OSP_STATUS_IDLE;                 // nothing left in the queue, let the caller know that
    else
        return OSP_STATUS_OK;                   // more to process
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
osp_status_t OSP_SubscribeOutputSensor(SensorDescriptor_t *pSensorDescriptor,
    OutputSensorHandle_t *pOutputHandle)
{
    int16_t index;

    if((pSensorDescriptor == NULL) || (pOutputHandle == NULL) ||
        (pSensorDescriptor->pOutputReadyCallback == NULL)) // just in case
        return OSP_STATUS_NULL_POINTER;

    if(FindResultTableIndexByType(pSensorDescriptor->SensorType) != ERROR) { // is this result type already subscribed?
        *pOutputHandle = NULL;
        return OSP_STATUS_ALREADY_SUBSCRIBED;
    }

    // we have a new request, validate the result descriptor here before entering it into our table.

    if(ValidateResultDescriptor(pSensorDescriptor) == ERROR)
        return OSP_STATUS_DESCRIPTOR_INVALID;

    // Check for room in the result table, if no room, return OSP_STATUS_NO_MORE_HANDLES

    index = FindEmptyResultTableIndex();
    if(index == ERROR) {                                    // if no room in the result table, return the error
        *pOutputHandle = NULL;                              //  and set the handle to NULL, so we can check for it later
        return OSP_STATUS_NO_MORE_HANDLES;
    }

    // check to be sure that we have all sensors registered that we need for this result. If so, mark them as "in use".
    // if not, return OSP_STATUS_NOT_REGISTERED, and set the result handle to NULL.

    if(ActivateResultSensors(pSensorDescriptor->SensorType) == ERROR) {
        *pOutputHandle = NULL;                              //  and set the handle to NULL, so we can check for it later
        return OSP_STATUS_NOT_REGISTERED;
    }

    // Setup the alg callbacks, and any thing else that is needed for this result.
    switch (pSensorDescriptor->SensorType) {

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        _SubscribedResults |= (1LL << SENSOR_ACCELEROMETER_UNCALIBRATED);
        //Note: Calibrated or uncalibrated result is specified in the descriptor flags
        //For Uncalibrated result no callback needs to be registered with the algorithms
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        _SubscribedResults |= (1LL << SENSOR_MAGNETIC_FIELD_UNCALIBRATED);
        //Note: Calibrated or uncalibrated result is specified in the descriptor flags
        //For Uncalibrated result no callback needs to be registered with the algorithms
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        _SubscribedResults |= (1LL << SENSOR_GYROSCOPE_UNCALIBRATED);
        //Note: Calibrated or uncalibrated result is specified in the descriptor flags
        //For Uncalibrated result no callback needs to be registered with the algorithms
        break;

    case SENSOR_CONTEXT_DEVICE_MOTION:
        _SubscribedResults |= (1LL << SENSOR_CONTEXT_DEVICE_MOTION);
        OSP_RegisterSignificantMotionCallback(OnSignificantMotionResult);
        break;

    case SENSOR_STEP_COUNTER:
        _SubscribedResults |= (1LL << SENSOR_STEP_COUNTER);
        OSP_RegisterStepCallback(OnStepResultsReady);
        break;

    default:
        return OSP_STATUS_UNKNOWN_REQUEST;

    }

    // Everything is setup, update our result table and return a handle
    _ResultTable[index].pResDesc = pSensorDescriptor;
    _ResultTable[index].Flags = 0;
    *pOutputHandle = (OutputSensorHandle_t *)&_ResultTable[index];

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
osp_status_t OSP_UnsubscribeOutputSensor(OutputSensorHandle_t OutputHandle)
{
    int16_t index;

    // Check the result table to be sure that this is a valid handle, if not return OSP_STATUS_INVALID_HANDLE error.
    // Also check that the handle points to a currently subscribed result, if not return OSP_STATUS_NOT_SUBSCRIBED.
    if(OutputHandle == NULL)      // just in case
        return OSP_STATUS_INVALID_HANDLE;
    index = FindResultTableIndexByHandle(OutputHandle);
    if((index == ERROR) || (_ResultTable[index].pResDesc == NULL))  // test for active subscription for this handle
        return OSP_STATUS_NOT_SUBSCRIBED;

    // Check to see if any of the other results that are still subscribed needs to use the sensors
    // that we used. If not, mark those sensors as unused so that data from them will not be processed.
    // All data in the input queues from sensors that we mark as unused should be marked as stale. We
    // will also send a "sensor off" if that facility is available.
    DeactivateResultSensors(_ResultTable[index].pResDesc->SensorType);

    // Now make sure that we won't call the users callback for this result
    switch (_ResultTable[index].pResDesc->SensorType) {

    case SENSOR_ACCELEROMETER_UNCALIBRATED:
        _SubscribedResults &= ~(1LL << SENSOR_ACCELEROMETER_UNCALIBRATED);
        break;

    case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
        _SubscribedResults &= ~(1LL << SENSOR_MAGNETIC_FIELD_UNCALIBRATED);
        break;

    case SENSOR_GYROSCOPE_UNCALIBRATED:
        _SubscribedResults &= ~(1LL << SENSOR_GYROSCOPE_UNCALIBRATED);
        break;

    case SENSOR_CONTEXT_DEVICE_MOTION:
        _SubscribedResults &= ~(1LL << SENSOR_CONTEXT_DEVICE_MOTION);
        break;

    case SENSOR_STEP_COUNTER:
        _SubscribedResults &= ~(1LL << SENSOR_STEP_COUNTER);
        break;

    default:
        return OSP_STATUS_UNKNOWN_REQUEST;

    }

    // remove result table entry.
    _ResultTable[index].pResDesc = NULL;
    _ResultTable[index].Flags = 0;

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
osp_status_t OSP_GetVersion(const OSP_Library_Version_t **pVersionStruct)
{
    *pVersionStruct = &libVersion;
    return OSP_STATUS_OK;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
