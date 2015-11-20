#ifndef _GRAVITY_LIN_H_
#define _GRAVITY_LIN_H_	1
#include "fp_sensor.h"

void OSP_gravity_init(void);
void OSP_gravity_process(struct ThreeAxis *acc, struct ThreeAxis *res);

void OSP_linear_acc_init(void);
void OSP_linear_acc_process(struct ThreeAxis *acc,
			struct ThreeAxis *gravity,
			struct ThreeAxis *res);

#endif
