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
#if !defined (ALG_CONVERSION_H)
#define   ALG_CONVERSION_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "Common.h"
#include "osp-fixedpoint-types.h"

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* (1 << (QFIXEDPOINT - 1) */
#define FIXEDPOINT_ROUNDING_VALUE       (2048)
#define MAX_SHORT                       (32767)
#define MIN_SHORT                       (-32768)
#define SPI_MAX_INT                     (2147483647LL)
#define SPI_MIN_INT                     (-2147483648LL)
#define M_SI_EARTH_GRAVITY              (9.805f)

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
// Input enums

//! These are sensor inputs to the API
typedef enum {
    SENSOR_TYPE_ACCELEROMETER = 0,
    SENSOR_TYPE_MAGNETIC_FIELD = 1,
    SENSOR_TYPE_GYROSCOPE = 2,
    SENSOR_TYPE_PRESSURE = 3,
    SENSOR_TYPE_TEMPERATURE = 4,
    SENSOR_TYPE_LIGHT = 5,
    SENSOR_TYPE_PROXIMITY = 6,
    SENSOR_TYPE_RELATIVE_HUMIDITY = 7,
    SENSOR_TYPE_ENUM_COUNT
} SensorType_t ;


//! specifies signed/unsigned data coming from a sensor described by a SensorDescriptor_t
/*!
 */
typedef enum {
    SENSOR_DATA_UNSIGNED = 0,
    SENSOR_DATA_SIGNED_TWO_COMP = 1,
    SENSOR_DATA_ENUM_COUNT
} SensorDataType_t ;

//! used to swap axes or conventions from sensor frame to body frame in a SensorDescriptor_t
/*!
 *  this is most often used:
 *  - when the different sensors on a board were not physically aligned to the same coordinate system
 *  - to bring data from a left handed system magnetometer into a right handed system
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

//! callback type used when the library needs to do an atomic operation
/*!
 *  this is absolutely necessary in systems that do background calibration
 */
typedef void (*FM_CriticalSectionCallback_t)(void);

//! describes system wide settings
/*!
 *  this cannot change after the call initialize
 *  if there is a smart sensor with its own timestamp, it should be converted into system timestamp units
 *  the API will handle 32 bit rollover and clients need to worry about this
 *
 *  \warning on a multi-threaded system if Enter and Exit Critical are NULL then there are no guarantees on data consistency
 */
typedef struct  {
    TIMECOEFFICIENT TstampConversionToSeconds;  //!< 1 count = this many seconds
    FM_CriticalSectionCallback_t EnterCritical; //!< callback for entering a critical section of code (i.e. no task switch), NULL if not implemented
    FM_CriticalSectionCallback_t ExitCritical;  //!< callback for exiting a critical section of code (i.e. task switch ok now), NULL if not implemented
} SystemDescriptor_t;

//! describes a logical sensor and its configuration
/*!
 * convert sensor data straight from the sensor into the system conventions of orientation and units
 * data is converted are applied in the order of AxisMapping, ConversionOffset, and ConversionScale
 * allows conversion from lhs sensor to rhs sensor hub system
 * must re-register sensors to change orientation

 * Sensor conversions convert native binary format into the following units:
      - Accelerometer - Units are in m/sec/sec
      - Magnetic field - Units are in uT
      - Gyroscope - Units are in radians/sec
      - Pressure - Units are in hPa
      - Temperature - Units are in degrees Celsius
      - Light (Illumination) - Units are in lux
      - Proximity - Units are in cm
      - Relative Humidity - Units are in Percent R.H. (%RH)

 * There is no callback for the library to read calibration. The sensor hub must read its calibration
   from NVM before registering/re-registering this sensor If there is no stored calibration data available,
   pass a NULL.
  When the the FreeMotion library has computed new calibration data, it will update the data structure and call
  pOptionalWriteDataCallback(),if it is available, so that the sensor hub can store the updated calibration data to NVM.
*/

typedef struct  {
    SensorType_t SensorType;                    //!< accelerometer, gyro, etc
    SensorDataType_t DataType;                  //!< signed 2s comp/unsigned/signed ones comp
    uint32_t DataWidthMask;                     //!< how much of the data word that is sent significant
    AxisMapType_t AxisMapping[3];               //!< e.g. use to switch from left handed sensor to right handed system  or swap X and Y axes of a two axis sensor
    int32_t ConversionOffset[3];                //!< e.g. unsigned mag 1024 = zero (this is in raw data units, applied before we scale the data)
    NTPRECISE ConversionScale[3];               //!< e.g. 9.81/1024,  200dps/count, etc
    NTEXTENDED MaxValue;                        //!< in engineering units. i.e. a +/- 2000 dps gyro would have a max value of 34.907 rad/sec
    NTEXTENDED MinValue;                        //!< in engineering units. i.e. a +/- 2000 dps gyro would have a min value of -34.907 rad/sec
    //void * pCalibrationData;                    //!< a per sensor calibration data structure (can be NULL if not needed)
    //FM_WriteCalDataCallback_t pOptionalWriteDataCallback; //!< this routine will be called when new cal data is ready to write to NVM (NULL if not used)
    //FM_SensorControlCallback_t pOptionalSensorControlCallback; //!< Optional callback (NULL if not used) to request sensor control (on/off, low rate, etc...)
    //NTPRECISE Noise[3];                         //!< noise based on Power Spectral Density test results
    NTPRECISE NominalSampleRateInSeconds;       //!< rate in which data is sent to the library (for accel, 50Hz and 100 Hz are the only supported rates)
    NTPRECISE SensorDelay;                      //!< time (in seconds) from sensor signal acquisition to time stamp (including filter propagation delay)
    //uint32_t On_Time;                           //!< time (in microseconds) when going from OFF mode to ON mode to valid data available
    //uint32_t Wake_Time;                         //!< time (in microseconds) when going from SLEEP mode to ON mode to valid data available
    //uint16_t Hpf_Cutoff;                        //!< high pass filter 3db cutoff in Hz (0 if none)
    //uint16_t Lpf_Cutoff;                        //!< low pass filter 3db cutoff in Hz (0 if none)
    //char* SensorName;                           //!< short human readable description, Null terminated
    //uint32_t VendorId;                          //!< sensor vendor ID, as assigned by SPI
    //uint32_t ProdId;                            //!< sensor product ID, as defined by each vendor
    //uint32_t Version;                           //!< sensor version
    //uint32_t PlatformId;                        //!< platform ID, as defined by each vendor
    //NTPRECISE xyzPositionFromPlatformOrigin[3]; //!< in meters (NTPRECISE gives us sub-micron resolution)
    //uint16_t Flags;                             //!< defined on a per sensor type basis
    //void* ptrSensorSpecificData;                //!< used in conjunction with Flags
} SensorDescriptor_t;

//! handle type returned by FM_RegisterSensor() or FM_ReplaceSensor() necessary when calling FM_SetForegroundData() or FM_SetBackgroundData
typedef void* SensorHandle_t;

//! use to send raw sensor data from a driver into this API
/*!
 *  \note even if you have single axis or 2 axis data this is the type to use
 */
typedef struct  {
    uint32_t TimeStamp;                         // raw time stamp
    int32_t Data[3];                            // Raw sensor data
} TriAxisSensorRawData_t;

typedef struct {

    SensorDescriptor_t *pSenDesc;
    uint16_t Flags;                 // in-use, etc
} _SenDesc_t;

typedef struct {

    SensorHandle_t Handle;          // handle for this sensor
    TriAxisSensorRawData_t Data;    // raw data & time stamp from sensor
} _SensorDataBuffer_t;


typedef struct  {
    NTTIME TimeStamp;               // time stamp
    NTEXTENDED Data[3];             // processed sensor data
} _TriAxisSensorCookedData_t;

typedef struct {
    NTTIME TimeStamp;             // time stamp
    NTEXTENDED Data[3];            // result data for SPI conventions
} SPI_3AxisResult_t;

// SPI convention results

//! 32Q24 fixed point data representing m/s^2. Gravity is a positive force on Z when flat on table. +X axis points out the front of the device (where android would have +Y)
typedef struct {
    NTTIME TimeStamp;               // time stamp
    NTEXTENDED X;
    NTEXTENDED Y;
    NTEXTENDED Z;
} SPI_UnCalibratedAccelResultData_t;

//! 32Q24 fixed point data representing uT.  +X axis points out the front of the device (where android would have +Y)
typedef struct {
    NTTIME TimeStamp;               // time stamp
    NTEXTENDED X;
    NTEXTENDED Y;
    NTEXTENDED Z;
} SPI_UnCalibratedMagResultData_t;

//! 32Q24 fixed point data representing rad/sec, right handed.  +X axis points out the front of the device (where android would have +Y)
typedef struct {
    NTTIME TimeStamp;               // time stamp
    NTEXTENDED X;
    NTEXTENDED Y;
    NTEXTENDED Z;
} SPI_UnCalibratedGyroResultData_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
extern SystemDescriptor_t gPlatformDesc;

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
__inline static uint32_t mult_uint16_uint16(uint16_t a, uint16_t b)
{
    return ((uint32_t) a * (uint32_t)b);
}

int32_t ConvertSensorData(SensorDescriptor_t *pSenDesc, TriAxisSensorRawData_t *pRawData, SPI_3AxisResult_t *pResult);
Bool GetTimeFromCounter(NTTIME * pTime,TIMECOEFFICIENT counterToTimeConversionFactor,uint32_t counterHigh, uint32_t counterLow);



#endif /* ALG_CONVERSION_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
