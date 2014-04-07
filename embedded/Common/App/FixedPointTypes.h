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

#include "FM_DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef int16_t NT; /* 16 bit, Q12 fixed point! */
typedef int32_t NTPRECISE; /* 32 bit, Q24 fixed point */
typedef int32_t NTDELTATIME; /* 32 bit, Q24 fixed point.  Useful when you're just working with time deltas and only want to use a 32 bit number  */
typedef int32_t NTEXTENDED; /* 32 bit, Q12 fixed point */
typedef uint32_t TIMECOEFFICIENT; /* U32Q32 which is a coefficient to convert counter tics to NTTIME */

typedef int64_t NTTIME; /* 64 bit, Q24 fixed point */

/* for fixed point
 *
 *  Fixed point = S16Q12
 *  Fixed point precise = S32Q24
 *  Fixed point time = S64Q24
 */

#define K_NUM_FIXED_POINT_BITS (16)
#define QFIXEDPOINT 12
#define QFIXEDPOINTPRECISE 24
#define QFIXEDPOINTEXTENDED 12
#define QFIXEDPOINTTIME 24
#define QFIXEDPOINTDELTATIME 24


/*
 *  Conversion between different formats
 *
 */

/* Float to fixed point */
#define TOFIX(x)  ((NT)(((fm_float_t)x) * (fm_float_t)(1UL << QFIXEDPOINT) ))

/* fm_float_t to fixed point precise */
#define CONST_PRECISE(x) ((NTPRECISE)(((fm_float_t)x) * (fm_float_t)(1UL << QFIXEDPOINTPRECISE) ))
#define TOFIX_PRECISE(x) ((NTPRECISE)(((fm_float_t)x) * (fm_float_t)(1UL << QFIXEDPOINTPRECISE) ))


/* Float to fixed point time */
#define TOFIX_TIME(x) ((NTTIME)(((fm_dbl_t)x) * (fm_dbl_t)(1UL << QFIXEDPOINTTIME)))

/* Conversion from float to time coefficient, valid range is between 0 and 0.999999 */
#define TOFIX_TIMECOEFFICIENT(x) ((uint32_t)(((fm_dbl_t)x) * (fm_dbl_t)(4294967296.0) ))

/* Float to fixed point extended */
#define CONST_EXTENDED(x) ((NTEXTENDED)(((fm_float_t)x) * (fm_float_t)(1UL << QFIXEDPOINTEXTENDED) ))
#define TOFIX_EXTENDED(x) ((NTEXTENDED)(((fm_float_t)x) * (fm_float_t)(1UL << QFIXEDPOINTEXTENDED) ))

/* fixed point number of arbitrary q to a floating point number  */
#define TOFLT_CUSTOM(x,q) ((fm_float_t)(x) / (fm_float_t)(1UL << (q)))

/* to floating point from fixed point */
#define TOFLT(x) ((fm_float_t)(x) / (fm_float_t)(1 << QFIXEDPOINT))

/* to floating point from fixed point precise */
#define TOFLT_PRECISE(x) ((fm_float_t)(x) / (fm_float_t)(1UL << QFIXEDPOINTPRECISE))

/* to floating point from fixed point time */
#define TOFLT_TIME(x) ((fm_dbl_t)(x) / (fm_dbl_t)(1UL << QFIXEDPOINTTIME))

/*to floating point from fixed point extended */
#define TOFLT_EXTENDED(x) ((fm_float_t)(x) / (fm_float_t)(1UL << QFIXEDPOINTEXTENDED))

#ifdef __cplusplus
}
#endif

#endif
