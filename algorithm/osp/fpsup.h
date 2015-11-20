#ifndef FP_SUP_H
#define FP_SUP_H	1
#include <stdint.h>

#ifndef M_PI
#define M_PI	3.141592f
#endif
typedef int32_t	Q15_t;
#define Q15_SHIFT	15
#define FP_to_Q15(v)	((v)*(1<<Q15_SHIFT))
#define INT_to_Q15(v)	((v)<< Q15_SHIFT)

typedef int64_t LQ15_t;

#ifdef ENABLE_Q24
typedef int32_t	Q24_t;
#define Q24_SHIFT	24
#define FP_to_Q24(v)	((v)*(1<<Q24_SHIFT))
#define q15_to_q24(v)	((v) << (Q24_SHIFT-Q15_SHIFT))
#endif

#ifdef ENABLE_Q12
typedef int32_t Q12_t;
#define Q12_SHIFT	12

Q12_t MUL_Q12(Q12_t, Q12_t);
Q12_t sqrt_q12(Q12_t);
#define FP_to_Q12(v)	((v)*(1<<Q12_SHIFT))
#define Q15_to_Q12(x) ((x) >> (Q15_SHIFT-Q12_SHIFT))
#define Q12_to_Q15(x) ((x) << (Q15_SHIFT-Q12_SHIFT))
Q12_t RECIP_Q12(Q12_t);
Q12_t DIV_Q12(Q12_t, Q12_t);
#endif

#define MAX_Q15	0x7fffffff
#define MIN_Q15	0x80000000

Q15_t DIV_Q15(Q15_t, Q15_t);
LQ15_t DIV_LQ15(LQ15_t, LQ15_t);
Q15_t MUL_Q15(Q15_t, Q15_t);
LQ15_t MUL_LQ15(LQ15_t, LQ15_t);
Q15_t RECIP_Q15(Q15_t);
LQ15_t RECIP_LQ15(LQ15_t);

extern const Q15_t q15_pi;
extern const Q15_t q15_c360;
extern const Q15_t q15_c180;
extern const Q15_t q15_c90;
extern const Q15_t q15_c1;
extern const Q15_t q15_c2;
extern const Q15_t q15_half;
extern const Q15_t q15_quarter;

#ifdef ENABLE_Q12
const Q12_t q12_pi;
const Q12_t q12_c360;
const Q12_t q12_c180;
const Q12_t q12_c90;
const Q12_t q12_c1;
const Q12_t q12_c2;
const Q12_t q12_half;
const Q12_t q12_quarter;
#endif

#ifdef ENABLE_Q24
Q24_t DIV_Q24(Q24_t, Q24_t);
Q24_t MUL_Q24(Q24_t, Q24_t);
Q24_t RECIP_Q24(Q24_t);

const Q24_t q24_pi;
const Q24_t q24_c360;
const Q24_t q24_c180;
const Q24_t q24_c90;
const Q24_t q24_c1;
const Q24_t q24_c2;
const Q24_t q24_half;
const Q24_t q24_quarter;

double Q24_to_FP(Q24_t);
#endif
double Q15_to_FP(Q15_t);
double LQ15_to_FP(LQ15_t);

#ifdef ENABLE_Q12
double Q12_to_FP(Q12_t);
#endif

Q15_t atan2_q15(Q15_t, Q15_t);
Q15_t tan_q15(Q15_t);
Q15_t sqrt_q15(Q15_t);
Q15_t arccos_q15(Q15_t);
Q15_t arcsin_q15(Q15_t);
#ifdef ENABLE_Q24
Q15_t sqrt_q24(Q15_t);
#endif

Q15_t pow_q15(Q15_t, Q15_t);
Q15_t abs_q15(Q15_t);
LQ15_t abs_lq15(LQ15_t);
#endif
