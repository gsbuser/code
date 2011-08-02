#-----------------------------------------------------------------------------
#title: Acquisition Logic A/D Board Kernel Driver Checkout Program
#version: Linux 1.0
#date:  March 2004
#designer: Michael Wyrick                                                   
#programmer: Michael Wyrick                                                
#platform: Linux 2.4.x
#language: GCC 3.3.1
#module: BoardAL4108
#-----------------------------------------------------------------------------
#  Purpose: Library for the AL4108
#  Docs:                                  
#-----------------------------------------------------------------------------
#
#-----------------------------------------------------------------------------
INCLUDEDIR	= -I/opt/intel/ipp/5.2/ia32/include
# note: -O3 seems to crash on the quad cores
CFLAGS		= -O3 -openmp -g -xN -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 ${INCLUDEDIR} -DPARALLEL -DOLDCLOCK # -ggdb -c -Wall 
#CFLAGS		= -O0 -g -openmp -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 ${INCLUDEDIR} -DPARALLEL 
CC              = /opt/openmpi-1.2.2/bin/mpicc #gcc
#CC             = mpicc
CXX   = /opt/openmpi-1.2.2/bin/mpiCC
ICXX   = icc
#CXX            = mpiCC
TARGET		= mpi_dual

SOURCES	        = dual.cpp reg.cpp Board4108.cpp Dacs.cpp EEPROM_24xx64.cpp I2C_4108.cpp BoardEnum.cpp
LIBS            = -lipps -lippm -L/opt/intel/ipp/5.2/ia32/sharedlib#-lstdc++
OBJECTS		= ${SOURCES:.cpp=.o} xfft.o xmac.o util.o ippfft.o rtcorr.o
HEADERS		= Exceptions.h I2C_4108.h EEPROM_24xx64.h Board4108.h xfft.h

.SUFFIXES:
.SUFFIXES:	.cpp .c .o

.cpp.o:
	${CC} ${CFLAGS} -o ${<:.cpp=.o} $<

${TARGET}:	${OBJECTS}
	${CXX} -openmp -o ${TARGET} ${CFLAGS} ${OBJECTS} ${LIBS}  four1.o fftnr2.o

xmac.o: xmac.c
	${CC} ${CFLAGS} -c $<

xfft.o: xfft.c
	${CC} ${CFLAGS} -c $<

rtcorr.o: rtcorr.c
	${CC} ${CFLAGS} -c $<

dual.o: dual.cpp
	${CXX} ${CFLAGS} -c $<

reg.o: reg.cpp
	${CXX} ${CFLAGS} -c $<

Board4108.o: Board4108.cpp
	${ICXX} ${CFLAGS} -c $<

Dacs.o: Dacs.cpp
	${CXX} ${CFLAGS} -c $<

EEPROM_24xx64.o: EEPROM_24xx64.cpp
	${CXX} ${CFLAGS} -c $<

I2C_4108.o: I2C_4108.cpp
	${CXX} ${CFLAGS} -c $<

BoardEnum.o: BoardEnum.cpp
	${ICXX} ${CFLAGS} -c $<

${OBJECTS}:	${SOURCES} ${HEADERS}

clean:
	rm -f ${OBJECTS} ${TARGET} *~

