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
#if !defined (BATCH_MANAGER_H)
#define   BATCH_MANAGER_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Queue.h"
#include "SensorPackets.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Enum for different  */
typedef enum _BatchSensorFIFOType
{
    BATCH_SENSOR_WAKEUP_FIFO,
    BATCH_SENSOR_NONWAKEUP_FIFO,
    BATCH_SENSOR_NONWAKEUP_ONCHANGE_FIFO,
    BATCH_SENSOR_NUM,
} BatchSensorFIFOType_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
int16_t BatchManagerInitialize(void);
int16_t BatchManagerSensorRegister( ASensorType_t SensorType, uint64_t SamplingRate, uint64_t ReportLatency );
int16_t BatchManagerSensorEnable( ASensorType_t SensorType );
int16_t BatchManagerSensorDeRegister( ASensorType_t SensorType );
int16_t BatchManagerSensorDisable( ASensorType_t SensorType );
int16_t BatchManagerGetSensorState( ASensorType_t sensorType, int32_t * state);
int16_t BatchManagerSensorDataEnQueue( HostIFPackets_t *pTodoPacket, uint16_t packetSize, uint32_t sensorType );
int16_t BatchManagerDeQueue( uint8_t *pBuf, uint32_t *pLength );
int16_t BatchManagerControlResponseEnQueue( HostIFPackets_t *pHiFControlPacket, uint16_t packetSize );
int16_t BatchManagerQueueFlush( Q_Type_t qType);
uint32_t BatchManagerMaxQCount( void );;


#endif /* BATCH_MANAGER_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
