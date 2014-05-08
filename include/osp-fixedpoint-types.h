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
#ifndef _FIXEDPOINTTYPES_H_
#define _FIXEDPOINTTYPES_H_

/*-------------------------------------------------------------------------------------------------*\
 |    I N C L U D E   F I L E S
\*-------------------------------------------------------------------------------------------------*/
#include "osp-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------------------------------*\
 |    C O N S T A N T S   &   M A C R O S
\*-------------------------------------------------------------------------------------------------*/
/* for fixed point
 *
 *  Fixed point = S16Q12
 *  Fixed point precise = S32Q24
 *  Fixed point time = S64Q24
 */

#define K_NUM_FIXED_POINT_BITS          16
#define QFIXEDPOINT                     12
#define QFIXEDPOINTPRECISE              24
#define QFIXEDPOINTEXTENDED             12
#define QFIXEDPOINTTIME                 24
#define QFIXEDPOINTDELTATIME            24
#define FIXEDPOINT_ROUNDING_VALUE       (2048)


/*
 *  Conversion between different formats
 *
 */

/* Float to fixed point */
#define TOFIX(x)  ((NT)(((float)x) * (float)(1UL << QFIXEDPOINT) ))

/* float to fixed point precise */
#define CONST_PRECISE(x) ((NTPRECISE)(((float)x) * (float)(1UL << QFIXEDPOINTPRECISE) ))
#define TOFIX_PRECISE(x) ((NTPRECISE)(((float)x) * (float)(1UL << QFIXEDPOINTPRECISE) ))


/* Float to fixed point time */
#define TOFIX_TIME(x) ((NTTIME)(((double)x) * (double)(1UL << QFIXEDPOINTTIME)))

/* Conversion from float to time coefficient, valid range is between 0 and 0.999999 */
#define TOFIX_TIMECOEFFICIENT(x) ((uint32_t)(((double)x) * (double)(4294967296.0) ))

/* Float to fixed point extended */
#define CONST_EXTENDED(x) ((NTEXTENDED)(((float)x) * (float)(1UL << QFIXEDPOINTEXTENDED) ))
#define TOFIX_EXTENDED(x) ((NTEXTENDED)(((float)x) * (float)(1UL << QFIXEDPOINTEXTENDED) ))

/* fixed point number of arbitrary q to a floating point number  */
#define TOFLT_CUSTOM(x,q) ((float)(x) / (float)(1UL << (q)))

/* to floating point from fixed point */
#define TOFLT(x) ((float)(x) / (float)(1 << QFIXEDPOINT))

/* to floating point from fixed point precise */
#define TOFLT_PRECISE(x) ((float)(x) / (float)(1UL << QFIXEDPOINTPRECISE))

/* to floating point from fixed point time */
#define TOFLT_TIME(x) ((double)(x) / (double)(1UL << QFIXEDPOINTTIME))

/*to floating point from fixed point extended */
#define TOFLT_EXTENDED(x) ((float)(x) / (float)(1UL << QFIXEDPOINTEXTENDED))

#define ABS(x) ( ( x ) < 0 ? ( ((x)== -(x))? -(x+1) : - ( x ) ) : ( x ) )

/*-------------------------------------------------------------------------------------------------*\
 |    T Y P E   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/
typedef int16_t NT; /* 16 bit, Q12 fixed point! */
typedef int32_t NTPRECISE; /* 32 bit, Q24 fixed point */
typedef int32_t NTDELTATIME; /* 32 bit, Q24 fixed point.  Useful when you're just working with time deltas and only want to use a 32 bit number  */
typedef int32_t NTEXTENDED; /* 32 bit, Q12 fixed point */
typedef uint32_t TIMECOEFFICIENT; /* U32Q32 which is a coefficient to convert counter tics to NTTIME */

typedef int64_t NTTIME; /* 64 bit, Q24 fixed point */

/*-------------------------------------------------------------------------------------------------*\
 |    E X T E R N A L   V A R I A B L E S   &   F U N C T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   V A R I A B L E S   D E F I N I T I O N S
\*-------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------*\
 |    P U B L I C   F U N C T I O N   D E C L A R A T I O N S
\*-------------------------------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif
