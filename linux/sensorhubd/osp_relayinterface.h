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
#if !defined (_RELAY_INTERFACE_H_)
#define   _RELAY_INTERFACE_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <string>
#include "osp-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum {
    ACCEL_INDEX                     = 0,
    MAG_INDEX                       = 1,
    GYRO_INDEX                      = 2,
    //STEP_COUNTER_INDEX              = 3,
    //SIG_MOTION_INDEX                = 4,
    MAX_NUM_SENSORS_TO_HANDLE
} deviceIndex;

typedef struct {
    std::string uinputName;
    std::string sysDelayPath;  //Sysfs path for setting polling interval
    std::string sysEnablePath; //Sysfs path for enable
    int32_t     enableValue;   //Value that enables the device (typically 1)*
    int32_t     disableValue;  //Value that disables the device (typically 0)
    int32_t     fd;
    int32_t     repubFd;
    osp_float_t  conversion[3];
    int         swap[3];
} DeviceConfig_t;

/* per-cpu buffer info */
typedef struct
{
    size_t produced;
    size_t consumed;
    size_t max_backlog; /* max # sub-buffers ready at one time */
} RelayBufStatus_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* _RELAY_INTERFACE_H_ */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
