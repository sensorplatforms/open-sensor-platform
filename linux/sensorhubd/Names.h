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
#ifndef _NAMES_H_
#define _NAMES_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <string>
#include "osp_datatypes.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E / C L A S S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
class Names {
public:
    typedef  const char* NameString;

    // Linux Input Manager Input Event Types
    static constexpr NameString INPUT_EVENT_TIMESTAMP = "EVENT_TIMESTAMP";
    static constexpr NameString INPUT_EVENT_EMBEDDED_32_BIT_TIMESTAMP = "EMBEDDED_32_BIT_TIMESTAMP";
    static constexpr NameString INPUT_EVENT_EMBEDDED_64_BIT_TIMESTAMP = "EMBEDDED_64_BIT_TIMESTAMP";


    static constexpr NameString SYSTEM_EVENT = "SYSTEM_EVENT";
    static constexpr NameString RAW_ACCELEROMETER = "RAW_ACCELEROMETER";
    static constexpr NameString RAW_MAGNETOMETER = "RAW_MAGNETOMETER";
    static constexpr NameString RAW_GYROSCOPE = "RAW_GYROSCOPE";

};

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

#endif // _NAMES_H_
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
