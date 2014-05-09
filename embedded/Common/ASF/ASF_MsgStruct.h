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
#if !defined (ASF_MSGSTRUCT_H)
#define   ASF_MSGSTRUCT_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "asf_types.h"
#include "AppMsgStruct.h"
#include "ASF_TQEnum.h"

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#if defined (ASF_MSG_DEF)
# undef ASF_MSG_DEF
#endif

#define ASF_MSG_DEF(MsgID, Union)   Union;


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* ASF common message structures */
typedef struct MsgGenericTag
{
    uint32_t dword;
    uint16_t word;
    uint8_t  byte;
} MsgGeneric;

typedef struct MsgNoDataTag
{
    uint32_t dummy; //Can't really be empty!
} MsgNoData;

typedef struct MsgTimerExpiryTag
{
    uint16_t userValue;
    TimerId  timerId;
} MsgTimerExpiry;


union Message
{
    #include "ASF_MsgDef.h"
};


#define M_GetMaxMessageLength()     (sizeof(union Message) + ASF_CHECK_VALUES_SIZE)


/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#if defined (ASF_MSG_DEF)
#   undef ASF_MSG_DEF
#endif

#define ASF_MSG_DEF(MsgID, Union)   MsgID,


/** Defines the size of header in the Message Block structure. Header size includes the message id.
 offsetof takes care of structure packing also.
 */
#define M_GetMsgHeaderLength()  (offsetof(MessageBlock, rec.msg))

/**
 * @def M_GetMsgBlockFromBuffer
 *
 * BlockPtr :- A MessageBlock pointer (destination)
 * MsgPtr   :- A MessageBuffer pointer (source)
 *
 * Derives the MessageBlock from a MessageBuffer.
 */
#define M_GetMsgBlockFromBuffer(BlockPtr, MsgPtr)   \
            (BlockPtr) = (MessageBlock *) ((uint8_t *) \
            &((MsgPtr)->msgId) - (offsetof(MessageBlock, rec.msgId)))

/**
 * @def M_GetMsgRecFromBuffer
 * RecPtr :- A MessageRec pointer (destination)
 * MsgPtr :- A MessageBuffer pointer (source)
 *
 * Derives the MessageRec from a given Message Buffer.
 */
#define M_GetMsgRecFromBuffer(RecPtr, MsgPtr)   \
    (RecPtr) = (MessageRec *)((uint8_t*)&((MsgPtr)->msgId) - (offsetof(MessageRec, msg)))



#define ASFCreateMessage(id, sz, bufp)      \
    _ASFCreateMessage(id, sz, bufp, __MODULE__, __LINE__)

#define ASFSendMessage( id, pm )   \
    _ASFSendMessage( id, pm, __MODULE__, __LINE__ )

#define ASFReceiveMessage( id, pm )    \
    _ASFReceiveMessage( id, pm, __MODULE__, __LINE__ )

#define ASFReceiveMessagePoll( id, pm )    \
    _ASFReceiveMessagePoll( id, pm, __MODULE__, __LINE__ )

#define ASFDeleteMessage( pm )  \
    _ASFDeleteMessage( pm, __MODULE__, __LINE__ )


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef enum AsfResultCodesTag
{
    ASF_OK                  = 0,
    ASF_ERR_Q_FULL          = 1,
    ASF_ERR_MSG_BUFF        = 2,
    ASF_ERR_TIMER_IN_USE    = 3,
    ASF_ERR_TIMEOUT         = 4,
} AsfResult_t;

/* Message Buffer Definitions */
typedef enum MessageIdTag
{
    #include "ASF_MsgDef.h"

    NUM_MSG_ID          ///< Total messages

} MessageId;

typedef struct MsgHeaderTag
{
    TaskId      destTask;   ///< Destination task identifier
    uint16_t    length;     ///< Length of the message structure

} MsgHeader;

typedef struct MessageRecTag
{
    MessageId   msgId;      ///< Enumerated identifier for the message (declared by ASF_MSG_DEF)
    union Message msg;      ///< This encapsulates all message types

} MessageRec;

typedef MessageRec   MessageBuffer;

/** The message block is allocated for creating a message type */
typedef struct MessageBlockTag
{
    MsgHeader   header;     ///< See MsgHeader definition
    MessageRec  rec;        ///< See MessageRec definition

} MessageBlock;


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
void ASFMessagingInit( void );
AsfResult_t _ASFCreateMessage( MessageId msgId, uint16_t msgSize, MessageBuffer **pMbuf, char *_file, int _line );
AsfResult_t _ASFSendMessage ( TaskId destTask, MessageBuffer *pMbuf, char *_file, int _line );
void _ASFReceiveMessage ( TaskId rcvTask, MessageBuffer **pMbuf, char *_file, int _line );
void _ASFDeleteMessage ( MessageBuffer **pMbuf, char *_file, int _line );
osp_bool_t _ASFReceiveMessagePoll ( TaskId rcvTask, MessageBuffer **pMbuf, char *_file, int _line );


#endif /* ASF_MSGSTRUCT_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
