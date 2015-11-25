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
#if !defined (OSP_SENSORS_H)
#define   OSP_SENSORS_H

/* This file is meant to provide a common definition of sensor related enumerations/defines and
 * generally should not depend on any other includes
 */

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <stdint.h>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define SENSOR_SUBTYPE_UNUSED               0   //!< Subtype is not used for the sensor type
#define SENSOR_SUBTYPE_START                1   //!< Subtype enumeration starts with 1
#define SENSOR_DEVICE_PRIVATE_BASE          0x10000 //!< Android defined private sensor type base
#define SENSOR_DEVICE_PRIVATE_MASK          0xFFFF  //!< Private sensor type mask

#define M_PSensorToAndroidBase(type)        ((type) | SENSOR_DEVICE_PRIVATE_BASE)
#define M_ToBaseSensorEnum(type)            ((type) & SENSOR_DEVICE_PRIVATE_MASK)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
//! use to specify the kind of sensor output result
/*!
 * \sa OSP_SubscribeSensorResult
 *
 *  Final units of input/outputs are defined by the sensor data convention field of the sensor descriptor.
 *  Flags in the descriptor specify if sensor is calibrated/uncalibrated and/or used as input
 *  If a sensor type not is supported by the library implementation, an error will be returned on its usage
 */
/* Android sensor types - this should match the defines/enumeration in Android's sensors.h */
/* Note that these are defined as SENSOR_ instead of SENSOR_TYPE_ to avoid clash with Android defines in
 * situations where this header in included for packet processing in Sensor HAL
 */
typedef enum _ASensorType {
    SENSOR_META_DATA                        = 0,
    SENSOR_ACCELEROMETER                    = 1,
    SENSOR_GEOMAGNETIC_FIELD                = 2,
    SENSOR_MAGNETIC_FIELD                   = SENSOR_GEOMAGNETIC_FIELD,
    SENSOR_ORIENTATION                      = 3,
    SENSOR_GYROSCOPE                        = 4,
    SENSOR_LIGHT                            = 5,
    SENSOR_PRESSURE                         = 6,
    SENSOR_TEMPERATURE                      = 7,
    SENSOR_PROXIMITY                        = 8,
    SENSOR_GRAVITY                          = 9,
    SENSOR_LINEAR_ACCELERATION              = 10,
    SENSOR_ROTATION_VECTOR                  = 11,
    SENSOR_RELATIVE_HUMIDITY                = 12,
    SENSOR_AMBIENT_TEMPERATURE              = 13,
    SENSOR_MAGNETIC_FIELD_UNCALIBRATED      = 14,
    SENSOR_GAME_ROTATION_VECTOR             = 15,
    SENSOR_GYROSCOPE_UNCALIBRATED           = 16,
    SENSOR_SIGNIFICANT_MOTION               = 17,
    SENSOR_STEP_DETECTOR                    = 18,
    SENSOR_STEP_COUNTER                     = 19,
    SENSOR_GEOMAGNETIC_ROTATION_VECTOR      = 20,
    SENSOR_HEART_RATE                       = 21,
    SENSOR_WAKE_UP_TILT_DETECTOR            = 22,
    SENSOR_TILT_DETECTOR                    = 23,
    SENSOR_WAKE_GESTURE                     = 24,
    SENSOR_GLANCE_GESTURE                   = 25,
    SENSOR_PICK_UP_GESTURE                  = 26,

    NUM_ANDROID_SENSOR_TYPE,                      //!< Total number of Android sensor type

    /* Private Sensor types (translates to Android SENSOR_TYPE_DEVICE_PRIVATE_BASE start) */

    PSENSOR_ENUM_FIRST_SENSOR                =  NUM_ANDROID_SENSOR_TYPE,

    PSENSOR_DEBUG_TUNNEL                     =  PSENSOR_ENUM_FIRST_SENSOR, //!< Debug message pipe to host
    PSENSOR_ACCELEROMETER_RAW,              //!< raw accelerometer data (direct from sensor)
    PSENSOR_MAGNETIC_FIELD_RAW ,            //!< magnetometer data (direct from sensor)
    PSENSOR_GYROSCOPE_RAW,                  //!< calibrated gyroscope data (direct from sensor)
    PSENSOR_LIGHT_UV,                       //!< UV light sensor data (Android Units)
    PSENSOR_LIGHT_RGB,                      //!< RGB light sensor data (Android Units)
    PSENSOR_STEP,                           //!< step data
    PSENSOR_ACCELEROMETER_UNCALIBRATED,     //!< uncalibrated accelerometer data (Android Units)
    PSENSOR_ORIENTATION,                    //!< yaw, pitch, roll (also use this for Win8 Inclinometer)
    PSENSOR_MOTION_SIGNIFICANT_STILLNESS,   //!< Significant stillness detected
    PSENSOR_CONTEXT_DEVICE_MOTION,          //!< context of device relative to world frame
    PSENSOR_CONTEXT_CARRY,                  //!< context of device relative to user
    PSENSOR_CONTEXT_POSTURE,                //!< context of user relative to world frame
    PSENSOR_CONTEXT_TRANSPORT,              //!< context of environment relative to world frame
    PSENSOR_CONTEXT_GESTURE_EVENT,          //!< gesture event such as a double-tap or shake
    PSENSOR_HEART_RATE,                     //!< heart-rate data
    PSENSOR_CONTEXT_SEGMENT_DETECTOR,       //!< Intermediate results for context calculations
    SYSTEM_REAL_TIME_CLOCK,                 //!< Real time clock used for time stamp
    PSENSOR_MAGNETIC_FIELD_ANOMALY,         //!< Indication for magnetic field anomaly detected

    NUM_SENSOR_TYPE,                        //!< Total number of sensor type
    NUM_PRIVATE_SENSOR_TYPE = ( NUM_SENSOR_TYPE - NUM_ANDROID_SENSOR_TYPE ),    //!< Total number of Private sensor type

    AP_PSENSOR_DEBUG_TUNNEL                 =  M_PSensorToAndroidBase(PSENSOR_DEBUG_TUNNEL),
    AP_PSENSOR_ACCELEROMETER_RAW            =  M_PSensorToAndroidBase(PSENSOR_ACCELEROMETER_RAW),
    AP_PSENSOR_MAGNETIC_FIELD_RAW           =  M_PSensorToAndroidBase(PSENSOR_MAGNETIC_FIELD_RAW),
    AP_PSENSOR_GYROSCOPE_RAW                =  M_PSensorToAndroidBase(PSENSOR_GYROSCOPE_RAW),
    AP_PSENSOR_LIGHT_UV                     =  M_PSensorToAndroidBase(PSENSOR_LIGHT_UV),
    AP_PSENSOR_LIGHT_RGB                    =  M_PSensorToAndroidBase(PSENSOR_LIGHT_RGB),
    AP_PSENSOR_STEP                         =  M_PSensorToAndroidBase(PSENSOR_STEP),
    AP_PSENSOR_ACCELEROMETER_UNCALIBRATED   =  M_PSensorToAndroidBase(PSENSOR_ACCELEROMETER_UNCALIBRATED),
    AP_PSENSOR_ORIENTATION                  =  M_PSensorToAndroidBase(PSENSOR_ORIENTATION),
    AP_PSENSOR_MOTION_SIGNIFICANT_STILLNESS =  M_PSensorToAndroidBase(PSENSOR_MOTION_SIGNIFICANT_STILLNESS),
    AP_PSENSOR_CONTEXT_DEVICE_MOTION        =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_DEVICE_MOTION),
    AP_PSENSOR_CONTEXT_CARRY                =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_CARRY),
    AP_PSENSOR_CONTEXT_POSTURE              =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_POSTURE),
    AP_PSENSOR_CONTEXT_TRANSPORT            =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_TRANSPORT),
    AP_PSENSOR_CONTEXT_GESTURE_EVENT        =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_GESTURE_EVENT),
    AP_PSENSOR_HEART_RATE                   =  M_PSensorToAndroidBase(PSENSOR_HEART_RATE),
    AP_PSENSOR_CONTEXT_SEGMENT_DETECTOR     =  M_PSensorToAndroidBase(PSENSOR_CONTEXT_SEGMENT_DETECTOR),
    AP_SYSTEM_REAL_TIME_CLOCK               =  M_PSensorToAndroidBase(SYSTEM_REAL_TIME_CLOCK),
    AP_PSENSOR_MAG_FIELD_ANOMALY            =  M_PSensorToAndroidBase(PSENSOR_MAGNETIC_FIELD_ANOMALY),

} ASensorType_t;



/* Sensor Parameter Identifiers */
typedef enum _SensorParamId {
    SENSOR_PARAM_ERROR_CODE                 = 0, //!< Used to convey error code instead of parameter value
    SENSOR_PARAM_OFFSET                     = 1, //!< Offset or bias of a sensor
    SENSOR_PARAM_DATA_RATE                  = 2, //!< Datarate for the sensor
    SENSOR_PARAM_BAND_WIDTH                 = 3, //!< Bandwidth setting for the sensor
    SENSOR_PARAM_HP_FILTER                  = 4, //!< High Pass filter setting for the sensor
    SENSOR_PARAM_LP_FILTER                  = 5, //!< Low Pass filter setting for the sensor
    SENSOR_PARAM_ENABLE                     = 6, //!< Sensor Enable control

    NUM_SENSOR_PARAM
} SensorParamId_t;


/* Enumeration for the input sensors. */
typedef enum _InputSensor
{
    ACCEL_INPUT_SENSOR,
    MAG_INPUT_SENSOR,
    GYRO_INPUT_SENSOR,
    LIGHT_INPUT_SENSOR,
    PRESSURE_INPUT_SENSOR,
    TEMPERATURE_INPUT_SENSOR,
    PROXIMITY_INPUT_SENSOR,
    RELATIVE_HUMIDITY_INPUT_SENSOR,
    AMBIENT_TEMPERATURE_INPUT_SENSOR,

    NUM_INPUT_SENSORS,
    UNKNOWN_INPUT_SENSOR = 0xFF
} InputSensor_t;

// TODO:  Need more description how these are use.
//! Segment detector subtypes
typedef enum _SegmentSubType{
    CONTEXT_CHANGE_DETECT,
    CONTEXT_STEP_SEGMENT_DETECT,
    CONTEXT_TILT_DETECT,

    NUM_PSENSOR_CONTEXT_SEGMENT_DETECTOR_SUBTYPE
} SegmentSubType_t;

//!  Use these values as a sub-type for  STEP result
typedef enum _StepSubType {
    CONTEXT_STEP  = SENSOR_SUBTYPE_START,  //! Please proper description for this subtype
    STEP_SEGMENT_DETECTOR,                 //!< low compute trigger for analyzing if step may have occured

    NUM_PSENSOR_STEP_SUBTYPE
} StepSubType_t;

//! Use these values as a sub-type for CONTEXT_DEVICE_MOTION result
typedef enum _ContextDeviceMotionSubType {
    CONTEXT_DEVICE_MOTION_STILL  = SENSOR_SUBTYPE_START,
    CONTEXT_DEVICE_MOTION_ACCELERATING,
    CONTEXT_DEVICE_MOTION_ROTATING,
    CONTEXT_DEIVCE_MOTION_TRANSLATING,
    CONTEXT_DEVICE_MOTION_FREE_FALLING,

    NUM_PSENSOR_CONTEXT_DEVICE_MOTION_SUBTYPE
} ContextDeviceMotionSubType_t;

//!  Use these values as a sub-type for  CONTEXT_CARRY result
typedef enum _ContextCarrySubType {
    CONTEXT_CARRY_IN_POCKET     = SENSOR_SUBTYPE_START,
    CONTEXT_CARRY_IN_HAND,
    CONTEXT_CARRY_NOT_ON_PERSON,
    CONTEXT_CARRY_IN_HAND_FRONT,
    CONTEXT_CARRY_IN_HAND_SIDE,

    NUM_PSENSOR_CONTEXT_CARRY_SUBTYPE
} ContextCarrySubType_t;

//!  Use these values as a sub-type for CONTEXT_POSTURE result
typedef enum _ContextPostureSubType {
    CONTEXT_POSTURE_WALKING     = SENSOR_SUBTYPE_START,
    CONTEXT_POSTURE_STANDING,
    CONTEXT_POSTURE_SITTING,
    CONTEXT_POSTURE_JOGGING,
    CONTEXT_POSTURE_RUNNING,

    NUM_PSENSOR_CONTEXT_POSTURE_SUBTYPE
} ContextPostureSubType_t;


//!  Use these values as a sub-type for  CONTEXT_TRANSPORT result
typedef enum _ContextTransportSubType {
    CONTEXT_TRANSPORT_VEHICLE   = SENSOR_SUBTYPE_START,
    CONTEXT_TRANSPORT_CAR,
    CONTEXT_TRANSPORT_TRAIN,
    CONTEXT_TRANSPORT_UP_STAIRS,
    CONTEXT_TRANSPORT_DOWN_STAIRS,
    CONTEXT_TRANSPORT_UP_ELEVATOR,
    CONTEXT_TRANSPORT_DOWN_ELEVATOR,
    CONTEXT_TRANSPORT_ON_BIKE,

    NUM_PSENSOR_CONTEXT_TRANSPORT_SUBTYPE
} ContextTransportSubType_t;

//!  Use these values as a sub-type for  GESTURE_EVENT result
typedef enum _GestureSubType {
    SENSOR_GESTURE_TAP          = SENSOR_SUBTYPE_START,
    SENSOR_GESTURE_DOUBLE_TAP,
    SENSOR_GESTURE_SHAKE,

    NUM_PSENSOR_GESTURE_SUBTYPE
} GestureSubType_t;


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

