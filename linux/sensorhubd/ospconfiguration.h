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
#ifndef _OSP_CONFIGURATION__H
#define _OSP_CONFIGURATION__H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <string>
#include <map>
#include <vector>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
namespace {
const osp_size_t DIMENSION_DYNAMIC = 0;
const osp_size_t SIZE_DYNAMIC = 0;
}

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E / C L A S S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
enum class ESensorType: unsigned short {
    SENSOR_RAW_ACCELEROMETER,
    SENSOR_RAW_MAGNETOMETER,
    SENSOR_RAW_GYROSCOPE,
    SENSOR_RAW_LIGHT,
    SENSOR_RAW_PROXIMITY,
    SENSOR_RAW_PRESSURE ,
    SENSOR_CHANGE_DETECTOR,
    SENSOR_STEP_DATA,
    SENSOR_SYSTEM_EVENT,
    SENSOR_ATTITUDE,
    SENSOR_UNDEFINED
};

namespace OSP {

//class FileBasedConfigParser;
//class ConfigParser;

/* The purpose of this class is to maintain a store of key/value
       pairs, constant throughout a run, that represent the configuration
       of the system.
       Setter are private to prevent general setting of config items,
       and access is controlled through "friends".  Friends should only
       populate items once and only on initialization */
class OspConfiguration{
public:
    typedef const char*  cstring;

    /* A list of common names to use.  Use these in lieu of direct
           strings to prevent possible misspellings that won't be caught
           by compiler*/
    static constexpr cstring POLICY_VERBOSE = "policy.verbose";
    static constexpr cstring POLICY_USE_DEBUG = "policy.use-debug-logging";
    static constexpr cstring POLICY_FORCE_UINPUT = "policy.force-uinput";
    static constexpr cstring POLICY_BACKGROUND_GYROCAL = "policy.background-gyrocal";

    static constexpr cstring PROTOCOL_SERIAL_BAUD_RATE = "protocol.serial_baud_rate";

    static constexpr cstring PROTOCOL_RELAY_DRIVER = "protocol.relay_driver";
    static constexpr cstring PROTOCOL_RELAY_TICK_USEC = "protocol.relay_tick_usec";


    static constexpr cstring SENSOR_INPUT_NAME = "input-name";
    static constexpr cstring SENSOR_DRIVER_NAME = "driver";
    static constexpr cstring SENSOR_DIMENSION = "dimension";
    static constexpr cstring SENSOR_RATE = "defaultrate";
    static constexpr cstring SENSOR_PROTOCOL = "protocol";
    static constexpr cstring SENSOR_NOISE = "noise";
    static constexpr cstring SENSOR_ENABLE_PATH = "enable-path";
    static constexpr cstring SENSOR_ENABLE_VALUE = "enable-value";
    static constexpr cstring SENSOR_DISABLE_VALUE = "disable-value";
    static constexpr cstring SENSOR_DELAY_PATH = "delay-path";
    static constexpr cstring SENSOR_FACTORYCAL = "factorycal";
    static constexpr cstring SENSOR_SWAP = "swap";
    static constexpr cstring SENSOR_CONVERSION = "conversion";
    static constexpr cstring SENSOR_EXP_NORM = "expectednorm";
    static constexpr cstring SENSOR_DELAY = "delay";
    static constexpr cstring SENSOR_RESOLUTION = "resolution";
    static constexpr cstring SENSOR_USE_MEDIAN_FILTER = "use-median-filter";
    static constexpr cstring SENSOR_SATURATION = "saturation";
    static constexpr cstring SENSOR_NON_LINEAR = "non-linear";
    static constexpr cstring SENSOR_SHAKE = "shake";
    static constexpr cstring SENSOR_BIAS_STABILITY = "bias-stability";
    static constexpr cstring SENSOR_TYPE = "type";
    static constexpr cstring SENSOR_LONG_NAME = "long-name";
    static constexpr cstring SENSOR_REPUB_NAME = "repub-name";

    //to give access to setters:
    //...

    /* Get a key from an item and property pair.  This is useful
           when getting a property for a dynamically named item such as
           a sensor */
    inline static std::string keyFrom( const char* const item,
                                       const char* const property){
        return std::string(item) + "." + property;
    }

    /*
         * returns empty string if no named config item found,
         *   or the string value of the config item if found and is of a string type
         */
    static const char* const getConfigItem( const char* const name );

    static const char* const getNamedConfigItem( const char* const name,
                                                 const char* const property){
        static char empty[1] = {0};
        if(!name || !property) return "";
        const char* retval =  getConfigItem( keyFrom(name, property).c_str());
        if (!retval)
            return empty;
        return retval;
    }

    static std::vector< const char* >
    getConfigItemsMultiple( const char* const name );

    /*
         * return null if no named config item found,
         *   or the float array value of the config item if found is of a float type
         */
    static const float *
    getConfigItemFloat( const char* const name,  unsigned int* size );

    static const float *
    getNamedConfigItemFloat( const char* const name,
                             const char* const property,
                             unsigned int* size ){
        return getConfigItemFloat( keyFrom(name, property).c_str(),
                                   size);
    }

    /*
         * return null if no named config item found,
         *   or the int array value of the config item if found is of a int type
         */
    static const int   *
    getConfigItemInt( const char* const name,  unsigned int* const size );

    static const int   *
    getNamedConfigItemInt( const char* const name,
                           const char* const property,
                           unsigned int* const size ){
        return getConfigItemInt( keyFrom(name, property).c_str(),
                                 size );
    }


    static int
    getConfigItemIntV( const char* const name,
                       const int default_value,
                       int* const status );

    static int
    getNamedConfigItemIntV( const char* const name,
                            const char* const property,
                            const int default_value,
                            int* const status  = NULL){
        return getConfigItemIntV( keyFrom(name, property).c_str(),
                                  default_value,
                                  status);
    }


    static float
    getConfigItemFloatV( const char* const name,
                         const float defaultValue ,
                         int * const status = (int * const)NULL){
        unsigned int size;
        const float* value = getConfigItemFloat( name, &size);
        if (size != 1){ if(status){*status = -1;} return defaultValue;}
        else { if(status){*status = 0;} return value[0];}
    }

    static float
    getNamedConfigItemFloatV( const char* const name,
                              const char* const property,
                              const float defaultValue ,
                              int * const status = NULL){
        return getConfigItemFloatV( keyFrom(name, property).c_str(),
                                    defaultValue,
                                    status);
    }


    static bool
    getConfigItemBool( const char* const name , int* status = NULL){

        const int value = getConfigItemIntV(name, 0, status);
        if (value < 0){ if(status){*status = -1;} return false;}
        return value >0;
    }

    static void clear( const bool final = false);


    static void establishCartesianSensor( const char* const name,
                                          const char* const protocol,
                                          const char* const type,
                                          const float period,
                                          const float noise[3],
                                          const float * const bias = NULL,
                                          const float * const nonlinear = NULL,
                                          const float * const shake = NULL );

    static int dump( const char* const filename );

    static void establishDefaultConfig( const char* const protocol = "domain_socket");
    static const unsigned short getSizeFromName( const char* const shortName);
    static const ESensorType getTypeFromName( const char* const shortName );

    static const int getDimensionFromName( const char* const shortName);

private:

    class Init{
    public:
        Init();
    };
    static Init initializer;
    /*
         * set the string value of a config item;
         * returns -1 if name is already in use and override is not allowed
         * or if name or value are NULL, 0 otherwise
         */
    static int
    setConfigItem( const char* const name,
                   const char* const value,
                   const bool allowMultiple = false,
                   const bool override = false);

    /*
         * set the float-array value of a config item;
         * returns -1 if name is already in use and override is not allowed, 0 otherwise
         */
    static int
    setConfigItemFloat( const char* const name,
                        const float* value,
                        const unsigned int size ,
                        const bool override = false);

    /*
         * set the int-array value of a config item;
         * returns -1 if name is already in use and override is not
         * allowed, 0 otherwise
         */
    static int
    setConfigItemInt( const char* const name,
                      const int* const value,
                      const unsigned int size,
                      const bool override = false);

    static
    std::map< std::string, std::vector<const char*> > configItemsString;

    static
    std::map< std::string, std::pair< const float *,  unsigned int> > configItemsFloat;

    static
    std::map< std::string, std::pair< const int *,  unsigned int> > configItemsInt;
    static std::map< std::string, int> _typeToDimension;
    static std::map< std::string, unsigned short> _typeToSize;

};

}

typedef OSP::OspConfiguration OSPConfig;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

#endif //_OSP_CONFIGURATION__H
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
