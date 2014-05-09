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
#if !defined (HOSTINTERFACE_H)
#define   HOSTINTERFACE_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* Register addresses */
#define SH_REG_WHO_AM_I                 0x00
#define SH_REG_VERSION0                 0x01
#define SH_REG_VERSION1                 0x02
#define SH_REG_IRQ_CAUSE                0x03
#define SH_REG_RD_LEN                   0x04
#define SH_REG_ACK                      0x05
#define SH_REG_REQUEST                  0x06
#define SH_REG_REQ_LEN                  0x07
#define SH_REG_SENSOR_CTRL              0x08
#define SH_REG_RD_MEM                   0x40 /* upto 32 bytes read  */
#define SH_REG_WR_MEM                   0x60 /* Upto 32 bytes write */

#define SH_MSG_IRQCAUSE_NEWMSG          (1<<0)
#define SH_MSG_IRQCAUSE_OVERRUN         (1<<1)
#define SH_MSG_IRQCAUSE_MORE            (1<<2)

#define SH_MSG_REQ_ENABLE               0x60
#define SH_MSG_REQ_ENABLE_ACC           (1<<0)
#define SH_MSG_REQ_ENABLE_GYRO          (1<<1)
#define SH_MSG_REQ_ENABLE_MAG           (1<<2)
#define SH_MSG_REQ_ENABLE_QUAT          (1<<3)
#define SH_MSG_REQ_ENABLE_EULER         (1<<4)

#define SH_MSG_REQ_DISABLE              0x61
#define SH_MSG_REQ_SETRATE              0x62
#define SH_MSG_REQ_SUSPEND              0x63
#define SH_MSG_REQ_RESUME               0x64

#define SH_MSG_TYPE_ABS_ACCEL           0x10
#define SH_MSG_TYPE_ABS_GYRO            0x11
#define SH_MSG_TYPE_ABS_MAG             0x12
#define SH_MSG_TYPE_CD                  0x13
#define SH_MSG_TYPE_QUAT                0x14
#define SH_MSG_TYPE_EULER               0x15



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
    uint8_t ack;
    uint8_t rd_req;
    uint8_t rd_req_len;
    uint8_t sensor_ctrl;
    uint8_t reserv1[0x40-0x9];
    uint8_t rd_mem[32];
    uint8_t wr_mem[32];
    uint8_t endMarker;
} SH_RegArea_t;

#define spi_pack __attribute__ ((__packed__))

typedef struct spi_pack Timestamp40Tag {
    uint32_t timestamp32;
    uint8_t timestamp40;
} Timestamp40_t;

typedef struct spi_pack sh_motion_sensor_data {
    /*
     * raw time stamp in sensor time capture ticks
     */
    Timestamp40_t TimeStamp;
    int16_t Data[3];    /* x/y/z Raw sensor data */
} ShMotionSensor_t;

typedef struct spi_pack sh_segment_data {
    Timestamp40_t endTime;     /* in sensor ticks  */
    Timestamp40_t duration;    /* in sensor ticks  */
    uint8_t type;
} ShSegment_t;

typedef struct spi_pack sh_quaternion_data {
    /*
     * raw time stamp in sensor time capture ticks
     */
    Timestamp40_t TimeStamp;
    int32_t Data[4];	/* w/x/y/z Raw sensor data */
} ShQuaternion_t;

typedef struct spi_pack sh_sensor_data {
    uint8_t sensorId;	/* enum SPIHUB_DATA */
    union spi_pack {
        struct sh_motion_sensor_data  sensorData;
        struct sh_segment_data        changeDetectorData;
        struct sh_quaternion_data     quaternionData;
    } data;
} ShGenericSensor_t;

typedef struct spi_pack sh_command_sensor_header {
    uint8_t command;	/* enum SPI_SH_SENSOR_COMMANDS */
    uint8_t sensorId;	/* enum SPI_SH_SENSOR_ID */
} ShCmdHeader_t;


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
