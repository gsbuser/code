#FC=/opt/openmpi-1.2.2/bin/mpif77
FC=mpif77
FFLAGS=-O0 -g -fpe0 -auto -openmp #-mcmodel=large
FFLAGS0=-O0 -g -fpe0 -auto -openmp 
FFLAGS1=-O3 -g -fpe0 -auto -openmp 
FFLAGS3=-O3 -g -fpe0 -auto -openmp -parallel 
#LIB=-L /opt/intel/mkl/9.0/lib/em64t/ -lmkl_lapack -lmkl_em64t   #-lmkl_def -lmkl -lmkl_lapack /usr/lib/gcc-lib/i386-redhat-linux/2.96/libg2c.a
#LIB=-L/opt/intel/mkl/lib/32 -L/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/ -llapack -lg2c #-lmkl_lapack -lmkl  -lg2c
#LIB=-L/opt/intel/mkl/10.0.3.020/lib/32 -Wl,--start-group -Wl,--end-group -lguide -lpthread -lmkl_intel_thread /opt/intel/mkl/10.0.3.020/lib/32/libmkl_lapack.a -lmkl_core #/opt/intel/mkl/10.0.3.020/lib/32/libmkl_core.a 
#LIB=-L/opt/intel/mkl/10.1.0.015/lib/32 -Wl,--start-group -lmkl_intel_thread -lmkl_core -Wl,--end-group -lguide -lpthread /opt/intel/mkl/10.1.0.015/lib/32/libmkl_lapack.a
LIB=-L/opt/intel/mkl/10.0.3.020/lib/32 -lmkl_intel -lmkl_intel_thread -lmkl_core -liomp5 -lpthread
#LIB=-L/opt/intel/mkl/10.1.0.015/lib/32  -lmkl_core -lmkl_intel_thread -lmkl_lapack




all: svd_uvec.x svd_vvec.x svd_model.x

svd_parallel.x: svd_parallel.f90
	$(FC) $(FFLAGS) $< -o $@  $(LIB)

testdgemm.x: testdgemm.f90
	$(FC) $(FFLAGS0) $< -o $@  $(LIB)

svd_uvec.x: svd_uvec.f90 hexprint.f90
	$(FC) $(FFLAGS0) $< hexprint.f90 -o $@  $(LIB)

svd_test.x: svd_test.f90 hexprint.f90
	$(FC) -i_dynamic -mcmodel=medium $(FFLAGS0) $< hexprint.f90 -o $@  $(LIB)

svd_vvec.x: svd_vvec.f90 hexprint.f90
	$(FC) $(FFLAGS1) $< hexprint.f90 -o $@  $(LIB)

svd_model.x: svd_model.f90 hexprint.f90
	$(FC) $(FFLAGS1) $< hexprint.f90 -o $@  $(LIB)

run: svd_parallel.x
	time mpirun -np 16 ./svd_parallel.x 

rfimode.x:rfimode.f90 fft1.f90 four1.for
	ifort rfimode.f90 fft1.f90 four1.for -o rfimode.x

rfiposlag.x:rfiposlag.f90
	ifort -O3 -openmp rfiposlag.f90 -o rfiposlag.x

clean:
	rm *.o *.x

