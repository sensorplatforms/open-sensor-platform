/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Table look up tan/sin/cos
 */
#include "fpsup.h"
#include "trig_sin.c"


Q15_t sin_q15(Q15_t ang)
{
	int adj = 1;
	int i;

	do {
	/* Sine is odd */
		if (ang >=  q15_pi/2 && ang < q15_pi) {
			ang -= q15_pi/2;
			break;
		} else if (ang >= q15_pi) {
			ang -= q15_pi;
			adj = -1 * adj;
		} else if (ang < 0) {
			ang += q15_pi;
			adj = -1 * adj;
		} else {
			/* Between 0 - pi/2 */
			break;
		}
	} while(1);

	for (i = 0; i < sizeof(trig_sin)/sizeof(struct TRIG_SIN); i++) {
		if (ang < trig_sin[i].ang) {
			return trig_sin[i].sin * adj;
		}
	}
	return 0;
}

Q15_t cos_q15(Q15_t ang)
{
	return sin_q15(ang + q15_pi/2);
}

Q15_t tan_q15(Q15_t ang)
{
	return DIV_Q15(sin_q15(ang),cos_q15(ang));
}

Q15_t arcsin_q15(Q15_t v)
{
	Q15_t l, ang;
	int i, adj;

	if (v < 0) adj = -1; else adj = 1;
	l = v * adj;

	for (i = 0; i < sizeof(trig_sin)/sizeof(struct TRIG_SIN); i++) {
		if (l < trig_sin[i].sin) {
			ang = trig_sin[i].ang;
			break;
		}
	}
	return ang*adj;

}

Q15_t arccos_q15(Q15_t v)
{
	Q15_t ang;

	ang = arcsin_q15(v);
	return (q15_pi/2 - ang);
}
