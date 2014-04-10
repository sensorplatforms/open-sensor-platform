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
#ifndef FM_DATATYPES_H
#define FM_DATATYPES_H

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
/* We shall use STDINT types such as uint32_t, int8_t, etc. for integer types in all of our code. */
#include <stdint.h>

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE (!TRUE)
#endif


/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
/* Other types that are not provided by stdint */
typedef char            fm_char_t;
/* Note that char cannot be replaced by int8_t or uint8_t as most compiler treats
   char, signed char and unsigned char as different because char itself maybe signed or unsigned */
typedef unsigned char   fm_byte_t;
typedef double          fm_dbl_t;
typedef float           fm_float_t;

#ifdef __cplusplus
  typedef bool          fm_bool_t;
#else
  typedef char          fm_bool_t;
#endif

typedef unsigned short fm_size_t;
typedef int fm_status_t;

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#endif /* FM_DATATYPES_H */
/*-------------------------------------------------------------------------------------------------*\
 |    E N D   O F   F I L E
\*-------------------------------------------------------------------------------------------------*/
