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
/* This file maybe included more than once */

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/**
** @def ASF_MSG_DEF ( MsgId, MsgStructure MsgStructVar )
**      This macro declares a message type identified by MsgId with the
**      associated message structure MsgStructure.
**
** Parameters:
**      MsgId -    User defined C-Style identifier for the message.
**                 E.g. RTC_READ_REQ. This identifier will be used in the
**                 application to access the message and its associated
**                 contents.
**      MsgStructure - Type-defined structure name of the message structure.
**      MsgStructVar - A C-style variable of the type MsgStructure.
**                 The MsgStructure – MsgStructVar pair is used to define a
**                 message union of all message types used in the application.
**
**      Example:   ASF_MSG_DEF ( RTC_READ_REQ, RtcDateTime rtcDateTime )
**
*/
ASF_MSG_DEF( MSG_DUMMY_TEST,        MsgGeneric          msgDummy            )
ASF_MSG_DEF( MSG_TIMER_EXPIRY,      MsgTimerExpiry      msgTimerExpiry      )
ASF_MSG_DEF( MSG_PROFILING_REQ,     MsgNoData           msgProfilingReq     )

/* Now include application specific messages */
#include "App_MsgDef.h"

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
