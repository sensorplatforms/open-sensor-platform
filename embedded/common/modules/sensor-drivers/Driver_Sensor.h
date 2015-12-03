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
#ifndef __DRIVER_SENSOR_H
#define __DRIVER_SENSOR_H

#include "Driver_Common.h"

#define OSP_SENSOR_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,00)

typedef void (*OSP_Sensor_SignalEvent_t) (uint32_t sensor_flags);

typedef struct _OSP_SENSOR_DETAILS {
  const char* name;                     ///<
  const char* part_num;                 ///<
  uint32_t    sensor_type;               ///< conforms to Android IDs
  uint32_t    unit_scaling_8Q24;         ///< fixed point factor to convert raw data to physical units
  uint32_t    max_range_12Q20;            ///<
  uint16_t    num_data_elems;            ///<
  uint16_t    supported_flags;           ///< use with SENSOR_FLAG_*
} OSP_SENSOR_DETAILS;


#define SENSOR_FLAG_CONTINUOUS_DATA         (       0)  ///<
#define SENSOR_FLAG_WAKE_UP                 (1U <<  0)  ///<
#define SENSOR_FLAG_ON_CHANGE_MODE          (1U <<  1)  ///<
#define SENSOR_FLAG_ONE_SHOT_MODE           (1U <<  2)  ///<
#define SENSOR_FLAG_MOTION_WAKEUP           (1U <<  3)  ///<
#define SENSOR_FLAG_DATA_INJECTION          (1U <<  4)  ///<


typedef struct _OSP_DRIVER_SENSOR {
  ARM_DRIVER_VERSION  (*GetVersion)    (void);
  int32_t             (*Initialize)    (OSP_Sensor_SignalEvent_t cb, void *data);
  int32_t             (*Uninitialize)  (void);
  int32_t             (*PowerControl)  (ARM_POWER_STATE state);
  int32_t             (*Activate)      (uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency);
  int32_t             (*ReadData)      (void *data, uint16_t num_samples);
  int32_t             (*Deactivate)    (void);
  int32_t             (*GetDetails)    (void *data);
  int32_t             (*InjectData)    (void *data);
} const OSP_DRIVER_SENSOR;

#define OSP_BUILD_DRIVER_SENSOR(pre1, pre2, priv) \
  static ARM_DRIVER_VERSION pre1##_GetVersion (void) { \
    return pre2##_GetVersion (priv); \
  } \
  static int32_t pre1##_Initialize (OSP_Sensor_SignalEvent_t cb, void *data) { \
    return pre2##_Initialize (cb, data, priv); \
  } \
  static int32_t pre1##_Uninitialize (void) { \
    return pre2##_Uninitialize (priv); \
  } \
  static int32_t pre1##_PowerControl (ARM_POWER_STATE state) { \
    return pre2##_PowerControl (state, priv); \
  } \
  static int32_t pre1##_Activate (uint16_t flags, uint32_t us_delay, uint16_t fifo_num_samples, uint32_t max_report_latency) { \
    return pre2##_Activate (flags, us_delay, fifo_num_samples, max_report_latency, priv); \
  } \
  static int32_t pre1##_ReadData (void *data, uint16_t num_samples) { \
    return pre2##_ReadData (data, num_samples, priv); \
  } \
  static int32_t pre1##_Deactivate (void) { \
    return pre2##_Deactivate (priv); \
  } \
  static int32_t pre1##_GetDetails (void *data) { \
    return pre2##_GetDetails (data, priv); \
  } \
  static int32_t pre1##_InjectData (void *data) { \
    return pre2##_InjectData (data, priv); \
  } \
  OSP_DRIVER_SENSOR Driver_##pre1 = { \
    pre1##_GetVersion, \
    pre1##_Initialize, \
    pre1##_Uninitialize, \
    pre1##_PowerControl, \
    pre1##_Activate, \
    pre1##_ReadData, \
    pre1##_Deactivate, \
    pre1##_GetDetails, \
    pre1##_InjectData, \
  };

#endif /* __DRIVER_SENSOR_H */
