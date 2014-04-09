/*
    sensor_relay.h - Linux kernel module for publishing sensor measuremnents using RELAY

    This file declares helper functions for the sysfs class "sensor_relay",
    for use by sensors drivers.

    Copyright (C) 2013 <sungerfeld@sensorplatforms.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
*/

#ifndef _SENSOR_RELAY_H_
#   define _SENSOR_RELAY_H_

#   ifdef __KERNEL__
#      include <linux/device.h>
#   endif

#   define SENSOR_RELAY_NUM_RELAY_BUFFERS 300
#   define SENSOR_RELAY_BUFF_SIZE 50

enum SENSOR_RELAY_SENSOR_ID {
    SENSOR_RELAY_SENSOR_ID_FIRST = 0,

    SENSOR_RELAY_SENSOR_ID_ACCELEROMETER = SENSOR_RELAY_SENSOR_ID_FIRST,
    SENSOR_RELAY_SENSOR_ID_MAGNETOMETER,
    SENSOR_RELAY_SENSOR_ID_GYROSCOPE,
    SENSOR_RELAY_SENSOR_ID_PRESSURE,
    SENSOR_RELAY_SENSOR_ID_LIGHT,
    SENSOR_RELAY_SENSOR_ID_PROXIMITY,
    SENSOR_RELAY_SENSOR_ID_TEMPERATURE,
    SENSOR_RELAY_SENSOR_ID_CHANGE_DETECTOR,
    SENSOR_RELAY_SENSOR_ID_SENSOR_CAMERA_ATTITUDE,
    SENSOR_RELAY_SENSOR_ID_CAMERA_RELATIVE_ATTITUDE,
    SENSOR_RELAY_SENSOR_ID_CAMERA_RELATIVE_POSITION,
    SENSOR_RELAY_SENSOR_ID_STEPS,
    SENSOR_RELAY_SENSOR_ID_QUATERNION,

    SENSOR_RELAY_SENSOR_ID_COUNT    /* may not exceed SENSOR_RELAY_SENSOR_ID_TYPE_MASK */
};


struct  sensor_relay_motion_sensor_broadcast_node {
    uint8_t sensorId;   /* enum SENSOR_RELAY_SENSOR_ID  - MUST be 1st*/
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int16_t Data[3];    /* Raw sensor data for 3axis */
};


struct  sensor_relay_quaternion_sensor_broadcast_node {
    uint8_t sensorId;    /* enum SENSOR_RELAY_SENSOR_ID  - MUST be 1st*/
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int32_t Data[4];    /* Quaternion data */
};
struct  sensor_relay_steps_broadcast_node {
    uint8_t sensorId;   /* enum SENSOR_RELAY_SENSOR_ID  - MUST be 1st*/
    uint64_t TimeStamp; /* raw time stamp in sensor time capture ticks */

    int16_t Data[1];    /* step count */
};

struct sensor_relay_motion_change_detector_broadcast_node {
    uint8_t sensorId;   /* enum SENSOR_RELAY_SENSOR_ID  - MUST be 1st*/
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

#   ifdef __KERNEL__

struct sensor_relay_classdev {
    char *sensor_name;
    enum SENSOR_RELAY_SENSOR_ID sensorId;
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
    int (
    *enable_set) (
        struct sensor_relay_classdev * sensor_relay_cdev,
        bool enable);
    /*
     * Get Sensor Enable
     */
    int (
    *enable_get) (
        struct sensor_relay_classdev * sensor_relay_cdev,
        bool * enable);

    /*
     * Set Sensor Enable
     */
    /*
     * Must not sleep, use a workqueue if needed
     */
    int (
    *delay_set) (
        struct sensor_relay_classdev * sensor_relay_cdev,
        unsigned int delay);
    /*
     * Get Sensor Enable
     */
    int (
    *delay_get) (
        struct sensor_relay_classdev * sensor_relay_cdev,
        unsigned int *delay);


    struct device *dev;
    struct list_head node;
};


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
#   endif

#endif

