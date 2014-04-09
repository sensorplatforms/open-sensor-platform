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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/input.h>


#include <unistd.h>
#include <fcntl.h>


int openInputEventDeviceExt(const char *inputName, char *actual) {
    int fd = -1;
    const char *dirname = "/dev/input";
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;
    char failure_msg[40];
    snprintf(failure_msg, 40, "Could not open device %s", inputName);
    dir = opendir(dirname);
    if(dir == NULL)
        return -1;
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while((de = readdir(dir))) {
        if((de->d_name[0] == '.') &&
                ((de->d_name[1] == '\0') ||
                 ((de->d_name[1] == '.') && (de->d_name[2] == '\0'))))
            continue;
        strcpy(filename, de->d_name);
        fd = open(devname, O_RDONLY);
        if (fd>=0) {
            char name[80];
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
                name[0] = '\0';
            }
            if (!strcmp(name, inputName)) {
                if (actual) strcpy(actual, filename);
                break;
            } else {
                close(fd);
                fd = -1;
            }
        }
    }
    closedir(dir);

    return fd;
}

int openInputEventDevice(const char* inputName) {
    return openInputEventDeviceExt(inputName, NULL);
}
