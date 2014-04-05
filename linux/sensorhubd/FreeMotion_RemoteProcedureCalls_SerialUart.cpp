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
#include "FreeMotion_RemoteProcedureCalls.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string>

//// Macros
#define LOGE(...) fprintf(stderr, __VA_ARGS__)
#define LOGW(...) fprintf(stderr, __VA_ARGS__)

#ifdef NDEBUG
  #define LOGI(...)
  #define LOGD(...)
  #define LOGT(...)
  #define LOGS(...)
#else
  #define LOGI(...) printf(__VA_ARGS__)
  #define LOGD(...) printf(__VA_ARGS__)
  #define LOGT(...) fprintf(stderr, __VA_ARGS__)
  #define LOGS(...) printf(__VA_ARGS__)
#endif

///// Private Data
static pthread_t _serialThread;
static volatile bool _serialThreadActive = false;

static FMRPC_ResultDataCallback_t _resultReadyCallbacks[COUNT_OF_SENSOR_TYPES] = {0};

///// Private Methods
static void* _processSerialDevice(void* data);
static void _serialReadAndProcessSensorData(int openFd, FMRPC_ResultDataCallback_t dataCallbacks[]);
static int _serialDeviceInit(const char* deviceName, const int baud_rate);


///// Implementations
OSP_STATUS_t FMRPC_Initialize(void) {
    OSP_STATUS_t result = OSP_STATUS_OK;
    LOGT("%s\r\n", __FUNCTION__);

    if ( pthread_create(&_serialThread, NULL, _processSerialDevice, NULL )!= 0) {
        LOGE("Unable to create serial device processing thread\n");
        return -1;
    }


    return result;
}

OSP_STATUS_t FMRPC_GetVersion(char* versionString, int bufSize) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

    return result;
}

OSP_STATUS_t FMRPC_SubscribeResult(uint32_t sensorType, FMRPC_ResultDataCallback_t dataReadyCallback ) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);
//    _subscribeMask|= _enableMasks[resultType];

    _resultReadyCallbacks[sensorType]= dataReadyCallback;

    return result;
}

OSP_STATUS_t FMRPC_UnsubscribeResult(uint32_t sensorType) {
    OSP_STATUS_t result = OSP_STATUS_OK;

    LOGT("%s\r\n", __FUNCTION__);

//    _subscribeMask&= ~(_enableMasks[resultType]);

    _resultReadyCallbacks[sensorType]= NULL;

    return result;
}


OSP_STATUS_t FMRPC_Deinitialize(void) {
    int threadStatus;
    OSP_STATUS_t result = OSP_STATUS_OK;
    LOGT("%s\r\n", __FUNCTION__);

    _serialThreadActive= false;
    pthread_join(_serialThread, (void**) &threadStatus );

    return result;
}

static void* _processSerialDevice(void* data)
{
    const char* deviceName = "/dev/ttyUSB0"; //For Prolific USB-Uart
    const int baudRate= B230400;
    FMRPC_ThreeAxisData_t dummyData ={0};
    LOGT("%s\r\n", __FUNCTION__);

    //Open the device
    int serialFd= _serialDeviceInit(deviceName, baudRate);
    if (serialFd >= 0) {
        _serialThreadActive= true;
        /* Send indication to SH that we are ready! */
        write(serialFd, "syn=0\r\n",7);
    }

    //Main processing loop
    while (_serialThreadActive) {
        _serialReadAndProcessSensorData(serialFd, _resultReadyCallbacks);
    }
    LOGI("serial thread exiting...\n");

    return 0;
}

static char * _pStrTokState =NULL;

static void _parseThreeAxisAndPublish(uint32_t sensorType, FMRPC_ResultDataCallback_t dataCallbacks[], int numExpectedArgs =3) {
    FMRPC_ThreeAxisData_t parsedData= {0};
    const char delimiters[] = " ,";
    char * pToken;

    //get the timestamp first
    pToken = strtok_r(NULL, delimiters, &_pStrTokState);
    parsedData.timestamp= strtod(pToken, NULL);

    //now grab out the data
    for (int i=0; i<numExpectedArgs; ++i) {
        pToken = strtok_r(NULL, delimiters, &_pStrTokState);
        parsedData.data[i]= strtod(pToken, NULL);
        LOGS("%s->data[%d] '%s' as flt %f\n", __FUNCTION__, i, pToken, parsedData.data[i]);
    }

    if (dataCallbacks[sensorType] != NULL) {
        dataCallbacks[sensorType](sensorType, &parsedData);
    }

}

#define SERIAL_BUFFER_SIZE 255
static int _serialReadLine( const int fd, const char *&data){
    static std::string buffer;
    static std::string retval;
    static char line[SERIAL_BUFFER_SIZE];
    int pos;

    /* If there was stuff remaining in buffer, deal with it first */
    if ((pos = buffer.find("\r\n")) != std::string::npos){
        retval = buffer;
        retval.erase(retval.begin()+pos, retval.end());
        //int pos2;
        //if ((pos2 = retval.find("\r")) != std::string::npos){
        //    retval.erase(pos2);
        //}
        buffer.erase(0, pos+2);

        if(retval.size()){
            data = retval.c_str();
            fprintf(stdout, "#%s\n", retval.c_str());
            return 0;
        }
    }

    /* Try reading a new line now */
    while(1){
        int status;
        /* Blocking read, call returns on finding '\n' or full buffer */
        status = read( fd, line, SERIAL_BUFFER_SIZE-1);
        if (status > 0){
            line[status] = 0; /* Set end of string */
            buffer += line; //in case remaining buffer has half of a msg
        } else if ((status == 0) && (errno!= 0) && (errno != EAGAIN)){
            LOGE("I/O error on serial connection: %s\n",
                 strerror(errno));
            return OSP_STATUS_ERROR;
        } else if( status < 0) {
            LOGE( "I/O error reading device: %s\n", strerror(errno));
            return OSP_STATUS_ERROR;
        } else if (status == 0 && errno != EAGAIN){
            //write(fd, "\r\n\r\n",4);
            tcflush(fd, TCOFLUSH);
            // write(fd, "log=7E3\r\n",10);
            tcflush(fd, TCOFLUSH);
            usleep(500000);
        }

        if ((pos = buffer.find("\r\n")) != std::string::npos){
            retval = buffer;
            retval.erase(retval.begin()+pos, retval.end());
            //int pos2;
            //if ((pos2 = retval.find("\r")) != std::string::npos){
            //    retval.erase(pos2);
            //}
            buffer.erase(0,pos+2);

            if(retval.size()){
                data = retval.c_str();
                //fprintf(stderr, ":%s\n", data);
                break;
            }
        }
    }
    return 0;
}

static void _serialReadAndProcessSensorData(int fd, FMRPC_ResultDataCallback_t dataCallbacks[])
{
#define STRCMP_MATCH 0
#define ONLY_PARSE_ONE_VALUE 1
    const char *line= NULL;
    char workingMsg[SERIAL_BUFFER_SIZE];

    //Read
    _serialReadLine(fd, line);

    if (!line) {
        return;
    }

    //Parse
    const char delimiters[] = "{ ,!";
    char *pToken;

    if (line[0] != '{') {
        struct timespec sysTime;
        char sanitizedMsg[60];
        int len;
        strncpy(sanitizedMsg, (const char*)line, 60);
        LOGI("skipping: %s\n", sanitizedMsg);
        fprintf(stdout, ":: %s\n", sanitizedMsg);
        if ((line[0] == '#') && (line[1] == '$') && (line[2] == '*')) {
            clock_gettime(CLOCK_MONOTONIC, &sysTime);
            //clock_gettime(CLOCK_REALTIME, &sysTime);
            len = snprintf(sanitizedMsg, sizeof(sanitizedMsg),
                     "syn=%X%08X\r\n", sysTime.tv_sec, sysTime.tv_nsec);
            write(fd, sanitizedMsg, len);
        }
        return;
    }

    strncpy(workingMsg, (char*)line, SERIAL_BUFFER_SIZE);
    pToken = strtok_r (workingMsg, delimiters, &_pStrTokState);

    if (strcmp("RA", pToken) == STRCMP_MATCH) {
        _parseThreeAxisAndPublish(SENSOR_TYPE_ACCELEROMETER, dataCallbacks);

    } else if (strcmp("RM", pToken)  == STRCMP_MATCH) {
        _parseThreeAxisAndPublish(SENSOR_TYPE_MAGNETIC_FIELD, dataCallbacks);

    } else if (strcmp("RG", pToken)  == STRCMP_MATCH) {
        _parseThreeAxisAndPublish(SENSOR_TYPE_GYROSCOPE, dataCallbacks);

    } else if (strcmp("SIGM", pToken)  == STRCMP_MATCH) {
        _parseThreeAxisAndPublish(SENSOR_TYPE_SIGNIFICANT_MOTION, dataCallbacks, ONLY_PARSE_ONE_VALUE);

    } else if (strcmp("STC", pToken)  == STRCMP_MATCH) {
        _parseThreeAxisAndPublish(SENSOR_TYPE_STEP_COUNTER, dataCallbacks, ONLY_PARSE_ONE_VALUE);

    } else {
        char sanitizedMsg[60];
        strncpy(sanitizedMsg, (const char*)line, 60);
        LOGE("parse error on token '%s' in: %s\n", pToken, sanitizedMsg);
        return;
    }


}

static int _serialDeviceInit(const char* deviceName, const int baud_rate)
{
    LOGT("%s\r\n", __FUNCTION__);
    int fd;

    fd = open(deviceName,  O_RDWR | O_NOCTTY);
    if (fd < 0) {
        LOGE("open failed on %s", deviceName);
        return OSP_STATUS_ERROR;
    }

    //open in cooked mode for ASCII
    struct termios options;

    // Get the current options for the port.
    //tcgetattr(fd, &options);
    bzero(&options, sizeof(options));
    options.c_cflag = baud_rate | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;

    /* Enable canonical input */
    options.c_lflag = ICANON;

    /*
      initialize all control characters
      default values can be found in include/termios.h, and are given
      in the comments, but we don't need them here
    */
    options.c_cc[VINTR]    = 0;     /* Ctrl-c */
    options.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
    options.c_cc[VERASE]   = 0;     /* del */
    options.c_cc[VKILL]    = 0;     /* @ */
    options.c_cc[VEOF]     = 0;     /* Ctrl-d */
    options.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    options.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
    options.c_cc[VSWTC]    = 0;     /* '\0' */
    options.c_cc[VSTART]   = 0;     /* Ctrl-q */
    options.c_cc[VSTOP]    = 0;     /* Ctrl-s */
    options.c_cc[VSUSP]    = 0;     /* Ctrl-z */
    options.c_cc[VEOL]     = 0;     /* '\0' */
    options.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    options.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    options.c_cc[VWERASE]  = 0;     /* Ctrl-w */
    options.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
    options.c_cc[VEOL2]    = 0;     /* '\0' */

    // Flush the input buffer
    tcflush(fd, TCIFLUSH);
    // And set the new options for the port.
    if (tcsetattr(fd, TCSANOW, &options) != 0){
        LOGE("ERROR setting serial properties\n");
        return OSP_STATUS_ERROR;
    }

    return fd;
}
