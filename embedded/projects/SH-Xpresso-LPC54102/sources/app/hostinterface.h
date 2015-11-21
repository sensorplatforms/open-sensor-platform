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
#if !defined (HOSTINTERFACE_H)
#define   HOSTINTERFACE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* Register addresses */
#define OSP_WHOAMI          0x00     /* RO */
#define OSP_VERSION0        0x01     /* RO */
#define OSP_VERSION1        0x02     /* RO */
#define OSP_INT_REASON      0x03     /* RO [-------1] */
#define OSP_INT_NONE        0x00
#define OSP_INT_DRDY        0x01
#define OSP_INT_OVER        0x02
#define OSP_DATA_LEN        0x04
#define OSP_CONFIG          0x05     /* RW [-------1] */
#define OSP_DATA_LEN_L      0x06     /* RO */
#define OSP_DATA_LEN_H      0x07
#define OSP_INT_LEN         0x08    /* 16 bit value of INT and LEN */
/* These registers are mainly for debugging. */
#define OSP_ENABLE0         0x10        /* RW, shadow enable */
#define OSP_ENABLE1         0x11        /* RW, shadow enable */
#define OSP_RESULT          0x40        /* Shadow data */
#define OSP_RESULT_END      0x7f

#define OSP_DATA_OUT        0x80
#define OSP_DATA_OUT_END    0xbf
#define OSP_DATA_IN         0x0c
#define OSP_DATA_IN_END     0xff

/*
 * General protocol notes:
 *    1. Reads of OSP_INT_REASON has side effects. Host should
 * cache values and NOT read OSP_INT_REASON multiple times unless
 * needed.
 *    2. A read of OSP_INT_REASON will cause the rest of the
 * registers to be updated with a new set of results. 
 *    3. Writes to OSP_DATA_IN should be done in one burst. 
 * OSP will process the OSP_DATA_IN when the burst is done.
 *    4. Interrupts are LEVEL triggered.
 *    5. Expected host protocol is:
 *        a. Wait for interrupt.
 *        b. When an interrupt happens, read OSP_INT_REASON.
 *        c. If OSP_INT_REASON has a valid reason for interrupt,
 *           process it. Otherwise, goto (a).
 *        d. Do not read OSP_INT_REASON again. Rest of the
 *           registers are held static til the next read of 
 *           OSP_INT_REASON.
 *        e. Upon completion, read OSP_INT_REASON again. Goto (c).
 *    6. Interrupt will remain asserted as long as data is pending.
 *    7. Interrupt may deassert as soon as OSP_INT_REASON is done.
 *    8. Upon driver start, host should write a 1 to OSP_CONFIG.0.
 * This is to clear any pending data.
 * 
 * Alternative non interrupt based processing:
 *     1. Host can periodically poll the OSP_INT_REASON register.
 * Polling should be at least as frequent as data is expected to be
 * generated. It need not be regular but should not be delayed
 * longer then there is FIFO space.
 *    2. If OSP_INT_REASON shows a pending event, process
 * as in the interrupt case.
 *
 */

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Register Area for slave device */
typedef struct SH_RegArea_tag
{
    uint8_t whoami;
    uint8_t version0;
    uint8_t version1;
    uint8_t irq_cause;
    uint8_t read_len;
    uint8_t read_len_h;
    uint16_t read_len2;
    uint32_t intlen;
    uint8_t ack;
    uint8_t rd_req;
    uint8_t rd_req_len;
    uint8_t sensor_ctrl;
    uint8_t rd_mem[4096];
    uint8_t wr_mem[32];
    uint8_t endMarker;
} SH_RegArea_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* HOSTINTERFACE_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
