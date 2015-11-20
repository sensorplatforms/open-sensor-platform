#ifndef _SIGMOT_H_
#define _SIGMOT_H_
#include "fp_sensor.h"

void OSP_sigmot_init(void);
void OSP_sigmot_process(struct ThreeAxis *, struct ThreeAxis *);

#endif
