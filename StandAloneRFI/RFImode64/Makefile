FC=ifort -g

all: rfimode.x rfiposlag.x rfipos.x

rfimode.x: fft1.f90 four1.for rfimode.f90 hexprint.f90
	$(FC) fft1.f90 four1.for hexprint.f90 rfimode.f90 -o rfimode.x

rfiposlag.x: rfiposlag.f90
	$(FC) -O3 -openmp rfiposlag.f90 -o rfiposlag.x

rfiposlag_delay.x: rfiposlag_delay.f90
	$(FC) -O3 -openmp rfiposlag_delay.f90 -o rfiposlag_delay.x

rfipos.x: rfipos.f90
	$(FC) -O3 -openmp rfipos.f90 -o rfipos.x
