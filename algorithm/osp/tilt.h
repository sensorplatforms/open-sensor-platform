#ifndef _TILT_H_
#define _TILT_H_

#include "fp_sensor.h"
void OSP_tilt_init(void);

void OSP_tilt_process(struct ThreeAxis *acc, struct ThreeAxis *tilt);

#endif
