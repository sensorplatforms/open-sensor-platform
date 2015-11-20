/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 */


/*
 * Compute rotation vector from accel/mag.
 * Based on Android implementation. Converted to fixed point.
 */

#include <stdio.h>
#include "fpsup.h"
#include "fp_sensor.h"
#include "rotvec.h"

#include <math.h>

static Q15_t copysign_q15(Q15_t v, Q15_t s)
{
	if (s < 0) return -v;
	else return v;
}


static Q15_t clamp_q15(Q15_t v)
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
	struct Quat *rot)
{
	Q15_t normH;
	Q15_t accMag2;

	Q15_t magMag2;
	Q15_t Hx, Hy, Hz;
	Q15_t Ex, Ey, Ez;

	Q15_t invA;
	Q15_t invH;

	Q15_t Ax, Ay, Az;
	Q15_t Mx, My, Mz;

	Q15_t qw, qx, qy, qz;

	Ax = acc->x;
	Ay = acc->y;
	Az = acc->z;

	if (Ax == 0 && Ay == 0 && Az == 0) return;

	Ex = mag->x;
	Ey = mag->y;
	Ez = mag->z;

#if 0
	printf("RV using: Ax: %f Ay: %f: Az: %f, Mx: %f, My: %f, Mz: %f\n",
			Q15_to_FP(Ax), Q15_to_FP(Ay), Q15_to_FP(Az),
			Q15_to_FP(Ex), Q15_to_FP(Ey), Q15_to_FP(Ez));
#endif
	Hx = MUL_Q15(Ey, Az) - MUL_Q15(Ez, Ay);
	Hy = MUL_Q15(Ez, Ax) - MUL_Q15(Ex, Az);
	Hz = MUL_Q15(Ex, Ay) - MUL_Q15(Ey, Ax);


	magMag2 = MUL_Q15(Hx,Hx);
	magMag2 += MUL_Q15(Hy, Hy);
	magMag2 += MUL_Q15(Hz, Hz);

	normH = sqrt_q15(magMag2);

	if (normH < FP_to_Q15(0.1f)) {
		printf("Bad mag? %f (from %f)\n", Q15_to_FP(normH), Q15_to_FP(magMag2));
		printf("RV using: Ax: %f Ay: %f: Az: %f, Mx: %f, My: %f, Mz: %f Hx: %f Hy: %f Hz: %f (%f, %f, %f)\n",
			Q15_to_FP(Ax), Q15_to_FP(Ay), Q15_to_FP(Az),
			Q15_to_FP(Ex), Q15_to_FP(Ey), Q15_to_FP(Ez),
			Q15_to_FP(Hx), Q15_to_FP(Hy), Q15_to_FP(Hz),
			Q15_to_FP(MUL_Q15(Hx,Hx)),
			Q15_to_FP(MUL_Q15(Hy,Hy)),
			Q15_to_FP(MUL_Q15(Hz,Hz))
			);
		return;
	}
	invH = RECIP_Q15(normH);
	if (invH <= 0) {
		printf("OF/UF?\n");
	}

	accMag2 = MUL_Q15(Ax, Ax);
	accMag2 += MUL_Q15(Ay, Ay);
	accMag2 += MUL_Q15(Az, Az);

	if (accMag2 < 0) {
		printf("OF? - 2\n");
	}

	invA = RECIP_Q15(sqrt_q15(accMag2));

	Hx = DIV_Q15(Hx, normH);
	Hy = DIV_Q15(Hy, normH);
	Hz = DIV_Q15(Hz, normH);

	Ax = MUL_Q15(Ax, invA);
	Ay = MUL_Q15(Ay, invA);
	Az = MUL_Q15(Az, invA);

	Mx = MUL_Q15(Ay, Hz) - MUL_Q15(Az, Hy);
	My = MUL_Q15(Az, Hx) - MUL_Q15(Ax, Hz);
	Mz = MUL_Q15(Ax, Hy) - MUL_Q15(Ay, Hx);
#if 0
	qw = sqrt_q12(MUL_Q12(clamp_q12( Hx+My+Az+q12_c1), q12_quarter));
	qx = sqrt_q12(MUL_Q12(clamp_q12( Hx-My-Az+q12_c1), q12_quarter));
	qy = sqrt_q12(MUL_Q12(clamp_q12(-Hx+My-Az+q12_c1), q12_quarter));
	qz = sqrt_q12(MUL_Q12(clamp_q12(-Hx-My+Az+q12_c1), q12_quarter));
#else
	{
		Q15_t t;

		t = MUL_Q15(clamp_q15( Hx+My+Az+q15_c1), q15_quarter);
		qw = sqrt_q15(t);

		t = MUL_Q15(clamp_q15( Hx-My-Az+q15_c1), q15_quarter);
		qx = sqrt_q15(t);

		t = MUL_Q15(clamp_q15(-Hx+My-Az+q15_c1), q15_quarter);
		qy = sqrt_q15(t);

		t = MUL_Q15(clamp_q15(-Hx-My+Az+q15_c1), q15_quarter);
		qz = sqrt_q15(t);
	}
#endif

	qx = copysign_q15(qx, Ay - Mz);
	qy = copysign_q15(qy, Hz - Ax);
	qz = copysign_q15(qz, Mx - Hy);

	rot->x = qx;
	rot->y = qy;
	rot->z = qz;
	rot->w = qw;
}
