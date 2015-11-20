/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 */

/*
 * Fixed point math support library.
 * Q15 - value is scaled by 1<<15 and stored in an int32_t
 * Q24 - value is scaled by 1<<24 and stored in an int32_t
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "fpsup.h"

const Q15_t q15_pi = FP_to_Q15(M_PI);
const Q15_t q15_c360 = FP_to_Q15(360.0f);
const Q15_t q15_c180 = FP_to_Q15(180.0f);
const Q15_t q15_c90 = FP_to_Q15(90.0f);
const Q15_t q15_c1 = FP_to_Q15(1.0f);
const Q15_t q15_c2 = FP_to_Q15(2.0f);
const Q15_t q15_half = FP_to_Q15(0.5f);
const Q15_t q15_quarter = FP_to_Q15(0.25f);

#ifdef ENABLE_Q12
const Q15_t q12_pi = FP_to_Q12(M_PI);
const Q15_t q12_c360 = FP_to_Q12(360.0f);
const Q15_t q12_c180 = FP_to_Q12(180.0f);
const Q15_t q12_c90 = FP_to_Q12(90.0f);
const Q15_t q12_c1 = FP_to_Q12(1.0f);
const Q15_t q12_c2 = FP_to_Q12(2.0f);
const Q15_t q12_half = FP_to_Q12(0.5f);
const Q15_t q12_quarter = FP_to_Q12(0.25f);
#endif

#ifdef ENABLE_Q24
const Q24_t q24_pi = FP_to_Q24(M_PI);
const Q24_t q24_c90 = FP_to_Q24(90.0f);
const Q24_t q24_c1 = FP_to_Q24(1.0f);
const Q24_t q24_c2 = FP_to_Q24(2.0f);
const Q24_t q24_half = FP_to_Q24(0.5f);
const Q24_t q24_quarter = FP_to_Q24(0.25f);
#endif

/* MUL_LQ15 */
/*
 *  LQ15 - 64 bit storage:
 *    1 signed bit
 *    63 bits of value divided into:
 *        48bits of integer
 *        15bits of fractional.
 *  Multiplication of 2 LQ15 produces 126bits of value.
 *  15bits needs to be thrown away to convert it to 15 bits of
 *  fractional. This leaves 111 bits of numbers.
 *
 * LQ15 is to support intermediates used for sqrt() Q15 guesses. So
 * the largest value needed is:
 * 	1 sign bit
 * 	2*16 integer bits
 * 	15 fractional bits
 * 	------
 * 	48bits total. Thus the upper 63 bits is always zero. (except for
 * 		sign extension).
 */
LQ15_t MUL_LQ15(LQ15_t a, LQ15_t b)
{
	/* Long multiplication principle:
	 * (From: http://www.edaboard.com/thread253439.html)
	 *
	 * x,y is the 2 multiplier (signed 64 bit)
	 * after sign valuation and (if need) sign inversion
	 * it is split into xh xl yh yl (unsigned 32 bit)
	 * the operations performed are:
	 *
	 *               xh xl  *
	 *               yh yl  =
	 *               -------
	 *               xl_yl  +
	 *            xh_yl     +
	 *            xl_yh     +
	 *         xh_yh        =
	 *         -------------
	 *  (sign) z3 z1 z2 z0
	 *
	 */

	int8_t sign = 1;

	/* Intermediates */
	uint64_t al_bl, ah_bl, al_bh, ah_bh;

	uint32_t al, ah;
	uint32_t bl, bh;

	uint64_t res0, res1;

	if (a<0) {
		a = -a;
		sign = -sign;
	}

	if (b<0) {
		b = -b;
		sign = -sign;
	}

	al = a & 0xffffffff; ah = a>>32;
	bl = b & 0xffffffff; bh = b>>32;
	if (ah != 0 || bh != 0) {
		printf("HY-DBG: OVERFLOW in %s\n", __func__);
		return 0;
	}

	al_bl = (uint64_t)al*(uint64_t)bl;
	ah_bl = (uint64_t)ah*(uint64_t)bl;
	al_bh = (uint64_t)al*(uint64_t)bh;
	ah_bh = (uint64_t)ah*(uint64_t)bh;

	res0 = al_bl & 0xffffffff;

	res1 = (al_bl>>32)+(ah_bl & 0xffffffff)+(al_bh & 0xffffffff);
	res0 |= (res1&0xffffffff) << 32;

	res1 >>= 32;              // this is 'carry out' of the previous sum
	res1 += (ah_bl>>32) + (al_bh>>32) +ah_bh;

	res0 >>= Q15_SHIFT;

	res0 &= ~(((uint64_t)(1<<15)-1) << (64-Q15_SHIFT));
	res0 |= (res1 & ((1<<15)-1))  << (64-Q15_SHIFT);

	return res0;
}

/* MUL_Q15 */

Q15_t MUL_Q15(Q15_t a, Q15_t b)
{
	int64_t tmp;

	tmp = (int64_t)a * (int64_t)b;
	return tmp >> Q15_SHIFT;
}

#ifdef ENABLE_Q12
Q12_t MUL_Q12(Q12_t a, Q12_t b)
{
	int64_t tmp;

	tmp = (int64_t)a * (int64_t)b;
	return tmp >> Q12_SHIFT;
}
#endif

#ifdef ENABLE_Q24
/* MUL_Q24 */
Q24_t MUL_Q24(Q24_t a, Q24_t b)
{
	int64_t tmp;

	tmp = (int64_t)a * (int64_t)b;
	return tmp >> Q24_SHIFT;
}
#endif
LQ15_t DIV_LQ15(LQ15_t a, LQ15_t b)
{
	int64_t tmp;
	tmp = (int64_t)a << Q15_SHIFT;

	return tmp/b;
}
/* DIV_Q15 */
Q15_t DIV_Q15(Q15_t a, Q15_t b)
{
	int64_t tmp;
	tmp = (int64_t)a << Q15_SHIFT;

	return tmp/b;
}
#ifdef ENABLE_Q12
/* DIV_Q12 */
Q12_t DIV_Q12(Q12_t a, Q12_t b)
{
	int64_t tmp;
	tmp = (int64_t)a << Q12_SHIFT;

	return tmp/b;
}
#endif

#ifdef ENABLE_Q24
/* DIV_Q24 */
Q24_t DIV_Q24(Q24_t a, Q24_t b)
{
	int64_t tmp;
	tmp = (int64_t)a << Q24_SHIFT;

	return tmp/b;
}
#endif
/* RECIP_Q15 */

LQ15_t RECIP_LQ15(LQ15_t a)
{
	return DIV_LQ15(FP_to_Q15(1.0f), a);
}


Q15_t RECIP_Q15(Q15_t a)
{
	return DIV_Q15(q15_c1, a);
}

#ifdef ENABLE_Q12
Q12_t RECIP_Q12(Q12_t a)
{
	return DIV_Q12(q12_c1, a);
}
#endif

#ifdef ENABLE_Q24
/* RECIP_Q24 */
Q24_t RECIP_Q24(Q24_t a)
{
	return DIV_Q24(q24_c1, a);
}
#endif

double Q15_to_FP(Q15_t v)
{
	return ((double)v)/(double)(1<<Q15_SHIFT);
}

double LQ15_to_FP(LQ15_t v)
{
	return ((double)v)/(double)(1<<Q15_SHIFT);
}

#ifdef ENABLE_Q12
double Q12_to_FP(Q12_t v)
{
	return ((double)v)/(double)(1<<Q12_SHIFT);
}
#endif

#ifdef ENABLE_Q24
double Q24_to_FP(Q24_t v)
{
	return ((double)v)/(double)(1<<Q24_SHIFT);
}
#endif
LQ15_t abs_lq15(LQ15_t x)
{
	if (x < 0) return -x;
	else return x;
}

Q15_t abs_q15(Q15_t x)
{
	if (x < 0) return -x;
	else return x;
}
#if 0
Q15_t atan2_q15(Q15_t x, Q15_t y)
{
	return FP_to_Q15(atan2(Q15_to_FP(x), Q15_to_FP(y)));
}
#endif
#if 0
Q15_t sqrt_q15(Q15_t r)
{
	return FP_to_Q15(sqrt(Q15_to_FP(r)));
}
#endif
#if 0
Q15_t pow_q15(Q15_t x, Q15_t y)
{
	return FP_to_Q15(pow(Q15_to_FP(x), Q15_to_FP(y)));
}
#endif
#if 0
Q15_t tan_q15(Q15_t a)
{
	return FP_to_Q15(tan(Q15_to_FP(a)));
}
#endif

#ifdef TEST_Q15
int main(int argc, char **argv)
{
	double pi = 3.1415926;
	double npi, n2d;
	Q15_t	q;
	Q15_t	n2;

	q = FP_to_Q15(pi);
	npi = Q15_to_FP(q);

	n2 = MUL_Q15(FP_to_Q15(pi), FP_to_Q15(2.0f));
	n2d = Q15_to_FP(n2);	

	printf("Values: %f - %f\n", n2d, 1.0);
	
	n2 = DIV_Q15(FP_to_Q15(pi), FP_to_Q15(2.0f));
	n2d = Q15_to_FP(n2);	
	printf("Values: %f - %f\n", n2d, 1.0);

	n2 = RECIP_Q15(FP_to_Q15(2.0f));
	n2d = Q15_to_FP(n2);	
	
	printf("Values: %f - %f\n", n2d, 1.0);

	return 0;	
}
#endif
