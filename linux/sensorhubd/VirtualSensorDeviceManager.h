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
#ifndef VIRTUALSENSORDEVICEMANAGER_H
#define VIRTUALSENSORDEVICEMANAGER_H

#include <stdint.h>
#include <vector>
#include <linux/input.h>

//! manages the lifecycle of virtual sensor device file descriptors
class VirtualSensorDeviceManager
{
public:
    VirtualSensorDeviceManager( const int sleepus = 10000);
    ~VirtualSensorDeviceManager();

    int createSensor(const char* name, const char* physname, int absMin =-2048,
                     int absMax =2047);
    void publish(int deviceFd, input_event data);
    void publish(int deviceFd, int* data,
                 const unsigned int* const timeInMillis = 0);
    void publish(int deviceFd, const int32_t data[],
                 const int64_t time64, int numAxis=3);

protected:

    void fatalErrorIf(bool condition, int code, const char* msg);

private:
    std::vector<int> _deviceFds;
    const int _sleepus;
};

#endif // VIRTUALSENSORDEVICEMANAGER_H
