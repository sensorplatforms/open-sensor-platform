/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 * Math support routines for sqrt.
 */
/* Apache Licensed code from PebbleFit */
#include <stdio.h>
#include "fpsup.h"

#ifdef ENABLE_Q12
Q12_t sqrt_q12(Q12_t num)
{
	Q12_t a, p, e = FP_to_Q12(0.001f), b;
	Q12_t t;
	int nb = 0;

	a = num;

	p = MUL_Q12(a, a);;
	while ((p - num >= e) && (nb++ < 40)) {
		t = RECIP_Q12(a);
		b = a + MUL_Q12(num, t);
		a = b >> 1;
		p = MUL_Q12(a, a);
	}

	return a;
}
#endif
Q15_t sqrt_q15(Q15_t num)
{
	Q15_t ans;
	LQ15_t a, b;
	LQ15_t p, e = FP_to_Q15(0.0001f);
	LQ15_t t, numl;
	int nb = 0;

	numl = num;
	a = numl;

	p = MUL_LQ15(a, a);
	while ((abs_lq15(p - num) >= e) && (nb++ < 64)) {
		t = RECIP_LQ15(a);
		b = a + MUL_LQ15(numl, t);
		a = b >> 1;
		p = MUL_LQ15(a, a);
		nb++;
	}
	ans = a;
#if 0
	printf("SQRT: %f = %f nb = %i\n", LQ15_to_FP(numl), LQ15_to_FP(a), nb);
#endif
	return ans;
}

#ifdef ENABLE_Q24
Q24_t sqrt_q24(Q24_t num)
{
	Q24_t a, p, e = FP_to_Q24(0.001f), b;
	Q24_t t;
	int nb = 0;

	a = num;

	p = MUL_Q24(a, a);;
	while ((abs_q24(p - num) >= e) && (nb++ < 40)) {
		t = RECIP_Q24(a);
		b = a + MUL_Q24(num, t);
		a = b >> 1;
		p = MUL_Q24(a, a);
		nb++;
	}
	return a;
}
#endif

#ifdef TEST_SQRT
#include <math.h>
int main(int argc, char **argv)
{
	float testv[] = {
		37483.949219f,
		-1.0f
	};
	int i;

	for (i = 0; testv[i] > 0; i++) {
		printf("SQRT(%f) = (Q15: %f, float: %f)\n",
				testv[i], Q15_to_FP(sqrt_q15(FP_to_Q15(testv[i]))),
				sqrt(testv[i]));
	}
	return 0;
}

#endif
