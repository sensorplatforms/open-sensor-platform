/*****************************************************************************
 *                                                                           *
 *                       Sensor Platforms Inc.                               *
 *                   2860 Zanker Road, Suite 210                             *
 *                        San Jose, CA 95134                                 *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *               Copyright (c) 2012 Sensor Platforms Inc.                    *
 *                       All Rights Reserved                                 *
 *                                                                           *
 *                   Proprietary and Confidential                            *
 *             Use only under license described in EULA.txt                  *
 *                                                                           *
 ****************************************************************************/
/*! \file                                                                    *
 *                                                                           *
 *  @author Sensor Platforms Inc: http://www.sensorplatforms.com             *
 *  @author        Support Email: support@sensorplatforms.com                *
 *                                                                           *
 ****************************************************************************/
#ifndef EXAMPLEPLATFORMIMPLEMENTATION_H
#define EXAMPLEPLATFORMIMPLEMENTATION_H

#include <stdio.h>

#define PRINTF(fmt, ...)    printf(fmt, __VA_ARGS__)

#include "FM_EmbeddedApi_FixedPoint.h"

void Platform_Initialize(void);
void Platform_HandleErrorIf(fm_char_t isError, const fm_char_t* msg);
SensorDescriptor_t* Platform_GetSensorDescriptorByName(const char* sensorType);
const SystemDescriptor_t* Platform_GetSystemDescriptor();
uint32_t Platform_BlockOnNewSensorReadings();

#endif // EXAMPLEPLATFORMIMPLEMENTATION_H
