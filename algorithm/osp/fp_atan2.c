/*
 * Approximate atan2
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "fpsup.h"

/*
 * arctan of y/x
 */

static const Q15_t q15_pi_4 = FP_to_Q15(M_PI/4.0);

Q15_t atan2_q15(Q15_t y, Q15_t x)
{
	const Q15_t q15_3_4_PI = FP_to_Q15(3.0*M_PI/4.0f);
	Q15_t abs_y, r, rt, angle;

	if (y == 0 && x < 0) return q15_pi;
	if (y == 0 && x > 0) return 0;
	if (y < 0  && x == 0) return -(q15_pi >> 1); /* -PI/2 */
	if (y > 0  && x == 0) return (q15_pi >> 1);  /*  PI/2 */
	if (y == 0 && x == 0) return q15_pi;
	
	abs_y = abs_q15(y);

	if (x >= 0) {
		rt = RECIP_Q15(x+abs_y);
		r = MUL_Q15((x-abs_y), rt);	

		angle = q15_pi_4 - MUL_Q15(q15_pi_4, r);
	} else {
		rt = RECIP_Q15(abs_y-x);
		r = MUL_Q15((x+abs_y), rt);
		angle = q15_3_4_PI - MUL_Q15(q15_pi_4, r);
	}

	if (y < 0)
		return -angle;
	else
		return angle;
}
#ifdef TEST
int main(int argc, char **argv)
{
	float ang;
	float ang_rad;
	Q15_t qy, qx, qat;
	float fy, fx, fat;
	float err = 0.0;

	if (argc > 1) {
		err = atof(argv[1]);
		if (err > 2.0)
			err = 0.0;
	}

	fprintf(stderr, "Printing errors greater then %f\n", err);

	for (ang = 0; ang < 360.0f; ang+= 0.5) {
		ang_rad = ang * M_PI/180.0f;
		qx = q15_c1;
		fx = 1.0;
		fy = tan(ang_rad);
		qy = FP_to_Q15(fy);

		fat = atan2(fy, fx);
		qat = atan2_q15(qy, qx);
		if ((Q15_to_FP(qat)-fat) > err) {

			fprintf(stderr,
				"ANG: %f - tan = %f (%f), atan() = %f (%f) err = %f\n",
				ang, fy, Q15_to_FP(qy),
				fat, Q15_to_FP(qat),
				Q15_to_FP(qat)-fat);
		}
	}
}

#endif
