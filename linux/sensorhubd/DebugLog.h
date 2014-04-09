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
#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <android/log.h>

//// Macros

#ifdef NDEBUG
# define LOGI(...)
# define LOGE(...)
# define LOGT(...)  /* Function trace log */
# define LOGS(...)  /* Sensor data log */
#else
# define LOGT(...) fprintf(stderr, __VA_ARGS__)
# define LOGS(...) /* printf(__VA_ARGS__) */
# define LOGE(...)   {\
    __android_log_print(ANDROID_LOG_ERROR, "YSSH", __VA_ARGS__); \
    }
# define LOGI(...)   {\
    __android_log_print(ANDROID_LOG_INFO, "YSSH", __VA_ARGS__); \
    }
#endif

#endif // DEBUGLOG_H
