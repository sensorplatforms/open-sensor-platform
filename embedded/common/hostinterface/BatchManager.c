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
#include <string.h>
#include "common.h"
#include "BlockMemory.h"
#include "SensorPackets.h"
#include "ConfigManager.h"
#include "BatchState.h"
#include "Queue.h"
#include "BatchManager.h"


//#define DECIMATION_MASK //Untested feature for adding decimation logic to control sensor rates

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

#define MAX_NUMBER_SENSORS                          ( NUM_SENSOR_TYPE )
/* On Change non wakeup sensors are stored locally, it can store up to this define */
#define NUM_ONCHANGE_NONWAKEUP_SENSOR               (10)

#define DEFAULT_REPORT_LATENCY                      (-1)
#define MIN_SAMPLING_PERIOD                         (1)
#define MAX_SAMPLING_FREQ_HZ                        (1000)
#define TIME_1SEC_NS_UNIT                           (1000000000)
#define DEFAULT_HIGH_THRESHOLD                      (1)

/* Q Size Common Definitions */
#define QUEUE_LOW_THR                               (0)
#define QUEUE_HIGH_THR                              (1)
#define HOST_WAKEUP_TOLERANCE_SIZE                  (75) 
#define HIF_PACKET_SIZE                             M_CalcBufferSize(sizeof(HostIFPackets_t))

/* Sensor Data Queue Size Definition */
/* HIF Queue size indicate number of packet a single Q can hold */
#define HIF_SENSOR_DATA_QUEUE_SIZE                  (100)
#define NUM_DATA_QUEUE                              (2)

/* Combined HIF Sensor Data Packet pool size (Wakeup + Non Wakeup) */
#define HIF_SENSOR_DATA_PACKET_POOL_SIZE            ( ( HIF_SENSOR_DATA_QUEUE_SIZE * ( NUM_DATA_QUEUE )) + HOST_WAKEUP_TOLERANCE_SIZE )

/* Sensor Control Queue Size Definition */
/* HIF Queue size indicate number of Event a single Q can hold */
#define HIF_CONTROL_QUEUE_SIZE                      (10)
#define NUM_CONTROL_QUEUE                           (1)

/* HIF Control Packet pool size */
#define HIF_CONTROL_PACKET_POOL_SIZE                ( HIF_CONTROL_QUEUE_SIZE * ( NUM_CONTROL_QUEUE ))

/* Queue empty Flags */
#define QUEUE_WAKEUP_EMPTY_BIT                      (0x01)
#define QUEUE_NONWAKEUP_EMPTY_BIT                   (0x02)
#define QUEUE_CONTROL_RESPONSE_EMPTY_BIT            (0x04)
#define QUEUE_ALL_EMPTY_MASK                        (0x07)

/* Mapping table for Sensor type to FFIO Type */
static const uint32_t AndroidTypeToFIFOTypeMap[MAX_NUMBER_SENSORS] =
{
    [SENSOR_META_DATA]                      = BATCH_SENSOR_NUM,
    [SENSOR_ACCELEROMETER]                  = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_GEOMAGNETIC_FIELD]              = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_MAGNETIC_FIELD]                 = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_ORIENTATION]                    = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_GYROSCOPE]                      = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_LIGHT]                          = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_PRESSURE]                       = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_TEMPERATURE]                    = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_PROXIMITY]                      = BATCH_SENSOR_WAKEUP_FIFO,
    [SENSOR_GRAVITY]                        = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_LINEAR_ACCELERATION]            = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_ROTATION_VECTOR]                = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_RELATIVE_HUMIDITY]              = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_AMBIENT_TEMPERATURE]            = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_MAGNETIC_FIELD_UNCALIBRATED]    = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_GAME_ROTATION_VECTOR]           = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_GYROSCOPE_UNCALIBRATED]         = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_SIGNIFICANT_MOTION]             = BATCH_SENSOR_WAKEUP_FIFO,
    [SENSOR_STEP_DETECTOR]                  = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SENSOR_STEP_COUNTER]                   = BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO,
    [SENSOR_GEOMAGNETIC_ROTATION_VECTOR]    = BATCH_SENSOR_NONWAKEUP_FIFO,

    /* Private Sensor Type */
    [PSENSOR_ACCELEROMETER_RAW]             = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_MAGNETIC_FIELD_RAW]            = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_GYROSCOPE_RAW]                 = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_LIGHT_UV]                      = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_LIGHT_RGB]                     = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_STEP]                          = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_ACCELEROMETER_UNCALIBRATED]    = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_ORIENTATION]                   = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_CONTEXT_DEVICE_MOTION]         = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_CONTEXT_CARRY]                 = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_CONTEXT_POSTURE]               = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_CONTEXT_TRANSPORT]             = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_CONTEXT_GESTURE_EVENT]                 = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_HEART_RATE]                    = BATCH_SENSOR_NONWAKEUP_FIFO,
    [SYSTEM_REAL_TIME_CLOCK]                = BATCH_SENSOR_NONWAKEUP_FIFO,
    [PSENSOR_MAGNETIC_FIELD_ANOMALY]        = BATCH_SENSOR_NONWAKEUP_FIFO,
};


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* Structures to hold batching state */
typedef struct _BatchParam
{
    uint64_t    SamplingRate;   /* Min. Sampling Rate for batched sensor */
    uint64_t    ReportLatency;  /* Max. Report Latency for batched sensor */
#ifdef DECIMATION_MASK 
    uint32_t    SampleCnt;      /* Sample count for sensor*/
    uint32_t    DecimationCnt;  /* Decimation count for sensor*/
#endif
    osp_bool_t   isValidEntry;   /* Flag for Valid entry */
} BatchParam_t;

typedef struct _Batch
{
    BatchParam_t  SensorList[MAX_NUMBER_SENSORS];         /* Batching sensor list */
    Queue_t       *pQ;                                    /* pointer to sensor Q */
    uint64_t      MinReportLatency;                       /* min. report latency from sensor list */
    uint8_t       NumBatchedSensor;                       /* Number of sensor currently batched */
} Batch_t;

/* Structures to hold On Change Sensor Sample */
typedef struct _BatchOnChangeSensor
{
    BufferHeader_t      Header;                 /* Buffer header */
    HostIFPackets_t     Sample;                 /* HiF Packet */
    ASensorType_t       SensorType;             /* Sensor Type of current HiF Packet */
    osp_bool_t          ValidFlag;              /* HiF Packet is valid Flag */
} BatchOnChangeSensor_t;

typedef struct _OnChangeSenorSample
{
    BatchOnChangeSensor_t   SensorList[NUM_ONCHANGE_NONWAKEUP_SENSOR];      /* On change sensor list */
    osp_bool_t               QEmpty;                                         /* Mark Q is empty*/
} OnChangeSenorSample_t;


/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static Batch_t NonWakeupBatch, WakeupBatch;
static OnChangeSenorSample_t NonWakeupOnChangeSensor;
static osp_bool_t isBatchManagerInitialized = FALSE;
static Q_Type_t CurrQType = NUM_QUEUE_TYPE;

Queue_t *_HiFNonWakeupQueue = NULL;
Queue_t *_HiFWakeUpQueue = NULL;
Queue_t *_HiFControlQueue = NULL;

/* Memory Pool for Sensor Data Packet for NonWakeup sensor and Wakeup sensor */
DECLARE_BLOCK_POOL( SensorDataPacketPool, HIF_PACKET_SIZE, HIF_SENSOR_DATA_PACKET_POOL_SIZE );

/* Memory Pool for Sensor Control Packet */
DECLARE_BLOCK_POOL( SensorControlResponsePacketPool, HIF_PACKET_SIZE, HIF_CONTROL_PACKET_POOL_SIZE );


/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static void QHighThresholdCallBack( Q_Type_t QType );
static void QEmptyCallBack( Q_Type_t QType );
static int16_t GetCurrentQType( Q_Type_t *pQType );
static uint32_t CalculateHighThreshold( Batch_t *pBatchAttribute );
static int16_t RegisterSensorBatch( Batch_t *pBatchAttribute, uint16_t sensorType, void *pSensorConfig);
static int16_t DeRegisterSensorBatch( Batch_t *pBatchAttribute, uint32_t sensorType );
static int16_t FindMinReportLatency( Batch_t *pBatchAttribute );
static int16_t QInitialize( void );
static int16_t BatchManagerAllocHifPacket( Buffer_t **pHifPacket, uint32_t *pPacketPool);
static int16_t EnqueueOnChangeSensorQ( HostIFPackets_t *pHiFDataPacket, uint16_t packetSize, uint32_t sensorType );
static int16_t DequeueOnChangeSensorQ( Buffer_t **pBuf );


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
volatile uint8_t QEmptyRegister = QUEUE_ALL_EMPTY_MASK;


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      QHighThresholdCallBack
 *          This Callback called when queue reach to high threshold mark
 *
 * @param  [IN] QType - Type of Queue
 *
 * @return  none
 *
 ***************************************************************************************************/
static void QHighThresholdCallBack( Q_Type_t QType )
{
    int16_t status;
    BatchStateType_t currState;

    switch ( QType )
    {
    case QUEUE_NONWAKEUP_TYPE:
        /* Get current state of Batch state machine */
        status = BatchStateGet( &currState );
        ASF_assert( status == OSP_STATUS_OK );

        /* return if Batch state is in batching and host suspend */
        if ( currState == BATCH_ACTIVE_HOST_SUSPEND)
        {
            return;
        }
        else
        {
            /* Set GPIO to indicate valid Packets in Queue */
            QEmptyRegister &= ~(QUEUE_NONWAKEUP_EMPTY_BIT);
            SensorHubAssertInt();
        }
        break;

    case QUEUE_WAKEUP_TYPE:
        /* If CB is from Wakeup Q then assert host interrupt irrespective of Batch State */
        QEmptyRegister &= ~(QUEUE_WAKEUP_EMPTY_BIT);
        SensorHubAssertInt();
        break;

    case QUEUE_CONTROL_RESPONSE_TYPE:
        /* If CB is from Control Response Q then assert host interrupt irrespective of Batch State */
        QEmptyRegister &= ~(QUEUE_CONTROL_RESPONSE_EMPTY_BIT);
        SensorHubAssertInt();
        break;

    default:
        break;
    }
}


/****************************************************************************************************
 * @fn      QEmptyCallBack
 *          Callback for queue when it gets empty
 *
 * @param  [IN] QType - Type of Queue
 *
 * @return  none
 * 
 ***************************************************************************************************/
static void QEmptyCallBack( Q_Type_t QType )
{

    /* Mark Q empty */
    switch ( QType )
    {
    case QUEUE_WAKEUP_TYPE:
        QEmptyRegister |= QUEUE_WAKEUP_EMPTY_BIT;
        break;

    case QUEUE_NONWAKEUP_TYPE:
        /* Before set Non wakeup empty flag check Locally stored NonWakeupOnchange Q empty */
        if (NonWakeupOnChangeSensor.QEmpty)
        {
            QEmptyRegister |= QUEUE_NONWAKEUP_EMPTY_BIT;
        }
        break;

    case QUEUE_CONTROL_RESPONSE_TYPE:
        QEmptyRegister |= QUEUE_CONTROL_RESPONSE_EMPTY_BIT;
        break;

    default:
        break;
    }

    /* All Qs are empty so clear Host interrupt pin */
    if ( QEmptyRegister == QUEUE_ALL_EMPTY_MASK )
    {
        SensorHubDeAssertInt();
    }
}


/****************************************************************************************************
 * @fn      GetCurrentQType
 *          This is helper function for BatchManagerDeQueue, This function checks QEmptyRegister
 *          to find which Q is full so BatchManagerDeQueue can Dequeue from that Q.
 *
 * @param  [IN] pQType - Pointer which hold current Q to be Dequeue.
 *
 * @return  OSP_STATUS_OK or error code
 ***************************************************************************************************/
static int16_t GetCurrentQType( Q_Type_t *pQType )
{

    if ( (pQType == NULL) )
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Control response Q is empty */
    if ( (QEmptyRegister & QUEUE_CONTROL_RESPONSE_EMPTY_BIT) )
    {
        /* Set Current Q type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current Q is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current Q type to control response */
            *pQType = QUEUE_CONTROL_RESPONSE_TYPE;
            return OSP_STATUS_OK;
        }
    }

    /* Wakeup Q is empty */
    if ( (QEmptyRegister & QUEUE_WAKEUP_EMPTY_BIT) )
    {
        /* Set Current Q type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current Q is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current Q type to Wakeup */
            *pQType = QUEUE_WAKEUP_TYPE;
            return OSP_STATUS_OK;
        }
    }

    /* Non Wakeup Q is empty */
    if( QEmptyRegister & QUEUE_NONWAKEUP_EMPTY_BIT )
    {
        /* Set Current Q type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current Q is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current Q type to NonWakeup */
            *pQType = QUEUE_NONWAKEUP_TYPE;
            return OSP_STATUS_OK;
        }
    }
    return (OSP_STATUS_QUEUE_EMPTY);
}


/****************************************************************************************************
 * @fn      CalculateHighThreshold
 *          Calculate new value of high threshold for new sensor
 *
 * @param   [IN] pBatchAttribute - pointer to batching attribute structure
 *
 * @return Calculated high threshold value
 *
 ***************************************************************************************************/
static uint32_t CalculateHighThreshold( Batch_t *pBatchAttribute )
{
    uint8_t i;
    uint32_t highThr = 0;

    /* Find min. Report latency before we calculate high threshold value */
    FindMinReportLatency( pBatchAttribute );

    /* If Min. Report Latency is 0 then Sensor Event should be send as soon as it occurs */
    if ( (pBatchAttribute->MinReportLatency == 0) || (pBatchAttribute->MinReportLatency == (uint64_t)DEFAULT_REPORT_LATENCY) )
    {
        highThr = DEFAULT_HIGH_THRESHOLD;
        return highThr;
    }

    /* Calculate High threshold value */
    for (i = 0; i < MAX_NUMBER_SENSORS; i++)
    {
        /* Check that valid sampling rate entry before calculation */
        if( pBatchAttribute->SensorList[i].SamplingRate != MIN_SAMPLING_PERIOD )
        {
            /* Calculate high threshold value for one sensor entry and accumulate to previous value*/
            highThr += (pBatchAttribute->MinReportLatency / pBatchAttribute->SensorList[i].SamplingRate);
        }
    }

    return highThr;
}


/****************************************************************************************************
 * @fn      RegisterSensorBatch
 *          Register Sensor parameter (Sampling Rate and Report latency) to Batching structure
 *
 * @param   [IN] pBatchAttribute - pointer to batching attribute structure
 * @param   [IN] sensorType - SensorType - Android Sensor Type
 * @param   [IN] pSensorConfig - pointer to Sensor config data
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t RegisterSensorBatch( Batch_t *pBatchAttribute, uint16_t sensorType, void *pSensorConfig)
{
    BatchParam_t  *pSensorParam = (BatchParam_t *)pSensorConfig;

    /* Validate Batch Attribute */
    if ((pBatchAttribute == NULL) || (pSensorConfig == NULL))
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Register sensor to be batched */
    pBatchAttribute->SensorList[sensorType].ReportLatency   = pSensorParam->ReportLatency;
    pBatchAttribute->SensorList[sensorType].SamplingRate    = pSensorParam->SamplingRate;
    pBatchAttribute->SensorList[sensorType].isValidEntry    = TRUE;

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      DeRegisterSensorBatch
 *          DeRegister Sensor parameter (Sampling Rate and Report latency) to Batching structure
 *
 * @param   [IN] pBatchAttribute - Pointer of Batching attribute
 * @param   [IN] sensorType - Type of Sensor

 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t DeRegisterSensorBatch( Batch_t *pBatchAttribute, uint32_t sensorType )
{
    /* Validate Batch Attribute */
    if (pBatchAttribute == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Unregister sensor to remove from batching */
    pBatchAttribute->SensorList[sensorType].ReportLatency   = (uint64_t)DEFAULT_REPORT_LATENCY;
    pBatchAttribute->SensorList[sensorType].SamplingRate    = MIN_SAMPLING_PERIOD;
    pBatchAttribute->SensorList[sensorType].isValidEntry    = FALSE;

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      FindMinReportLatency
 *          Find Min. Report Latency for given Batching structure
 *
 * @param   [IN] pBatchAttribute - Pointer of Batching attribute
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t FindMinReportLatency( Batch_t *pBatchAttribute )
{
    uint8_t i;

    /* Validate Batch Attribute */
    if (pBatchAttribute == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Set min. Report latency to Max initially */
    pBatchAttribute->MinReportLatency = (uint64_t)DEFAULT_REPORT_LATENCY;

    /* Search min.Report latency number from Batching Attribute */
    for (i = 0; i < MAX_NUMBER_SENSORS; i++)
    {
        if (pBatchAttribute->SensorList[i].ReportLatency < pBatchAttribute->MinReportLatency)
        {
            pBatchAttribute->MinReportLatency = pBatchAttribute->SensorList[i].ReportLatency;
        }
    }

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      BatchManagerAllocHifPacket
 *          Allocate packet from Sensor Data packet pool
 *
 * @param   [OUT] pHifPacket - pointer to hold allocated packet
 * @param   [IN] pPacketPool - Pointer to Packet pool from which packet needs to be allocated
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t BatchManagerAllocHifPacket( Buffer_t **pHifPacket, uint32_t *pPacketPool)
{

    *pHifPacket = (Buffer_t *) AllocBlock( pPacketPool );
    if (*pHifPacket != NULL)
    {
        return OSP_STATUS_OK;
    } 
    else
    {
        return OSP_STATUS_NULL_POINTER;
    }
}


/****************************************************************************************************
 * @fn      QInitialize
 *          Initialize Non-Wakeup, Wakeup and Control Response Q
 *
 * @param   none
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t QInitialize( void )
{
    int16_t errCode;

    /* Initialize Data Packet pool */
    //TODO - Opportunity for future optimization here by having different PACKET 
    // size for this pool since not all sensors will be wakeup type and most likely
    // the wakeup sensors data will be smaller in size as compared to e.g. Rotation Vector
    InitBlockPool( SensorDataPacketPool, sizeof(SensorDataPacketPool), HIF_PACKET_SIZE );

        /* Create Non wakeup Queue */
    _HiFNonWakeupQueue = QueueCreate( HIF_SENSOR_DATA_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert(_HiFNonWakeupQueue != NULL);

    /* Create Wakeup Sensor Queue */
    _HiFWakeUpQueue = QueueCreate( HIF_SENSOR_DATA_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert(_HiFWakeUpQueue != NULL);

    /* Register Call backs for High and Low Thresholds   */
    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_EMPTY_CB, (fpQueueEvtCallback_t) QEmptyCallBack, 
                                     (void *)QUEUE_NONWAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_HIGH_THRESHOLD_CB, 
                                     (fpQueueEvtCallback_t) QHighThresholdCallBack, 
                                     (void *)QUEUE_NONWAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    /* Register Call backs for High and Low Thresholds */
    errCode = QueueRegisterCallBack( _HiFWakeUpQueue , QUEUE_EMPTY_CB, (fpQueueEvtCallback_t) QEmptyCallBack, 
                                    (void *)QUEUE_WAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    errCode = QueueRegisterCallBack( _HiFWakeUpQueue , QUEUE_HIGH_THRESHOLD_CB, 
                                    (fpQueueEvtCallback_t) QHighThresholdCallBack, (void *)QUEUE_WAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    /* Initialize Control Packet pool */
    InitBlockPool( SensorControlResponsePacketPool, sizeof(SensorControlResponsePacketPool), HIF_PACKET_SIZE );

    /* Create Control Queue */
    _HiFControlQueue = QueueCreate( HIF_CONTROL_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert( _HiFControlQueue != NULL );

    /* Register Call backs for High and Low Thresholds */
    errCode = QueueRegisterCallBack( _HiFControlQueue , QUEUE_EMPTY_CB, (fpQueueEvtCallback_t) QEmptyCallBack,
                                    (void *)QUEUE_CONTROL_RESPONSE_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    errCode = QueueRegisterCallBack( _HiFControlQueue , QUEUE_HIGH_THRESHOLD_CB, (fpQueueEvtCallback_t) QHighThresholdCallBack,
                                     (void *) QUEUE_CONTROL_RESPONSE_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      OnChnageSensorEnqueue
 *          Enqueue on change non wakeup sensors in locally Sample pool.
 *
 * @param   [IN] pHiFDataPacket - pointer of control response packet
 * @param   [IN] packetSize - Size of control response packet
 * @param   [IN] sensorType - Type of sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t EnqueueOnChangeSensorQ( HostIFPackets_t *pHiFDataPacket, uint16_t packetSize, uint32_t sensorType )
{
    int16_t i;

    for ( i = 0; i < NUM_ONCHANGE_NONWAKEUP_SENSOR; i++ )
    {
        /* Check packet is available */
        if ( NonWakeupOnChangeSensor.SensorList[i].ValidFlag )
        {
            /* Match sensor type before overwriting packet */
            if ( NonWakeupOnChangeSensor.SensorList[i].SensorType == sensorType )
            {
                /* Overwrite data to onChange sensor type */
                SH_MEMCPY( &(NonWakeupOnChangeSensor.SensorList[i].Sample), pHiFDataPacket, packetSize );
                NonWakeupOnChangeSensor.SensorList[i].ValidFlag = TRUE;
                NonWakeupOnChangeSensor.SensorList[i].Header.Length = packetSize;
                NonWakeupOnChangeSensor.SensorList[i].SensorType = (ASensorType_t)sensorType;
                NonWakeupOnChangeSensor.QEmpty = FALSE;
                return OSP_STATUS_OK;
            }
        }
        else
        {
            /* Copy data to onChange sensor type */
            SH_MEMCPY( &(NonWakeupOnChangeSensor.SensorList[i].Sample), pHiFDataPacket, packetSize );
            NonWakeupOnChangeSensor.SensorList[i].ValidFlag = TRUE;
            NonWakeupOnChangeSensor.SensorList[i].Header.Length = packetSize;
            NonWakeupOnChangeSensor.SensorList[i].SensorType = (ASensorType_t)sensorType;
            NonWakeupOnChangeSensor.QEmpty = FALSE;
            return OSP_STATUS_OK;
        }
    }
    return (OSP_STATUS_INVALID_PARAMETER);
}


/****************************************************************************************************
 * @fn      DequeueOnChangeSensorQ
 *          Dequeue locally stored on change non wakeup sensors.
 *
 * @param   [OUT] pBuf - Buffer pointer to hold sample
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t DequeueOnChangeSensorQ( Buffer_t **pBuf )
{
    static int16_t index = 0 ;

    if ( index < NUM_ONCHANGE_NONWAKEUP_SENSOR )
    {
        /* check Valid sample for current sensor */
        if ( NonWakeupOnChangeSensor.SensorList[index].ValidFlag )
        {
            *pBuf = ( Buffer_t *)&NonWakeupOnChangeSensor.SensorList[index].Header;
            index++;
        }
        else
        {
            index++;
            return ( OSP_STATUS_INVALID_PARAMETER );
        }
    }
    else
    {
        /* We check all sensor in list and now there is no valid entry */
        NonWakeupOnChangeSensor.QEmpty = TRUE;
        index = 0;
        return (OSP_STATUS_QUEUE_EMPTY);
    }
    return OSP_STATUS_OK;
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/
/****************************************************************************************************
 * @fn      BatchManagerInitialize
 *          Initialize Batch manager Module
 *
 * @param   none
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerInitialize( void )
{
    uint8_t i;

    if (!isBatchManagerInitialized)
    {
        /* Initialize Qs */
        QInitialize();

        /* Validate Qs */
        if ( ( _HiFWakeUpQueue == NULL) || (_HiFNonWakeupQueue == NULL) )
        {
            return (OSP_STATUS_NULL_POINTER);
        }
    
        /* Initialize Batch structure with Q */
        NonWakeupBatch.pQ   = (Queue_t *)_HiFNonWakeupQueue;
        WakeupBatch.pQ      = (Queue_t *)_HiFWakeUpQueue;

        /* Initialize Batching Attribute structure */
        for (i = 0; i < MAX_NUMBER_SENSORS; i++)
        {
            NonWakeupBatch.SensorList[i].ReportLatency  = (uint64_t)DEFAULT_REPORT_LATENCY;
            NonWakeupBatch.SensorList[i].SamplingRate   = MIN_SAMPLING_PERIOD;
            NonWakeupBatch.SensorList[i].isValidEntry   = FALSE;
#ifdef DECIMATION_MASK
            NonWakeupBatch.SensorList[i].DecimationCnt  = 0;
            NonWakeupBatch.SensorList[i].SampleCnt      = 0;
#endif

            WakeupBatch.SensorList[i].ReportLatency = (uint64_t)DEFAULT_REPORT_LATENCY;
            WakeupBatch.SensorList[i].SamplingRate  = MIN_SAMPLING_PERIOD;
            WakeupBatch.SensorList[i].isValidEntry  = FALSE;

#ifdef DECIMATION_MASK
            WakeupBatch.SensorList[i].DecimationCnt  = 0;
            WakeupBatch.SensorList[i].SampleCnt      = 0;
#endif
        }
    
        /* Initialize min. Report latency to max */
        NonWakeupBatch.MinReportLatency = (uint64_t)DEFAULT_REPORT_LATENCY;
        WakeupBatch.MinReportLatency    = (uint64_t)DEFAULT_REPORT_LATENCY;
    
        NonWakeupBatch.NumBatchedSensor   = 0;
        WakeupBatch.NumBatchedSensor      = 0;

        /* Initialize on change non wakeup sensor list */
        for (i = 0; i < NUM_ONCHANGE_NONWAKEUP_SENSOR; i++)
        {
            NonWakeupOnChangeSensor.SensorList[i].ValidFlag = FALSE;
            NonWakeupOnChangeSensor.QEmpty = TRUE;
        }

        isBatchManagerInitialized = TRUE;

    }

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      BatchManagerSensorRegister
 *          Register given sensor parameters in Batching module This function corresponds to batch()
 *          in Android Sensor HAL
 *
 * @param   [IN] SensorType - Type of Sensor
 * @param   [IN] SamplingRate - Sampling Rate for given sensor
 * @param   [IN] ReportLatency - Report Latency for given sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerSensorRegister( ASensorType_t SensorType, uint64_t SamplingRate, uint64_t ReportLatency )
{
    Batch_t *pCurrBatchAttribute;
    int16_t errCode             = OSP_STATUS_INVALID_PARAMETER;
    BatchParam_t  BatchParam;
    uint16_t sampleFreq;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    BatchParam.SamplingRate = SamplingRate;
    BatchParam.ReportLatency = ReportLatency;

    /* Convert given sampling period to sampling frequency */
    sampleFreq = ( TIME_1SEC_NS_UNIT / BatchParam.SamplingRate );

    /* Validate Data rate value */
    if (sampleFreq > MAX_SAMPLING_FREQ_HZ)
    {
        return (errCode);
    }

    /* Get Type of Batching Q for give sensor Type */
    switch (AndroidTypeToFIFOTypeMap[sType])
    {
    case BATCH_SENSOR_WAKEUP_FIFO:
        pCurrBatchAttribute = &WakeupBatch;
        break;

    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        pCurrBatchAttribute = &NonWakeupBatch;
        break;

    default:
        return errCode;
    }

#ifdef DECIMATION_MASK
    pCurrBatchAttribute->SensorList[sType].DecimationCnt = (uint32_t)(SamplingRate / SensorDriverRate);

    if ( pCurrBatchAttribute->SensorList[sType].DecimationCnt == 0 )
    {
        pCurrBatchAttribute->SensorList[sType].DecimationCnt = 1;
    }

#endif
    /* Checks sensor is already register or not, if it is register already return */
    if ( !pCurrBatchAttribute->SensorList[sType].isValidEntry )
    {
        /* Register sensor with Batching module */
        errCode = RegisterSensorBatch( pCurrBatchAttribute, sType, &BatchParam );

        if (errCode == OSP_STATUS_OK)
        {
            /* Increase number of batched sensor */
            pCurrBatchAttribute->NumBatchedSensor++;
        }
    } 
    else
    {
        return (OSP_STATUS_SENSOR_ALREADY_REGISTERED);
    }

    return errCode;
}


/****************************************************************************************************
 * @fn      BatchManagerSensorEnable
 *          Enable batching for given sensor 
 *
 * @param   [IN] SensorType - Type of Sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerSensorEnable( ASensorType_t SensorType )
{
    Batch_t *pCurrBatchAttribute;
    uint32_t highThreshold;
    int16_t errCode             = OSP_STATUS_INVALID_PARAMETER;
    BatchStateType_t currState;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Get Type of Batching Q for give sensor Type */
    switch (AndroidTypeToFIFOTypeMap[sType])
    {
    case BATCH_SENSOR_WAKEUP_FIFO:
        pCurrBatchAttribute = &WakeupBatch;
        break;

    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        pCurrBatchAttribute = &NonWakeupBatch;
        break;

    default:
        return errCode;
    }

    /* Check valid entry for given sensor type */
    if ( !pCurrBatchAttribute->SensorList[sType].isValidEntry )
    {
        return (errCode);
    }

    /* Change state to Batch Active if it is in Idle state */
    errCode = BatchStateGet( &currState );
    ASF_assert( errCode == OSP_STATUS_OK );

    if ( currState == BATCH_IDLE )
    {
        errCode = BatchStateSet( BATCH_ACTIVE );
        ASF_assert( errCode == OSP_STATUS_OK );
    }

    /* Calculate and validate High threshold value */
    highThreshold = CalculateHighThreshold( pCurrBatchAttribute );

    /* Set High Threshold value for given Q */
    errCode = QueueHighThresholdSet( pCurrBatchAttribute->pQ, highThreshold );

    return errCode;
}


/****************************************************************************************************
 * @fn      BatchManagerSensorDeRegister
 *          De register given sensor from Batching module
 *
 * @param   [IN] SensorType - Type of Sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerSensorDeRegister( ASensorType_t SensorType )
{
    Batch_t *pCurrBatchAttribute;
    int16_t errCode             = OSP_STATUS_INVALID_PARAMETER;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Get Type of Batching Q for give sensor Type */
    switch (AndroidTypeToFIFOTypeMap[sType])
    {
    case BATCH_SENSOR_WAKEUP_FIFO:
        pCurrBatchAttribute = &WakeupBatch;
        break;

    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        pCurrBatchAttribute = &NonWakeupBatch;
        break;

    default:
        return errCode;
    }

    /* Checks sensor is already unregister or not, if it is unregister already return */
    if ( pCurrBatchAttribute->SensorList[sType].isValidEntry )
    {
        /* DeRegister sensor with Batching module */
        errCode = DeRegisterSensorBatch( pCurrBatchAttribute, sType);

        if (errCode == OSP_STATUS_OK )
        {
            /* decrease number of batched sensor */
            pCurrBatchAttribute->NumBatchedSensor--;
        }
    }
    else
    {
        return OSP_STATUS_OK;
    }

    return errCode;
}


/****************************************************************************************************
 * @fn      BatchManagerSensorDisable
 *          Disable batching for given sensor
 *
 * @param   [IN] sensorType - Type of Sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerSensorDisable( ASensorType_t sensorType )
{
    Batch_t *pCurrBatchAttribute;
    uint32_t highThreshold;
    int16_t errCode             = OSP_STATUS_INVALID_PARAMETER;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (sensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Get Type of Batching Q for give sensor Type */
    switch (AndroidTypeToFIFOTypeMap[sType])
    {
    case BATCH_SENSOR_WAKEUP_FIFO:
        pCurrBatchAttribute = &WakeupBatch;
        errCode = BatchManagerQueueFlush(QUEUE_WAKEUP_TYPE);
        break;

    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        pCurrBatchAttribute = &NonWakeupBatch;
        errCode = BatchManagerQueueFlush(QUEUE_NONWAKEUP_TYPE);
        break;

    default:
        return errCode;
    }

    /* Check that entry is removed from Sensor list before calculate new threshold value */
    if ( pCurrBatchAttribute->SensorList[sType].isValidEntry )
    {
        return (errCode);
    }

#ifdef DECIMATION_MASK

    pCurrBatchAttribute->SensorList[sType].DecimationCnt   = 0;
    pCurrBatchAttribute->SensorList[sType].SampleCnt       = 0;

#endif

    /* Calculate New High threshold value */
    highThreshold = CalculateHighThreshold( pCurrBatchAttribute );

    /* Set New High Threshold value for given Q */
    errCode = QueueHighThresholdSet( pCurrBatchAttribute->pQ, highThreshold );

    /* There is no sensor to be batched so change Batch state to BATCH_IDLE */
    if (NonWakeupBatch.NumBatchedSensor == 0 || WakeupBatch.NumBatchedSensor == 0)
    {
        errCode = BatchStateSet( BATCH_IDLE );
        ASF_assert( errCode == OSP_STATUS_OK );
    }

    return errCode;
}

/****************************************************************************************************
 * @fn      BatchManagerGetSensorState
 *          Get Sensor Status
 *
 * @param   [IN] sensorType - Type of Sensor
 * @param   [OUT] state - ENABLE or DISABLE
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerGetSensorState( ASensorType_t sensorType, int32_t * state)
{
    Batch_t *pCurrBatchAttribute;
    int16_t errCode             = OSP_STATUS_INVALID_PARAMETER;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (sensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Get Type of Batching Q for give sensor Type */
    switch (AndroidTypeToFIFOTypeMap[sType])
    {
    case BATCH_SENSOR_WAKEUP_FIFO:
        pCurrBatchAttribute = &WakeupBatch;
        break;

    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        pCurrBatchAttribute = &NonWakeupBatch;
        break;

    default:
        return errCode;
    }

    /* Check that entry is removed from Sensor list before calculate new threshold value */
    if ( pCurrBatchAttribute->SensorList[sType].isValidEntry )
    {
        *state = ENABLE;
    }
    else
    {
        *state = DISABLE;
    }


    return errCode;
}



/****************************************************************************************************
 * @fn      BatchManagerSensorDataEnQueue
 *          Enqueue given Sensor data packet in given Q
 *
 * @param   [IN] pHiFDataPacket - Pointer of control response packet
 * @param   [IN] packetSize - Size of control response packet
 * @param   [IN] sensorType - Type of sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerSensorDataEnQueue( HostIFPackets_t *pHiFDataPacket, uint16_t packetSize, uint32_t sensorType )
{
    int16_t status;
    Buffer_t *pTempHifPacket;
    BatchStateType_t currBatchState;
    Buffer_t *pHifPacket;

    /* Change sensor base */
    sensorType = M_ToBaseSensorEnum (sensorType);

#ifdef DECIMATION_MASK
    switch ( AndroidTypeToFIFOTypeMap[sensorType] )
    {
    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        /* Check packet needs to be decimated */
        if ( (NonWakeupBatch.SensorList[sensorType].SampleCnt++ % 
            NonWakeupBatch.SensorList[sensorType].DecimationCnt ) != 0)
        {
            return;
        }
        break;

    case BATCH_SENSOR_WAKEUP_FIFO:
        /* Check packet needs to be decimated */
        if ( (WakeupBatch.SensorList[sensorType].SampleCnt++ % 
            WakeupBatch.SensorList[sensorType].DecimationCnt ) != 0)
        {
            return;
        }
        break;

    default:
        status = ( OSP_STATUS_INVALID_PARAMETER );
        break;
    }

#endif

    /* Allocate Packet from Sensor Data Pool */
    status = BatchManagerAllocHifPacket( &pHifPacket, SensorDataPacketPool);

    /* Return as there is no memory in Data Pool */
    if (status != OSP_STATUS_OK)
    {
        return status;
    }

    /* Copy packet to Packet pool */
    SH_MEMCPY( &(pHifPacket->DataStart), pHiFDataPacket, packetSize );

    /* update length of packet */
    pHifPacket->Header.Length = packetSize;

    /* EnQueue packet based on Sensor Type */
    switch ( AndroidTypeToFIFOTypeMap[sensorType] )
    {
    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        /* EnQueue Packet */
        status = EnQueue(_HiFNonWakeupQueue , pHifPacket);

        /* Check Q is full */
        if(status == (OSP_STATUS_QUEUE_FULL))
        {
            /* Get current state of Batch state machine */
            status = BatchStateGet( &currBatchState );
            ASF_assert( status == OSP_STATUS_OK );

            if ( currBatchState == BATCH_ACTIVE_HOST_SUSPEND )
            {
                /* Q is full and host is suspended so Old packet needs to removed to make space for new packets.*/
                DeQueue( _HiFNonWakeupQueue, &pTempHifPacket );

                /* EnQueue packet again */
                status = EnQueue( _HiFNonWakeupQueue , pHifPacket );
                ASF_assert( status == OSP_STATUS_OK );
            }
            else
            {
                status = BatchManagerQueueFlush( QUEUE_NONWAKEUP_TYPE );
            }
        }
        /* If non wakeup on change sensor then save packet in locally */
        if (AndroidTypeToFIFOTypeMap[sensorType] == BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO )
        {
            status = EnqueueOnChangeSensorQ( pHiFDataPacket, packetSize, sensorType );
        }
        break;

    case BATCH_SENSOR_WAKEUP_FIFO:
        /* EnQueue Packet */
        status = EnQueue( _HiFWakeUpQueue , pHifPacket );

        /* Check Q is full */
        if (status == (OSP_STATUS_QUEUE_FULL))
        {
            status = BatchManagerQueueFlush( QUEUE_WAKEUP_TYPE );
        }
        break;

    default:
        status = ( OSP_STATUS_INVALID_PARAMETER );
        break;

    }
    return status;
}


/****************************************************************************************************
 * @fn      BatchManagerDeQueue
 *          DeQueue HiF packets from Sensor Data Q or Control Response Q
 *
 * @param   [OUT] pBuf - Pointer for buffer where packet needs to be store after DeQueue
 * @param   [OUT] pLength - Hold length of packet
 *
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerDeQueue( uint8_t *pBuf, uint32_t *pLength )
{
    int16_t status;
    Buffer_t *pHIFPkt;

    *pLength = 0; //Set length to zero in case we return error status

    /* Get Current Q type */
    status = GetCurrentQType( &CurrQType );

    /* All Qs are empty */
    if (status != OSP_STATUS_OK)
    {
        return status;
    }

    switch(CurrQType)
    {
    case QUEUE_NONWAKEUP_TYPE:
        /* DeQueue packet from the non Wake up Q */
        status = DeQueue( _HiFNonWakeupQueue, &pHIFPkt );

        if ( status == OSP_STATUS_OK )
        {
            /* Copy packet to given buffer */
            SH_MEMCPY( pBuf, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

            /* update length of DeQueue packet */
            *pLength = pHIFPkt->Header.Length;

            /* Free Block from PacketPool */
            status = FreeBlock( SensorDataPacketPool, pHIFPkt );
            ASF_assert(status == OSP_STATUS_OK);
        }

        /* If Non Wakeup Q is Empty check Local On change Sensor Packets */
        if ( status == (OSP_STATUS_QUEUE_EMPTY))
        {
            /* Dequeue packet from on change sensor */
            do 
            {
                status = DequeueOnChangeSensorQ( &pHIFPkt );
            } while ( status == ( OSP_STATUS_INVALID_PARAMETER ));

            /* If it is valid packet then */
            if ( status == OSP_STATUS_OK )
            {
                /* Copy packet to given buffer */
                SH_MEMCPY( pBuf, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

                /* update length of DeQueue packet */
                *pLength = pHIFPkt->Header.Length;
            }
            else
            {
                /* Set Non wakeup Q Empty bit */
                QEmptyCallBack(QUEUE_NONWAKEUP_TYPE);
            }
        }
        break;

    case QUEUE_WAKEUP_TYPE:
        /* DeQueue packet from the Wake up Q */
        status = DeQueue( _HiFWakeUpQueue, &pHIFPkt );

        if ( status == OSP_STATUS_OK )
        {
            /* Copy packet to given buffer */
            SH_MEMCPY( pBuf, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

            /* update length of DeQueue packet */
            *pLength = pHIFPkt->Header.Length;

            /* Free Block from PacketPool */
            status = FreeBlock( SensorDataPacketPool, pHIFPkt );
            ASF_assert(status == OSP_STATUS_OK);
        }
        break;

    case QUEUE_CONTROL_RESPONSE_TYPE:
        /* DeQueue packet from Control Response Q */
        status = DeQueue( _HiFControlQueue, &pHIFPkt );

        if ( status == OSP_STATUS_OK )
        {
            /* Copy packet to given buffer */
            SH_MEMCPY( pBuf, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

            /* update length of DeQueue packet */
            *pLength = pHIFPkt->Header.Length;

            /* Free Block from PacketPool */
            status = FreeBlock( SensorControlResponsePacketPool, pHIFPkt );
            ASF_assert(status == OSP_STATUS_OK);
        }
        break;

    default:
        return (OSP_STATUS_INVALID_PARAMETER);
    }

    return status;
}


/****************************************************************************************************
 * @fn      BatchManagerControlResponseEnQueue
 *          Enqueue control response packet in control response Q
 *
 * @param   [IN] pHiFControlPacket - Pointer of control response packet
   @param   [IN] packetSize - Size of control response packet
 *
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerControlResponseEnQueue( HostIFPackets_t *pHiFControlPacket, uint16_t packetSize )
{
    int16_t status;
    Buffer_t *pHifControlResponsePacket;

    /* Allocate Packet from Sensor Data Pool */
    status = BatchManagerAllocHifPacket( &pHifControlResponsePacket, SensorControlResponsePacketPool );

    /* Return as there is no memory in Packet Pool */
    if (status != OSP_STATUS_OK)
    {
        return status;
    }

    /* Copy packet to Packet pool */
    SH_MEMCPY( &(pHifControlResponsePacket->DataStart), pHiFControlPacket, packetSize );

    /* update length of packet */
    pHifControlResponsePacket->Header.Length = packetSize;

    /* EnQueue Packet in Control response Q */
    status = EnQueue( _HiFControlQueue , pHifControlResponsePacket );

    /* Check Q is full */
    if (status == (OSP_STATUS_QUEUE_FULL))
    {
        status = BatchManagerQueueFlush( QUEUE_WAKEUP_TYPE );
    }

    return status;
}


/****************************************************************************************************
 * @fn      BatchManagerQueueFlush
 *          Flushes given Q
 *
 * @param   [IN] Q Type 
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerQueueFlush( Q_Type_t qType)
{
    int16_t errCode;

    switch(qType)
    {
    case QUEUE_NONWAKEUP_TYPE:
        if (_HiFNonWakeupQueue == NULL)
        {
            errCode = OSP_STATUS_INVALID_PARAMETER;
            return errCode;
        }
        break;

    case QUEUE_WAKEUP_TYPE:
        if (_HiFWakeUpQueue == NULL)
        {
            errCode = OSP_STATUS_INVALID_PARAMETER;
            return errCode;
        }
        break;

    case QUEUE_CONTROL_RESPONSE_TYPE:
        if (_HiFControlQueue == NULL)
        {
            errCode = OSP_STATUS_INVALID_PARAMETER;
            return errCode;
        }
        break;

    default:
        errCode = OSP_STATUS_INVALID_PARAMETER;
        break;
    }

    /* Start Q threshold call back */
    QHighThresholdCallBack( qType );

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      BatchManagerMaxQCount
 *          Provide Maximum event stored by NonWakup and Wakeup Qs.
 *
 * @return  Max. Number of event handle by Batchmanager Q 
 *
 ***************************************************************************************************/
uint32_t BatchManagerMaxQCount( void )
{
    /* Sum up Q capacity for both Wakeup and NonWakeup Q */
    return( NonWakeupBatch.pQ->Capacity + WakeupBatch.pQ->Capacity );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
