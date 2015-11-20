/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Project magnetometer data onto the X/Y plane using the
 * accel. Angles are computed from that using trig functions.
 */

#include <stdio.h>
#include <stdint.h>
#include "fpsup.h"
#include "fp_sensor.h"
#include "ecompass.h"

struct Heading {
	float roll;
	float pitch;
	float yaw;
};

static struct ThreeAxis HardIron;
static int calstate;
static int samplecount = 0;
static struct ThreeAxis minMag, maxMag;

static Q15_t deg_atan2(Q15_t y, Q15_t x)
{
	return MUL_Q15(atan2_q15(y, x), FP_to_Q15(180.0/M_PI));
}


static void adj_mag(struct ThreeAxis *mag)
{
	mag->x -= HardIron.x;
	mag->y -= HardIron.y;
	mag->z -= HardIron.z;
}


static void adj_acc(struct ThreeAxis *acc)
{
}

void OSP_ecompass_init(void)
{
	calstate = 0;
	HardIron.x = 0; HardIron.y = 0; HardIron.z = 0;
}

void OSP_ecompass_process(struct ThreeAxis *mag,
		struct ThreeAxis *acc,
		struct Euler *result)
{
	Q15_t pMx, pMy;
	Q15_t hyp;

	samplecount++;

	if (calstate) {
		/* Assume 30Hz, this is about 30 seconds */
		if (samplecount > 900) {
			calstate = 0;
			do {
				if (maxMag.x == MIN_Q15 ||
					maxMag.y == MIN_Q15 ||
					maxMag.z == MIN_Q15)
					break;
				if (minMag.x == MAX_Q15 ||
					minMag.y == MAX_Q15 ||
					minMag.z == MAX_Q15)
					break;

				HardIron.x = MUL_Q15((maxMag.x - minMag.x), q15_half);
				HardIron.x = HardIron.x - maxMag.x;


				HardIron.y = MUL_Q15((maxMag.y - minMag.y), q15_half);
				HardIron.y = HardIron.y - maxMag.y;

				HardIron.z = MUL_Q15((maxMag.z - minMag.z), q15_half);
				HardIron.z = HardIron.z - maxMag.z;

			} while (0);
			printf("Cal completed - Offsets: %f %f %f\n",
					Q15_to_FP(HardIron.x),
					Q15_to_FP(HardIron.y),
					Q15_to_FP(HardIron.z));
		} else {
			if (mag->x > maxMag.x) maxMag.x = mag->x;
			if (mag->y > maxMag.y) maxMag.y = mag->y;
			if (mag->z > maxMag.z) maxMag.z = mag->z;

			if (mag->x < minMag.x) minMag.x = mag->x;
			if (mag->y < minMag.y) minMag.y = mag->y;
			if (mag->z < minMag.z) minMag.z = mag->z;
		}
	}

	adj_mag(mag);
	adj_acc(acc);
#if 0
	acc->y = -acc->y;
	acc->x = -acc->x;
	acc->z = -acc->z;
#endif
	result->roll = deg_atan2(acc->x, acc->z);
#if 0
	result->roll = result->roll+FP_to_Q15(180.0f);
	if (result->roll > q15_c180) result->roll -= q15_c360;
#endif

	result->pitch = deg_atan2(acc->y, acc->z);
#if 0
	result->pitch = result->roll + q15_c180;
	if (result->pitch > q15_c180) result->pitch -= q15_c360;
#endif
	result->pitch = -result->pitch;
	hyp = sqrt_q15(MUL_Q15(acc->x, acc->x) + MUL_Q15(acc->z, acc->z));
	if (hyp == 0) return;

	pMx = (MUL_Q15(mag->x, DIV_Q15(acc->z,hyp)) + MUL_Q15(mag->z, DIV_Q15(acc->x,hyp)));

	hyp = sqrt_q15(MUL_Q15(acc->y, acc->y) + MUL_Q15(acc->z, acc->z));
	if (hyp == 0) return;

	pMy = (MUL_Q15(mag->y, DIV_Q15(acc->z,hyp)) + MUL_Q15(mag->z, DIV_Q15(acc->y,hyp)));

	result->yaw = -1 * deg_atan2(pMx, pMy);
	result->yaw += q15_c180;
#if 0
	printf("%s roll %f pitch %f yaw %f\n", __func__, Q15_to_FP(result->roll), Q15_to_FP(result->pitch), Q15_to_FP(result->yaw));
#endif
}

void OSP_ecompass_cal(void)
{
	calstate = 1;
	samplecount = 0;
	minMag.x = MAX_Q15;
	minMag.y = MAX_Q15;
	minMag.z = MAX_Q15;

	maxMag.x = MIN_Q15;
	maxMag.y = MIN_Q15;
	maxMag.z = MIN_Q15;

	printf("Cal requested\n");
}
