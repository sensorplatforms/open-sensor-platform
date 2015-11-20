/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Adopted from alg from PebbleFit, Apache Licensed .
 */

#include <stdio.h>
#include "fpsup.h"
#include "fp_sensor.h"
#include "step.h"

static const Q15_t q15_0_20c = FP_to_Q15(0.20f);
static Q15_t sens_Const = FP_to_Q15(200.0f);
static Q15_t threshold = 0;

void OSP_step_init(void)
{
}

/*
 * Det is number of steps detected.
 * Count is count accumated count. On return:
 *  *count += det;
 * Assumes a call back at a period of 0.020sec
 */
void OSP_step_process(struct ThreeAxis *acc, struct StepInfo *step)
{
	static int sampcnt = 0;	/* Used for avaeraging */
	static Q15_t x = 0, y = 0, z = 0;
	static Q15_t dAcc, sampleOld = 0, sampleNew = 0;
	Q15_t min = FP_to_Q15(10000.0f), max = FP_to_Q15(-10000.0f);
	static int lastStepSamp = 0;
	static int sampcnt2 = 0;
	static int regulation = 0;
	

	/* Process groups of 5 samples */
	x += acc->x; y += acc->y; z+= acc->z;
	sampcnt++;
	sampcnt2++;

	step->detect = 0;

	if (sampcnt > 5) {
		sampcnt = 0;
		x = MUL_Q15(x, q15_0_20c);
		y = MUL_Q15(y, q15_0_20c);
		z = MUL_Q15(z, q15_0_20c);
		dAcc = MUL_Q15(x,x) + MUL_Q15(y,y) + MUL_Q15(z,z);
		dAcc = sqrt_q15(dAcc);
		sampleOld = sampleNew;
		if (abs_q15(dAcc - sampleNew) > sens_Const)
			sampleNew = dAcc;
		if ((sampleNew < threshold &&
			sampleOld > threshold) &&
			sampleOld != 0) {
			/* Cadence sanity check:
			 *   0.2sec - 2sec between steps.
			 */
			if (lastStepSamp &&
				(sampcnt2 - lastStepSamp) > 9 &&
				(sampcnt2 - lastStepSamp) < 100) {
				/* Require 5 consecutive steps for validity */
				regulation++;
			
				if (regulation == 5) {
					step->detect = 1;	
					step->count += 5;
				} else if (regulation > 5) {
					step->detect = 1;
					step->count += 1;
				}
			} else {
				regulation = 0;
			}
			lastStepSamp = sampcnt2;
		}
		if (dAcc < min) min = dAcc;
		if (dAcc > max) max = dAcc;

		threshold = (max+min) >> 1;
	}

	/* Avoid wrap */
	if (sampcnt2 > (1 << 30)) {
		if (lastStepSamp == 0) {
			sampcnt2 -= (1 << 30);
		} else  if (lastStepSamp > 0 && lastStepSamp > (1<<30)) {
			sampcnt2 -= (1 << 30);
			lastStepSamp -= (1 << 30);
		} else if ((sampcnt2 - lastStepSamp) > 1000) {
			sampcnt2 -= (1 << 30);
			lastStepSamp = 0;
		}
	}
}
