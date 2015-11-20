/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Interface layer for OSP. Implements a subset of the public
 * functions as defined by the OSP API.
 */

#include <stdio.h>

#include "osp-api.h"

/* Dummy routines for OSP */

OSP_STATUS_t     OSP_Initialize(const SystemDescriptor_t* pSystemDesc)
{
	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_DoForegroundProcessing(void)
{
	return OSP_STATUS_IDLE;
}

OSP_STATUS_t OSP_DoBackgroundProcessing(void)
{
	return OSP_STATUS_IDLE;
}

OSP_STATUS_t OSP_SetInputData(InputSensorHandle_t h, TriAxisSensorRawData_t *d)
{
	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_RegisterInputSensor(const SensorDescriptor_t *pSensorDescriptor, InputSensorHandle_t *pReturnedHandle)
{
	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_SubscribeSensorResult(ResultDescriptor_t *pResultDescriptor, ResultHandle_t *pResultHandle)
{
	return OSP_STATUS_OK;
}


static const OSP_Library_Version_t Version = {
	.VersionNumber = 0x99889988,
	.VersionString = "OSP Dummy stub library",
	.buildTime = __DATE__"@"__TIME__,
};

OSP_STATUS_t OSP_GetVersion(const OSP_Library_Version_t **ppVersionStruct)
{
	*ppVersionStruct = &Version;	
	return OSP_STATUS_OK;
}

