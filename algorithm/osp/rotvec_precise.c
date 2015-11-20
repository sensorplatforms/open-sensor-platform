/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 */


/*
 * Precise version of rotation vector using Q24's.
 * Based on Android implementation. Converted to fixed point.
 */
#include <stdio.h>
#include "fpsup.h"
#include "fp_sensor.h"
#include "rotvec.h"

static Q24_t copysign_q24(Q24_t v, Q24_t s)
{
	if (s < 0) return -v;
	else return v;
}


static Q24_t clamp_q24(Q24_t v)
{
	if (v < 0) return 0;
	return v;
}

void OSP_rotvec_init()
{
}

void OSP_rotvec_process(
	struct ThreeAxis *mag,
	struct ThreeAxis *acc,
	struct Quat_precise *rot)
{
	Q24_t normH;
	Q24_t magMag2, accMag2;
	Q24_t invH, invA;
	Q24_t Hx, Hy, Hz;
	Q24_t Ax, Ay, Az;
	Q24_t Mx, My, Mz;
	Q24_t Ex, Ey, Ez;

	Q15_t qw, qx, qy, qz;

	Ax = q15_to_q24(acc->x);
	Ay = q15_to_q24(acc->y);
	Az = q15_to_q24(acc->z);

	Ex = q15_to_q24(mag->x);
	Ey = q15_to_q24(mag->y);
	Ez = q15_to_q24(mag->z);
#if 0
	printf("RV using: Ax: %f Ay: %f: Az: %f, Mx: %f, My: %f, Mz: %f\n",
			Q15_to_FP(Ax), Q15_to_FP(Ay), Q15_to_FP(Az),
			Q15_to_FP(Ex), Q15_to_FP(Ey), Q15_to_FP(Ez));
#endif
	Hx = MUL_Q24(Ey, Az) - MUL_Q24(Ez, Ay);
	Hy = MUL_Q24(Ez, Ax) - MUL_Q24(Ex, Az);
	Hz = MUL_Q24(Ex, Ay) - MUL_Q24(Ey, Ax);

	magMag2 = MUL_Q24(Hx,Hx);
	magMag2 += MUL_Q24(Hy, Hy);
	magMag2 += MUL_Q24(Hz, Hz);

	normH = sqrt_q24(magMag2);

	if (normH < FP_to_Q24(0.1f)) {
		printf("Bad mag? %f\n", Q24_to_FP(normH));
		return;
	}
	invH = RECIP_Q24(normH);

	accMag2 = MUL_Q24(Ax, Ax);
	accMag2 += MUL_Q24(Ay, Ay);
	accMag2 += MUL_Q24(Az, Az);

	invA = RECIP_Q24(sqrt_q24(accMag2));

	Hx = MUL_Q24(Hx, invH);
	Hy = MUL_Q24(Hy, invH);
	Hz = MUL_Q24(Hz, invH);

	Ax = MUL_Q24(Ax, invA);
	Ay = MUL_Q24(Ay, invA);
	Az = MUL_Q24(Az, invA);

	Mx = MUL_Q24(Ay, Hz) - MUL_Q15(Az, Hy);
	My = MUL_Q24(Az, Hx) - MUL_Q15(Ax, Hz);
	Mz = MUL_Q24(Ax, Hy) - MUL_Q15(Ay, Hx);

	qw = sqrt_q24(MUL_Q24(clamp_q24( Hx+My+Az+q24_c1), q24_quarter));
	qx = sqrt_q24(MUL_Q24(clamp_q24( Hx-My-Az+q24_c1), q24_quarter));
	qy = sqrt_q24(MUL_Q24(clamp_q24(-Hx+My-Az+q24_c1), q24_quarter));
	qz = sqrt_q24(MUL_Q24(clamp_q24(-Hx-My+Az+q24_c1), q24_quarter));

	qx = copysign_q24(qx, Ay - Mz);
	qy = copysign_q24(qy, Hz - Ax);
	qz = copysign_q24(qz, Mx - Hy);

	rot->x = qx;
	rot->y = qy;
	rot->z = qz;
	rot->w = qw;
}
