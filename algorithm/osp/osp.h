#ifndef OSP_H
#define OSP_H	1

#include "osp-fixedpoint-types.h"
#include "fpsup.h"
#if 0
void OSP_init(void);
void OSPalg_Process(void);
void OSPalg_SetDataAcc(Q15_t, Q15_t, Q15_t, NTTIME);
void OSPalg_SetDataMag(Q15_t, Q15_t, Q15_t, NTTIME);
void OSPalg_SetDataGyr(Q15_t, Q15_t, Q15_t, NTTIME);
void OSPalg_cal(void);
void OSPalg_EnableSensorCB(unsigned int, void (*ready)(struct Results *, int));
void OSPalg_DisableSensor(unsigned int);
#endif
#endif