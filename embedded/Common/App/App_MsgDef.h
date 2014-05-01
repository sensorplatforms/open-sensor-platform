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
/* This file is only included in ASF_MsgDef and contains application specific message defines */

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

/* Add application message definitions here */

ASF_MSG_DEF( MSG_ACC_DATA,          MsgAccelData        msgAccelData        )
ASF_MSG_DEF( MSG_MAG_DATA,          MsgMagData          msgMagData          )
ASF_MSG_DEF( MSG_GYRO_DATA,         MsgGyroData         msgGyroData         )
ASF_MSG_DEF( MSG_QUATERNION_DATA,   MsgQuaternionData   msgQuaternionData   )
ASF_MSG_DEF( MSG_CAL_EVT_NOTIFY,    MsgGeneric          msgCalEvtNotify     )
ASF_MSG_DEF( MSG_SENSOR_DATA_RDY,   MsgSensorDataRdy    msgSensorDataRdy    )
ASF_MSG_DEF( MSG_CD_SEGMENT_DATA,   MsgCDSegmentData    msgCDSegmentData    )
ASF_MSG_DEF( MSG_HOST_READY,        MsgGeneric          msgHostReady        )
ASF_MSG_DEF( MSG_TRIG_ALG_BG,       MsgNoData           msgTrigAlgBg        )

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
