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
    SENSOR_ACCELEROMETER                   =  0, //!< accelerometer data
    SENSOR_MAGNETIC_FIELD                  =  1, //!< magnetometer data
    SENSOR_GYROSCOPE                       =  2, //!< gyroscope data
    SENSOR_LIGHT                           =  3, //!< light data
    SENSOR_PRESSURE                        =  4, //!< barometer pressure data
    SENSOR_PROXIMITY                       =  5, //!< proximity data
    SENSOR_RELATIVE_HUMIDITY               =  6, //!< relative humidity data
    SENSOR_AMBIENT_TEMPERATURE             =  7, //!< ambient temperature data
    SENSOR_GRAVITY                         =  8, //!< gravity part of acceleration in body frame 
    SENSOR_LINEAR_ACCELERATION             =  9, //!< dynamic acceleration 
    SENSOR_ORIENTATION                     = 10, //!< yaw, pitch, roll (also use this for Win8 Inclinometer)
    SENSOR_AUG_REALITY_COMPASS             = 11, //!< heading which switches to aug-reality mode when camera towards horizon (Win8 compass)
    SENSOR_ROTATION_VECTOR                 = 12, //!< accel+mag+gyro quaternion
    SENSOR_GEOMAGNETIC_ROTATION_VECTOR     = 13, //!< accel+mag quaternion
    SENSOR_GAME_ROTATION_VECTOR            = 14, //!< accel+gyro quaternion
    SENSOR_VIRTUAL_GYROSCOPE               = 15, //!< virtual gyroscope data from accel+mag
    SENSOR_STEP_DETECTOR                   = 16, //!< precise time a step occured
    SENSOR_STEP_COUNTER                    = 17, //!< count of steps
    SENSOR_CONTEXT_DEVICE_MOTION           = 18, //!< context of device relative to world frame
    SENSOR_CONTEXT_CARRY                   = 19, //!< context of device relative to user
    SENSOR_CONTEXT_POSTURE                 = 20, //!< context of user relative to world frame
    SENSOR_CONTEXT_TRANSPORT               = 21, //!< context of environment relative to world frame
    SENSOR_CONTEXT_CHANGE_DETECTOR         = 22, //!< low compute trigger for seeing if context may have changed
    SENSOR_STEP_SEGMENT_DETECTOR           = 23, //!< low compute trigger for analyzing if step may have occured
    SENSOR_GESTURE_EVENT                   = 24, //!< gesture event such as a double-tap or shake
    SENSOR_MESSAGE                         = 25, //!< warnings from the library: e.g. excessive timestamp jitter, need calibration
    SENSOR_RGB_LIGHT                       = 26, //!< RGB light data
    SENSOR_UV_LIGHT                        = 27, //!< UV light data
    SENSOR_HEART_RATE                      = 28, //!< heart-rate data
    SENSOR_BLOOD_OXYGEN_LEVEL              = 29, //!< blood-oxygen level data
    SENSOR_SKIN_HYDRATION_LEVEL            = 30, //!< skin-hydration level data
    SENSOR_BREATHALYZER                    = 31, //!< breathalyzer data
    SENSOR_ENUM_COUNT
} SensorType_t ;


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
