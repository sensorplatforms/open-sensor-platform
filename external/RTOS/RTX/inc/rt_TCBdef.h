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
#if !defined (RT_TCBDEF_H)
#define   RT_TCBDEF_H

#ifdef ASF_PROFILING
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* IMPORTANT: Make sure the offset defines match with the structure layout */
#define TCB_RETVAL      32        /* 'ret_val' offset                        */
#define TCB_RETUPD      33        /* 'ret_upd' offset                        */
#define TCB_TSTACK      36        /* 'tsk_stack' offset                      */


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef void               (*FUNCP)(void);

/* IMPORTANT: if adding members to this structure - update OS_TCB_SIZE define in RTX_lib.c */
typedef struct OS_TCB {
   /* General part: identical for all implementations.                       */
   U8     cb_type;                /* Control Block Type                      */
   U8     state;                  /* Task state                              */
   U8     prio;                   /* Execution priority                      */
   U8     task_id;                /* Task ID value for optimized TCB access  */
   struct OS_TCB *p_lnk;          /* Link pointer for ready/sem. wait list   */
   struct OS_TCB *p_rlnk;         /* Link pointer for sem./mbx lst backwards */
   struct OS_TCB *p_dlnk;         /* Link pointer for delay list             */
   struct OS_TCB *p_blnk;         /* Link pointer for delay list backwards   */
   U16    delta_time;             /* Time until time out                     */
   U16    interval_time;          /* Time interval for periodic waits        */
   U16    events;                 /* Event flags                             */
   U16    waits;                  /* Wait flags                              */
   void   **msg;                  /* Direct message passing when task waits  */
   U8     ret_val;                /* Return value upon completion of a wait  */

   /* Hardware dependant part: specific for CM processor                     */
   U8     ret_upd;                /* Updated return value                    */
   U16    priv_stack;             /* Private stack size, 0= system assigned  */
   U32    tsk_stack;              /* Current task Stack pointer (R13)        */
   U32    *stack;                 /* Pointer to Task Stack memory block      */

   /* Task entry point used for uVision debugger                             */
   FUNCP  ptask;                  /* Task entry address                      */

   /* >RKV< Profiling data */
   U32    currentRunStartTime;    /* Start time (RTC Tick) capture at the start     */
   U32    cumulativeRunTime;      /* Cumulative time in RTC ticks                   */
   U32    runCount;               /* Number of times this task was scheduled to run */
   /* IMPORTANT: if adding members to the structure - update OS_TCB_SIZE define in RTX_lib.c */

} OS_TCB_t, *P_TCB;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
#endif

#endif /* RT_TCBDEF_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
