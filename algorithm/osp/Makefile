CC=gcc
CFLAGS=-Wall -g -Iinclude -I../../include -DFEAT_STEP

OSP_OBJS=SecondOrderLPF.o ecompass.o fp_atan2.o fp_sqrt.o fp_trig.o fpsup.o gravity_lin.o osp.o rotvec.o step.o tilt.o sigmot.o

all: libOSP.a

libOSP.a: $(OSP_OBJS)
	ar r libOSP.a $(OSP_OBJS)

clean:
	rm -f *.o libOSP.a
