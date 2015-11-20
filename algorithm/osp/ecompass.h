#ifndef _ECOMPAAS_H_
#define _ECOMPASS_H_	1
#include "fp_sensor.h"

/* Start a cal sequence */
void OSP_ecompass_cal(void);
void OSP_ecompass_process(struct ThreeAxis *,
		struct ThreeAxis *,
		struct Euler *result);
void OSP_ecompass_init(void);

#endif
