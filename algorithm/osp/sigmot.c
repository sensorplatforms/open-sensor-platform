/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Initial attempt at the significant motion sensor.
 */


#include "sigmot.h"

enum {
	SIGMOT_RESET,
	SIGMOT_WAIT,
};

int state;

void OSP_sigmot_init(void)
{
	state = SIGMOT_WAIT;
}

void OSP_sigmot_process(struct ThreeAxis *acc, struct ThreeAxis *res)
{
	res->x = 0;
}
