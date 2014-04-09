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
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <csignal>

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "DebugLog.h"
#include "VirtualSensorDeviceManager.h"
#include "FreeMotion_RemoteProcedureCalls.h"


#define MAX_NUM_FDS(x,y) ((x) > (y) ? (x) : (y))


//// Public


//// Constants
#define TWENTY_MS_IN_US (20000)
#define ENABLE_PIPE_NAME_TEMPLATE "/data/misc/osp-%s-enable"

////Private
static void _logErrorIf(bool condition, const char* msg);
static void _fatalErrorIf(bool condition, int code, const char* msg);
static void _handleQuitSignals(int signum);
static void _initialize();
static void _deinitialize();
static void _parseAndHandleEnable(int sensorIndex, char* buffer, ssize_t numBytesInBuffer);

typedef enum {
    SENSORHUBD_ACCELEROMETER_INDEX =0,
    SENSORHUBD_MAGNETOMETER_INDEX,
    SENSORHUBD_GYROSCOPE_INDEX,
    //SENSORHUBD_SIG_MOTION_INDEX,
    //SENSORHUBD_STEP_COUNTER_INDEX,
    SENSORHUBD_RESULT_INDEX_COUNT
} SensorIndices_t;

static VirtualSensorDeviceManager* _pVsDevMgr;
static int _evdevFds[SENSORHUBD_RESULT_INDEX_COUNT] ={-1};
static int _resultHandles[SENSORHUBD_RESULT_INDEX_COUNT] = {0};

static int _enablePipeFds[SENSORHUBD_RESULT_INDEX_COUNT] ={-1};
static const char* _sensorNames[SENSORHUBD_RESULT_INDEX_COUNT] = {
    "accel", "mag", "gyro", /*"sig-motion", "step-count"*/};
static uint32_t _fmResultCodes[SENSORHUBD_RESULT_INDEX_COUNT]= {
    SENSOR_TYPE_ACCELEROMETER,
    SENSOR_TYPE_MAGNETIC_FIELD,
    SENSOR_TYPE_GYROSCOPE,
    //SENSOR_TYPE_SIGNIFICANT_MOTION,
    //SENSOR_TYPE_STEP_COUNTER
};

void _onTriAxisSensorResultDataUpdate(uint32_t sensorType, void* pData);


//// Implementation

int main(int argc, char** argv) {
#define MS_TO_US 1000
    int result =0;
    fd_set readFdSet;
    fd_set errFdSet;
    int32_t maxNumFds = 0;
    int selectResult;

    //
    signal(SIGINT, _handleQuitSignals);

    //create this on the stack so we know it always gets cleaned up properly
    VirtualSensorDeviceManager vsDevMgr;
    _pVsDevMgr= &vsDevMgr;

    //After initialize, all the magic happens in the callbacks such as _onAccelerometerResultDataUpdate
    _initialize();

    /* This loop handles sensor enable/disable requests */
    // FROM WHERE??
    while (1) {

        // setup the select to read on all pipes
        FD_ZERO(&readFdSet);
        FD_ZERO(&errFdSet);

        for (int i=0; i < SENSORHUBD_RESULT_INDEX_COUNT; ++i) {
            FD_SET( _enablePipeFds[i], &readFdSet);
            FD_SET( _enablePipeFds[i], &errFdSet);
            maxNumFds = MAX_NUM_FDS( maxNumFds, _enablePipeFds[i]);
        }
        //Wait for data on one of the pipes
        selectResult = select(maxNumFds+1, &readFdSet, NULL, &errFdSet, NULL);
        _logErrorIf(selectResult<0, "error on select() of named pipes");

        if ( selectResult > 0 ) {

            for (int sensorIndex=0; sensorIndex < SENSORHUBD_RESULT_INDEX_COUNT; ++sensorIndex) {
                _logErrorIf(FD_ISSET(_enablePipeFds[sensorIndex], &errFdSet), "error on FD!\n");
                if (FD_ISSET(_enablePipeFds[sensorIndex], &readFdSet) ) {
                    char readBuf[255];
                    ssize_t bytesRead = read(_enablePipeFds[sensorIndex], readBuf, 255);
                    _logErrorIf(bytesRead < 0, "failed on read of enable pipe for");

                    FD_CLR(_enablePipeFds[sensorIndex], &readFdSet );
                    if (0 == bytesRead) {
                        char pipename[255];

                        //close and reopen the pipe or we'll spike CPU usage b/c select will always
                        //return available and we'll always get 0 bytes from read
                        close(_enablePipeFds[sensorIndex]);
                        snprintf(pipename, 255, ENABLE_PIPE_NAME_TEMPLATE, _sensorNames[sensorIndex]);
                        int fd = open(pipename, O_RDONLY|O_NONBLOCK);
                        _fatalErrorIf(fd < 0, -1, "could not open named pipe for reading");
                        _enablePipeFds[sensorIndex]= fd;

                        continue;
                    }
                    _parseAndHandleEnable(sensorIndex, readBuf, bytesRead);
                }
            }

        }

    }

    return result;
}

static void _logErrorIf(bool condition, const char* msg) {
    if (condition) {
        LOGE("%s\n", msg);
    }
}


void _fatalErrorIf(bool condition, int code, const char* msg) {
    if (condition) {
        LOGE("%s\n", msg);
        _deinitialize();
        _exit(code);
    }
}

void _parseAndHandleEnable(int sensorIndex, char* buffer, ssize_t numBytesInBuffer) {

    if (NULL == buffer) {
        LOGE("buffer should never be NULL!!!\n");
        return;
    }

    if (numBytesInBuffer < 0) {
        LOGE("not going to try parsing empty buffer\n");
        return;
    }
    if (numBytesInBuffer == 0) {
        return;
    }
    LOGT("%s: sensorIndex %d\r\n", __FUNCTION__, sensorIndex);

    if ('0' == buffer[0]) {
        LOGI("UNsubscribe to sensorcode %d\n", (int)sensorIndex);
        OSP_STATUS_t status= FMRPC_UnsubscribeResult(_fmResultCodes[sensorIndex]);
        _logErrorIf(status != OSP_STATUS_OK, "error unsubscribing from result\n");

    } else if ('1' == buffer[0]) {
        LOGI("SUBSCRIBE to sensorcode %d\n", (int)sensorIndex);
        OSP_STATUS_t status= FMRPC_SubscribeResult(_fmResultCodes[sensorIndex], _onTriAxisSensorResultDataUpdate);
        _logErrorIf(status != OSP_STATUS_OK, "error subscribing to result\n");

    } else {
        //LOGE("unexpected data in enable buffer: %s", buffer);
    }


}


void _initializeNamedPipes() {
    LOGT("%s:%d\r\n", __FUNCTION__, __LINE__);

    // create an enable pipe for each sensor type
    for (int i=0; i < SENSORHUBD_RESULT_INDEX_COUNT; ++i) {
        char pipename[255];
        int fd;

        snprintf(pipename, 255, ENABLE_PIPE_NAME_TEMPLATE, _sensorNames[i]);

        //Try and remove the pipe if it's already there, but don't complain if it's not
        unlink(pipename);

        _fatalErrorIf(mkfifo(pipename, 0666) != 0, -1, "could not create named pipe");

        fd = open(pipename, O_RDONLY|O_NONBLOCK);
        _fatalErrorIf(fd < 0, -1, "could not open named pipe for reading");
        _enablePipeFds[i]= fd;
    }

}

void _onTriAxisSensorResultDataUpdate(uint32_t sensorType, void* pData) {
#define SCALE_FOUR_DECIMAL_PLACES       (1000.0f)
#define NSEC_PER_SEC                    (1000000000.0f)

    FMRPC_ThreeAxisData_t* pSensorData= (FMRPC_ThreeAxisData_t*)pData;
    int32_t uinputCompatibleDataFormat[3];
    int64_t timeInNano =0;


    switch(sensorType)  {
    case SENSOR_TYPE_ACCELEROMETER:
        uinputCompatibleDataFormat[0]= (int)(pSensorData->data[0] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[1]= (int)(pSensorData->data[1] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[2]= (int)(pSensorData->data[2] * SCALE_FOUR_DECIMAL_PLACES);
        timeInNano= (int64_t)(NSEC_PER_SEC * pSensorData->timestamp);

        LOGS("RA %.3f (0x%8x), %.3f (0x%8x), %.3f (0x%8x), %lld\n",
             pSensorData->data[0], uinputCompatibleDataFormat[0],
             pSensorData->data[1], uinputCompatibleDataFormat[1],
             pSensorData->data[2], uinputCompatibleDataFormat[2],
             timeInNano);

        _pVsDevMgr->publish(_evdevFds[SENSORHUBD_ACCELEROMETER_INDEX], uinputCompatibleDataFormat, timeInNano);
        break;

    case SENSOR_TYPE_MAGNETIC_FIELD:
        LOGS("RM %.3f (0x%8x), %.3f (0x%8x), %.3f (0x%8x)\n", pSensorData->data[0], (int)(pSensorData->data[0] * SCALE_FOUR_DECIMAL_PLACES),
             pSensorData->data[1], (int)(pSensorData->data[1] * SCALE_FOUR_DECIMAL_PLACES),
             pSensorData->data[2], (int)(pSensorData->data[2] * SCALE_FOUR_DECIMAL_PLACES));

        uinputCompatibleDataFormat[0]= (int)(pSensorData->data[0] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[1]= (int)(pSensorData->data[1] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[2]= (int)(pSensorData->data[2] * SCALE_FOUR_DECIMAL_PLACES);
        timeInNano= (int64_t)(NSEC_PER_SEC * pSensorData->timestamp);

        _pVsDevMgr->publish(_evdevFds[SENSORHUBD_MAGNETOMETER_INDEX], uinputCompatibleDataFormat, timeInNano);
        break;

    case SENSOR_TYPE_GYROSCOPE:
        LOGS("RG %.3f (0x%8x), %.3f (0x%8x), %.3f (0x%8x)\n", pSensorData->data[0], (int)(pSensorData->data[0] * SCALE_FOUR_DECIMAL_PLACES),
             pSensorData->data[1], (int)(pSensorData->data[1] * SCALE_FOUR_DECIMAL_PLACES),
             pSensorData->data[2], (int)(pSensorData->data[2] * SCALE_FOUR_DECIMAL_PLACES));

        uinputCompatibleDataFormat[0]= (int)(pSensorData->data[0] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[1]= (int)(pSensorData->data[1] * SCALE_FOUR_DECIMAL_PLACES);
        uinputCompatibleDataFormat[2]= (int)(pSensorData->data[2] * SCALE_FOUR_DECIMAL_PLACES);
        timeInNano= (int64_t)(NSEC_PER_SEC * pSensorData->timestamp);

        _pVsDevMgr->publish(_evdevFds[SENSORHUBD_GYROSCOPE_INDEX], uinputCompatibleDataFormat, timeInNano);
        break;
#if 0
    case SENSOR_TYPE_SIGNIFICANT_MOTION:
        //LOGS("SIGM %.3f (0x%8x), %.3f (0x%8x), %.3f (0x%8x)\n", pSensorData->data[0]);

        uinputCompatibleDataFormat[0]= (int)(pSensorData->data[0]);
        timeInNano= (int64_t)(NSEC_PER_SEC * pSensorData->timestamp);

        _pVsDevMgr->publish(_evdevFds[SENSORHUBD_SIG_MOTION_INDEX], uinputCompatibleDataFormat, timeInNano, 1);
        break;
#endif
    default:
        LOGE("%s unexpected result type %d\n", __FUNCTION__, sensorType);
        break;
    }

}

static void _subscribeToAllResults() {
    OSP_STATUS_t status;
    LOGT("%s:%d\r\n", __FUNCTION__, __LINE__);

    status = FMRPC_SubscribeResult(SENSOR_TYPE_ACCELEROMETER, _onTriAxisSensorResultDataUpdate);
    _logErrorIf(status != OSP_STATUS_OK, "error subscribing to RESULT_UNCALIBRATED_ACCELEROMETER");

    status = FMRPC_SubscribeResult(SENSOR_TYPE_MAGNETIC_FIELD, _onTriAxisSensorResultDataUpdate);
    _logErrorIf(status != OSP_STATUS_OK, "error subscribing to RESULT_UNCALIBRATED_ACCELEROMETER");

    status = FMRPC_SubscribeResult(SENSOR_TYPE_GYROSCOPE, _onTriAxisSensorResultDataUpdate);
    _logErrorIf(status != OSP_STATUS_OK, "error subscribing to RESULT_UNCALIBRATED_GYRO");

    //    status = FMRPC_SubscribeResult(SENSOR_TYPE_SIGNIFICANT_MOTION, _onTriAxisSensorResultDataUpdate);
    //    _logErrorIf(status != OSP_STATUS_OK, "error subscribing to RESULT_SIGNIFICANT_MOTION");

    //    status = FMRPC_SubscribeResult(SENSOR_TYPE_STEP_COUNTER, _onTriAxisSensorResultDataUpdate);
    //    _logErrorIf(status != OSP_STATUS_OK, "error subscribing to RESULT_STEP_COUNTER");

}

static void _initialize() {
    OSP_STATUS_t status;
    static int retryCount =0;

    LOGT("%s:%d\r\n", __FUNCTION__, __LINE__);

    //create our raw input virtual sensors
    _evdevFds[SENSORHUBD_ACCELEROMETER_INDEX]= _pVsDevMgr->createSensor("osp-accelerometer", "acc0",  INT_MIN, INT_MAX);
    _evdevFds[SENSORHUBD_MAGNETOMETER_INDEX]= _pVsDevMgr->createSensor("osp-magnetometer", "mag0",  INT_MIN, INT_MAX);
    _evdevFds[SENSORHUBD_GYROSCOPE_INDEX]= _pVsDevMgr->createSensor("osp-gyroscope", "gyr0",  INT_MIN, INT_MAX);
    //_evdevFds[SENSORHUBD_SIG_MOTION_INDEX]= _pVsDevMgr->createSensor("osp-significant-motion", "sigm0",  INT_MIN, INT_MAX);
    //_evdevFds[SENSORHUBD_STEP_COUNTER_INDEX]= _pVsDevMgr->createSensor("osp-step-counter", "stc0",  INT_MIN, INT_MAX);

    //Initialize freeMotion
    LOGT("%s:%d\r\n", __FUNCTION__, __LINE__);
    status= FMRPC_Initialize();

    LOGT("%s:%d\r\n", __FUNCTION__, __LINE__);
    _fatalErrorIf(status!= OSP_STATUS_OK, status, "Failed on FreeMotion RPC Initialization!");

    //print out the FreeMotion version
    //char versionString[255];
    //FMRPC_GetVersion(versionString, 255);
    //LOGI("freeMotion version %s\n", versionString);

    _initializeNamedPipes();

    //!!! Debug only
    _subscribeToAllResults();

}


static void _stopAllResults() {
    LOGT("%s\r\n", __FUNCTION__);

    OSP_STATUS_t status= FMRPC_UnsubscribeResult(SENSOR_TYPE_ACCELEROMETER);
    _logErrorIf(status != OSP_STATUS_OK, "error Unsubscribing to RESULT_UNCALIBRATED_ACCELEROMETER");

    status= FMRPC_UnsubscribeResult(SENSOR_TYPE_MAGNETIC_FIELD);
    _logErrorIf(status != OSP_STATUS_OK, "error Unsubscribing to RESULT_UNCALIBRATED_MAGNETOMETER");

    status= FMRPC_UnsubscribeResult(SENSOR_TYPE_GYROSCOPE);
    _logErrorIf(status != OSP_STATUS_OK, "error Unsubscribing to RESULT_UNCALIBRATED_GYRO");

    status= FMRPC_UnsubscribeResult(SENSOR_TYPE_SIGNIFICANT_MOTION);
    _logErrorIf(status != OSP_STATUS_OK, "error unsubscribing to RESULT_SIGNIFICANT_MOTION");

    status= FMRPC_UnsubscribeResult(SENSOR_TYPE_STEP_COUNTER);
    _logErrorIf(status != OSP_STATUS_OK, "error unsubscribing to RESULT_STEP_COUNTER");
}

static void _deinitialize() {
    LOGT("%s\r\n", __FUNCTION__);

    _stopAllResults();

    FMRPC_Deinitialize();
}

static void _handleQuitSignals(int signum) {
    LOGT("%s\r\n", __FUNCTION__);

    _deinitialize();
    exit(0);
}
