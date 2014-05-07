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
#include "osp_remoteprocedurecalls.h"
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/input.h>
#include <assert.h>
#include "osp_relayinterface.h"
#include "DebugLog.h"
#include "ospconfiguration.h"
#include "sensor_relay.h"

extern "C" {
#include "UinputUtils.h"
}

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define PROCESS_INPUT_EVT_THRES         1
#define MAX_NUM_FDS(x,y) ((x) > (y) ? (x) : (y))

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E   T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    S T A T I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
static int32_t _relay_fd;
static int32_t _relayTickUsec;
static DeviceConfig_t _deviceConfig[MAX_NUM_SENSORS_TO_HANDLE];
static std::string _deviceRelayInputName;
static std::vector<RelayBufStatus_t> _relayStatus;
static std::vector<int> _relay_file;
static std::vector<unsigned char *> __relay_buffer;
/* control files */
static std::vector<int> _produced_file;
static std::vector<int> _consumed_file;

static const char* const sensornames[MAX_NUM_SENSORS_TO_HANDLE] = {
    "acc1",
    "mag1",
    "gyr1",
};

static pthread_t _relayThread;
static volatile bool _relayThreadActive = false;

static OSPD_ResultDataCallback_t _resultReadyCallbacks[COUNT_OF_SENSOR_TYPES] = {0};


/*-------------------------------------------------------------------------------------------------*\
 |    F O R W A R D   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/
static int32_t InitializeRelayInput( void );
static void *_processRelayInput(void *pData);
static void _relayReadAndProcessSensorData(int fd, OSPD_ResultDataCallback_t dataCallbacks[]);
static void ProcessInputEventsRelay(void);

/*-------------------------------------------------------------------------------------------------*\
 |    P R I V A T E     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      _sensorDataPublish
 *          Parse the sensor data and invoke result callbacks
 *
 ***************************************************************************************************/
static void _sensorDataPublish(uint32_t sensorIndex, OSPD_ThreeAxisData_t *pSensData)
{
    int32_t sensorType = -1;

    switch(sensorIndex) {
    case ACCEL_INDEX:
        sensorType = SENSOR_TYPE_ACCELEROMETER;
        LOG_Info("RA %.3f, %.3f, %.3f, %lld\n",
             pSensData->data[0].f,
             pSensData->data[1].f,
             pSensData->data[2].f,
             pSensData->timestamp.ll);
        break;

    case GYRO_INDEX:
        sensorType = SENSOR_TYPE_GYROSCOPE;
        //LOG_Info("RG %.3f, %.3f, %.3f, %lld\n",
        //     pSensData->data[0].f,
        //     pSensData->data[1].f,
        //     pSensData->data[2].f,
        //     pSensData->timestamp.ll);
        break;

    case MAG_INDEX:
        sensorType = SENSOR_TYPE_MAGNETIC_FIELD;
        //LOG_Info("RM %.3f, %.3f, %.3f, %lld\n",
        //     pSensData->data[0].f,
        //     pSensData->data[1].f,
        //     pSensData->data[2].f,
        //     pSensData->timestamp.ll);
        break;

    default:
        break;
    }

    if (( sensorType > 0) && (_resultReadyCallbacks[sensorType] != NULL)) {
        _resultReadyCallbacks[sensorType](sensorType, pSensData);
    }
}


/****************************************************************************************************
 * @fn      _doAxisSwap
 *          Helper function that does the axis swap as specified in configuration
 *
 ***************************************************************************************************/
int32_t _doAxisSwap(int32_t eventCode, int32_t sensorIndex, const int swap[3])
{
    int32_t result = 0;
    int32_t zeroIndexedDeviceAxis = -1;

    assert((sensorIndex >= ACCEL_INDEX) && (sensorIndex < MAX_NUM_SENSORS_TO_HANDLE));
    switch(eventCode) {
    case ABS_X:
    case ABS_Y:
    case ABS_Z:
        zeroIndexedDeviceAxis = eventCode - ABS_X;
        break;

    case ABS_RX:
    case ABS_RY:
    case ABS_RZ:
        zeroIndexedDeviceAxis = eventCode - ABS_RX;
        break;

    case ABS_GAS:   //used as code for light sensor
    case ABS_DISTANCE:  //used as code for prox sensor
    case ABS_PRESSURE:  // used for Barometer
        zeroIndexedDeviceAxis = 0;
        break;

    default:
        result = -1;
        break;
    }
    if (result == 0) {
        //TODO: This is based on assumption - may need fixing
        if (sensorIndex < MAX_NUM_SENSORS_TO_HANDLE) {
            assert( zeroIndexedDeviceAxis >= 0 );
            assert( zeroIndexedDeviceAxis < 3 );
            result = swap[zeroIndexedDeviceAxis];
        }
#if 0
        else {
            assert( zeroIndexedDeviceAxis >= 0, "Quaternion Axis index out of bounds!"); //should never happen
            assert( zeroIndexedDeviceAxis < 4, "Quaternion Axis index out of bounds!"); //should never happen
            result = zeroIndexedDeviceAxis; // no need for Axis conversion
        }
#endif
    } else {
        if (result < 0) {
            LOG_Err("Received unknown event code: %d", eventCode );
        }
        result  = -1;
    }
    return result;
}

/****************************************************************************************************
 * @fn      Initialize
 *          Main Initialization routine. Initializes device configuration
 *
 ***************************************************************************************************/
static int32_t Initialize( void )
{
    int32_t result;
    const char *temp;
    const int *swap;
    unsigned int swaplen;
    unsigned int convlen;
    const osp_float_t * conv;

    for (int index = 0; index < MAX_NUM_SENSORS_TO_HANDLE; ++index){
        const char* const drivername = OSPConfig::getNamedConfigItem(
                    sensornames[index],
                    OSPConfig::SENSOR_DRIVER_NAME);
        auto sensorlist = OSPConfig::getConfigItemsMultiple("sensor");
        if(drivername && strlen(drivername)){
            _deviceConfig[index].uinputName =  drivername;
        } else {
            _deviceConfig[index].uinputName = "";
            for (unsigned short i2 = 0; i2 < sensorlist.size(); ++i2){
                if ( strcmp( sensorlist[i2], sensornames[index]) == 0){
                    _deviceConfig[index].uinputName = sensornames[index];
                }
            }
        }


        swap = OSPConfig::getNamedConfigItemInt(
                    sensornames[index], OSPConfig::SENSOR_SWAP, &swaplen);
        if (!swap){
            for (unsigned int j = 0; j < 3; ++j){
                _deviceConfig[index].swap[j] = j;
            }
        } else if (swaplen == 3){
            for (unsigned int j = 0; j < 3; ++j){
                _deviceConfig[index].swap[j] = swap[j];
            }
        } else {
            LOG_Err("Invalid swap indices length of %d fo %s. ABORTING",
                    swaplen, sensornames[index]);
            assert(swaplen == 3);
        }

        conv = OSPConfig::getNamedConfigItemFloat(
                    sensornames[index], OSPConfig::SENSOR_CONVERSION, &convlen);
        if (!conv){
            for (unsigned int j = 0; j < 3; ++j){
                _deviceConfig[index].conversion[j] = 1.0f;
            }
        } else if (convlen == 1){
            for (unsigned int j = 0; j < 3; ++j){
                _deviceConfig[index].conversion[j] = conv[0];
            }
        } else if (convlen == 3){
            for (unsigned int j = 0; j < convlen; ++j){
                _deviceConfig[index].conversion[j] = conv[j];
            }
        } else {
            LOG_Err("Invalid conversion value array length of %d fo %s",
                    convlen, sensornames[index]);
            assert(convlen == 3);
        }
    }

    temp = OSPConfig::getConfigItem(OSPConfig::PROTOCOL_RELAY_DRIVER);
    _deviceRelayInputName = temp? temp : "";

    if (!_deviceRelayInputName.empty()) {
        result = InitializeRelayInput();
        if (result != OSP_STATUS_OK) {
            LOG_Err("InitializeRelayInput failed (%d)", result);
        }
    }

    /* Initialize the micro-second per tick value */
    _relayTickUsec = OSPConfig::getConfigItemIntV(
                OSPConfig::PROTOCOL_RELAY_TICK_USEC,
                1,
                NULL);
    LOG_Info("Relay Ticks per us: %d", _relayTickUsec);

    return result;
}


/****************************************************************************************************
 * @fn      InitializeRelayInput
 *          Initializes the relay-fs interface and associated data structures.
 *
 ***************************************************************************************************/
static int32_t InitializeRelayInput( void )
{
    osp_char_t devname[256];
    osp_char_t *sysfs = NULL;
    unsigned int cpu;

    memset(devname, 0, sizeof(devname));

    _relay_fd = -1;

    for ( unsigned char i = 0; i < MAX_NUM_SENSORS_TO_HANDLE;++i){
        _deviceConfig[i].sysEnablePath.clear();
        _deviceConfig[i].sysDelayPath.clear();
    }

    //open up the real sensor drivers
    _relay_fd = openInputEventDeviceExt(_deviceRelayInputName.c_str(), devname);
    if (_relay_fd < 0) {
        LOG_Err("Unable to open relay input device with name %s",
                _deviceRelayInputName.c_str() );
        return OSP_STATUS_UNKNOWN_INPUT;
    }
    LOG_Info("Open relay input device with name %s",
             _deviceRelayInputName.c_str() );

    for (unsigned char i = 0; i < MAX_NUM_SENSORS_TO_HANDLE; ++i) {
        if (!_deviceConfig[i].uinputName.empty()) {
            if (OSPConfig::getNamedConfigItem(sensornames[i],
                                             OSPConfig::SENSOR_ENABLE_PATH)) {
                _deviceConfig[i].enableValue = OSPConfig::getNamedConfigItemIntV
                        ( sensornames[i],
                          OSPConfig::SENSOR_ENABLE_VALUE,
                          1);
                _deviceConfig[i].disableValue =  OSPConfig::getNamedConfigItemIntV( sensornames[i],
                                                                                   OSPConfig::SENSOR_DISABLE_VALUE, 1);
                if(asprintf(&sysfs, "/sys/class/sensor_relay/%s/%s",
                            _deviceConfig[i].uinputName.c_str(),
                            OSPConfig::getNamedConfigItem(sensornames[i],
                                                         OSPConfig::SENSOR_ENABLE_PATH))< 0) {
                    LOG_Err("asprintf call failed!");
                } else {
                    LOG_Info("Sysfs Enable Path: %s, %d, %d", sysfs, _deviceConfig[i].enableValue,
                             _deviceConfig[i].disableValue);
                    _deviceConfig[i].sysEnablePath.assign(sysfs);
                    free(sysfs);
                }
            }
            if (OSPConfig::getNamedConfigItem( sensornames[i],
                                              OSPConfig::SENSOR_DELAY_PATH)) {
                if(asprintf(&sysfs, "/sys/class/sensor_relay/%s/%s",
                            _deviceConfig[i].uinputName.c_str(),
                            OSPConfig::getNamedConfigItem( sensornames[i],
                                                          OSPConfig::SENSOR_DELAY_PATH)) < 0) {
                    LOG_Err("asprintf call failed!");
                } else {
                    LOG_Info("Sysfs Delay Path: %s", sysfs);
                    _deviceConfig[i].sysDelayPath.assign(sysfs);
                    free(sysfs);
                }
            }
        }
    }

    for (cpu = 0; (cpu < 16); ) {
        const RelayBufStatus_t dummyBufStatus = {0, 0, 0};

        sprintf(devname, "/sys/kernel/debug/sensor_relay_kernel%d", cpu);
        const int fileHandle = open(devname, O_RDONLY | O_NONBLOCK);

        if (fileHandle < 0) {
            if (cpu == 0) {
                LOG_Err("Couldn't open relay file %s: errcode = %s\n",
                        devname, strerror(errno));
            }
            break;
        }
        LOG_Info("cpu %d file", cpu);

        unsigned char *bufferP = (unsigned char *) mmap(
                    NULL,
                    sizeof(union sensor_relay_broadcast_node) *
                    SENSOR_RELAY_NUM_RELAY_BUFFERS, PROT_READ,
                    MAP_PRIVATE | MAP_POPULATE, fileHandle,
                    0);

        if(bufferP == MAP_FAILED)
        {
            LOG_Err("Couldn't mmap relay file cpu %d, subbuf_size (%d) * n_subbufs(%d), error = %s \n",
                    cpu,
                    (int)sizeof(union sensor_relay_broadcast_node),
                    SENSOR_RELAY_NUM_RELAY_BUFFERS,
                    strerror(errno));
            close(fileHandle);
            break;
        }

        LOG_Info("cpu %d mmap", cpu);

        sprintf(devname, "/sys/kernel/debug/sensor_relay_kernel%d.produced", cpu);
        const int producerFile = open(devname, O_RDONLY);
        if (producerFile < 0) {
            LOG_Err("Couldn't open control file %s\n", devname);
            break;
        }

        LOG_Info("cpu %d producer", cpu);

        sprintf(devname, "/sys/kernel/debug/sensor_relay_kernel%d.consumed", cpu);
        const int consumedFile = open(devname, O_RDWR);
        if (consumedFile < 0) {
            LOG_Err("Couldn't open control file %s\n", devname);
            break;
        }
        LOG_Info("cpu %d consumed", cpu);

        _relay_file.push_back(fileHandle);
        __relay_buffer.push_back(bufferP);
        _produced_file.push_back(producerFile);
        _consumed_file.push_back(consumedFile);
        _relayStatus.push_back(dummyBufStatus);

        _relayStatus[cpu].produced = 0;
        _relayStatus[cpu].consumed = 0;
        cpu++;
    }

    if (cpu == 0) {
        LOG_Err("No CPU relay files found");
        return OSP_STATUS_UNKNOWN_INPUT;
    }
    return OSP_STATUS_OK;
}

/****************************************************************************************************
 * @fn      _processRelayInput
 *          Thread entry function for relay input processing
 *
 ***************************************************************************************************/
static void *_processRelayInput(void *pData)
{
    LOG_Info("%s", __FUNCTION__);

    if (_relay_fd > 0) {
        _relayThreadActive = true;
    }

    //Main processing loop
    while (_relayThreadActive) {
        _relayReadAndProcessSensorData(_relay_fd, _resultReadyCallbacks);
    }

    LOG_Info("Relay thread exiting...");

    return 0;
}


/****************************************************************************************************
 * @fn      _relayReadAndProcessSensorData
 *          Helper routine for reading and handling sensor data coming via RelayFS
 *
 ***************************************************************************************************/
static void _relayReadAndProcessSensorData(int fd, OSPD_ResultDataCallback_t dataCallbacks[])
{
    fd_set readFdSet;
    fd_set excFdSet;
    int32_t bytesRead, nfds = 0;
    int32_t selectResult;
    struct input_event inputEvents[PROCESS_INPUT_EVT_THRES];

    //Since select() modifies its fdset, if the call is being used in a loop, then
    //the set must be re-initialized before each call
    FD_ZERO(&readFdSet);
    FD_ZERO(&excFdSet);

    FD_SET(_relay_fd, &readFdSet);
    FD_SET(_relay_fd, &excFdSet);
    nfds = MAX_NUM_FDS( nfds, _relay_fd);

    /* Wait to recieve data on the relay pipe */
    selectResult = select(nfds+1, &readFdSet, NULL, &excFdSet, NULL);
    if (!_relayThreadActive){
        return;
    }

    if (selectResult > 0) {
        //read and publish the events
        if (FD_ISSET(_relay_fd, &excFdSet) ) {
            LOG_Err("ERROR: exception on select of socket for relay");
        } else {
            //make sure this FD has data before trying to read
            if (FD_ISSET(_relay_fd, &readFdSet) ) {
                bytesRead = read(_relay_fd, &inputEvents[0], sizeof(struct input_event));
                if (bytesRead < 0){
                    LOG_Err("I/O read error on relay input device %d", _relay_fd);
                } else if (bytesRead == 0){
                    FD_CLR( _relay_fd, &readFdSet );
                    FD_CLR( _relay_fd, &excFdSet );
                } else {
                    if ((bytesRead % sizeof(struct input_event)) != 0) {
                        LOG_Err("partial event struct read. Would lose some samples!");
                    }
                    for (int i = 0; i < (bytesRead / sizeof(struct input_event)); i++) {
                        if (inputEvents[i].code == ABS_VOLUME) {
                            ProcessInputEventsRelay();
                        }
                    }
                }
            }
        }

    }
}


/****************************************************************************************************
 * @fn      ProcessInputEventsRelay
 *          Helper routine for processing sensor data coming via RelayFS
 *
 ***************************************************************************************************/
static void ProcessInputEventsRelay(void)
{
    size_t size;
    int32_t sensorIndex;

    for (unsigned int cpu = 0; cpu < _produced_file.size(); cpu++) {
        lseek(_produced_file[cpu], 0, SEEK_SET);
        if (read(_produced_file[cpu], &size,
                 sizeof(size)) < 0) {
            LOG_Info("Couldn't read from consumed file for cpu %d, exiting: errcode = %d: %s\n",
                     cpu,
                     errno,
                     strerror(errno));
            break;
        }
        _relayStatus[cpu].produced = size;

#if 0
        LOG_Info("wakeup  CPU %d produced %d consumed %d  \n",
                 cpu,
                 _relayStatus[cpu].produced,
                 _relayStatus[cpu].consumed);
#endif

        size_t bufidx, start_subbuf, subbuf_idx;

        size_t subbufs_consumed = 0;

        unsigned char *subbuf_ptr;

        size_t subbufs_ready = _relayStatus[cpu].produced - _relayStatus[cpu].consumed + 1;

        start_subbuf = _relayStatus[cpu].consumed % SENSOR_RELAY_NUM_RELAY_BUFFERS;

        if ((_relayStatus[cpu].produced == 0) && (_relayStatus[cpu].consumed == 0))
            subbufs_ready = 0;
#if 0
        LOG_Info("produced  %d   consumed %d\n",
                 _relayStatus[cpu].produced,
                 _relayStatus[cpu].consumed);
#endif

        for (bufidx = start_subbuf; subbufs_ready-- > 0 ; bufidx++) {
            subbuf_idx = bufidx % SENSOR_RELAY_NUM_RELAY_BUFFERS;
            subbuf_ptr = __relay_buffer[cpu] + (subbuf_idx * sizeof(union sensor_relay_broadcast_node));
            union  sensor_relay_broadcast_node *sensorNode = (union  sensor_relay_broadcast_node *) subbuf_ptr;

#if 0

            LOG_Info("relay_buffer[%d] index %d of %d\n"
                     "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n"
                     "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x\n",
                     cpu,
                     bufidx,
                     subbufs_ready,
                     subbuf_ptr[0],  subbuf_ptr[1],   subbuf_ptr[2],
                     subbuf_ptr[3],  subbuf_ptr[4],   subbuf_ptr[5],
                     subbuf_ptr[6],  subbuf_ptr[7],   subbuf_ptr[8],
                     subbuf_ptr[9],  subbuf_ptr[10],  subbuf_ptr[11],
                     subbuf_ptr[12], subbuf_ptr[13],  subbuf_ptr[14],
                     subbuf_ptr[15], subbuf_ptr[16],  subbuf_ptr[17],
                     subbuf_ptr[18], subbuf_ptr[19],  subbuf_ptr[20],
                     subbuf_ptr[21], subbuf_ptr[22],  subbuf_ptr[23]);
#endif

            if (_deviceConfig[sensorNode->sensorData.sensorId].uinputName.empty()) {
                subbufs_consumed++;
                continue;
            }

            sensorIndex = -1;

            switch (sensorNode->sensorData.sensorId) {
            case SENSOR_RELAY_SENSOR_ID_ACCELEROMETER:
                sensorIndex = ACCEL_INDEX;
                //LOG_Info("Accel Data %d", sensorNode->sensorData.sensorId);
                //strncpy(label,"RA", sizeof(label));
                //goto ProcessInputEventsCommon;
                break;

            case SENSOR_RELAY_SENSOR_ID_GYROSCOPE:
                sensorIndex = GYRO_INDEX;
                //strncpy(label,"RG", sizeof(label));
                //goto ProcessInputEventsCommon;
                break;

            case SENSOR_RELAY_SENSOR_ID_MAGNETOMETER:
                sensorIndex = MAG_INDEX;
                //strncpy(label,"RM", sizeof(label));
                //goto ProcessInputEventsCommon;
                break;

            default:
                LOG_Err("Invalid sensor id received from relay 0x%-2.2x",
                        sensorNode->sensorData.sensorId);
                subbufs_consumed++;
                continue;
                break;
            }

            //ProcessInputEventsCommon:
            if (_deviceConfig[sensorIndex].uinputName.empty()) {
                subbufs_consumed++;
                continue;
            }

            /* Axis unit conversions & result callbacks */
            OSPD_ThreeAxisData_t floatSensorData;
            uint64_t timeTicks = sensorNode->sensorData.TimeStamp;
            int64_t timeNsec = (int64_t)(_relayTickUsec * timeTicks * 1000);

            switch(sensorIndex) {
            case ACCEL_INDEX:
            case MAG_INDEX:
            case GYRO_INDEX:
                for (int eventCode = ABS_X; eventCode <= ABS_Z; eventCode++) {
                    int32_t axisIndex = 0;
                    axisIndex = _doAxisSwap(
                                eventCode,
                                sensorIndex ,
                                _deviceConfig[sensorIndex].swap);

                    floatSensorData.data[axisIndex].f =
                            _deviceConfig[sensorIndex].conversion[axisIndex]*
                            sensorNode->sensorData.Data[eventCode - ABS_X];
                    floatSensorData.timestamp.ll = timeNsec;
                }
                _sensorDataPublish(sensorIndex, &floatSensorData);
                break;

            default:
                //TODO Error Handling??
                LOG_Err("Bad Sensor Index!!");
                return;
            }

#if 0
#ifdef ANDROID
            switch(sensorIndex) {
            case ACCEL_INDEX:
            case MAG_INDEX:
            case GYRO_INDEX:
                LOG_Info("bufidx %-3.3d Sensor %-2.2x  TimeStamp 0x%-8.8llx x 0x%-8.8x  y 0x%-8.8x  z 0x%-8.8x tv_usec %-20.6f   \n",
                         bufidx,
                         sensorNode->sensorData.sensorId,
                         sensorNode->sensorData.TimeStamp,
                         sensorNode->sensorData.Data[0],
                         sensorNode->sensorData.Data[1],
                         sensorNode->sensorData.Data[2],
                         timeUsec / 1000000.0);

                LOG_Info("{!%s, %20.6f , %10.6f , %10.6f , %10.6f, 0 ,!}\n",
                         label,
                         (float) timeUsec / (float) 1000000.0,
                         (float)_sfloatSensorData[sensorIndex][0],
                         (float)_sfloatSensorData[sensorIndex][1],
                         (float)_sfloatSensorData[sensorIndex][2]);
                break;

            default:
                LOG_Info("bufidx %-3.3d Sensor %-2.2x  TimeStamp 0x%-8.8llx value 0x%-8.8x  tv_usec %-20.6f   \n",
                         bufidx,
                         sensorNode->sensorData.sensorId,
                         sensorNode->sensorData.TimeStamp,
                         sensorNode->sensorData.Data[0],
                         timeUsec / 1000000.0);

                LOG_Info("{!%s, %20.6f , %10.6f , 0 ,!}\n",
                         label,
                         (float) timeUsec / (float) 1000000.0,
                         (float)_sfloatSensorData[sensorIndex][0]);
                break;
            }
#endif
#endif


            //            _vsDevMgr.publish(_deviceConfig[sensorIndex].repubFd,
            //                              &sensorNode->sensorData.Data[0],
            //                              lastSensorEvent - ABS_X + 1,
            //                              timeTicks);
#if 0
            SensorIndexToInputProducerMap::iterator iIndexToProducer;
            iIndexToProducer= _sensorIndexToInputProducerMap.find(sensorIndex);

            if(iIndexToProducer != _sensorIndexToInputProducerMap.end() ) {
                InputProducerInterface* pProducer= iIndexToProducer->second;

                if (pProducer) {
                    switch (pProducer->getType()) {
                    case PRODUCER_IS_THREEAXIS:
                    {
                        dynamic_cast<ThreeAxisSensorProducer*>(pProducer)->SetDataByFloat
                                (eventTimeDbl,
                                 _sfloatSensorData[sensorIndex],
                                 3);
                    }
                        break;
                    case PRODUCER_IS_STEP_DATA:
                    {
                        NTTIME eventTime = TOFIX_TIME(eventTimeDbl);
                        StepData_t data;

                        data.startTime = eventTime;
                        data.stopTime =  eventTime;
                        data.stepLength = 0;
                        data.stepFrequency = 0;
                        data.numStepsTotal = _numStepsTotal;
                        data.numStepsSinceWalking = 0;
                        data.numStepsUp = 0;
                        data.numStepsDown = 0;
                        data.numStepsLevel = 0;
                        dynamic_cast<StepDataProducer*>(pProducer)->SetData(data);
                    }
                        break;
                    case PRODUCER_IS_QUATERNION:
                    {
                        NTTIME eventTime = TOFIX_TIME(eventTimeDbl);
                        Quat quat;
                        quat <<
                                (float)_sfloatSensorData[sensorIndex][0],
                                (float)_sfloatSensorData[sensorIndex][1],
                                (float)_sfloatSensorData[sensorIndex][2],
                                (float)_sfloatSensorData[sensorIndex][3];

                        AttitudeData data(eventTime, quat, 0.0f, 0.0f);

                        dynamic_cast<AttitudeProducer*>(pProducer)->SetData(data);
                    }
                        break;
                    default:
                        LOG_Err("No mapping found for producer at sensor index %d as PRODUCER_IS_THREEAXIS nor PRODUCER_IS_STEP_DATA",
                                sensorIndex );
                        break;
                    }
                } else {
                    LOG_Err("No mapping found for producer at sensor index %d",
                            sensorIndex );
                }
            } else {
                LOG_Err("No mapping found for producer at sensor index %d",
                        sensorIndex );
            }
#endif
            subbufs_consumed++;
        }

        if (subbufs_consumed) {
            if (subbufs_consumed == SENSOR_RELAY_NUM_RELAY_BUFFERS)
                LOG_Err("cpu %d buffer full.  Consider using a larger buffer size", cpu);
            if (subbufs_consumed > _relayStatus[cpu].max_backlog)
                _relayStatus[cpu].max_backlog = subbufs_consumed;

            _relayStatus[cpu].consumed += subbufs_consumed;
#if 0
# ifdef ANDROID
            LOG_Info("cpu %d consumed %d\n", cpu, subbufs_consumed);
# endif
#endif
            if (write(_consumed_file[cpu], &subbufs_consumed, sizeof(subbufs_consumed)) < 0) {
                LOG_Err("Couldn't write to consumed file for cpu %d, exiting: errcode = %d: %s",
                        cpu, errno, strerror(errno));
                exit(1);
            }
            subbufs_consumed = 0;
        }
    }
}


/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C     F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/****************************************************************************************************
 * @fn      OSPD_Initialize
 *          Initialize remote procedure call for the daemon
 *
 ***************************************************************************************************/
OSP_STATUS_t OSPD_Initialize(void) {
    OSP_STATUS_t result = OSP_STATUS_OK;
    //int tick_us = 24;
    LOGT("%s\r\n", __FUNCTION__);

    _deviceRelayInputName.assign("");
    _deviceConfig[ACCEL_INDEX].uinputName.assign("");
    _deviceConfig[MAG_INDEX].uinputName.assign("");
    _deviceConfig[GYRO_INDEX].uinputName.assign("");

    /* Setup default configuration for testing */
    //OSPConfig::setConfigItem(OSPConfig::PROTOCOL_RELAY_DRIVER, "sensor_relay_kernel");
    //OSPConfig::setConfigItemInt(OSPConfig::PROTOCOL_RELAY_TICK_USEC, &tick_us, 1);
    OSPConfig::establishDefaultConfig("relay");

    /* Dump config for debug */
    OSPConfig::dump("/data/tmp/config-dump.txt");

    result = Initialize();
    if (result != OSP_STATUS_OK) {
        LOG_Err("Initialize failed (%d)", result);

    } else if ( pthread_create(&_relayThread, NULL, _processRelayInput, NULL )!= 0) {
        LOG_Err("Unable to create relay input processing thread\n");
        return OSP_STATUS_ERROR;
    }
    return result;
}

/****************************************************************************************************
 * @fn      OSPD_GetVersion
 *          Helper routine for getting daemon version information
 *
 ***************************************************************************************************/
OSP_STATUS_t OSPD_GetVersion(char* versionString, int bufSize) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}

/****************************************************************************************************
 * @fn      OSPD_SubscribeResult
 *          Enables subscription for results
 *
 ***************************************************************************************************/
OSP_STATUS_t OSPD_SubscribeResult(uint32_t sensorType, OSPD_ResultDataCallback_t dataReadyCallback ) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    _resultReadyCallbacks[sensorType] = dataReadyCallback;

    return result;
}

/****************************************************************************************************
 * @fn      OSPD_UnsubscribeResult
 *          Unsubscribe from sensor results
 *
 ***************************************************************************************************/
OSP_STATUS_t OSPD_UnsubscribeResult(uint32_t sensorType) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    _resultReadyCallbacks[sensorType] = NULL;

    return result;
}


/****************************************************************************************************
 * @fn      OSPD_Deinitialize
 *          Tear down RPC interface function
 *
 ***************************************************************************************************/
OSP_STATUS_t OSPD_Deinitialize(void) {
    int threadStatus;
    OSP_STATUS_t result = OSP_STATUS_OK;
    LOGT("%s\r\n", __FUNCTION__);

    _relayThreadActive = false;
    pthread_join(_relayThread, (void**) &threadStatus);
    return result;
}


/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
