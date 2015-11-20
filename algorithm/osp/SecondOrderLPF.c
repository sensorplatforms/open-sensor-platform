/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Second order IIR BiCubicFilter. based on
 * Implementation from Android AOSP.
 */

#include <stdio.h>
#include "string.h"
#include "fpsup.h"
#include "lpf.h"
#if 0
struct LPF {
	Q15_t iQ;
	Q15_t fc;
	Q15_t K;
	Q15_t iD;
	Q15_t a0;
	Q15_t a1;
	Q15_t b1;
	Q15_t b2;

	Q15_t x1;
	Q15_t x2;
	Q15_t y1;
	Q15_t y2;
};

struct LPF_CBQF {
	struct LPF mA;
	struct LPF mB;
};
#endif

void LPF_init(struct LPF *lpf, Q15_t Q, Q15_t fc)
{
	lpf->iQ = RECIP_Q15(Q);
	lpf->fc = fc;
}

void LPF_setSamplingPeriod(struct LPF *lpf, Q15_t dT)
{
	
	Q15_t tmp, k2;
#if 1
	tmp = MUL_Q15(q15_pi, lpf->fc);
	tmp = MUL_Q15(tmp, dT);
#else
	tmp = MUL_Q15(q15_pi, q15_c1);
#endif
	lpf->K = tan_q15(tmp);
	k2 = MUL_Q15(lpf->K, lpf->K);
	
	lpf->iD = RECIP_Q15(k2 + MUL_Q15(lpf->K, lpf->iQ) + q15_c1);
	lpf->a0 = MUL_Q15(k2, lpf->iD);
	lpf->a1 = MUL_Q15(q15_c2, lpf->a0);
	lpf->b1 = MUL_Q15(q15_c2, lpf->iD);
	lpf->b1 = MUL_Q15(k2 - q15_c1, lpf->b1);
	lpf->b2 = MUL_Q15(lpf->iD, (k2 - MUL_Q15(lpf->K, lpf->iQ) + q15_c1));
}

Q15_t LPF_BQF_init(struct LPF *lpf, Q15_t x)
{
	lpf->x1 = x;
	lpf->x2 = x;

	lpf->y1 = x;
	lpf->y2 = x;

	return x;
}

Q15_t LPF_BQF_data(struct LPF *lpf, Q15_t x)
{
	Q15_t y;

	y = MUL_Q15((x+lpf->x2), lpf->a0);
	y += MUL_Q15(lpf->x1, lpf->a1);
	y -= MUL_Q15(lpf->y1, lpf->b1);
	y -= MUL_Q15(lpf->y2, lpf->b2);

	lpf->x2 = lpf->x1;
	lpf->y2 = lpf->y1;
	lpf->x1 = x;
	lpf->y1 = y;

	return y;
}


void LPF_CBQF_init(struct LPF_CBQF *clpf, struct LPF *lpf, Q15_t x)
{
	memcpy(&clpf->mA, lpf, sizeof(struct LPF));
	memcpy(&clpf->mB, lpf, sizeof(struct LPF));

	LPF_BQF_init(&clpf->mA, x);
	LPF_BQF_init(&clpf->mB, x);
}


Q15_t LPF_CBQF_data(struct LPF_CBQF *clpf, Q15_t x)
{
	return LPF_BQF_data(&clpf->mB, LPF_BQF_data(&clpf->mA, x));
}

#if 0
int main(int argc, char **argv)
{
	struct LPF lpf;
	struct LPF_CBQF clpf;
	Q15_t d;

	int i;

	LPF_init(&lpf,FP_to_Q15(0.707107f), FP_to_Q15(1.5f));
	LPF_setSamplingPeriod(&lpf, FP_to_Q15(0.020f));
	LPF_CBQF_init(&clpf, &lpf, FP_to_Q15(9.8f));

	for  (i = 0; i < 200; i++) {
		d = LPF_CBQF_data(&clpf, FP_to_Q15(9.8f));
		printf("Data: %f\n", Q15_to_FP(d));
	}
}
#endif
