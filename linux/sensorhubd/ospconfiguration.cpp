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
/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <assert.h>
#include "osp_datatypes.h"
#include "Names.h"
#include "DebugLog.h"
#include "ospconfiguration.h"
#include "osp_remoteprocedurecalls.h" //For Status codes -- FIXME!
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
//TBD - Move to platform header
#define DEFAULT_MAGNETOMETER_NOISE  1.0f
#define DEFAULT_GYROSCOPE_NOISE     1.0f
#define DEFAULT_ACCELEROMETER_NOISE 1.0f

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E/C L A S S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* used for stl sort algorithm:
*/
namespace{
class CompareKeys{
public:
    bool operator()( const std::string & left,
                     const std::string & right){
        if (left.find("protocol") == 0 && right.find("protocol") == 0){
            return left < right;
        } else if (left.find("protocol")== 0){
            return true;
        } else if (right.find("protocol") == 0){
            return false;
        }
        if (left.find("sensor=") == 0 && right.find("sensor=") == 0){
            return left < right;
        } else if (left.find("sensor=")== 0){
            auto leftkey = left;

            return leftkey.erase( 0,7 ) < right;
        } else if (right.find("sensor") == 0){
            auto rightkey = right;
            return left < rightkey.erase(0,7);
        }
        return left < right;
    }
};
}



/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      getSizeFromName
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const unsigned short SPI::OspConfiguration::getSizeFromName( const char* const shortName){
    static bool initialized = false;
    static std::map<std::string, unsigned short> _typeToSize;
    if (!initialized){

        _typeToSize[ Names::RAW_ACCELEROMETER ] = sizeof(double) + sizeof(osp_float_t)*3;
        _typeToSize[ Names::RAW_MAGNETOMETER ] = sizeof(double) + sizeof(osp_float_t)*3;
        _typeToSize[ Names::RAW_GYROSCOPE ] = sizeof(double) + sizeof(osp_float_t)*3;
        initialized = true;
    }
    const std::string type = getNamedConfigItem(shortName, SENSOR_TYPE);
    if ( _typeToSize.find(type) != _typeToSize.end()){
        return _typeToSize[type];
    }
    return -1;

}


/****************************************************************************************************
 * @fn      getDimensionFromName
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const int SPI::OspConfiguration::getDimensionFromName( const char* const shortName){
    static bool initialized = false;
    static std::map<std::string, int> _typeToDimension;
    if (!initialized){
        _typeToDimension[ Names::RAW_ACCELEROMETER ] = 3;
        _typeToDimension[ Names::RAW_MAGNETOMETER ] = 3;
        _typeToDimension[ Names::RAW_GYROSCOPE ] = 3;
        initialized = true;
    }
    const std::string type = getNamedConfigItem(shortName, SENSOR_TYPE);
    if ( _typeToDimension.find( type ) != _typeToDimension.end()){
        return _typeToDimension[ type ];
    }
    return SIZE_DYNAMIC;
}


/****************************************************************************************************
 * @fn      getTypeFromName
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const ESensorType SPI::OspConfiguration::getTypeFromName( const char* const shortName ){
    static bool initialized = false;
    static std::map<std::string, ESensorType> _nameToSensorType;
    if (!initialized){
        _nameToSensorType[ Names::RAW_ACCELEROMETER ] = ESensorType::SENSOR_RAW_ACCELEROMETER;
        _nameToSensorType[ Names::RAW_MAGNETOMETER ]  = ESensorType::SENSOR_RAW_MAGNETOMETER;
        _nameToSensorType[ Names::RAW_GYROSCOPE ] = ESensorType::SENSOR_RAW_GYROSCOPE;
        initialized = true;
    }
    const std::string type = getNamedConfigItem(shortName, SENSOR_TYPE);
    if ( _nameToSensorType.find(type) != _nameToSensorType.end()){
        return _nameToSensorType[type];
    }
    return ESensorType::SENSOR_UNDEFINED;

}


/****************************************************************************************************
 * @fn      Init
 *          Init routine
 *
 ***************************************************************************************************/
SPI::OspConfiguration::Init::Init(){
}


/****************************************************************************************************
 * @fn      getConfigItem
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const char* const
SPI::OspConfiguration::getConfigItem( const char* const name ){
    if (name == NULL){
        LOG_Err("Attempt to get config item for null name");
    } else if( configItemsString.find(std::string(name)) != configItemsString.end()){
        if (configItemsString.find(name)->second.size() >= 1){
            return  (configItemsString.find(name)->second)[0];
        }
    }
    return NULL;
}


/****************************************************************************************************
 * @fn      getConfigItemsMultiple
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
std::vector<const char* >
SPI::OspConfiguration::getConfigItemsMultiple( const char* const name ){
    std::vector< const char*  > retval;
    if (name == NULL){
        LOG_Err("Attempt to get config item for null name");
    }

    if (name != NULL && configItemsString.find(name) != configItemsString.end()){
        for (auto it = configItemsString.find(name)->second.begin();
             it != configItemsString.find(name)->second.end();
             ++it){
            if(*it)
                retval.push_back( *it );
        }
    }
    return retval;
}


/****************************************************************************************************
 * @fn      getConfigItemFloat
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const float *
SPI::OspConfiguration::getConfigItemFloat( const char* const name ,  unsigned int* size ){
    if (name == NULL){
        if(size) *size = 0;
        return NULL;
    }
    if ( configItemsFloat.end() == configItemsFloat.find(name)){
        if(size)  *size = 0;
        return NULL;
    }

    std::pair< const float*, unsigned int> pair = configItemsFloat[name];
    if(size) *size = pair.second;
    return pair.first;
}


/****************************************************************************************************
 * @fn      getConfigItemIntV
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
int
SPI::OspConfiguration::getConfigItemIntV(
        const char* const name,
        const int defaultValue,
        int* status){
    unsigned  int size;
    if(status)
        *status = OSP_STATUS_OK;
    const int* item = NULL;
    if( name == NULL){
        LOG_Err("Attempt to get int item from null name");
        if(status)*status = -1;
        item = NULL;
    } else {
        item = getConfigItemInt( name, &size);
        if (item && size != 1){
            LOG_Err("Request for integer config item %s that is not a single integer value: %d", name, size);
            if(status)*status = -1;
            item = NULL;
        }

    }
    return item?*item:defaultValue;
}


/****************************************************************************************************
 * @fn      getConfigItemInt
 *          Helper routine for getting configuration parameter
 *
 ***************************************************************************************************/
const int *
SPI::OspConfiguration::getConfigItemInt(
        const char* const name,
        unsigned int* size ){
    if (name == NULL){
        if (size) *size = 0;
        return NULL;
    }
    if ( configItemsInt.end() == configItemsInt.find(name)){
        if(size) *size = 0;
        return NULL;
    }
    std::pair< const int*, unsigned int> pair = configItemsInt[name];
    if(size) *size = pair.second;
    return pair.first;
}


/****************************************************************************************************
 * @fn      setConfigItem
 *          Helper routine for setting configuration parameter
 *
 ***************************************************************************************************/
int
SPI::OspConfiguration::setConfigItem(
        const char* const name,
        const char* const value,
        const bool allowMultiple,
        const bool override){
    LOG_Info("Setting config item %s", name);
    int status = -1;
    if ( name == NULL || value == NULL){
        status = -1;
    } else if (!allowMultiple && !override && configItemsString.find( name ) != configItemsString.end() ){
        status = -1;
    } else {
        if (configItemsString.find(name) == configItemsString.end()){
            configItemsString.insert(std::pair<std::string,
                                     std::vector<const char*> >(name, std::vector<const char*>()));
        }
        if (allowMultiple){
            bool found = false;
            for(  auto item =  configItemsString.find(name)->second.begin();
                  item  !=  configItemsString.find(name)->second.end();
                  ++item){
                found = found || (std::string(value) == std::string(*item));
            }
            if(!found){
                char *clone = new char[strlen(value)+1];
                clone[strlen(value)] = 0;
                memcpy( clone, value, strlen(value));
                configItemsString.find(name)->second.push_back(clone);
            }
        } else {
            char *clone = new char[strlen(value)+1];
            clone[strlen(value)] = 0;
            memcpy( clone, value, strlen(value));
            configItemsString.find(name)->second.clear();
            configItemsString.find(name)->second.push_back(clone);
        }
        status = 0;
    }
    return status;
}


/****************************************************************************************************
 * @fn      setConfigItemFloat
 *          Helper routine for setting configuration parameter
 *
 ***************************************************************************************************/
int
SPI::OspConfiguration::setConfigItemFloat(
        const char* const name,
        const float* const value,
        const unsigned int size,
        const bool override){
    LOG_Info("Setting config item %s", name);
    int status = -1;
    if (!override && configItemsFloat.find( name ) != configItemsFloat.end() ){
        status = -1;
    } else {
        if (configItemsFloat.find(name) != configItemsFloat.end() ){
            std::pair< const float * , unsigned int> pair = configItemsFloat[name];
            delete [] pair.first;
            //configItemsFloat.clear();
        }
        std::pair< const float*, unsigned int> pair;
        pair.first = new float[ size ];
        pair.second = size;
        for (unsigned int i =0; i < size; ++i ){
            const_cast< float* >( pair.first )[i] = value[i];
        }
        configItemsFloat.insert(std::pair<std::string, std::pair<const float*,unsigned int> >(name, pair) );
        status = 0;
    }
    return status;
}


/****************************************************************************************************
 * @fn      clear
 *          Helper routine for clearing all configuration parameters
 *
 ***************************************************************************************************/
void
SPI::OspConfiguration::clear(const bool final){
    typedef std::map<std::string,  std::pair< const float *,  unsigned int> >::iterator it_type;
    for(it_type iterator = configItemsFloat.begin(); iterator != configItemsFloat.end(); iterator++) {
        delete[] (*iterator).second.first;
        (*iterator).second.first = NULL;
        (*iterator).second.second = 0;
    }
    typedef std::map<std::string,  std::pair< const int *,  unsigned int> >::iterator intit_type;
    for(intit_type iterator = configItemsInt.begin(); iterator != configItemsInt.end(); iterator++) {
        delete[] (*iterator).second.first;
        (*iterator).second.first = NULL;
        (*iterator).second.second = 0;
    }
    for (auto it = configItemsString.begin();
         it != configItemsString.end();
         ++it){
        for (auto strit = configItemsString.find(it->first)->second.begin();
             strit != configItemsString.find(it->first)->second.end();
             ++strit){
            delete [] *strit;
        }
    }
    configItemsInt.clear();
    configItemsFloat.clear();
    configItemsString.clear();
}


/****************************************************************************************************
 * @fn      setConfigItemInt
 *          Helper routine for setting configuration parameter
 *
 ***************************************************************************************************/
int
SPI::OspConfiguration::setConfigItemInt(
        const char* const name,
        const int* const value,
        const unsigned int size,
        const bool override){
    LOG_Info("Setting config item %s", name);
    int status = -1;
    if (!override && configItemsInt.find( name ) != configItemsInt.end() ){
        status = -1;
    } else {
        if (configItemsInt.find(name) != configItemsInt.end()){
            std::pair< const int * , unsigned int> pair = configItemsInt[name];
            delete [] pair.first;
        }
        std::pair< const int*, unsigned int> pair;
        pair.first = new int[ size ];
        pair.second = size;
        for (unsigned int i =0; i < size; ++i ){
            const_cast< int* >( pair.first )[i] = value[i];
        }
        configItemsInt.insert(std::pair<std::string, std::pair<const int*, unsigned int> >(name, pair) );
        status = 0;
    }
    return status;
}


/****************************************************************************************************
 * @fn      establishAsynchronousSensor
 *          Helper routine for sensor configuration
 *
 ***************************************************************************************************/
void
SPI::OspConfiguration::establishAsynchronousSensor(
        const char* name,
        const char* protocol,
        const char* type){
    float conversion[1] = {1.0f};
    assert(OSPConfig::setConfigItem( "sensor", name, true)==0);
    OSPConfig::setConfigItem( OSPConfig::keyFrom( name, SENSOR_PROTOCOL).c_str(), protocol);
    OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name,SENSOR_CONVERSION).c_str(),conversion,1);
    OSPConfig::setConfigItem(
                OSPConfig::keyFrom( name, OSPConfig::SENSOR_TYPE).c_str(),
                type);
}


/****************************************************************************************************
 * @fn      establishCartesianSensor
 *          Helper routine for creating a default sensor configuration
 *
 ***************************************************************************************************/
void
SPI::OspConfiguration::establishCartesianSensor(
        const char* name,
        const char* protocol,
        const char* type,
        const float period,
        const float noise[3],
        const float * const bias,
        const float * const nonlinear,
        const float * const shake){
    float conversion[3] = {1.0f, 1.0f, 1.0f};
    int swap[3] = {0,1,2};
    assert(OSPConfig::setConfigItem( "sensor", name, true)==0);
    OSPConfig::setConfigItem( OSPConfig::keyFrom( name,SENSOR_PROTOCOL).c_str(), protocol);
    OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name, SENSOR_NOISE).c_str(), noise ,3);
    OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name,SENSOR_RATE).c_str(), &period, 1);
    if ( bias ){
        OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name,SENSOR_BIAS_STABILITY).c_str(),bias,3);
    }
    OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name, SENSOR_CONVERSION).c_str(),conversion,3);
    if( nonlinear){
        OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name, SENSOR_NON_LINEAR).c_str(),nonlinear,12);
    }
    if (shake){
        OSPConfig::setConfigItemFloat( OSPConfig::keyFrom( name, SENSOR_SHAKE).c_str(),shake,3);
    }

    OSPConfig::setConfigItemInt( OSPConfig::keyFrom( name, SENSOR_SWAP).c_str(),swap,3);
    OSPConfig::setConfigItem(
                OSPConfig::keyFrom( name, OSPConfig::SENSOR_TYPE).c_str(),
                type);
}


/****************************************************************************************************
 * @fn      establishDefaultConfig
 *          Helper routine for creating a default configuration as a starting point
 *
 ***************************************************************************************************/
void
SPI::OspConfiguration::establishDefaultConfig(const char* const protocol){
    int tick_us = 24;

    setConfigItem(OSPConfig::PROTOCOL_RELAY_DRIVER, "sensor_relay_kernel");
    setConfigItemInt(OSPConfig::PROTOCOL_RELAY_TICK_USEC, &tick_us, 1);
    /* MAG */
    {
        const float noise[3] = {
            DEFAULT_MAGNETOMETER_NOISE,
            DEFAULT_MAGNETOMETER_NOISE,
            DEFAULT_MAGNETOMETER_NOISE
        };
        const float bias[3] = {0.0f, 0.0f, 0.0f};
        float period = 0.02f;//50Hz
        float conversion[3] = {0.0625f,0.0625f,0.0625f};
        int swap[3] = {0,1,2};
        establishCartesianSensor(
                    "mag1",
                    protocol,
                    Names::RAW_MAGNETOMETER,
                    period,
                    noise,
                    bias);
        OSPConfig::setConfigItem(
                    OSPConfig::keyFrom("mag1", SENSOR_INPUT_NAME).c_str(),
                    Names::RAW_MAGNETOMETER );
        OSPConfig::setConfigItemFloat(
                    OSPConfig::keyFrom("mag1", SENSOR_CONVERSION).c_str(),
                    conversion,
                    3,
                    true);
        OSPConfig::setConfigItemInt( "mag1.swap",swap,3);
    }


    /* ACCEL */
    {
        const float noise[3] = {
            DEFAULT_ACCELEROMETER_NOISE,
            DEFAULT_ACCELEROMETER_NOISE,
            DEFAULT_ACCELEROMETER_NOISE
        };
        const float bias[3] = {0.0f, 0.0f, 0.0f};
        float period = 0.02f;//50Hz
        float conversion[3] = {0.01915893f,0.01915893f,0.01915893f};
        int swap[3] = {0,1,2};
        establishCartesianSensor(
                    "acc1",
                    protocol,
                    Names::RAW_ACCELEROMETER,
                    period,
                    noise,
                    bias);
        OSPConfig::setConfigItem(
                    OSPConfig::keyFrom("acc1", SENSOR_INPUT_NAME).c_str(),
                    Names::RAW_ACCELEROMETER );
        OSPConfig::setConfigItemFloat(
                    OSPConfig::keyFrom("acc1", SENSOR_CONVERSION).c_str(),
                    conversion,
                    3,
                    true);
        OSPConfig::setConfigItemInt( "acc1.swap",swap,3);
    }

    /* GYRO */
    {
        const float noise[3] = {
            DEFAULT_GYROSCOPE_NOISE,
            DEFAULT_GYROSCOPE_NOISE,
            DEFAULT_GYROSCOPE_NOISE
        };
        const float bias[3] = {0.0f, 0.0f, 0.0f};
        float period = 0.01f;//100Hz
        float conversion[3] = {0.001064127f,0.001064127f,0.001064127f};
        int swap[3] = {0,1,2};
        establishCartesianSensor(
                    "gyr1",
                    protocol,
                    Names::RAW_GYROSCOPE,
                    period,
                    noise,
                    bias);
        OSPConfig::setConfigItem(
                    OSPConfig::keyFrom("gyr1", SENSOR_INPUT_NAME).c_str(),
                    Names::RAW_GYROSCOPE );
        OSPConfig::setConfigItemFloat(
                    OSPConfig::keyFrom("gyr1", SENSOR_CONVERSION).c_str(),
                    conversion,
                    3,
                    true);
        OSPConfig::setConfigItemInt( "gyr1.swap",swap,3);
    }
}


/****************************************************************************************************
 * @fn      dump
 *          Helper routine for dumping the current configuration to a file for debugging
 *
 ***************************************************************************************************/
int
SPI::OspConfiguration::dump( const char* const filename ){
    FILE* f = ::fopen( filename, "w");
    if (!f){
        LOG_Err("Unable to open file '%s'",filename);
        return -1;
    }
    fprintf(f, "#for hash-code\n");
    fprintf(f, "?format_version = 2.0\n");

    /* collect all keys over string, float, int items for sorting*/
    std::vector< std::string > keys;
    for (auto it = configItemsString.begin();
         it != configItemsString.end();
         ++it){
        if (it->first == "sensor"){
            for( auto item = it->second.begin();
                 item != it->second.end();
                 ++item){
                keys.push_back( it->first+"="+*item );/* for sorting properly*/
            }
        } else{
            if (it->first != "debugOutputFileName" &&
                    it->first != "requested-result")
                keys.push_back( it->first );
        }
    }
    for (auto it = configItemsFloat.begin();
         it != configItemsFloat.end();
         ++it){
        keys.push_back( it->first );
    }
    for (auto it = configItemsInt.begin();
         it != configItemsInt.end();
         ++it){
        keys.push_back( it->first );
    }
    std::sort<  std::vector<std::string>::iterator, CompareKeys>
            ( keys.begin(), keys.end(), CompareKeys());
    /* loope over sorted keys and spit out to file */
    for (auto it = keys.begin();
         it != keys.end();
         ++it){
        const std::string key = *it;
        if (key.find("sensor") == 0 ){
            //handle sensor separately to get better ordering in file
            //and make it "flow" better
            fprintf(f,"\n\n%s\n", key.c_str());
        } else if (configItemsString.find(key) != configItemsString.end()){
            const std::vector<const char*>& items = configItemsString.find(key)->second;
            if( items.size()>1)
                fprintf(f,"\n");

            for(auto it2 = items.begin();
                it2 != items.end();
                ++it2){

                fprintf( f, "%s = \"%s\"\n", key.c_str(),
                         *it2);

            }
        } else if (configItemsFloat.find(key) != configItemsFloat.end()){
            std::pair< const float*, unsigned int> floatarray =
                    configItemsFloat[key];
            fprintf( f, "%s = ", key.c_str());
            for(unsigned int i = 0; i < floatarray.second; ++i){
                if (i==0)
                    fprintf( f, "%f", floatarray.first[i]);
                else
                    fprintf( f, ",%f", floatarray.first[i]);
            }
            fprintf(f, "\n");
        } else if (configItemsInt.find(key) != configItemsInt.end()){
            std::pair< const int*, unsigned int> intarray =
                    configItemsInt[key];
            fprintf( f, "%s = ", key.c_str());
            for(unsigned int i = 0; i < intarray.second; ++i){
                if (i==0)
                    fprintf( f, "%d", intarray.first[i]);
                else
                    fprintf( f, ",%d", intarray.first[i]);
            }
            fprintf(f, "\n");
        }

    }
    fclose(f);

    return 0;

}


extern "C++"{
std::map<std::string, std::vector<const char*> > SPI::OspConfiguration::configItemsString;
std::map<std::string, std::pair< const float*, unsigned int>  > SPI::OspConfiguration::configItemsFloat;
std::map<std::string, std::pair< const int *,  unsigned int>  > SPI::OspConfiguration::configItemsInt;
SPI::OspConfiguration::Init SPI::OspConfiguration::initializer;
}

/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
