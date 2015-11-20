#ifndef _ROTVEC_H_
#define _ROTVEC_H_

#include "fp_sensor.h"

void OSP_rotvec_init(void);
#ifdef FEAT_ROTVEC_PRECISE
void OSP_rotvec_process(struct ThreeAxis *, struct ThreeAxis *, struct Quat_precise *);
#else
void OSP_rotvec_process(struct ThreeAxis *, struct ThreeAxis *, struct Quat *);
#endif

#endif
