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

/* Queue Sizes Common Definitions */
#define QUEUE_LOW_THR                               (0)
#define QUEUE_HIGH_THR                              (1)
#define HOST_WAKEUP_TOLERANCE_PERCENT               (85)
#define HIF_PACKET_SIZE                             M_CalcBufferSize(sizeof(HostIFPackets_t))

/* Sensor Data Queue Size Definition */
/* HIF Queue size indicate number of packet a single queue can hold */
#define HIF_NWKUP_SENSOR_DATA_QUEUE_SIZE            (200)
#define HIF_WKUP_SENSOR_DATA_QUEUE_SIZE             (200)

/* Combined HIF Sensor Data Packet pool size (Wakeup + Non Wakeup) */
#define HIF_SENSOR_DATA_PACKET_POOL_SIZE            ( HIF_NWKUP_SENSOR_DATA_QUEUE_SIZE + HIF_WKUP_SENSOR_DATA_QUEUE_SIZE )


/* Sensor Control Queue Size Definition */
/* HIF Queue size indicate number of Event a single queue can hold */
#define HIF_CONTROL_QUEUE_SIZE                      (10)
#define NUM_CONTROL_QUEUE                           (1)

/* HIF Control Packet pool size */
#define HIF_CONTROL_PACKET_POOL_SIZE                ( HIF_CONTROL_QUEUE_SIZE * ( NUM_CONTROL_QUEUE ))

/* Queue empty Flags */
#define QUEUE_WAKEUP_EMPTY_BIT                      (0x01)
#define QUEUE_NONWAKEUP_EMPTY_BIT                   (0x02)
#define QUEUE_CONTROL_RESPONSE_EMPTY_BIT            (0x04)
#define QUEUE_ALL_EMPTY_MASK                        (0x07)


/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/* Batch Queue Type Enumeration */
typedef enum _BatchQType
{
    BATCH_WAKEUP_QUEUE,
    BATCH_NONWAKEUP_QUEUE,
    BATCH_QUEUE_NUM
}BatchQType_t;

/* Structures to hold batching sensor parameters */
typedef struct _BatchSensorParam
{
    uint64_t                ActualSamplingRate;     /* Actual Sampling Rate in ns for batched sensor */
    uint64_t                RequestedSamplingRate;  /* Requested Sampling Rate in ns for batched sensor */
    uint64_t                ReportLatency;          /* Max. Report Latency for batched sensor */
    BatchQType_t            QType;                  /* Sensor FIFO Type */
#ifdef DECIMATION_MASK
    uint32_t                SampleCnt;              /* Sample count for sensor*/
    uint32_t                DecimationCnt;          /* Decimation count for sensor*/
#endif
    osp_bool_t              isValidEntry;           /* Flag for Valid entry */
    osp_bool_t              isSensorEnabled;        /* Flag for Sensor Enable status */

} BatchSensorParam_t;

/* Structure to hold Batch Queue parameters */
typedef struct _BatchQParam
{
    Queue_t       *pQ;                              /* pointer to sensor queue */
    uint64_t      MinReportLatency;                 /* min. report latency from sensor list */
    uint8_t       NumBatchedSensor;                 /* Number of sensor currently batched */
}BatchQParam_t;

/* Structure to hold Batch Descriptor */
typedef struct _BatchDescriptor
{
    BatchSensorParam_t  SensorList[MAX_NUMBER_SENSORS];         /* Batching sensor list */
    BatchQParam_t       BatchQ[BATCH_QUEUE_NUM];                /* Batching Queues */
} BatchDescriptor_t;

/* Structures to hold On Change Sensor Sample */
typedef struct _BatchOnChangeSensor
{
    BufferHeader_t      Header;                     /* Buffer header */
    HostIFPackets_t     Sample;                     /* HiF Packet */
    ASensorType_t       SensorType;                 /* Sensor Type of current HiF Packet */
    osp_bool_t          ValidFlag;                  /* HiF Packet is valid Flag */
} BatchOnChangeSensor_t;

typedef struct _OnChangeSensorSample
{
    BatchOnChangeSensor_t   SensorList[NUM_ONCHANGE_NONWAKEUP_SENSOR];      /* On change sensor list */
    osp_bool_t              Empty;                                          /* Buffer empty flag */
} OnChangeSensorBuffer_t;

typedef struct _SensorTypeAndRateMap
{
    BatchSensorFIFOType_t    FIFOType;              /* Sensor FIFO Type */
    uint64_t                 SamplingRate;          /* Sensor Sampling Rate */
}SensorTypeAndRateMap_t;


/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Mapping table for Sensor type to get Sampling rate and FFIO Type */
static const SensorTypeAndRateMap_t SensorTypeAndRateMap[MAX_NUMBER_SENSORS] =
{
    [SENSOR_META_DATA]                   = { BATCH_SENSOR_NUM,                     ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_ACCELEROMETER]               = { BATCH_SENSOR_NONWAKEUP_FIFO,          ACC_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_GEOMAGNETIC_FIELD]           = { BATCH_SENSOR_NONWAKEUP_FIFO,          MAG_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_MAGNETIC_FIELD]              = { BATCH_SENSOR_NONWAKEUP_FIFO,          MAG_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_ORIENTATION]                 = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_GYROSCOPE]                   = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_LIGHT]                       = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_PRESSURE]                    = { BATCH_SENSOR_NONWAKEUP_FIFO,          PRES_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_TEMPERATURE]                 = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_PROXIMITY]                   = { BATCH_SENSOR_WAKEUP_FIFO,             ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_GRAVITY]                     = { BATCH_SENSOR_NONWAKEUP_FIFO,          ACC_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_LINEAR_ACCELERATION]         = { BATCH_SENSOR_NONWAKEUP_FIFO,          ACC_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_ROTATION_VECTOR]             = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_RELATIVE_HUMIDITY]           = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_AMBIENT_TEMPERATURE]         = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_MAGNETIC_FIELD_UNCALIBRATED] = { BATCH_SENSOR_WAKEUP_FIFO,             MAG_ACTUAL_SAMPLE_PERIOD  },
    [SENSOR_GAME_ROTATION_VECTOR]        = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_GYROSCOPE_UNCALIBRATED]      = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [SENSOR_SIGNIFICANT_MOTION]          = { BATCH_SENSOR_WAKEUP_FIFO,             ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_STEP_DETECTOR]               = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_STEP_COUNTER]                = { BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO, ON_CHANGE_SAMPLE_PERIOD   },
    [SENSOR_GEOMAGNETIC_ROTATION_VECTOR] = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },

    /* Private Sensor Type */
    [PSENSOR_ACCELEROMETER_RAW]          = { BATCH_SENSOR_NONWAKEUP_FIFO,          ACC_ACTUAL_SAMPLE_PERIOD  },
    [PSENSOR_MAGNETIC_FIELD_RAW]         = { BATCH_SENSOR_NONWAKEUP_FIFO,          MAG_ACTUAL_SAMPLE_PERIOD  },
    [PSENSOR_GYROSCOPE_RAW]              = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [PSENSOR_LIGHT_UV]                   = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_LIGHT_RGB]                  = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_STEP]                       = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_ACCELEROMETER_UNCALIBRATED] = { BATCH_SENSOR_NONWAKEUP_FIFO,          ACC_ACTUAL_SAMPLE_PERIOD  },
    [PSENSOR_ORIENTATION]                = { BATCH_SENSOR_NONWAKEUP_FIFO,          GYRO_ACTUAL_SAMPLE_PERIOD },
    [PSENSOR_CONTEXT_DEVICE_MOTION]      = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_CONTEXT_CARRY]              = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_CONTEXT_POSTURE]            = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_CONTEXT_TRANSPORT]          = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_GESTURE_EVENT]              = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_HEART_RATE]                 = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [SYSTEM_REAL_TIME_CLOCK]             = { BATCH_SENSOR_NONWAKEUP_FIFO,          ON_CHANGE_SAMPLE_PERIOD   },
    [PSENSOR_MAGNETIC_FIELD_ANOMALY]     = { BATCH_SENSOR_NONWAKEUP_FIFO,          MAG_ACTUAL_SAMPLE_PERIOD  },
};

static BatchDescriptor_t BatchDesc;
static OnChangeSensorBuffer_t NwOnChangeSensorBuffer;   /* for Non-Wakeup-On-Change Sensors */
static osp_bool_t isBatchManagerInitialized = FALSE;
static Q_Type_t CurrQType = NUM_QUEUE_TYPE;

static Queue_t *_HiFNonWakeupQueue = NULL;
static Queue_t *_HiFWakeUpQueue = NULL;
static Queue_t *_HiFControlQueue = NULL;

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
static uint32_t CalculateHighThreshold( BatchDescriptor_t *pBatchDesc, BatchQType_t QType );
static int16_t RegisterSensorBatch( BatchDescriptor_t *pBatchDesc, uint16_t sensorType, void *pSensorConfig );
static int16_t DeRegisterSensorBatch( BatchDescriptor_t *pBatchDesc, uint32_t sensorType );
static int16_t FindMinReportLatency( BatchDescriptor_t *pBatchDesc, BatchQType_t QType );
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
 *          This Callback is called when queue reaches high threshold mark
 *
 * @param  [IN] QType - Queue identifier
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

        /* return if Batch state is in batching and host is suspended */
        if ( currState == BATCH_ACTIVE_HOST_SUSPEND)
        {
            return;
        }
        else
        {
            /* Set Host Interrupt to indicate we have data for the Host */
            QEmptyRegister &= ~(QUEUE_NONWAKEUP_EMPTY_BIT);
            SensorHubAssertInt();
        }
        break;

    case QUEUE_WAKEUP_TYPE:
        /* If CB is from Wakeup queue then assert host interrupt irrespective of Batch State */
        QEmptyRegister &= ~(QUEUE_WAKEUP_EMPTY_BIT);
        SensorHubAssertInt();
        break;

    case QUEUE_CONTROL_RESPONSE_TYPE:
        /* If CB is from Control Response queue then assert host interrupt irrespective of Batch State */
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
 * @param  [IN] QType - Queue identifier
 *
 * @return  none
 *
 ***************************************************************************************************/
static void QEmptyCallBack( Q_Type_t QType )
{

    /* Mark queue empty */
    switch ( QType )
    {
    case QUEUE_WAKEUP_TYPE:
        QEmptyRegister |= QUEUE_WAKEUP_EMPTY_BIT;
        break;

    case QUEUE_NONWAKEUP_TYPE:
        /* Before set Non wakeup empty flag check Locally stored NonWakeupOnchange queue empty */
        if (NwOnChangeSensorBuffer.Empty)
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

    /* If all queues are empty deassert Host interrupt pin */
    if ( QEmptyRegister == QUEUE_ALL_EMPTY_MASK )
    {
        SensorHubDeAssertInt();
    }
}


/****************************************************************************************************
 * @fn      GetCurrentQType
 *          This is helper function for BatchManagerDeQueue, This function checks QEmptyRegister
 *          to find which queue is full so BatchManagerDeQueue can Dequeue from that queue.
 *
 * @param  [OUT] pQType - Pointer which returns current queue identifier that can be dequeued from
 *
 * @return  OSP_STATUS_OK or error code
 ***************************************************************************************************/
static int16_t GetCurrentQType( Q_Type_t *pQType )
{

    if ( (pQType == NULL) )
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Control response queue is empty */
    if ( (QEmptyRegister & QUEUE_CONTROL_RESPONSE_EMPTY_BIT) )
    {
        /* Set Current queue type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current queue is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current queue type to control response */
            *pQType = QUEUE_CONTROL_RESPONSE_TYPE;
            return OSP_STATUS_OK;
        }
    }

    /* Wakeup queue is empty */
    if ( (QEmptyRegister & QUEUE_WAKEUP_EMPTY_BIT) )
    {
        /* Set Current queue type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current queue is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current queue type to Wakeup */
            *pQType = QUEUE_WAKEUP_TYPE;
            return OSP_STATUS_OK;
        }
    }

    /* Non Wakeup queue is empty */
    if( QEmptyRegister & QUEUE_NONWAKEUP_EMPTY_BIT )
    {
        /* Set Current queue type to default */
        *pQType = NUM_QUEUE_TYPE;
    }
    else
    {
        /* Check current queue is set to default */
        if ( *pQType == NUM_QUEUE_TYPE )
        {
            /* Then set current queue type to NonWakeup */
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
 * @param   [IN] pBatchDesc - pointer to batching descriptor structure
 * @param   [IN] QType      - Queue Type
 *
 * @return  Calculated high threshold value
 *
 ***************************************************************************************************/
static uint32_t CalculateHighThreshold( BatchDescriptor_t *pBatchDesc, BatchQType_t QType )
{
    uint8_t i;
    uint32_t highThr = 0;
    uint32_t highThrLimit;

    /* Find min. Report latency before we calculate high threshold value */
    FindMinReportLatency( pBatchDesc, QType );

    /* calculate threshold limit based on FIFO size and a tolerance % to account for Host Wake up time */
    highThrLimit = ( pBatchDesc->BatchQ[QType].pQ->Capacity * HOST_WAKEUP_TOLERANCE_PERCENT ) / 100 ;

    /* If Min. Report Latency is 0 then Sensor Event should be send as soon as it occurs */
    if ( ( pBatchDesc->BatchQ[QType].MinReportLatency == 0 ) ||
         ( pBatchDesc->BatchQ[QType].MinReportLatency == (uint64_t)DEFAULT_REPORT_LATENCY ) )
    {
        highThr = DEFAULT_HIGH_THRESHOLD;
        return highThr;
    }

    /* Calculate High threshold value */
    for ( i = 0; i < MAX_NUMBER_SENSORS; i++ )
    {
        if ( pBatchDesc->SensorList[i].QType != QType )
        {
            continue; /* skip Sensors belonging to different Queue type */
        }

        /* Check only registered sensors for threshold calculation. Also skip On-Change sensors in thresold calculation */
        if ( ( pBatchDesc->SensorList[i].isSensorEnabled ) &&
             ( pBatchDesc->SensorList[i].ActualSamplingRate != ON_CHANGE_SAMPLE_PERIOD ) )
        {
            /* Calculate high threshold value for one sensor entry and accumulate to previous value*/
            highThr += ( pBatchDesc->BatchQ[QType].MinReportLatency / pBatchDesc->SensorList[i].ActualSamplingRate );
        }
    }

    /* If the calculated FIFO threshold based on MRL exceeds the FIFO capacity, report when the FIFO is full */
    if ( highThr > highThrLimit ) {

        highThr = highThrLimit;
    }
    else if ( highThr < DEFAULT_HIGH_THRESHOLD )
    {
        /* If the registered sensors are all ON-CHANGE, the calculated threshold value will be 0 */
        /* TODO: ensure MRL for On-change sensors */
        highThr = DEFAULT_HIGH_THRESHOLD;
    }

    return highThr;
}


/****************************************************************************************************
 * @fn      RegisterSensorBatch
 *          Register Sensor parameter (Sampling Rate and Report latency) with Batch Manager
 *
 * @param   [IN] pBatchDesc - pointer to batching descriptor structure
 * @param   [IN] sensorType - SensorType - Android Sensor Type
 * @param   [IN] pSensorConfig - pointer to Sensor config data
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t RegisterSensorBatch( BatchDescriptor_t *pBatchDesc, uint16_t sensorType, void *pSensorConfig )
{
    BatchSensorParam_t  *pSensorParam = (BatchSensorParam_t *)pSensorConfig;

    /* Validate Batch Descriptor */
    if ((pBatchDesc == NULL) || (pSensorConfig == NULL))
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Register sensor to be batched */
    /* TODO : Requested Sample Rate is not currently used for decimation calculations. */
    pBatchDesc->SensorList[sensorType].ReportLatency         = pSensorParam->ReportLatency;
    pBatchDesc->SensorList[sensorType].RequestedSamplingRate = pSensorParam->RequestedSamplingRate;
    pBatchDesc->SensorList[sensorType].isValidEntry          = TRUE;

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      DeRegisterSensorBatch
 *          DeRegister Sensor parameter (Sampling Rate and Report latency) with Batch Manager
 *
 * @param   [IN] pBatchDesc - Pointer of Batching Descriptor
 * @param   [IN] sensorType - Type of Sensor

 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t DeRegisterSensorBatch( BatchDescriptor_t *pBatchDesc, uint32_t sensorType )
{
    /* Validate Batch Descriptor */
    if (pBatchDesc == NULL)
    {
        return (OSP_STATUS_NULL_POINTER);
    }

    /* Unregister sensor to remove from batching */
    pBatchDesc->SensorList[sensorType].ReportLatency         = (uint64_t)DEFAULT_REPORT_LATENCY;
    pBatchDesc->SensorList[sensorType].RequestedSamplingRate = MIN_SAMPLING_PERIOD;
    pBatchDesc->SensorList[sensorType].isValidEntry          = FALSE;

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      FindMinReportLatency
 *          Find Min. Report Latency for given Batching structure
 *
 * @param   [IN] pBatchDesc - Pointer of Batching Descriptor
 * @param   [IN] QType      - Sensor Queue Type
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t FindMinReportLatency( BatchDescriptor_t *pBatchDesc, BatchQType_t QType )
{
    uint8_t i;

    /* Validate Batch Descriptor */
    if ( pBatchDesc == NULL )
    {
        return ( OSP_STATUS_NULL_POINTER );
    }

    /* Set min. Report latency to Max initially */
    pBatchDesc->BatchQ[QType].MinReportLatency = (uint64_t)DEFAULT_REPORT_LATENCY;

    /* Search min.Report latency number from Batching Descriptor */
    for ( i = 0; i < MAX_NUMBER_SENSORS; i++ )
    {
        if ( ( pBatchDesc->SensorList[i].QType == QType ) &&
             ( pBatchDesc->SensorList[i].ReportLatency < pBatchDesc->BatchQ[QType].MinReportLatency ) )
        {
            pBatchDesc->BatchQ[QType].MinReportLatency = pBatchDesc->SensorList[i].ReportLatency;
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
 *          Initialize Non-Wakeup, Wakeup and Control Response queue
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
    _HiFNonWakeupQueue = QueueCreate( HIF_NWKUP_SENSOR_DATA_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert(_HiFNonWakeupQueue != NULL);

    /* Create Wakeup Sensor Queue */
    _HiFWakeUpQueue = QueueCreate( HIF_WKUP_SENSOR_DATA_QUEUE_SIZE, QUEUE_LOW_THR, QUEUE_HIGH_THR );
    ASF_assert(_HiFWakeUpQueue != NULL);

    /* Register Callbacks for High and Low Thresholds   */
    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_EMPTY_CB, (fpQueueEvtCallback_t) QEmptyCallBack,
                                     (void *)QUEUE_NONWAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    errCode = QueueRegisterCallBack( _HiFNonWakeupQueue , QUEUE_HIGH_THRESHOLD_CB,
                                     (fpQueueEvtCallback_t) QHighThresholdCallBack,
                                     (void *)QUEUE_NONWAKEUP_TYPE );
    ASF_assert(errCode == OSP_STATUS_OK);

    /* Register Callbacks for High and Low Thresholds */
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

    /* Register Callbacks for High and Low Thresholds */
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
        if ( NwOnChangeSensorBuffer.SensorList[i].ValidFlag )
        {
            /* Match sensor type before overwriting packet */
            if ( NwOnChangeSensorBuffer.SensorList[i].SensorType == sensorType )
            {
                /* Overwrite data to onChange sensor type */
                SH_MEMCPY( &(NwOnChangeSensorBuffer.SensorList[i].Sample), pHiFDataPacket, packetSize );
                NwOnChangeSensorBuffer.SensorList[i].ValidFlag = TRUE;
                NwOnChangeSensorBuffer.SensorList[i].Header.Length = packetSize;
                NwOnChangeSensorBuffer.SensorList[i].SensorType = (ASensorType_t)sensorType;
                NwOnChangeSensorBuffer.Empty = FALSE;
                return OSP_STATUS_OK;
            }
        }
        else
        {
            /* Copy data to onChange sensor type */
            SH_MEMCPY( &(NwOnChangeSensorBuffer.SensorList[i].Sample), pHiFDataPacket, packetSize );
            NwOnChangeSensorBuffer.SensorList[i].ValidFlag = TRUE;
            NwOnChangeSensorBuffer.SensorList[i].Header.Length = packetSize;
            NwOnChangeSensorBuffer.SensorList[i].SensorType = (ASensorType_t)sensorType;
            NwOnChangeSensorBuffer.Empty = FALSE;
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
        if ( NwOnChangeSensorBuffer.SensorList[index].ValidFlag )
        {
            *pBuf = ( Buffer_t *)&NwOnChangeSensorBuffer.SensorList[index].Header;
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
        NwOnChangeSensorBuffer.Empty = TRUE;
        index = 0;
        return (OSP_STATUS_QUEUE_EMPTY);
    }
    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      DiscardPktsFromOnChangeSensorBuf
 *          Discards packet corresponding to a sensor type from on-change non wakeup local Sample pool.
 *
 * @param   [IN] sensorType - Type of sensor
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
static int16_t DiscardPktsFromOnChangeSensorBuf( uint32_t sensorType )
{
    int16_t i;
    int16_t validPackets = 0;

    for ( i = 0; i < NUM_ONCHANGE_NONWAKEUP_SENSOR; i++ )
    {
        /* Check is valid packet exists for the sensor */
        if ( NwOnChangeSensorBuffer.SensorList[i].ValidFlag )
        {
            validPackets++;
            if ( NwOnChangeSensorBuffer.SensorList[i].SensorType == sensorType )
            {
                NwOnChangeSensorBuffer.SensorList[i].ValidFlag = false;
                validPackets--;
             }
        }
    }

    /* Mark buffer empty if zero valid packets in it */
    if ( validPackets == 0 )
    {
        NwOnChangeSensorBuffer.Empty = true;
    }

    D1_printf("\r\n%s: Packets valid: %d\r\n",__FUNCTION__,validPackets );

    return (OSP_STATUS_OK);
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
        /* Initialize queues */
        QInitialize();

        /* Validate queues */
        if ( ( _HiFWakeUpQueue == NULL) || (_HiFNonWakeupQueue == NULL) )
        {
            return (OSP_STATUS_NULL_POINTER);
        }

        /* Initialize Batch structure with queue */
        BatchDesc.BatchQ[BATCH_WAKEUP_QUEUE].pQ    = (Queue_t *)_HiFWakeUpQueue;
        BatchDesc.BatchQ[BATCH_NONWAKEUP_QUEUE].pQ = (Queue_t *)_HiFNonWakeupQueue;

        /* Initialize Batching descriptor structure */
        for ( i = 0; i < MAX_NUMBER_SENSORS; i++ )
        {
            BatchDesc.SensorList[i].ReportLatency         = (uint64_t)DEFAULT_REPORT_LATENCY;
            BatchDesc.SensorList[i].RequestedSamplingRate = MIN_SAMPLING_PERIOD;
            BatchDesc.SensorList[i].ActualSamplingRate    = SensorTypeAndRateMap[i].SamplingRate;
            BatchDesc.SensorList[i].isValidEntry          = FALSE;
            BatchDesc.SensorList[i].isSensorEnabled       = FALSE;
            /* Use Wake up Queue for Wake up FIFO Sensors and NonWake Up Queue for Non Wake up and Non Wake up on change FIFOs */
            BatchDesc.SensorList[i].QType                 = ( SensorTypeAndRateMap[i].FIFOType == BATCH_SENSOR_WAKEUP_FIFO ) ?
                                                              (BATCH_WAKEUP_QUEUE):(BATCH_NONWAKEUP_QUEUE);
#ifdef DECIMATION_MASK
            BatchDesc.SensorList[i].DecimationCnt         = 0;
            BatchDesc.SensorList[i].SampleCnt             = 0;
#endif
        }

        /* Initialize min. Report latency to max */
        BatchDesc.BatchQ[BATCH_WAKEUP_QUEUE].MinReportLatency       = (uint64_t)DEFAULT_REPORT_LATENCY;
        BatchDesc.BatchQ[BATCH_NONWAKEUP_QUEUE].MinReportLatency    = (uint64_t)DEFAULT_REPORT_LATENCY;

        BatchDesc.BatchQ[BATCH_WAKEUP_QUEUE].NumBatchedSensor       = 0;
        BatchDesc.BatchQ[BATCH_NONWAKEUP_QUEUE].NumBatchedSensor    = 0;

        /* Initialize on change non wakeup sensor list */
        for (i = 0; i < NUM_ONCHANGE_NONWAKEUP_SENSOR; i++)
        {
            NwOnChangeSensorBuffer.SensorList[i].ValidFlag = FALSE;
            NwOnChangeSensorBuffer.Empty = TRUE;
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
    BatchDescriptor_t *pCurrBatchDesc = &BatchDesc;
    int16_t errCode                   = OSP_STATUS_INVALID_PARAMETER;
    BatchSensorParam_t BatchParam;
    BatchQType_t       QType;
    uint16_t           sampleFreq;
    uint32_t           sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* get queue type */
    QType = pCurrBatchDesc->SensorList[sType].QType;

    BatchParam.RequestedSamplingRate = SamplingRate;
    BatchParam.ReportLatency = ReportLatency;

    /* Convert given sampling period to sampling frequency */
    sampleFreq = ( TIME_1SEC_NS_UNIT / BatchParam.RequestedSamplingRate );

    /* Validate Data rate value */
    if (sampleFreq > MAX_SAMPLING_FREQ_HZ)
    {
        return (errCode);
    }
#ifdef DECIMATION_MASK
    pCurrBatchDesc->SensorList[sType].DecimationCnt = (uint32_t)(SamplingRate / SensorDriverRate);

    if ( pCurrBatchDesc->SensorList[sType].DecimationCnt == 0 )
    {
        pCurrBatchDesc->SensorList[sType].DecimationCnt = 1;
    }

#endif
    /* Checks sensor is already registered or not */
    if ( !pCurrBatchDesc->SensorList[sType].isValidEntry )
    {
        /* Register sensor with Batching module */
        errCode = RegisterSensorBatch( pCurrBatchDesc, sType, &BatchParam );

        if (errCode == OSP_STATUS_OK)
        {
            /* Increase number of batched sensor */
            pCurrBatchDesc->BatchQ[QType].NumBatchedSensor++;
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
    BatchDescriptor_t *pCurrBatchDesc = &BatchDesc;
    uint32_t          highThreshold;
    int16_t           errCode         = OSP_STATUS_INVALID_PARAMETER;
    BatchStateType_t  currState;
    BatchQType_t  QType;
    uint32_t sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Check valid entry for given sensor type */
    if ( !pCurrBatchDesc->SensorList[sType].isValidEntry )
    {
        return (errCode);
    }

    pCurrBatchDesc->SensorList[sType].isSensorEnabled = true;

    /* get current queue type */
    QType = pCurrBatchDesc->SensorList[sType].QType;

    /* Change state to Batch Active if it is in Idle state */
    errCode = BatchStateGet( &currState );
    ASF_assert( errCode == OSP_STATUS_OK );

    if ( currState == BATCH_IDLE )
    {
        errCode = BatchStateSet( BATCH_ACTIVE );
        ASF_assert( errCode == OSP_STATUS_OK );
    }

    /* Calculate and validate High threshold value */
    highThreshold = CalculateHighThreshold( pCurrBatchDesc, QType );

    /* Set High Threshold value for given queue */
    errCode = QueueHighThresholdSet( pCurrBatchDesc->BatchQ[QType].pQ, highThreshold );

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
    BatchDescriptor_t *pCurrBatchDesc = &BatchDesc;
    BatchQType_t       QType;
    int16_t            errCode        = OSP_STATUS_INVALID_PARAMETER;
    uint32_t           sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (SensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    QType = pCurrBatchDesc->SensorList[sType].QType;

    /* Checks sensor is already unregistered or not */
    if ( pCurrBatchDesc->SensorList[sType].isValidEntry )
    {
        /* DeRegister sensor with Batching module */
        errCode = DeRegisterSensorBatch( pCurrBatchDesc, sType);

        if (errCode == OSP_STATUS_OK )
        {
            /* decrease number of batched sensor */
            pCurrBatchDesc->BatchQ[QType].NumBatchedSensor--;
        }
    }
    else
    {
        return OSP_STATUS_SENSOR_NOT_REGISTERED;
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
    BatchDescriptor_t *pCurrBatchDesc = &BatchDesc;
    BatchQType_t      QType;
    Buffer_t          *pTempHifPacket;
    uint32_t          highThreshold;
    uint32_t          sType;
    uint32_t          size;
    int16_t           errCode         = OSP_STATUS_INVALID_PARAMETER;
    int16_t           stat;


    /* Change sensor base */
    sType = M_ToBaseSensorEnum (sensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* get current queue type */
    QType = pCurrBatchDesc->SensorList[sType].QType;

    /* Check that entry is removed from Sensor list before calculate new threshold value */
    if ( pCurrBatchDesc->SensorList[sType].isValidEntry )
    {
        return (errCode);
    }

#ifdef DECIMATION_MASK

    pCurrBatchDesc->SensorList[sType].DecimationCnt   = 0;
    pCurrBatchDesc->SensorList[sType].SampleCnt       = 0;

#endif

    pCurrBatchDesc->SensorList[sType].isSensorEnabled = false;

    /* Calculate New High threshold value */
    highThreshold = CalculateHighThreshold( pCurrBatchDesc, QType );

    /* Set New High Threshold value for given queue */
    errCode = QueueHighThresholdSet( pCurrBatchDesc->BatchQ[QType].pQ, highThreshold );

    /* Check if sensor FIFO type is Non Wake up On Change FIFO */
    if ( SensorTypeAndRateMap[sType].FIFOType == BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO )
    {
        DiscardPktsFromOnChangeSensorBuf( sType );
    }

    /* Get current Queue size */
    QueueGetSize( pCurrBatchDesc->BatchQ[QType].pQ, &size );    /* get number of entries */
    D1_printf( "\r\nQueue size before discard = %d\r\n", size );

    /* Discard packets for the disabled sensor from appropriate Queue
     * This is done so as to not retain and transfer stale sensor data when sensor is re-enabled
     */
    while ( size > 0 )
    {
        stat = DeQueue( pCurrBatchDesc->BatchQ[QType].pQ , &pTempHifPacket );
        ASF_assert( pTempHifPacket->Header.Length > SENSOR_DATA_PKT_HEADER_SIZE );    /*  check packet validity */

        /* check the packet header for sensor type */
        if ( ( *( &(pTempHifPacket->DataStart ) + PKT_SENSOR_ID_BYTE_OFFSET ) & SENSOR_TYPE_MASK ) == sType )
        {
            errCode = FreeBlock( SensorDataPacketPool, pTempHifPacket );
            ASF_assert( errCode == OSP_STATUS_OK );

        }
        else
        {
             /* enqueue back if it belongs to a different sensor */
             stat = EnQueue( pCurrBatchDesc->BatchQ[QType].pQ , pTempHifPacket );
             ASF_assert( stat == OSP_STATUS_OK );
        }
        size--;
    }

    QueueGetSize( pCurrBatchDesc->BatchQ[QType].pQ, &size );    /* get number of entries */
    D1_printf("\r\nQueue size after discard = %d\r\n",size);

    /* There is no sensor to be batched so change Batch state to BATCH_IDLE */
    if ( ( BatchDesc.BatchQ[BATCH_WAKEUP_QUEUE].NumBatchedSensor == 0 ) &&
         ( BatchDesc.BatchQ[BATCH_NONWAKEUP_QUEUE].NumBatchedSensor == 0 ) )
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
    BatchDescriptor_t *pCurrBatchDesc = &BatchDesc;
    int16_t            errCode        = OSP_STATUS_INVALID_PARAMETER;
    uint32_t           sType;

    /* Change sensor base */
    sType = M_ToBaseSensorEnum (sensorType);

    /* Check sensor type and data is valid */
    if ( sType >= MAX_NUMBER_SENSORS )
    {
        return (errCode);
    }

    /* Check that entry is removed from Sensor list before calculate new threshold value */
    if ( pCurrBatchDesc->SensorList[sType].isSensorEnabled)
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
 *          Enqueue given Sensor data packet in given queue
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
    int16_t                 status;
    Buffer_t                *pTempHifPacket;
    BatchStateType_t        currBatchState;
    Buffer_t                *pHifPacket;
    BatchSensorFIFOType_t   FIFOType;

    /* Change sensor base */
    sensorType = M_ToBaseSensorEnum (sensorType);

#ifdef DECIMATION_MASK
    /* Check packet needs to be decimated */
    if ( ( BatchDesc.SensorList[sensorType].SampleCnt++ %
           BatchDesc.SensorList[sensorType].DecimationCnt ) != 0)
    {
        return;
    }
#endif

    /* Allocate Packet from Sensor Data Pool */
    status = BatchManagerAllocHifPacket( &pHifPacket, SensorDataPacketPool);

    /* Return as there is no memory in Data Pool */
    if (status != OSP_STATUS_OK)
    {
        return status;
    }

    FIFOType = SensorTypeAndRateMap[sensorType].FIFOType;

    /* Copy packet to Packet pool */
    SH_MEMCPY( &(pHifPacket->DataStart), pHiFDataPacket, packetSize );

    /* update length of packet */
    pHifPacket->Header.Length = packetSize;

    /* EnQueue packet based on Sensor Type */
    switch ( FIFOType )
    {
    case BATCH_SENSOR_NONWAKEUP_FIFO:
    case BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO:
        /* EnQueue Packet */
        status = EnQueue(_HiFNonWakeupQueue , pHifPacket);

        /* Check queue is full */
        if(status == (OSP_STATUS_QUEUE_FULL))
        {
            /* Get current state of Batch state machine */
            status = BatchStateGet( &currBatchState );
            ASF_assert( status == OSP_STATUS_OK );

            if ( currBatchState == BATCH_ACTIVE_HOST_SUSPEND )
            {
                /* queue is full and host is suspended so Old packet needs to removed to make space for new packets.*/
                DeQueue( _HiFNonWakeupQueue, &pTempHifPacket );

                /* Free Block from PacketPool */
                status = FreeBlock( SensorDataPacketPool, pTempHifPacket );
                ASF_assert( status == OSP_STATUS_OK );

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
        if ( FIFOType == BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO )
        {
            status = EnqueueOnChangeSensorQ( pHiFDataPacket, packetSize, sensorType );
        }
        break;

    case BATCH_SENSOR_WAKEUP_FIFO:
        /* EnQueue Packet */
        status = EnQueue( _HiFWakeUpQueue , pHifPacket );

        /* Check queue is full */
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
 *          DeQueue HiF packets from Sensor Data queue or Control Response queue
 *
 * @param   [OUT] pBuf - Pointer for buffer where packet needs to be store after DeQueue
 * @param   [IN/OUT] pLength - [IN] Max buffer size for dequeue; [OUT] Total length of packets dequeued
 *
 *
 * @return  OSP_STATUS_OK or error code
 *
 ***************************************************************************************************/
int16_t BatchManagerDeQueue( uint8_t *pBuf, uint32_t *pLength )
{
    int16_t status;
    Buffer_t *pHIFPkt;

    uint32_t bufSize = *pLength;
    uint32_t pktLen  = 0;

    *pLength = 0; //Set length to zero in case we return error status

    do
    {
        /* Get Current queue type */
        status = GetCurrentQType( &CurrQType );

        /* All queues are empty */
        if (status != OSP_STATUS_OK)
        {
            break;
        }

        switch(CurrQType)
        {
        case QUEUE_NONWAKEUP_TYPE:
            /* DeQueue packet from the non Wake up queue */
            status = DeQueue( _HiFNonWakeupQueue, &pHIFPkt );

            if ( status == OSP_STATUS_OK )
            {
                /* Copy packet to given buffer */
                SH_MEMCPY( pBuf + *pLength, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

                /* update length of DeQueue packet */
                pktLen = pHIFPkt->Header.Length;
                *pLength += pktLen;

                /* Free Block from PacketPool */
                status = FreeBlock( SensorDataPacketPool, pHIFPkt );
                ASF_assert(status == OSP_STATUS_OK);
            }
            /* If Non Wakeup queue is Empty check Local On change Sensor Packets */
            else if ( status == OSP_STATUS_QUEUE_EMPTY)
            {
                /* Dequeue packet from on change sensor */
                do
                {
                    status = DequeueOnChangeSensorQ( &pHIFPkt );
                } while ( status == OSP_STATUS_INVALID_PARAMETER );

                /* If it is valid packet then */
                if ( status == OSP_STATUS_OK )
                {
                    /* Copy packet to given buffer */
                    SH_MEMCPY( pBuf + *pLength, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

                    /* update length of DeQueue packet */
                    pktLen = pHIFPkt->Header.Length;
                    *pLength += pktLen;
                }
                else
                {
                    /* Set Non wakeup queue Empty bit */
                    QEmptyCallBack(QUEUE_NONWAKEUP_TYPE);
                }
            }
            break;

        case QUEUE_WAKEUP_TYPE:
            /* DeQueue packet from the Wake up queue */
            status = DeQueue( _HiFWakeUpQueue, &pHIFPkt );

            if ( status == OSP_STATUS_OK )
            {
                /* Copy packet to given buffer */
                SH_MEMCPY( pBuf + *pLength, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

                /* update length of DeQueue packet */
                pktLen = pHIFPkt->Header.Length;
                *pLength += pktLen;

                /* Free Block from PacketPool */
                status = FreeBlock( SensorDataPacketPool, pHIFPkt );
                ASF_assert(status == OSP_STATUS_OK);
            }
            break;

        case QUEUE_CONTROL_RESPONSE_TYPE:
            /* DeQueue packet from Control Response queue */
            status = DeQueue( _HiFControlQueue, &pHIFPkt );

            if ( status == OSP_STATUS_OK )
            {
                /* Copy packet to given buffer */
                SH_MEMCPY( pBuf + *pLength, &(pHIFPkt->DataStart), pHIFPkt->Header.Length );

                /* update length of DeQueue packet */
                pktLen = pHIFPkt->Header.Length;
                *pLength += pktLen;

                /* Free Block from PacketPool */
                status = FreeBlock( SensorControlResponsePacketPool, pHIFPkt );
                ASF_assert(status == OSP_STATUS_OK);
            }
            break;

        default:
            return (OSP_STATUS_INVALID_PARAMETER);
        }
        bufSize -= pktLen;    /* update buffer space left after dequeue */
    } while ( bufSize >=  sizeof(HostIFPackets_t) );    /* loop until atleast one packet size space is left in buffer */

    return status;
}


/****************************************************************************************************
 * @fn      BatchManagerControlResponseEnQueue
 *          Enqueue control response packet in control response queue
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

    /* EnQueue Packet in Control response queue */
    status = EnQueue( _HiFControlQueue , pHifControlResponsePacket );

    /* Check queue is full */
    if (status == (OSP_STATUS_QUEUE_FULL))
    {
        status = BatchManagerQueueFlush( QUEUE_WAKEUP_TYPE );
    }

    return status;
}


/****************************************************************************************************
 * @fn      BatchManagerQueueFlush
 *          Flushes given queue
 *
 * @param   [IN] queue Type
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

    /* Start queue threshold call back */
    QHighThresholdCallBack( qType );

    return OSP_STATUS_OK;
}


/****************************************************************************************************
 * @fn      BatchManagerMaxQCount
 *          Provide Maximum event stored by NonWakup and Wakeup queues.
 *
 * @return  Max. Number of event handle by Batchmanager queue
 *
 ***************************************************************************************************/
uint32_t BatchManagerMaxQCount( void )
{
    /* Sum up queue capacity for both Wakeup and NonWakeup queue */
    return( BatchDesc.BatchQ[BATCH_WAKEUP_QUEUE].pQ->Capacity +
            BatchDesc.BatchQ[BATCH_NONWAKEUP_QUEUE].pQ->Capacity );
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/

