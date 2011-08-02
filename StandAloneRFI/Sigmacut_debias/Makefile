CC=mpicc
FC=ifort
MC=mpif77 -auto

MOBJ = debias.o -lm

CFLAGS = $(CFLAGS1) -g -O3 -Wall   -DSAVE_MEM -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 #-openmp
FFLAGS=-openmp -g -O3 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 


all: debias.x debiasf.x

.SUFFIXES:	.f90 .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

debias.x: $(MOBJ)
	$(CC) $(CFLAGS) -o $@ $(MOBJ) $(LIB)

debiasf.x: sigmacut_debias.f90 qsort.f90
	$(FC) $(FFLAGS) sigmacut_debias.f90 qsort.f90 -o debiasf.x


clean:
	rm *.o