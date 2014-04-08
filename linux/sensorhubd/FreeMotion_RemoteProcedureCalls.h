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
#ifndef FREEMOTION_RPC_H
#define FREEMOTION_RPC_H

#include <stdint.h>

//// Sensor Types from Android
#define SENSOR_TYPE_META_DATA                        (0)
#define SENSOR_TYPE_ACCELEROMETER                    (1)
#define SENSOR_TYPE_GEOMAGNETIC_FIELD                (2)
#define SENSOR_TYPE_MAGNETIC_FIELD  SENSOR_TYPE_GEOMAGNETIC_FIELD
#define SENSOR_TYPE_ORIENTATION                      (3)
#define SENSOR_TYPE_GYROSCOPE                        (4)
#define SENSOR_TYPE_LIGHT                            (5)
#define SENSOR_TYPE_PRESSURE                         (6)
#define SENSOR_TYPE_TEMPERATURE                      (7)
#define SENSOR_TYPE_PROXIMITY                        (8)
#define SENSOR_TYPE_GRAVITY                          (9)
#define SENSOR_TYPE_LINEAR_ACCELERATION             (10)
#define SENSOR_TYPE_ROTATION_VECTOR                 (11)
#define SENSOR_TYPE_RELATIVE_HUMIDITY               (12)
#define SENSOR_TYPE_AMBIENT_TEMPERATURE             (13)
#define SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED     (14)
#define SENSOR_TYPE_GAME_ROTATION_VECTOR            (15)
#define SENSOR_TYPE_GYROSCOPE_UNCALIBRATED          (16)
#define SENSOR_TYPE_SIGNIFICANT_MOTION              (17)
#define SENSOR_TYPE_STEP_DETECTOR                   (18)
#define SENSOR_TYPE_STEP_COUNTER                    (19)
#define SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR     (20)
#define COUNT_OF_SENSOR_TYPES                       (SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR+1)

////
typedef struct {
    double timestamp;
    float data[3];
} FMRPC_ThreeAxisData_t;

typedef int OSP_STATUS_t;
#define OSP_STATUS_OK       (  0)
#define OSP_STATUS_ERROR    ( -1)

typedef void (*FMRPC_ResultDataCallback_t)(uint32_t sensorType, void* data);


OSP_STATUS_t FMRPC_Initialize(void);

OSP_STATUS_t FMRPC_GetVersion(char* versionString, int bufSize);

OSP_STATUS_t FMRPC_SubscribeResult(uint32_t sensorType, FMRPC_ResultDataCallback_t dataReadyCallback );
OSP_STATUS_t FMRPC_UnsubscribeResult(uint32_t sensorType);


OSP_STATUS_t FMRPC_Deinitialize(void);


#endif // FREEMOTION_RPC_H
