/*
 * (C) Copyright 2015 HY Research LLC
 *     Author: hy-git@hy-research.com
 *
 * Apache License.
 *
 */

/* Compute a tilt sensor as defined by Android */
#include "fpsup.h"
#include "tilt.h"

#define NUM_SAMPLE2	50
#define NUM_SAMPLE1	25

static struct ThreeAxis accHist[NUM_SAMPLE2];
static struct ThreeAxis prevMean;
int sampcount;
int samp_head, samp_tail;

#define ABS(x) ((x > 0)?x:-x)

void OSP_tilt_init(void)
{
	sampcount = 0;
	samp_head = 49, samp_tail = 0;
}

static void computeMean(struct ThreeAxis *mean, struct ThreeAxis *buf, int beg, int end)
{
	int i, count;

	count = 0;
	mean->x = 0; mean->y = 0; mean->z = 0;
	for (i = beg; i != end; i++, i %= NUM_SAMPLE2) {
		mean->x += buf[i].x;
		mean->y += buf[i].y;
		mean->z += buf[i].z;
		count++;
	}
	mean->x = DIV_Q15(mean->x, INT_to_Q15(count));
	mean->y = DIV_Q15(mean->y, INT_to_Q15(count));
	mean->z = DIV_Q15(mean->z, INT_to_Q15(count));
}

static Q15_t dotProduct(struct ThreeAxis *v1, struct ThreeAxis *v2)
{
	Q15_t res;

	res = MUL_Q15(v1->x, v2->x);	
	res += MUL_Q15(v1->y, v2->y);	
	res += MUL_Q15(v1->z, v2->z);	

	return res;
}

static Q15_t norm(struct ThreeAxis *v1)
{
	Q15_t sum;

	sum = MUL_Q15(v1->x, v1->x);
	sum += MUL_Q15(v1->y, v1->y);
	sum += MUL_Q15(v1->z, v1->z);
	return sqrt_q15(sum);
}

static Q15_t computeAngle(struct ThreeAxis *v1, struct ThreeAxis *v2)
{
	Q15_t v, n;

	v = dotProduct(v1, v2);
	n = MUL_Q15(norm(v1), norm(v2));
	v = DIV_Q15(v, n);

	return arccos_q15(v);	
}

/* Assumes the sensor runs at 50Hz */
void OSP_tilt_process(struct ThreeAxis *acc, struct ThreeAxis *tilt)
{
	struct ThreeAxis cur;
	int ang = 0;

	if (sampcount > 10000) {
		tilt->x = 0;
		return;
	}


	accHist[samp_head] = *acc;		

	if (sampcount < (NUM_SAMPLE2-1)) {
		if (sampcount == (NUM_SAMPLE1-1)) {
			computeMean(&prevMean, accHist, 0, NUM_SAMPLE1);
		}
		sampcount++;
		samp_head++;
		samp_head %= NUM_SAMPLE2;
		tilt->x = 0;
		return;
	}

	computeMean(&cur, accHist, samp_tail, samp_head);
	samp_head++; samp_head %= NUM_SAMPLE2;
	samp_tail++; samp_tail %= NUM_SAMPLE2;
	ang = computeAngle(&cur, &prevMean);
	if (ABS(ang) > INT_to_Q15(35)) {
		tilt->x = 1;
		sampcount = 99999;	
	}
	return;
}
