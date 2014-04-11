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
#ifndef OSP_ALGORITHMSAPI_H__
#define OSP_ALGORITHMSAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*--------------------------------------------------------------------------*/
#include "Types.h"
#include "Common.h"
#include "FixedPointTypes.h"

/*--------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*--------------------------------------------------------------------------*/

/// flags to pass into result descriptors
#define OSP_NOTIFY_DEFAULT   (0)
#define OSP_OVERRIDE_ONESHOT (0xFFFFFFFF)
#define OSP_NO_FLAGS         (0)
#define OSP_NO_OPTIONAL_DATA ((void*)NULL)

/*--------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*--------------------------------------------------------------------------*/


// Input enums

//! used to swap axes or conventions from sensor frame to body frame in a SensorDescriptor_t
/*!
 *  this is most often used:
 *  - when the different sensors on a board were not physically aligned to the same coordinate system
 *  - to convert from a left-handed system magnetometer into a right-handed system
 */
typedef enum {
    AXIS_MAP_UNUSED = 0,
    AXIS_MAP_POSITIVE_X = 1,
    AXIS_MAP_NEGATIVE_X = 2,
    AXIS_MAP_POSITIVE_Y = 3,
    AXIS_MAP_NEGATIVE_Y = 4,
    AXIS_MAP_POSITIVE_Z = 5,
    AXIS_MAP_NEGATIVE_Z = 6,
    AXIS_MAP_ENUM_COUNT
} AxisMapType_t ;

// Input or Output enums

//! use to specify the kind of sensor input or result output when subscribing with a ResultRequestDescriptor_t
/*! \sa ResultRequestDescriptor_t OSP_SubscribeResult
 *
 *  Final units of results are defined by the host output convention.
 *  If a result type not supported, an error will be returned on registration attempt.
 */
typedef enum {
    SENSOR_ACCELEROMETER                   =  0, //!< calibrated accelerometer data
    SENSOR_ACCELEROMETER_UNCALIBRATED      =  1, //!< uncalibrated accelerometer data
    SENSOR_MAGNETIC_FIELD                  =  2, //!< calibrated magnetometer data
    SENSOR_MAGNETIC_FIELD_UNCALIBRATED     =  3, //!< uncalibrated magnetometer data
    SENSOR_GYROSCOPE                       =  4, //!< calibrated gyroscope data
    SENSOR_GYROSCOPE_UNCALIBRATED          =  5, //!< uncalibrated gyroscope data
    SENSOR_LIGHT                           =  6, //!< light data
    SENSOR_PRESSURE                        =  7, //!< barometer pressure data
    SENSOR_PROXIMITY                       =  8, //!< proximity data
    SENSOR_RELATIVE_HUMIDITY               =  9, //!< relative humidity data
    SENSOR_AMBIENT_TEMPERATURE             = 10, //!< ambient temperature data
    SENSOR_GRAVITY                         = 11, //!< gravity part of acceleration in body frame 
    SENSOR_LINEAR_ACCELERATION             = 12, //!< dynamic acceleration 
    SENSOR_ORIENTATION                     = 13, //!< yaw, pitch, roll (also use this for Win8 Inclinometer)
    SENSOR_AUG_REALITY_COMPASS             = 14, //!< heading which switches to aug-reality mode when camera towards horizon (Win8 compass)
    SENSOR_ROTATION_VECTOR                 = 15, //!< accel+mag+gyro quaternion
    SENSOR_GEOMAGNETIC_ROTATION_VECTOR     = 16, //!< accel+mag quaternion
    SENSOR_GAME_ROTATION_VECTOR            = 17, //!< accel+gyro quaternion
    SENSOR_VIRTUAL_GYROSCOPE               = 18, //!< virtual gyroscope data from accel+mag
    SENSOR_STEP_DETECTOR                   = 19, //!< precise time a step occured
    SENSOR_STEP_COUNTER                    = 20, //!< count of steps
    SENSOR_CONTEXT_DEVICE_MOTION           = 21, //!< context of device relative to world frame
    SENSOR_CONTEXT_CARRY                   = 22, //!< context of device relative to user
    SENSOR_CONTEXT_POSTURE                 = 23, //!< context of user relative to world frame
    SENSOR_CONTEXT_TRANSPORT               = 24, //!< context of environment relative to world frame
    SENSOR_CONTEXT_CHANGE_DETECTOR         = 25, //!< low compute trigger for seeing if context may have changed
    SENSOR_STEP_SEGMENT_DETECTOR           = 26, //!< low compute trigger for analyzing if step may have occured
    SENSOR_GESTURE_EVENT                   = 27, //!< gesture event such as a double-tap or shake
    SENSOR_NEED_CALIBRATION                = 28, //!< boolean indication that the user should perform a calibration sequence
    SENSOR_MESSAGE                         = 29, //!< warnings from the library: e.g. excessive timestamp jitter
    SENSOR_RGB_LIGHT                       = 30, //!< RGB light data
    SENSOR_UV_LIGHT                        = 31, //!< UV light data
    SENSOR_HEART_RATE                      = 32, //!< heart-rate data
    SENSOR_BLOOD_OXYGEN_LEVEL              = 33, //!< blood-oxygen level data
    SENSOR_SKIN_HYDRATION_LEVEL            = 34, //!< skin-hydration level data
    SENSOR_BREATHALYZER                    = 35, //!< breathalyzer data
    SENSOR_ENUM_COUNT
} SensorType_t ;

// Output enums

//! use to specify the type and units of sensor result outputs
/*!
 *  e.g When choosing FORMAT_WIN8 the ORIENTATION output will be as requested for INCLINOMETER: three int32 values in tenths of a degree
 *
 * \sa ResultRequestDescriptor_t OSP_SubscribeResult
 */
typedef enum {
    DATA_CONVENTION_ANDROID = 0,
    DATA_CONVENTION_WIN8    = 1,
    RESULT_FORMAT_ENUM_COUNT
} SensorDataConvention_t ;

//! handle type returned by OSP_RegisterSensor() or OSP_ReplaceSensor() necessary when calling OSP_SetForegroundData() or OSP_SetBackgroundData()
typedef void* SensorHandle_t;

//! handle type returned by OSP_SubscribeResult() or OSP_UnsubscribeResult()
typedef void* ResultHandle_t;

//! data passed back via OSP_SensorControlCallback_t to tell the sensor driver to change the operation of its physical sensors
typedef enum {
    SENSOR_CONTROL_SENSOR_OFF = 0,              //!< turn off sensor
    SENSOR_CONTROL_SENSOR_SLEEP = 1,            //!< put sensor in low power sleep mode w/ fast turn on
    SENSOR_CONTROL_SENSOR_ON = 2,               //!< turn on sensor
    SENSOR_CONTROL_SET_SAMPLE_RATE = 3,         //!< sample sensor at new rate, Data = sample time in seconds, NTPRECISE
    SENSOR_CONTROL_SET_LPF_FREQ = 4,            //!< set Low pass filter 3db cutoff frequency (in Hz) 0 = turn off filter
    SENSOR_CONTROL_SET_HPF_FREQ = 5,            //!< set High pass filter 3db cutoff frequency (in Hz) 0 = turn off filter
    SENSOR_CONTROL_ENUM_COUNT
} SensorControlCommand_t;

//! how enable/disable/setDelay type commands and data arepassed back to the sensor driver
typedef struct  {
    SensorHandle_t Handle;                      //!< handle that was returned from OSP_RegisterSensor()
    uint16_t Command;                           //!< command to sensor (power on/off, change rate, etc...)
    int32_t Data;                               //!< as need and appropriate for each command: e.g. high pass frequency in Hz
} SensorControl_t;

// Gesture results

//! Used for all gesture results in conjuntion with the enum GestureType_t
typedef struct {
    NTTIME TimeStamp;                  //!< Time in seconds
    NT *Probability;                   //!< Probability vector.  index into this with the appropriate GestureType_t enum
} GestureEventResultData_t;

//! Use these enums as indices into the probability vector of a GestureEventResultData_t in a GESTURE_EVENT result callback
typedef enum {
    GESTURE_TAP = 0,
    GESTURE_DOUBLE_TAP = 1,
    GESTURE_SHAKE = 2,
    GESTURE_ENUM_COUNT
} GestureType_t;


// Context results

//! Used for all context results in conjunction with the enums ContextMotionType_t, ContextPostureType_t, ContextCarryType_t
typedef struct {
    NTTIME TimeStamp;                           //!< Time in seconds
    NT *Probability;                            //!< Probability vector. index into this with the appropriate Context*Type_t enum
} ContextResultData_t;

//! Use these enums as indices into the probability vector of a ContextResultData_t in a CONTEXT_DEVICE_MOTION result callback
typedef enum {
    CONTEXT_MOTION_STILL = 0,
    CONTEXT_MOTION_ACCELERATING = 1,
    CONTEXT_MOTION_ROTATING = 2,
    CONTEXT_MOTION_TRANSLATING = 3,
    CONTEXT_MOTION_FREE_FALLING = 4,
    CONTEXT_MOTION_SIGNIFICANT_MOTION = 5,    //!< significant motion (as specified by Android HAL 1.0)
    CONTEXT_MOTION_SIGNIFICANT_STILLNESS = 6, //!< complement to significant motion
    CONTEXT_MOTION_ENUM_COUNT
} ContextMotionType_t;

//! Use these enums as indices into the probability vector of a ContextResultData_t in a CONTEXT_POSTURE result callback
typedef enum {
    CONTEXT_POSTURE_WALKING = 0,
    CONTEXT_POSTURE_STANDING = 1,
    CONTEXT_POSTURE_SITTING = 2,
    CONTEXT_POSTURE_JOGGING = 3,
    CONTEXT_POSTURE_RUNNING = 4,
    CONTEXT_POSTURE_ENUM_COUNT
} ContextPostureType_t;

//! Use these enums as indices into the probability vector of a ContextResultData_t in a CONTEXT_CARRY result callback
typedef enum {
    CONTEXT_CARRY_IN_POCKET = 0,
    CONTEXT_CARRY_IN_HAND = 1,
    CONTEXT_CARRY_NOT_ON_PERSON = 2,
    CONTEXT_CARRY_IN_HAND_FRONT = 3,
    CONTEXT_CARRY_IN_HAND_SIDE = 4,
    CONTEXT_CARRY_ENUM_COUNT
} ContextCarryType_t;

//! Use these enums as indices into the probability vector of a ContextResultData_t in a CONTEXT_TRANSPORT result callback
typedef enum {
    CONTEXT_TRANSPORT_VEHICLE = 0,
    CONTEXT_TRANSPORT_CAR = 1,
    CONTEXT_TRANSPORT_TRAIN = 2,
    CONTEXT_TRANSPORT_AIRPLANE = 3,
    CONTEXT_TRANSPORT_UP_STAIRS = 4,
    CONTEXT_TRANSPORT_DOWN_STAIRS = 5,
    CONTEXT_TRANSPORT_UP_ELEVATOR = 6,
    CONTEXT_TRANSPORT_DOWN_ELEVATOR = 7,
    CONTEXT_TRANSPORT_UP_ESCALATOR = 8,
    CONTEXT_TRANSPORT_DOWN_ESCALATOR = 9,
    CONTEXT_TRANSPORT_MOVING_WALKWAY = 10,
    CONTEXT_TRANSPORT_ENUM_COUNT
} ContextTransportType_t;


//! Use these enums as indices into the probability vector of a ContextResultData_t in a STEP result callback
typedef enum {
    CONTEXT_STEP = 0,                           //!< only one kind of step now
    CONTEXT_STEP_ENUM_COUNT
} ContextStepType_t;


//! detailed step data: time started, time it stopped, step length etc
typedef struct {
    StepData_t step;
} OSP_StepResultData_t;

//! describes the window of "interesting data" returned by RESULT_CONTEXT_CHANGE_DETECTOR 
typedef struct {
    Segment_t segment;             //!< Detected segment
} ContextChangeDetectorData_t;

//! calibrated acceleration in m/s^2. note positive Z when flat on table.
typedef struct {
    NTTIME TimeStamp;                             //!< Time in seconds
    NTPRECISE X;                                 //!< X axis 32Q24 fixed point data
    NTPRECISE Y;                                 //!< Y axis 32Q24 fixed point data
    NTPRECISE Z;                                 //!< Z axis 32Q24 fixed point data
} Android_CalibratedAccelResultData_t;

//! calibrated magnetometer in uT  +Y axis points out the top edge of the device
typedef struct {
    NTTIME TimeStamp;                             //!< Time in seconds
    NTEXTENDED X;                                 //!< X axis 32Q12 fixed point data
    NTEXTENDED Y;                                 //!< Y axis 32Q12 fixed point data
    NTEXTENDED Z;                                 //!< Z axis 32Q12 fixed point data
} Android_CalibratedMagResultData_t;

//! calibrated rotation rate in rad/s right handed +Y axis points out the top edge of the device
typedef struct {
    NTTIME TimeStamp;                             //!< Time in seconds
    NTPRECISE X;                                 //!< X axis 32Q24 fixed point data
    NTPRECISE Y;                                 //!< Y axis 32Q24 fixed point data
    NTPRECISE Z;                                 //!< Z axis 32Q24 fixed point data
} Android_CalibratedGyroResultData_t;


//! uncalibrated rotation rate in rad/s right handed +Y axis points out the top edge of the device
typedef struct {
    NTTIME TimeStamp;               //!< Time in seconds
    NTPRECISE X;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Y;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Z;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE X_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Y_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Z_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
} Android_UncalibratedGyroResultData_t;

//! uncalibrated magnetometer in uT  +Y axis points out the top edge of the device
typedef struct {
    NTTIME TimeStamp;               //!< Time in seconds
    NTEXTENDED X;                   //!< 32Q12 fixed point data representing uT.
    NTEXTENDED Y;                   //!< 32Q12 fixed point data representing uT. 
    NTEXTENDED Z;                   //!< 32Q12 fixed point data representing uT. 
    NTEXTENDED X_hardIron_offset;   //!< 32Q12 fixed point data representing uT   
    NTEXTENDED Y_hardIron_offset;   //!< 32Q12 fixed point data representing uT
    NTEXTENDED Z_hardIron_offset;   //!< 32Q12 fixed point data representing uT
} Android_UncalibratedMagnetometerResultData_t;

//! uncalibrated acceleration in m/s^2. note positive Z when flat on table.
typedef struct {
    NTTIME TimeStamp;               //!< Time in seconds
    NTPRECISE X;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Y;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Z;                   //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE X_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Y_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
    NTPRECISE Z_drift_offset;      //!< 32Q24 fixed point data representing rad/sec.
} Android_UncalibratedAccelResultData_t;

//! time at the start of a motion which is likely to lead to a change in position
typedef struct {
    NTTIME TimeStamp;                     //!< Time in seconds
    OSP_bool_t significantMotionDetected;  //!< always set to true when this result fires
} Android_SignificantMotionResultData_t;

//! indicates when each step is taken
typedef struct {
    NTTIME TimeStamp;               //!< Time in seconds
    OSP_bool_t StepDetected;         //!< always set to true, indicating a step was taken
} Android_StepDetectorResultData_t;

//! Android style step counter, but note that the host driver must bookkeep between sensorhub power on/off to meet android requirment 
typedef struct {
    NTTIME TimeStamp;         // timestamp
    uint32_t StepCount;       //!< steps since power on of the sensorhub (this is an important distinction from the full android requirement!)
} Android_StepCounterResultData_t;

//! positive, normalized quaternion used for the various flavors of ROTATION_VECTOR
typedef struct {
    NTTIME TimeStamp;                             //!< Time in seconds
    NT X;                                         //!< X component of normalized quaternion in 16Q12 fixed point
    NT Y;                                         //!< Y component of normalized quaternion in 16Q12 fixed point
    NT Z;                                         //!< Z component of normalized quaternion in 16Q12 fixed point
    NT W;                                         //!< W component of normalized quaternion in 16Q12 fixed point
} Android_RotationVectorResultData_t;


//! callback type used when the library needs to do an atomic operation
/*!
 *  This is absolutely necessary in systems that do background calibration.
 */
typedef void (*OSP_CriticalSectionCallback_t)(void);

//! describes system wide settings
/*!
 *  This cannot change after the call initialize.
 *  If there is a smart sensor with its own timestamp, it should be converted into system timestamp units.
 *  The API will not handle 32-bit rollover and clients need to worry about this.
 *
 *  \warning on a multi-threaded system if Enter and Exit Critical are NULL then there are no guarantees on data consistency
 *
 */
typedef struct  {
    TIMECOEFFICIENT TstampConversionToSeconds;  //!< 1 count = this many seconds
    OSP_CriticalSectionCallback_t EnterCritical; //!< callback for entering a critical section of code (i.e. no task switch), NULL if not implemented
    OSP_CriticalSectionCallback_t ExitCritical;  //!< callback for exiting a critical section of code (i.e. task switch ok now), NULL if not implemented
} SystemDescriptor_t;


//! called by calibration routines when there is new calibration coefficients that should be written to non-volatile memory.
/*!
 *  \warning SensorHandle can change from run to run. Do store the SensorHandle value into NVM to tag this cal data
 *
 *  \param SensorHandle INPUT the sensor this cal data belongs to.
 *  \param CalData      INPUT array of bytes
 *  \param TimeStamp    INPUT timestamp of when this calibration was calculated
 */
typedef void  (* OSP_WriteCalDataCallback_t)(SensorHandle_t SensorHandle, void * CalData, NTTIME TimeStamp);

//! called by the algorithms to notify the sensor hub that there is a new data item ready
/*!
 *  cast the pData to the expected result type (e.g. )  If you have a general callback handler use the 
 *  ResultHandle to lookup what result type this should be cast to.
 *
 * \warning the data pointed to by pData is only guaranteed to be valid during the lifetime of this callback
 * \param ResultHandle a handle returned from OSP_SubscribeResult()
 * \param pData the computed value you are interested in.  Cast as appropriate for each result, e.g. TriAxisData for CALIBRATED_GYRO_DATA,
 */
typedef void  (* OSP_ResultReadyCallback_t)(ResultHandle_t ResultHandle, void* pData);

//!
typedef uint16_t (* OSP_SensorControlCallback_t)(SensorControl_t* SensorControlCommand);


//! describes either a physical or logical sensor and its configuration
/*!
 * convert sensor data straight from the sensor into the system conventions of orientation and units
 * data is converted are applied in the order of AxisMapping, ConversionOffset, and ConversionScale
 * allows conversion from lhs sensor to rhs sensor hub system
 * must re-registrater sensors to change orientation
 * Sensor conversions convert native binary format into units dictated by DataConvention
 * There is no callback for the library to read calibration. The sensor hub must read its calibration
   from NVM before registering/re-registerating this sensor If there is no stored calibration data available,
   pass a NULL.
  When the the Open-Sensor-Platform library has computed new calibration data, it will update the data structure and call
  pOptionalWriteDataCallback(),if it is available, so that the sensor hub can store the updated calibration data to NVM.
*/

typedef struct  {
    SensorType_t SensorType;                    //!< accelerometer, gyro, etc
    SensorDataConvention_t DataConvention;      //!< Android, Win8, etc,...
    uint32_t DataWidthMask;                     //!< how much of the data word that is sent significant
    AxisMapType_t AxisMapping[3];               //!< swap or flip axes as necessary before conversion
    int32_t ConversionOffset[3];                //!< offset of incoming data before data is scaled
    NTPRECISE ConversionScale[3];               //!< conversion from raw to dimensionful units based on data convention e.g. 9.81/1024,  200dps/count
    NTEXTENDED MaxValue;                        //!< max value possible after conversion
    NTEXTENDED MinValue;                        //!< min value possible after conversion
    NTPRECISE Noise[3];                         //!< sensor noise based on Power Spectral Density in conversion units per sqrt(Hz)
    void * pCalibrationData;                    //!< a per sensor calibration data structure (can be NULL if not needed)
    OSP_ResultReadyCallback_t pOptionalResultReadyCallback; //!<  called only when a new output result is ready (usually NULL for input sensors)
    OSP_WriteCalDataCallback_t pOptionalWriteDataCallback; //!<  called when calibration data is ready to write to NVM (NULL if not used)
    OSP_SensorControlCallback_t pOptionalSensorControlCallback; //!< Optional callback (NULL if not used) to request sensor control (on/off, low rate, etc...)
    NTPRECISE NotifyThresholds[16];             //!<  0= unused; otherwise used to do Win8 style significance, or trigger only on a subset of a context category's values
    char* SensorName;                           //!< short human readable description, Null terminated
    uint32_t VendorId;                          //!< sensor vendor ID, as assigned by OSP
    uint32_t ProdId;                            //!< sensor product ID, as defined by each vendor
    uint32_t Version;                           //!< sensor version, as defined by each vendor
    uint32_t PlatformId;                        //!< platform ID, as defined by each vendor
    NTPRECISE xyzPositionFromPlatformOrigin[3]; //!< in meters (NTPRECISE gives us sub-micron resolution)
    uint16_t Flags;                             //!< defined on a per sensor type basis
    void* ptrSensorSpecificData;                //!< used in conjunction with Flags
} SensorDescriptor_t;


//! use to send raw sensor data from a driver into this API
/*!
 *  \note even if you have single axis or 2 axis data this is the type to use
 */
typedef struct  {
    uint32_t TimeStamp;                         // raw time stamp
    int32_t Data[3];                            // Raw sensor data
} TriAxisSensorRawData_t;


//! numeric and string formated version data, used with OSP_GetVersion()  
typedef struct  {
    uint32_t VersionNumber;                     //!< x.x.x.x one byte for major, minor, bugfix, and stamp
    char *VersionString;                        //!< Human readable null terminated string
} OSP_Library_Version_t;


/*--------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 |    A P I   F U N C T I O N   D E C L A R A T I O N S
\*--------------------------------------------------------------------------*/


//! Call immediately at startup to initialize the Open-Sensor-Platform Library and inform it of system characteristics
/*!
*  It is imperative to call this at cold boot, or after any time RAM has been lost, and before calling anything else.
*
*  Does internal initializations that the library requires.
*
*  \param pSystemDesc - INPUT pointer to a struct that describes things like time tick conversion value. This must not change
*         but may reside in read only memory.
*
*  \return status as specified in OSP_Types.h
*/
OSP_STATUS_t     OSP_Algorithms_Initialize(const SystemDescriptor_t* pSystemDesc);


//! Call at startup for each sensor in the system
/*!
 *  Tells the Open-Sensor-Platform Library what kind of sensor inputs it has to work with so its Resource Manager
 *  can choose the most appropriate algorithms to execute.
 *
 *  If you need to change a sensors operating mode at runtime see OSP_Algorithms_ReplaceSensor().
 *
 *  \warning the caller must preserve the data pointed to by pSensorDescriptor after this call
 *
 *  \param pSensorDesc INPUT pointer to data which describes all the details of this sensor and its
 *      current operating mode; e.g. sensor type, SI unit conversion factor
 *  \param pReturnedHandle OUTPUT a handle to use when feeding data in via OSP_Algorithms_SetData()
 *
 *  \return status as specified in OSP_Types.h
*/
OSP_STATUS_t     OSP_Algorithms_RegisterSensor(SensorDescriptor_t *pSensorDescriptor, SensorHandle_t *pReturnedHandle);


//! use to change a sensors operating mode (output rate, position, etc...)
/*!
 *  Tells the Open-Sensor-Platform Library to use this sensor instead of the previously registered sensor of the same type
 *
 *  Use Case: Standard Sensor Hub
 *     - after a change request interval/setDelay command from the host, create a new sensor descriptor with
 *       the updated sensor data rate and pass it OSP_Algorithms_ReplaceSensor()
 *
 *  Use Case: Transformer Ultra Book
 *     - when the host detects a change in device configuration (Tablet Mode --> Laptop Mode) which could change
 *       sensor position (e.g. sensors in screen have been rotated 180 degrees) or sensor
 *       environment (e.g. screen now interferes with magnetometer)
 *
 *  \warning the caller must preserve the data pointed to by pSensorDescriptor after this call
 *
 *  \param pSensorDesc INPUT pointer to data which describes all the details of the new sensor and its
 *      current operating mode; e.g. sensor type, SI unit conversion factor
*/
OSP_STATUS_t     OSP_Algorithms_ReplaceSensor(SensorDescriptor_t *pSensorDescriptor, SensorHandle_t *pSensorHandle);

//! queues sensor data which will be processed by OSP_DoForegroundProcessing() and OSP_DoBackgroundProcessing()
/*!
 *
 *  queueing data for un-registered sensors (or as sensors that) 
 *  queue size defaults to 8, though is implementation dependent and available via SENSOR_FG_DATA_Q_SIZE
 *
 *  \param sensorHandle INPUT requires a valid handle as returned by OSP_Algorithms_RegisterSensor()
 *  \param data INPUT pointer to timestamped raw sensor data
 *
 *  \return status. Will always be OSP_STATUS_OK. If there is no room in the queue,
 *   The last data will be overwritten and a warning will be triggered if you subscribe to RESULT_WARNING
*/
OSP_STATUS_t     OSP_Algorithms_SetData(SensorHandle_t SensorHandle, TriAxisSensorRawData_t *data);


//! triggers computation for primary algorithms  e.g ROTATION_VECTOR
/*!
 *  Separating OSP_Algorithms_DoForegroundProcessing and OSP_Algorithms_DoBackgroundProcessing calls allows for computation to happen in different thread contexts
 *  - Call at least as often as your fastest registered result output rate
 *  - Call from a medium priority task to ensure computation happens in a reasonable time
 *  
 *  Guideline: the forground task should not compute for more than 10ms on any platform
 *
 *  \note What algorithms get computed in the foreground versus the background are implementation dependent
 *
 *  \return status as specified in OSP_Types.h
*/
OSP_STATUS_t     OSP_Algorithms_DoForegroundProcessing(void);


//! triggers computation for less time critical background algorithms, e.g. sensor calibration
/*!
 *  Separating OSP_Algorithms_DoForegroundProcessing and OSP_Algorithms_DoBackgroundProcessing calls allows for computation to happen in different thread contexts
 *  - Call at least as often as your slowest registered sensor input
 *  - Call from the lowest priority task to ensure that more time critical functions can happen.
 *
 *  \warning may execute for tens of milliseconds (depending on clock rate and results chosen)
 *  \note What algorithms get computed in the foreground versus the background are implementation dependent
 *
 *  \return status as specified in OSP_Types.h
 */
OSP_STATUS_t     OSP_Algorithms_DoBackgroundProcessing(void);

//! call for each Open-Sensor-Platform result (STEP_COUNT, ROTATION_VECTOR, etc) you want computed and output
/*!
 *  Use Case: Standard Sensor Hub
 *     - after each enable command from the host, create a new result descriptor with
 *       the desired result type and output data rate and pass it to OSP_SubscribeResult()
 *
 *  Use Case: Sensor augmented GPS + Win8 Sensor Hub
 *     - as GPS requires it, create result descriptors requesting RESULT_UNCALIBRATED_ACCELEROMETER, 
 *       RESULT_UNCALIBRATED_MAGNETOMETER, RESULT_UNCALIBRATED_GYROSCOPE
 *     - as Win8 requests come in, create result descriptors per a standard Sensor Hub
 *     - be sure callbacks handle the WIN_ formated results differently than the SPI_UNCALIBRATED_ results
 *       as they have different units, orientations, and numeric formats
 *
 *
 *  \sa OSP_UnsubscribeResult
 *
 *  \param ResultRequestDescriptor_t INPUT pointer to data which describes the details of how the fusion should be
 *         computed: e.g output rate, sensors to use, etc.
 *  \param FusionResultHandle_t OUTPUT a handle to be used for OSP_UnsubscribeResult()
 *
 *  \return status as specified in OSP_Types.h. OSP_UNSUPPORTED_FEATURE for results that aren't available or licensed
 */
OSP_STATUS_t     OSP_SubscribeResult(ResultRequestDescriptor_t *pResultDescriptor, FusionResultHandle_t *pResultHandle);


//! stops the chain of computation for a registered result
/*!
 *  Use Case: Standard Sensor Hub
 *      - assume you are currently subscribed to ROTATION_VECTOR at 100Hz
 *      - a change request interval from the host for ROTATION_VECTOR at 50Hz
 *      - call OSP_UnsubscribeResult with the current result handle
 *      - modify the descriptor with the 50Hz output data rate and pass it to OSP_SubscribeResult()
 *
 *  \param ResultHandle INPUT FusionResultHandle_t that was received from OSP_SubscribeResult()
 *
 *  \return status as specified in OSP_Types.h
 */
OSP_STATUS_t     OSP_UnsubscribeResult(FusionResultHandle_t ResultHandle);

//! Use to get calibrated sensor data in cases where desired output rate is not an even multiple of input rate.
/*!
 *  the current calibrated data for the requested sensor
 *
 *  \param sensorHandle INPUT requires a valid handle as returned by OSP_RegisterSensor()
 *  \param data OUTPUT pointer to calibrated sensor data
 *
 *
 *  \return status as OSP_STATUS_IDLE if data has not exceeded the change sensitivity since the last
 *      calibrated result callback value for this sensor, or the last time that OSP_GetCalibratedSensor returned OSP_STATUS_OK.
 *      i.e. notification change is calculated from the last time we notified that a sensitivity change occurred.
 *      Note, if not subscribed to a calibrated result for this sensor, return will always be OSP_STATUS_IDLE.
 *      If data has exceeded the change sensitivity, will return OSP_STATUS_OK. All other errors apply.
 */
OSP_STATUS_t     OSP_Algorithms_GetCalibratedSensor(SensorHandle_t SensorHandle, int16_t *data);



//! provides version number and version string of the library implementation
/*!
 *
 *  \param pVersionStruct INPUT pointer to a pointer to the version data.
 *
 *  \return status as specified in OSP_Types.h
 */
OSP_STATUS_t     OSP_Algorithms_GetVersion(const OSP_Library_Version_t **ppVersionStruct);


#ifdef __cplusplus
}
#endif

#endif // OSP_ALGORITHMSAPI_H__

/*--------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*--------------------------------------------------------------------------*/
