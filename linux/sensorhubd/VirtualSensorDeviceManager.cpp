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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <climits>

#include <unistd.h>
#include <fcntl.h>

#include <linux/input.h>
#include <linux/types.h>
#include <math.h>

#include "uinput.h" //this is our local one b/c android doesn't include all the #defines we want

#include <errno.h>
#include "DebugLog.h"
#include "VirtualSensorDeviceManager.h"


#ifndef BUS_VIRTUAL
#define BUS_VIRTUAL 6
#endif // BUS_VIRTUAL

VirtualSensorDeviceManager::VirtualSensorDeviceManager( const int sleepus ):
    _sleepus(sleepus)
{
}

VirtualSensorDeviceManager::~VirtualSensorDeviceManager()
{
    for(std::vector<int>::iterator it = _deviceFds.begin(); it != _deviceFds.end(); ++it) {
        ioctl(*it, UI_DEV_DESTROY);
        close(*it);
    }
}


int VirtualSensorDeviceManager::createSensor(const char* name, const char* physname,
                                             int absMin, int absMax) {
    int result =-1;
    int status =-1;
    struct uinput_user_dev virtualSensorDev;

    //struct uinput_user_dev virtualSensorDev;
    memset(&virtualSensorDev, 0, sizeof(struct uinput_user_dev));
    result= open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (result < 0)
        return -1;
    strcpy(virtualSensorDev.name, name);

    virtualSensorDev.id.bustype = BUS_VIRTUAL;
    virtualSensorDev.id.vendor = 0x0591;
    virtualSensorDev.id.product = 0x1483;
    virtualSensorDev.id.version = 1;

    virtualSensorDev.absmin[ABS_X] = absMin;
    virtualSensorDev.absmax[ABS_X] = absMax;
    virtualSensorDev.absfuzz[ABS_X] = 0;
    virtualSensorDev.absflat[ABS_X] = 0;
    virtualSensorDev.absmin[ABS_Y] = absMin;
    virtualSensorDev.absmax[ABS_Y] = absMax;
    virtualSensorDev.absfuzz[ABS_Y] = 0;
    virtualSensorDev.absflat[ABS_Y] = 0;
    virtualSensorDev.absmin[ABS_Z] = absMin;
    virtualSensorDev.absmax[ABS_Z] = absMax;
    virtualSensorDev.absfuzz[ABS_Z] = 0;
    virtualSensorDev.absflat[ABS_Z] = 0;

    status= write(result, &virtualSensorDev, sizeof(struct uinput_user_dev));
    fatalErrorIf(status != sizeof(struct uinput_user_dev), -1, "error setIup\n");


    status= ioctl(result, UI_SET_EVBIT, EV_ABS);
    fatalErrorIf(status < 0, -1, "error evbit abs\n");

    status= ioctl(result, UI_SET_EVBIT, EV_REL);
    fatalErrorIf(status < 0, -1, "error evbit rel\n");

    status= ioctl(result, UI_SET_EVBIT, EV_SYN);
    fatalErrorIf(status < 0, -1, "error evbit syn\n");


    for(int i = ABS_X; i <= ABS_MAX; i++) {
        status= ioctl(result, UI_SET_ABSBIT, i);
        fatalErrorIf(status < 0, -1, "error setabsbit %d\n");
    }

    for(int i = REL_X; i <= REL_MAX; i++) {
        status= ioctl(result, UI_SET_RELBIT, i);
        fatalErrorIf(status < 0, -1, "error setrelbit %d\n");
    }

    //Set physical path name
    status = ioctl(result, UI_SET_PHYS, physname);
    fatalErrorIf(status < 0, -1, "error set phys\n");

    status= ioctl(result, UI_DEV_CREATE);
    if (status < 0){
        LOGE("Error on dev crate: %s", strerror(errno));
    }
    fatalErrorIf(status < 0, -1, "error create \n");

    _deviceFds.push_back(result);

    return result;

}
void VirtualSensorDeviceManager::publish(int deviceFd, input_event event) {
    int status;

    status= write(deviceFd, &event, sizeof(event));
    fatalErrorIf(status != sizeof(event), -1, "Error on send_event");
}

void VirtualSensorDeviceManager::publish(int deviceFd, int* data,
                                         const unsigned int* const timeInMillis) {
    struct input_event event;
    int status;

    memset(&event, 0, sizeof(event));

    event.type = EV_ABS;

    for (int i = 0; i < 3; i++) {
        event.code = ABS_X + i;
        event.value = data[i];
        gettimeofday(&event.time, NULL);
        status= write(deviceFd, &event, sizeof(event));
        fatalErrorIf(status != sizeof(event), -1, "Error on send_event");
    }

    if (timeInMillis){
        event.code = ABS_MISC;
        memcpy(&event.value, timeInMillis, sizeof(event.value));//copying unsigned to signed here
        status= write(deviceFd, &event, sizeof(event));
        fatalErrorIf(status != sizeof(event), -1, "Error on send_event");
    }

    memset(&event, 0, sizeof(event));
    event.type = EV_SYN;
    status= write(deviceFd, &event, sizeof(event));
    fatalErrorIf(status != sizeof(event), -1, "Error on send_event");

    return;
}

void VirtualSensorDeviceManager::publish(int deviceFd, const int32_t data[],
                                         const int64_t timeNanoSec, int numAxis) {
    struct input_event event;
    int status;
    int i;

    memset(&event, 0, sizeof(event));

    event.type = EV_ABS;
    for (i = 0; i < numAxis; i++) {
        event.code = ABS_X + i;
        event.value = data[i];
        status= write(deviceFd, &event, sizeof(event));
        fatalErrorIf(status != sizeof(event), -1, "Error on send_event");
    }

    memset(&event, 0, sizeof(event));

    event.type = EV_ABS;
    event.code = ABS_MISC;
    event.value = (uint32_t)(timeNanoSec & 0xFFFFFFFF);
    status= write(deviceFd, &event, sizeof(event));
    fatalErrorIf(status != sizeof(event), -1, "Error on send_event");

    memset(&event, 0, sizeof(event));

    event.type = EV_SYN;
    event.value = (uint32_t)(timeNanoSec >> 32);
    status= write(deviceFd, &event, sizeof(event));
    fatalErrorIf(status != sizeof(event), -1, "Error on send_event");

    return;
}




//! \todo this should call a registered error callback not give an exit code!
void VirtualSensorDeviceManager::fatalErrorIf(bool condition, int code, const char* msg) {
    if (condition) {
        LOGE("%s\n", msg);
        exit(code);
    }
}



