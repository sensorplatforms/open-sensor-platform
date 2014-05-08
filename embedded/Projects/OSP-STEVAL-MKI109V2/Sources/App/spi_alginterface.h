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
#if !defined (SPI_ALG_INTERFACE_H)
#define   SPI_ALG_INTERFACE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-types.h"
#include "osp-fixedpoint-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SPI_MAX_INT                     (2147483647LL)
#define SPI_MIN_INT                     (-2147483648LL)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Common bridge between the different data types for base sensors (Accel/Mag/Gyro) */
typedef struct  {
    NTTIME TimeStamp;               // time stamp
    uint8_t accuracy;
    union {
        NTEXTENDED  extendedData[3];             // processed sensor data
        NTPRECISE   preciseData[3];             // processed sensor data
    } data;
} Common_3AxisResult_t;

typedef struct  {
    NTTIME TimeStamp;           // time stamp
    uint8_t accuracy;
    union {     // processed sensor data
        NTEXTENDED  extendedData[3];
        NTPRECISE   preciseData[3];
    } data;
    union {     // Sensor bias value
        NTEXTENDED  extendedData[3];
        NTPRECISE   preciseData[3];
    } bias;
} SPI_Uncal3AxisResult_t;


/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
__inline static uint32_t mult_uint16_uint16(uint16_t a, uint16_t b)
{
    return ((uint32_t) a * (uint32_t)b);
}


#endif /* SPI_ALG_INTERFACE_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
