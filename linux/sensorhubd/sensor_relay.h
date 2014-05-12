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
#ifndef _SENSOR_RELAY_H_
#define _SENSOR_RELAY_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#ifdef __KERNEL__
# include <linux/device.h>
#endif
#include "osp-sensors.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SENSOR_RELAY_NUM_RELAY_BUFFERS  300
#define SENSOR_RELAY_BUFF_SIZE          50

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
struct  sensor_relay_motion_sensor_broadcast_node {
    uint8_t sensorId;   /* Holds Sensor type enumeration - MUST be 1st */
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int16_t Data[3];    /* Raw sensor data for 3axis */
};


struct  sensor_relay_quaternion_sensor_broadcast_node {
    uint8_t sensorId;   /* Holds Sensor type enumeration - MUST be 1st */
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int32_t Data[4];    /* Quaternion data */
};
struct  sensor_relay_steps_broadcast_node {
    uint8_t sensorId;   /* Holds Sensor type enumeration - MUST be 1st */
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int16_t Data[1];    /* step count */
};

struct sensor_relay_motion_change_detector_broadcast_node {
    uint8_t sensorId;   /* Holds Sensor type enumeration - MUST be 1st */
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    uint32_t duration;  /* raw time stamp in sensor time capture ticks */
    uint8_t type;       /* change detector segment type */
};


union  sensor_relay_broadcast_node {
    struct sensor_relay_motion_sensor_broadcast_node sensorData;
    struct sensor_relay_motion_change_detector_broadcast_node changeDetectorData;
    struct sensor_relay_steps_broadcast_node stepsdata;
    struct sensor_relay_quaternion_sensor_broadcast_node quaternionData;
};

#ifdef __KERNEL__

struct sensor_relay_classdev {
    char *sensor_name;
    SensorType_t sensorId;
    int flags;
    bool enable;
    unsigned int delay;

    /*
     * Lower 16 bits reflect status
     */
#      define SENSOR_SUSPENDED      (1 << 0)
    /*
     * Upper 16 bits reflect control information
     */
#      define SENSOR_CORE_SUSPENDRESUME (1 << 16)

    /*
     * Set Sensor Enable
     */
    /*
     * Must not sleep, use a workqueue if needed
     */
    int (*enable_set) (
            struct sensor_relay_classdev * sensor_relay_cdev,
            bool enable);
    /*
     * Get Sensor Enable
     */
    int (*enable_get) (
            struct sensor_relay_classdev * sensor_relay_cdev,
            bool * enable);

    /*
     * Set Sensor Enable
     */
    /*
     * Must not sleep, use a workqueue if needed
     */
    int (*delay_set) (
            struct sensor_relay_classdev * sensor_relay_cdev,
            unsigned int delay);
    /*
     * Get Sensor Enable
     */
    int (*delay_get) (
            struct sensor_relay_classdev * sensor_relay_cdev,
            unsigned int *delay);


    struct device *dev;
    struct list_head node;
};
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
#ifdef __KERNEL__
int sensor_relay_device_register(
        struct device *parent,
        struct sensor_relay_classdev *sensor_relay_cdev);

void sensor_relay_device_unregister(
        struct sensor_relay_classdev *sensor_relay_cdev);

int sensor_relay_write(
        struct sensor_relay_classdev *sensor_relay_cdev,
        union sensor_relay_broadcast_node *node);

int sensor_relay_wakeup(
        struct sensor_relay_classdev *sensor_relay_cdev);

#endif

#endif /* _SENSOR_RELAY_H_ */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
