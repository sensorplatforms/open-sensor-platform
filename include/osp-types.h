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
#if !defined (OSP_TYPES_H)
#define   OSP_TYPES_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include <stdint.h> /* Try to use std int types in the application code */

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#define NULLP               ((void *) 0)
#ifndef true
# define true               (1 == 1)
#endif
#ifndef false
# define false              (!true)
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE (!TRUE)
#endif

/* Error codes (corresponding to osp_status_t) */
#define OSP_STATUS_OK                    0
#define OSP_STATUS_IDLE                  1
#define OSP_STATUS_ERROR                -1
#define OSP_STATUS_DESCRIPTOR_INVALID   -2
#define OSP_STATUS_ALREADY_REGISTERED   -3
#define OSP_STATUS_NO_MORE_HANDLES      -4
#define OSP_STATUS_NULL_POINTER         -5
#define OSP_STATUS_CAL_NOT_VALID        -6
#define OSP_STATUS_NOT_REGISTERED       -7
#define OSP_STATUS_ALREADY_SUBSCRIBED   -8
#define OSP_STATUS_UNKNOWN_REQUEST      -9
#define OSP_STATUS_INVALID_HANDLE       -10
#define OSP_STATUS_NOT_SUBSCRIBED       -11
#define OSP_STATUS_QUEUE_FULL           -12
#define OSP_STATUS_NOT_IMPLEMENTED      -13
#define OSP_STATUS_UNKNOWN_INPUT        -14


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Other types that are not provided by stdint */
typedef char            osp_char_t;
/* Note that char cannot be replaced by int8_t or uint8_t as most compiler treats
   char, signed char and unsigned char as different because char itself maybe signed or unsigned */
typedef unsigned char   osp_byte_t;
typedef double          osp_dbl_t;
typedef float           osp_float_t;

#ifdef __cplusplus
  typedef bool          osp_bool_t;
#else
  typedef char          osp_bool_t;
#endif

typedef unsigned short osp_size_t;
typedef int osp_status_t;

#endif /* OSP_TYPES_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
