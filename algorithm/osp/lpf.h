#ifndef LPF_H
#define LPF_H	1

struct LPF {
	Q15_t iQ;
	Q15_t fc;
	Q15_t K;
	Q15_t iD;
	Q15_t a0;
	Q15_t a1;
	Q15_t b1;
	Q15_t b2;

	Q15_t x1;
	Q15_t x2;
	Q15_t y1;
	Q15_t y2;
};

struct LPF_CBQF {
	struct LPF mA;
	struct LPF mB;
};

void LPF_init(struct LPF *lpf, Q15_t Q, Q15_t fc);
void LPF_setSamplingPeriod(struct LPF *lpf, Q15_t dT);
Q15_t LPF_BQF_init(struct LPF *lpf, Q15_t x);
Q15_t LPF_BQF_data(struct LPF *lpf, Q15_t x);
void LPF_CBQF_init(struct LPF_CBQF *clpf, struct LPF *lpf, Q15_t x);
Q15_t LPF_CBQF_data(struct LPF_CBQF *clpf, Q15_t x);




#endif
