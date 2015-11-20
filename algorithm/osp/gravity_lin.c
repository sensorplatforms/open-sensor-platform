/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 */


/*
 * Gravity and linear acceleration computation from AOSP.
 */
#include "fpsup.h"
#include "lpf.h"
#include "fp_sensor.h"
#include "gravity_lin.h"

static struct LPF xlpf, ylpf, zlpf;
static struct LPF_CBQF xclpf, yclpf, zclpf;

static const Q15_t SAMPLE_PERIOD = FP_to_Q15(0.020f);

void OSP_gravity_init(void)
{
	LPF_init(&xlpf,FP_to_Q15(0.707107f), FP_to_Q15(1.5f));
	LPF_init(&ylpf,FP_to_Q15(0.707107f), FP_to_Q15(1.5f));
	LPF_init(&zlpf,FP_to_Q15(0.707107f), FP_to_Q15(1.5f));
	LPF_setSamplingPeriod(&xlpf, SAMPLE_PERIOD);
	LPF_setSamplingPeriod(&ylpf, SAMPLE_PERIOD);
	LPF_setSamplingPeriod(&zlpf, SAMPLE_PERIOD);
	LPF_CBQF_init(&xclpf, &xlpf, 0);
	LPF_CBQF_init(&yclpf, &ylpf, 0);
	LPF_CBQF_init(&zclpf, &zlpf, FP_to_Q15(9.8f));
}

void OSP_linear_acc_init(void)
{
}

void OSP_gravity_process(struct ThreeAxis *acc, struct ThreeAxis *res)
{
	res->x = LPF_CBQF_data(&xclpf, acc->x);
	res->y = LPF_CBQF_data(&yclpf, acc->y);
	res->z = LPF_CBQF_data(&zclpf, acc->z);
};

void OSP_linear_acc_process(struct ThreeAxis *acc,
				struct ThreeAxis *gravity,
				struct ThreeAxis *res)
{
	res->x = acc->x - (gravity->x);
	res->y = acc->y - (gravity->y);
	res->z = acc->z - (gravity->z);
}
