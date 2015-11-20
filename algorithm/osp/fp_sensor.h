#ifndef FP_SENSOR_H
#define FP_SENSOR_H	1

#include "fpsup.h"
#include "osp-sensors.h"

/* NTEXTENDED is int32_t Q12 */
/* NTPRECISE is int32_t Q24 */
#define Q15_to_NTEXTENDED(x)	(x >> (Q15_SHIFT-12))
#define Q15_to_NTPRECISE(x)	(x << (24-Q15_SHIFT))
#define NTEXTENDED_to_Q15(x)	(x << (Q15_SHIFT-12))
#define NTPRECISE_to_Q15(x)	(x >> (24-Q15_SHIFT))

#define FLAG(x) (1 << x)

struct ThreeAxis {
	Q15_t x;
	Q15_t y;
	Q15_t z;
};

struct Euler {
	Q15_t roll;
	Q15_t pitch;
	Q15_t yaw;
};

struct Quat {
	Q15_t x;
	Q15_t y;
	Q15_t z;
	Q15_t w;
};

#ifdef FEAT_ROTVEC_PRECISE
struct Quat_precise {
	Q24_t x;
	Q24_t y;
	Q24_t z;
	Q24_t w;
};
#endif

struct StepInfo {
	int32_t	count;
	int32_t detect;
};

struct Results {
	union {
		struct ThreeAxis result;
		struct Euler euler;
#ifdef FEAT_ROTVEC_PRECISE
		struct Quat_precise quat;
#else
		struct Quat quat;
#endif
#ifdef FEAT_STEP
		struct StepInfo step;
#endif
	} ResType;
	uint32_t time;
};

#endif
