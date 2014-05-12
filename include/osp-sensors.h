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
#if !defined (OSP_SENSORS_H)
#define   OSP_SENSORS_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
  /* This file is meant to provide a common definition of sensor related enumerations/defines and
   * generally should not depend on any other includes
   */
/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
//! use to specify the kind of sensor input or output 
/*! 
 * \sa OSP_RegisterInputSensor
 * \sa OSP_SubscribeOutputSensor
 *
 *  Final units of input/outputs are defined by the sensor data convention field of the sensor descriptor.
 *  Flags in the descriptor specify if sensor is calibrated/uncalibrated and/or used as input
 *  If a sensor type not is supported by the library implementation, an error will be returned on its usage
 */
typedef enum {
    SENSOR_MESSAGE                         =  0, //!< warnings from the library: e.g. excessive timestamp jitter, need calibration
    SENSOR_ACCELEROMETER                   =  1, //!< accelerometer data
    SENSOR_MAGNETIC_FIELD                  =  2, //!< magnetometer data
    SENSOR_GYROSCOPE                       =  3, //!< gyroscope data
    SENSOR_LIGHT                           =  4, //!< light data
    SENSOR_PRESSURE                        =  5, //!< barometer pressure data
    SENSOR_PROXIMITY                       =  6, //!< proximity data
    SENSOR_RELATIVE_HUMIDITY               =  7, //!< relative humidity data
    SENSOR_AMBIENT_TEMPERATURE             =  8, //!< ambient temperature data
    SENSOR_GRAVITY                         =  9, //!< gravity part of acceleration in body frame 
    SENSOR_LINEAR_ACCELERATION             = 10, //!< dynamic acceleration 
    SENSOR_ORIENTATION                     = 11, //!< yaw, pitch, roll (also use this for Win8 Inclinometer)
    SENSOR_AUG_REALITY_COMPASS             = 12, //!< heading which switches to aug-reality mode when camera towards horizon (Win8 compass)
    SENSOR_ROTATION_VECTOR                 = 13, //!< accel+mag+gyro quaternion
    SENSOR_GEOMAGNETIC_ROTATION_VECTOR     = 14, //!< accel+mag quaternion
    SENSOR_GAME_ROTATION_VECTOR            = 15, //!< accel+gyro quaternion
    SENSOR_VIRTUAL_GYROSCOPE               = 16, //!< virtual gyroscope data from accel+mag
    SENSOR_STEP_DETECTOR                   = 17, //!< precise time a step occured
    SENSOR_STEP_COUNTER                    = 18, //!< count of steps
    SENSOR_CONTEXT_DEVICE_MOTION           = 19, //!< context of device relative to world frame
    SENSOR_CONTEXT_CARRY                   = 20, //!< context of device relative to user
    SENSOR_CONTEXT_POSTURE                 = 21, //!< context of user relative to world frame
    SENSOR_CONTEXT_TRANSPORT               = 22, //!< context of environment relative to world frame
    SENSOR_CONTEXT_CHANGE_DETECTOR         = 23, //!< low compute trigger for seeing if context may have changed
    SENSOR_STEP_SEGMENT_DETECTOR           = 24, //!< low compute trigger for analyzing if step may have occured
    SENSOR_GESTURE_EVENT                   = 25, //!< gesture event such as a double-tap or shake
    SENSOR_RGB_LIGHT                       = 26, //!< RGB light data
    SENSOR_UV_LIGHT                        = 27, //!< UV light data
    SENSOR_HEART_RATE                      = 28, //!< heart-rate data
    SENSOR_ENUM_COUNT
} SensorType_t ;

//! Use these enums as indices into the probability vector of a ContextOutputData_t in a CONTEXT_DEVICE_MOTION result callback
typedef enum {
    CONTEXT_DEVICE_MOTION_STILL                 = 0,
    CONTEXT_DEVICE_MOTION_ACCELERATING          = 1,
    CONTEXT_DEVICE_MOTION_ROTATING              = 2,
    CONTEXT_DEIVCE_MOTION_TRANSLATING           = 3,
    CONTEXT_DEVICE_MOTION_FREE_FALLING          = 4,
    CONTEXT_DEVICE_MOTION_SIGNIFICANT_MOTION    = 5, //!< significant motion (as specified by Android HAL 1.0)
    CONTEXT_DEVICE_MOTION_SIGNIFICANT_STILLNESS = 6, //!< complement to significant motion
    CONTEXT_DEVICE_MOTION_ENUM_COUNT
} ContextMotionType_t;

//! Use these enums as indices into the probability vector of a ContextOutputData_t in a CONTEXT_POSTURE result callback
typedef enum {
    CONTEXT_POSTURE_WALKING     = 0,
    CONTEXT_POSTURE_STANDING    = 1,
    CONTEXT_POSTURE_SITTING     = 2,
    CONTEXT_POSTURE_JOGGING     = 3,
    CONTEXT_POSTURE_RUNNING     = 4,
    CONTEXT_POSTURE_ENUM_COUNT
} ContextPostureType_t;

//! Use these enums as indices into the probability vector of a ContextOutputData_t in a CONTEXT_CARRY result callback
typedef enum {
    CONTEXT_CARRY_IN_POCKET     = 0,
    CONTEXT_CARRY_IN_HAND       = 1,
    CONTEXT_CARRY_NOT_ON_PERSON = 2,
    CONTEXT_CARRY_IN_HAND_FRONT = 3,
    CONTEXT_CARRY_IN_HAND_SIDE  = 4,
    CONTEXT_CARRY_ENUM_COUNT
} ContextCarryType_t;

//! Use these enums as indices into the probability vector of a ContextOutputData_t in a CONTEXT_TRANSPORT result callback
typedef enum {
    CONTEXT_TRANSPORT_VEHICLE        = 0,
    CONTEXT_TRANSPORT_CAR            = 1,
    CONTEXT_TRANSPORT_TRAIN          = 2,
    CONTEXT_TRANSPORT_AIRPLANE       = 3,
    CONTEXT_TRANSPORT_UP_STAIRS      = 4,
    CONTEXT_TRANSPORT_DOWN_STAIRS    = 5,
    CONTEXT_TRANSPORT_UP_ELEVATOR    = 6,
    CONTEXT_TRANSPORT_DOWN_ELEVATOR  = 7,
    CONTEXT_TRANSPORT_UP_ESCALATOR   = 8,
    CONTEXT_TRANSPORT_DOWN_ESCALATOR = 9,
    CONTEXT_TRANSPORT_MOVING_WALKWAY = 10,
    CONTEXT_TRANSPORT_ON_BIKE        = 11,
    CONTEXT_TRANSPORT_ENUM_COUNT
} ContextTransportType_t;

//! Use these enums as indices into the probability vector of a ContextOutputData_t in a STEP result callback
typedef enum {
    CONTEXT_STEP = 0,              //!< only one kind of step now
    CONTEXT_STEP_ENUM_COUNT
} ContextStepType_t;

//! Use these enums as indices into the probability vector of a GestureEventOutputData_t in a GESTURE_EVENT result callback
typedef enum {
    GESTURE_TAP         = 0,
    GESTURE_DOUBLE_TAP  = 1,
    GESTURE_SHAKE       = 2,
    GESTURE_ENUM_COUNT
} GestureType_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* OSP_SENSORS_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
