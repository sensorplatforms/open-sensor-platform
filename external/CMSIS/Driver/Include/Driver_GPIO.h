/* -----------------------------------------------------------------------------
 * Copyright (c) 2015 Pinecone Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        8. June 2015
 * $Revision:    V1.00
 *
 * Project:      GPIO Driver definitions
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.00
 *    Initial release
 */

#ifndef __DRIVER_GPIO_H
#define __DRIVER_GPIO_H

#include "Driver_Common.h"

#define OSP_GPIO_API_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,00)

#define ARM_GPIO_DIR_INPUT       0
#define ARM_GPIO_DIR_OUTPUT      1

#define ARM_GPIO_TRIGGER_NONE    0
#define ARM_GPIO_TRIGGER_HIGH    1
#define ARM_GPIO_TRIGGER_LOW     2
#define ARM_GPIO_TRIGGER_RISING  3
#define ARM_GPIO_TRIGGER_FALLING 4
#define ARM_GPIO_TRIGGER_EDGE    5

typedef void (*ARM_GPIO_Handler_t) (uint32_t pin, void *data);

typedef struct _ARM_DRIVER_GPIO {
    ARM_DRIVER_VERSION (*GetVersion)   (void);
    int32_t            (*Initialize)   (void);
    int32_t            (*Uninitialize) (void);
    int32_t            (*PowerControl) (ARM_POWER_STATE state);
    int32_t            (*SetDirection) (uint32_t pin, uint32_t dir);
    int32_t            (*SetTrigger)   (uint32_t pin, uint32_t trigger);
    int32_t            (*WritePin)     (uint32_t pin, uint32_t value);
    int32_t            (*ReadPin)      (uint32_t pin);
    int32_t            (*SetHandler)   (uint32_t pin, ARM_GPIO_Handler_t handler, void *data);
} const ARM_DRIVER_GPIO;

#define ARM_BUILD_DRIVER_GPIO(pre1, pre2, priv) \
    static ARM_DRIVER_VERSION pre1##_GetVersion (void) { \
        return pre2##_GetVersion (priv); \
    } \
    static int32_t pre1##_Initialize (void) { \
        return pre2##_Initialize (priv); \
    } \
    static int32_t pre1##_Uninitialize (void) { \
        return pre2##_Uninitialize (priv); \
    } \
    static int32_t pre1##_PowerControl (ARM_POWER_STATE state) { \
        return pre2##_PowerControl (state, priv); \
    } \
    static int32_t pre1##_SetDirection (uint32_t pin, uint32_t dir) { \
        return pre2##_SetDirection (pin, dir, priv); \
    } \
    static int32_t pre1##_SetTrigger (uint32_t pin, uint32_t trigger) { \
        return pre2##_SetTrigger (pin, trigger, priv); \
    } \
    static int32_t pre1##_WritePin (uint32_t pin, uint32_t value) { \
        return pre2##_WritePin (pin, value, priv); \
    } \
    static int32_t pre1##_ReadPin (uint32_t pin) { \
        return pre2##_ReadPin (pin, priv); \
    } \
    static int32_t pre1##_SetHandler (uint32_t pin, ARM_GPIO_Handler_t handler, void *data) { \
        return pre2##_SetHandler (pin, handler, data, priv); \
    } \
    ARM_DRIVER_GPIO Driver_##pre1 = { \
        pre1##_GetVersion, \
        pre1##_Initialize, \
        pre1##_Uninitialize, \
        pre1##_PowerControl, \
        pre1##_SetDirection, \
        pre1##_SetTrigger, \
        pre1##_WritePin, \
        pre1##_ReadPin, \
        pre1##_SetHandler, \
    };

#endif /* __DRIVER_GPIO_H */
