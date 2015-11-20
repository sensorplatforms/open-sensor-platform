/*
 * (C) Copyright 2015 HY Research LLC
 * 
 * Author: hy-git@hy-research.com
 *
 *
 * Apache License.
 *
 * Interface layer for OSP. Implements a subset of the public
 * functions as defined by the OSP API. Translates between the
 * Fix point names used by OSP to a more generic Q24/Q12/Q15/etc 
 * naming scheme.
 */




#include <stdio.h>
#include <string.h>
#include "fp_sensor.h"
#include "fpsup.h"
#include "ecompass.h"
#include "gravity_lin.h"
#include "rotvec.h"
#include "osp-sensors.h"
#include "osp-fixedpoint-types.h"
#include "osp-version.h"
#include "osp-api.h"
#include "osp-alg-types.h"

#include "signalgenerator.h"
#include "significantmotiondetector.h"
#include "stepdetector.h"
#include "tilt.h"

#define CLEAN		0
#define DIRTY_IN	(1<<0)
#define DIRTY_OUT	(1<<1)

#define SEN_ENABLE	1

static struct Results RESULTS[NUM_ANDROID_SENSOR_TYPE];
static unsigned char dirty[NUM_ANDROID_SENSOR_TYPE];
//static SystemDescriptor_t const *sys;
static SensorDescriptor_t const *InputSensors[NUM_INPUT_SENSORS];

static const OSP_Library_Version_t libVersion = {
	.VersionNumber = (OSP_VERSION_MAJOR << 16) | (OSP_VERSION_MINOR << 8) | (OSP_VERSION_PATCH),
	.VersionString = OSP_VERSION_STRING,
	.buildTime = __DATE__" "__TIME__
};

static const uint64_t depend[NUM_ANDROID_SENSOR_TYPE] = 
{
	[SENSOR_ACCELEROMETER] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_MAGNETIC_FIELD] = FLAG(SENSOR_MAGNETIC_FIELD),
	[SENSOR_ORIENTATION] = FLAG(SENSOR_GRAVITY)|FLAG(SENSOR_MAGNETIC_FIELD),
	[SENSOR_ROTATION_VECTOR] = FLAG(SENSOR_GRAVITY)|FLAG(SENSOR_MAGNETIC_FIELD),
	[SENSOR_GEOMAGNETIC_ROTATION_VECTOR] = FLAG(SENSOR_GRAVITY)|FLAG(SENSOR_MAGNETIC_FIELD),
	[SENSOR_GRAVITY] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_LINEAR_ACCELERATION] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_GYROSCOPE] = FLAG(SENSOR_GYROSCOPE),
	[SENSOR_PRESSURE] = FLAG(SENSOR_PRESSURE),
	[SENSOR_STEP_DETECTOR] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_STEP_COUNTER] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_SIGNIFICANT_MOTION] = FLAG(SENSOR_ACCELEROMETER),
	[SENSOR_TILT_DETECTOR] = FLAG(SENSOR_ACCELEROMETER),
};


static ResultDescriptor_t * resHandles[NUM_ANDROID_SENSOR_TYPE];

static uint8_t sensor_state[NUM_ANDROID_SENSOR_TYPE];
static void (*readyCB[NUM_ANDROID_SENSOR_TYPE])(struct Results *, int);

static void OSP_SetDataMag(Q15_t x, Q15_t y, Q15_t z, NTTIME time)
{
	RESULTS[SENSOR_MAGNETIC_FIELD].ResType.result.x = x;
	RESULTS[SENSOR_MAGNETIC_FIELD].ResType.result.y = y;
	RESULTS[SENSOR_MAGNETIC_FIELD].ResType.result.z = z;
	RESULTS[SENSOR_MAGNETIC_FIELD].time = time;
	dirty[SENSOR_MAGNETIC_FIELD] = DIRTY_IN | DIRTY_OUT;
}

static void OSP_SetDataAcc(Q15_t x, Q15_t y, Q15_t z, NTTIME time)
{
	osp_float_t measurementFiltered[NUM_ACCEL_AXES];
	osp_float_t measurementFloat[NUM_ACCEL_AXES];
	NTTIME filterTime = time;

	RESULTS[SENSOR_ACCELEROMETER].ResType.result.x = x;
	RESULTS[SENSOR_ACCELEROMETER].ResType.result.y = y;
	RESULTS[SENSOR_ACCELEROMETER].ResType.result.z = z;
	RESULTS[SENSOR_ACCELEROMETER].time = time;
	dirty[SENSOR_ACCELEROMETER] = DIRTY_IN | DIRTY_OUT;

	measurementFloat[0] = Q15_to_FP(x);
	measurementFloat[1] = Q15_to_FP(y);
	measurementFloat[2] = Q15_to_FP(z);

	if (SignalGenerator_SetAccelerometerData(measurementFloat,
						measurementFiltered)){
		filterTime -= SIGNAL_GENERATOR_DELAY;

		//update significant motion alg
		SignificantMotDetector_SetFilteredAccelerometerMeasurement(
							filterTime,
							measurementFiltered);

		StepDetector_SetFilteredAccelerometerMeasurement(filterTime, 
							measurementFiltered);
	}
}

static void OSP_SetDataGyr(Q15_t x, Q15_t y, Q15_t z, NTTIME time)
{
	RESULTS[SENSOR_GYROSCOPE].ResType.result.x = x;
	RESULTS[SENSOR_GYROSCOPE].ResType.result.y = y;
	RESULTS[SENSOR_GYROSCOPE].ResType.result.z = z;
	RESULTS[SENSOR_GYROSCOPE].time = time;
	dirty[SENSOR_GYROSCOPE] = DIRTY_IN | DIRTY_OUT;
}

void OSPalg_SetDataBaro(Q15_t p, Q15_t t, NTTIME time)
{
	RESULTS[SENSOR_PRESSURE].ResType.result.x = p;
	RESULTS[SENSOR_PRESSURE].ResType.result.z = t;
	RESULTS[SENSOR_PRESSURE].time = time;
	dirty[SENSOR_PRESSURE] = DIRTY_IN | DIRTY_OUT;
}

static void OSPalg_EnableSensor(unsigned int sensor)
{
	int i;
	if (sensor_state[sensor] == SEN_ENABLE)
		return;

	if (depend[sensor] != FLAG(sensor)) {
		for (i = 0; i < NUM_ANDROID_SENSOR_TYPE; i++) {
			if (i == sensor)
				continue;
			if (depend[sensor] & FLAG(i)) {
				if (sensor_state[i] == SEN_ENABLE)
					continue;
				OSPalg_EnableSensor(i);
			}
		}		
	}
	sensor_state[sensor] = SEN_ENABLE;
}

/* Dispatch data in the different esoteric structure */
static void ResultReadyCB(struct Results *res, int sensor)
{
	union {
		Android_TriAxisPreciseData_t calresult;
		Android_TriAxisExtendedData_t calmag;
		Android_UncalibratedTriAxisPreciseData_t uncalresult;
		Android_UncalibratedTriAxisExtendedData_t uncalmag;
		Android_BooleanResultData_t sigmot;
		Android_OrientationResultData_t orient;
		Android_RotationVectorResultData_t rotvec;
	} r;

	if (!resHandles[sensor])
		return;
	switch(sensor) {
	case SENSOR_ACCELEROMETER:
		r.calresult.X = Q15_to_NTPRECISE(res->ResType.result.x);
		r.calresult.Y = Q15_to_NTPRECISE(res->ResType.result.y);
		r.calresult.Z = Q15_to_NTPRECISE(res->ResType.result.z);
		r.calresult.TimeStamp = res->time;
		break;
	case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
		r.uncalmag.X = Q15_to_NTPRECISE(res->ResType.result.x);
		r.uncalmag.Y = Q15_to_NTPRECISE(res->ResType.result.y);
		r.uncalmag.Z = Q15_to_NTPRECISE(res->ResType.result.z);
		r.uncalmag.TimeStamp = res->time;
		break;
	case SENSOR_MAGNETIC_FIELD:
		r.calmag.X = Q15_to_NTEXTENDED(res->ResType.result.x);
		r.calmag.Y = Q15_to_NTEXTENDED(res->ResType.result.y);
		r.calmag.Z = Q15_to_NTEXTENDED(res->ResType.result.z);
		r.calmag.TimeStamp = res->time;
		break;
	case SENSOR_GYROSCOPE_UNCALIBRATED:
		r.uncalresult.X = Q15_to_NTPRECISE(res->ResType.result.x);
		r.uncalresult.Y = Q15_to_NTPRECISE(res->ResType.result.y);
		r.uncalresult.Z = Q15_to_NTPRECISE(res->ResType.result.z);
		r.uncalresult.TimeStamp = res->time;
		break;
	case SENSOR_GYROSCOPE:
		r.calresult.X = Q15_to_NTPRECISE(res->ResType.result.x);
		r.calresult.Y = Q15_to_NTPRECISE(res->ResType.result.y);
		r.calresult.Z = Q15_to_NTPRECISE(res->ResType.result.z);
		r.calresult.TimeStamp = res->time;
		break;
	case SENSOR_ORIENTATION:
		r.orient.Pitch = Q15_to_NTEXTENDED(res->ResType.euler.pitch);
		r.orient.Roll = Q15_to_NTEXTENDED(res->ResType.euler.roll);
		r.orient.Yaw = Q15_to_NTEXTENDED(res->ResType.euler.yaw);
		r.orient.TimeStamp = res->time;
		break;
	case SENSOR_PRESSURE:
		return;
	case SENSOR_GRAVITY:
	case SENSOR_LINEAR_ACCELERATION:
		r.calresult.X = Q15_to_NTPRECISE(res->ResType.result.x);
		r.calresult.Y = Q15_to_NTPRECISE(res->ResType.result.y);
		r.calresult.Z = Q15_to_NTPRECISE(res->ResType.result.z);
		r.calresult.TimeStamp = res->time;
		break;

	case SENSOR_ROTATION_VECTOR:
		r.rotvec.X = Q15_to_NTPRECISE(res->ResType.quat.x);
		r.rotvec.Y = Q15_to_NTPRECISE(res->ResType.quat.y);
		r.rotvec.Z = Q15_to_NTPRECISE(res->ResType.quat.z);
		r.rotvec.W = Q15_to_NTPRECISE(res->ResType.quat.w);
		r.rotvec.TimeStamp = res->time;
		break;
	case SENSOR_SIGNIFICANT_MOTION:
	case SENSOR_TILT_DETECTOR:
		r.sigmot.data = true;
		r.sigmot.TimeStamp = res->time;
		break;
	default:
		return;
	}
	if (resHandles[sensor]->pResultReadyCallback) {
		printf("Calling: %i\n", sensor);
		resHandles[sensor]->pResultReadyCallback(resHandles[sensor],
			&r);
	}
}

static void OnStepResultsReady( StepDataOSP_t* stepData )
{
	if (resHandles[SENSOR_STEP_COUNTER]) {
		Android_StepCounterResultData_t callbackData;

		callbackData.StepCount = stepData->numStepsTotal;
		callbackData.TimeStamp = stepData->startTime; //!TODO - Double check if start time or stop time

		resHandles[SENSOR_STEP_COUNTER]->pResultReadyCallback(
			resHandles[SENSOR_STEP_COUNTER],
			&callbackData);
	}
}

static void OnSignificantMotionResult(NTTIME * eventTime)
{
	if (resHandles[SENSOR_SIGNIFICANT_MOTION]) {
		Android_BooleanResultData_t callbackData;

		callbackData.data = true;
		callbackData.TimeStamp = *eventTime;

		resHandles[SENSOR_SIGNIFICANT_MOTION]->pResultReadyCallback(
			resHandles[SENSOR_SIGNIFICANT_MOTION],
			&callbackData);
	}
}

static void OSPalg_EnableSensorCB(unsigned int sensor,
		void (*ready)(struct Results *, int))
{
	readyCB[sensor] = ready;
	OSPalg_EnableSensor(sensor);
}

static void OSPalg_DisableSensor(unsigned int sensor)
{
	int i;
	int busy = 0;

	if (sensor_state[sensor] != SEN_ENABLE)
		return;

	for (i = 0; i < NUM_ANDROID_SENSOR_TYPE; i++) {
		if (i == sensor) continue;

		if ((sensor_state[i] == SEN_ENABLE) && 
			(depend[i] & FLAG(sensor))) {
			busy++;		
		}
	}
	if (busy == 0) {
		sensor_state[sensor] = 0;
    }
}

void OSPalg_cal(void)
{
	OSP_ecompass_cal();
}

OSP_STATUS_t OSP_DoBackgroundProcessing(void)
{
	return OSP_STATUS_IDLE;
}

OSP_STATUS_t OSP_DoForegroundProcessing(void)
{
	int i;
	if (sensor_state[SENSOR_GRAVITY] == SEN_ENABLE) 
		if (dirty[SENSOR_ACCELEROMETER] & DIRTY_IN) {
			OSP_gravity_process(&RESULTS[SENSOR_ACCELEROMETER].ResType.result,
					&RESULTS[SENSOR_GRAVITY].ResType.result);
			dirty[SENSOR_GRAVITY] = DIRTY_IN|DIRTY_OUT;
			RESULTS[SENSOR_GRAVITY].time = RESULTS[SENSOR_ACCELEROMETER].time;
		}

	if (sensor_state[SENSOR_LINEAR_ACCELERATION] == SEN_ENABLE) 
		if (dirty[SENSOR_ACCELEROMETER] & DIRTY_IN || 
			dirty[SENSOR_GRAVITY] & DIRTY_IN) {
			OSP_linear_acc_process(&RESULTS[SENSOR_ACCELEROMETER].ResType.result,
				&RESULTS[SENSOR_GRAVITY].ResType.result,
				&RESULTS[SENSOR_LINEAR_ACCELERATION].ResType.result);
			dirty[SENSOR_LINEAR_ACCELERATION] = DIRTY_IN|DIRTY_OUT;
			RESULTS[SENSOR_LINEAR_ACCELERATION].time = RESULTS[SENSOR_ACCELEROMETER].time;
		}

	if (sensor_state[SENSOR_ORIENTATION] == SEN_ENABLE)
		if (dirty[SENSOR_MAGNETIC_FIELD] & DIRTY_IN ||
			dirty[SENSOR_GRAVITY] & DIRTY_IN) {
			OSP_ecompass_process(&RESULTS[SENSOR_MAGNETIC_FIELD].ResType.result,
				&RESULTS[SENSOR_GRAVITY].ResType.result,
				&RESULTS[SENSOR_ORIENTATION].ResType.euler);
			dirty[SENSOR_ORIENTATION] = DIRTY_IN|DIRTY_OUT;
			RESULTS[SENSOR_ORIENTATION].time = RESULTS[SENSOR_GRAVITY].time;
		}

	if (sensor_state[SENSOR_ROTATION_VECTOR] == SEN_ENABLE)
		if (dirty[SENSOR_MAGNETIC_FIELD] & DIRTY_IN ||
			dirty[SENSOR_GRAVITY] & DIRTY_IN) {
			OSP_rotvec_process(&RESULTS[SENSOR_MAGNETIC_FIELD].ResType.result,
				&RESULTS[SENSOR_GRAVITY].ResType.result,
				 &RESULTS[SENSOR_ROTATION_VECTOR].ResType.quat);
			dirty[SENSOR_ROTATION_VECTOR] = DIRTY_IN|DIRTY_OUT;
			RESULTS[SENSOR_ROTATION_VECTOR].time = RESULTS[SENSOR_GRAVITY].time;
		}
	if (sensor_state[SENSOR_TILT_DETECTOR] == SEN_ENABLE)
		if (dirty[SENSOR_ACCELEROMETER] & DIRTY_IN || 
			dirty[SENSOR_TILT_DETECTOR] & DIRTY_IN) {
			OSP_tilt_process(&RESULTS[SENSOR_ACCELEROMETER].ResType.result,
				&RESULTS[SENSOR_TILT_DETECTOR].ResType.result);
			dirty[SENSOR_TILT_DETECTOR] = DIRTY_IN|DIRTY_OUT;
			RESULTS[SENSOR_TILT_DETECTOR].time = RESULTS[SENSOR_ACCELEROMETER].time;
		}

#ifdef FEAT_STEP
	if (sensor_state[SENSOR_STEP_COUNTER] == SEN_ENABLE ||
		sensor_state[SENSOR_STEP_DETECTOR] == SEN_ENABLE) 
		if (dirty[SENSOR_ACCELEROMETER] & DIRTY_IN) {
			OSP_step_process(&RESULTS[SENSOR_ACCELEROMETER].ResType.result,
					&RESULTS[SENSOR_STEP_COUNTER].ResType.step);
			if (RESULTS[SENSOR_STEP_DETECTOR].ResType.step.detect) {
				dirty[SENSOR_STEP_COUNTER] = DIRTY_IN|DIRTY_OUT;
				dirty[SENSOR_STEP_DETECTOR] = DIRTY_IN|DIRTY_OUT;
			}
			RESULTS[SENSOR_STEP_DETECTOR].time = RESULTS[SENSOR_ACCELEROMETER].time;
		}
#endif
	for (i = 0; i < NUM_ANDROID_SENSOR_TYPE; i++) {
		if (dirty[i]) {	
			if (readyCB[i] && sensor_state[i] == SEN_ENABLE)  {
				(readyCB[i])(&RESULTS[i], i);
			}
			dirty[i] = CLEAN;
		}
	}

	return OSP_STATUS_IDLE;
}

OSP_STATUS_t OSP_RegisterInputSensor(SensorDescriptor_t *SenDesc,
		InputSensorHandle_t *rHandle)
{
	if (!SenDesc) return OSP_STATUS_SENSOR_INVALID_DESCRIPTOR;

	switch(SenDesc->SensorType) {
	case ACCEL_INPUT_SENSOR:
	case MAG_INPUT_SENSOR:
	case GYRO_INPUT_SENSOR:
	case PRESSURE_INPUT_SENSOR:
		if (InputSensors[SenDesc->SensorType])
			return OSP_STATUS_SENSOR_ALREADY_REGISTERED;
		InputSensors[SenDesc->SensorType] = SenDesc;
		*rHandle = &InputSensors[SenDesc->SensorType];
		return OSP_STATUS_OK;
	default:
		break;

	}
	return OSP_STATUS_SENSOR_INVALID_DESCRIPTOR;
}


/* BUG: Mag needs to be handled differently. */
OSP_STATUS_t OSP_SetInputData(InputSensorHandle_t handle,
	OSP_InputSensorData_t *data)
{
	SensorDescriptor_t *s, **v;
	Q15_t x, y, z;
	int32_t rawX, rawY, rawZ;
	int32_t xidx = 0, yidx = 1, zidx = 2;
	int32_t xs = 1, ys = 1, zs = 1;
	int i;
	int val[5]; //, vallen;
	unsigned long long ts;

	if (!handle) return OSP_STATUS_INVALID_HANDLE;
	v = handle;
	s = *v;
	//vallen = extractOSP(s->SensorType, data, &ts, val);

	for (i = 0; i < 3; i++) {
		switch (s->AxisMapping[i]) {
		case AXIS_MAP_POSITIVE_X:
			xidx = i;
			xs = 1;
			break;
		case AXIS_MAP_NEGATIVE_X:
			xidx = i;
			xs = -1;
			break;
		case AXIS_MAP_POSITIVE_Y:
			yidx = i;
			ys = 1;
			break;
		case AXIS_MAP_NEGATIVE_Y:
			yidx = i;
			ys = -1;
			break;
		case AXIS_MAP_POSITIVE_Z:
			zidx = i;
			zs = 1;
			break;
		case AXIS_MAP_NEGATIVE_Z:
			zidx = i;
			zs = -1;
			break;
		default:
			break;
		}
	}
	rawX = xs*val[xidx] - s->ConversionOffset[xidx];
	rawY = ys*val[yidx] - s->ConversionOffset[yidx];
	rawZ = zs*val[zidx] - s->ConversionOffset[zidx];
	x = MUL_Q15(INT_to_Q15(rawX), NTPRECISE_to_Q15(s->ConversionScale[xidx]));
	y = MUL_Q15(INT_to_Q15(rawY), NTPRECISE_to_Q15(s->ConversionScale[yidx]));
	z = MUL_Q15(INT_to_Q15(rawZ), NTPRECISE_to_Q15(s->ConversionScale[zidx]));

	switch(s->SensorType) {
	case SENSOR_ACCELEROMETER:
		OSP_SetDataAcc(x, y, z, ts);
		break;
	case SENSOR_MAGNETIC_FIELD:
	case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
		OSP_SetDataMag(x, y, z, ts);
		break;
	case SENSOR_GYROSCOPE:
	case SENSOR_GYROSCOPE_UNCALIBRATED:
		OSP_SetDataGyr(x, y, z, ts);
		break;
	case SENSOR_PRESSURE:
		/* Ignore for now */
		break;
	default:
		break;
	}
	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_SubscribeSensorResult(ResultDescriptor_t *ResDesc,
			ResultHandle_t *ResHandle)
{
	if (!ResDesc) return OSP_STATUS_SENSOR_INVALID_DESCRIPTOR;
	switch (ResDesc->SensorType) {
	case SENSOR_ACCELEROMETER:
	case SENSOR_MAGNETIC_FIELD_UNCALIBRATED:
	case SENSOR_MAGNETIC_FIELD:
	case SENSOR_GYROSCOPE_UNCALIBRATED:
	case SENSOR_GYROSCOPE:
	case SENSOR_ORIENTATION:
	case SENSOR_PRESSURE:
	case SENSOR_GRAVITY:
	case SENSOR_LINEAR_ACCELERATION:
	case SENSOR_ROTATION_VECTOR:
		if (resHandles[ResDesc->SensorType] != NULL) 
			return OSP_STATUS_RESULT_IN_USE;
		resHandles[ResDesc->SensorType] = ResDesc;
		OSPalg_EnableSensorCB(ResDesc->SensorType,
				ResultReadyCB);
		*ResHandle = resHandles[ResDesc->SensorType];
		break;
	case SENSOR_STEP_DETECTOR:
	case SENSOR_STEP_COUNTER:
		if (resHandles[ResDesc->SensorType] != NULL) 
			return OSP_STATUS_RESULT_IN_USE;
		resHandles[ResDesc->SensorType] = ResDesc;

		StepDetector_Init(OnStepResultsReady, NULL);
		break;
	case SENSOR_SIGNIFICANT_MOTION:
		if (resHandles[ResDesc->SensorType] != NULL) 
			return OSP_STATUS_RESULT_IN_USE;
		resHandles[ResDesc->SensorType] = ResDesc;
		SignificantMotDetector_Init(OnSignificantMotionResult);
		break;
	case SENSOR_TILT_DETECTOR:
		if (resHandles[ResDesc->SensorType] != NULL) 
			return OSP_STATUS_RESULT_IN_USE;
		resHandles[ResDesc->SensorType] = ResDesc;
		OSP_tilt_init();
		break;
	default:
		return OSP_STATUS_SENSOR_INVALID_TYPE;
	}
	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_UnsubscribeSensorResult(ResultHandle_t ResHandle)
{
	ResultDescriptor_t **rd;

	rd = ResHandle;

	if (!rd) return OSP_STATUS_INVALID_HANDLE;

	if (*rd == NULL)
		return OSP_STATUS_NOT_SUBSCRIBED;

	if ((*rd)->SensorType == SENSOR_TILT_DETECTOR)
		OSP_tilt_init();

	if ((*rd)->SensorType == SENSOR_SIGNIFICANT_MOTION)
		SignificantMotDetector_Init(NULL);
	if ((*rd)->SensorType == SENSOR_STEP_COUNTER)
		StepDetector_Init(NULL, NULL);

	/* Do magic with disabling callbacks */
	OSPalg_DisableSensor((*rd)->SensorType);
	*(rd) = NULL;

	return OSP_STATUS_OK;
}

OSP_STATUS_t OSP_Initialize(const SystemDescriptor_t *sysdesc)
{
	int i;

	//sys = sysdesc;

	for (i = 0; i < NUM_INPUT_SENSORS; i++) {
		InputSensors[i] = NULL;
	}

	for (i = 0; i < NUM_ANDROID_SENSOR_TYPE; i++) {
		dirty[i] = CLEAN;
		sensor_state[i] = 0;
		readyCB[i] = NULL;
		resHandles[i] = NULL;
		memset(&RESULTS[i], 0, sizeof(struct Results));
	}
	OSP_gravity_init();
	OSP_ecompass_init();
	OSP_rotvec_init();
	OSP_linear_acc_init();
#ifdef FEAT_STEP
	OSP_step_init();
#endif
	OSP_tilt_init();
	//Initialize signal generator
	SignalGenerator_Init();

	//Initialize algs
	SignificantMotDetector_Init(NULL);
	StepDetector_Init(NULL, NULL);


	return OSP_STATUS_OK;
}


OSP_STATUS_t OSP_GetLibraryVersion(const OSP_Library_Version_t **v)
{
	*v = &libVersion;
	return OSP_STATUS_OK;
}
