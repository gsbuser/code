# Object files, for use in linker
CC= /opt/openmpi-1.2.2/bin/mpicc 
FC=ifort

NOVAS = novas-c201/novas.o novas-c201/novascon.o novas-c201/solsys3.o\
        novas-c201/readeph0.o
FOBJ = Gibbs.o
LOCALOBJ = util2.o read_data.o utils.o froutines.o beam_model.o sampling.o
OBJ = $(LOCALOBJ) $(FOBJ) $(NOVAS) -lm
COBJ = util2.o read_data.o utils.o sampling.o $(NOVAS) -lm
MOBJ = fft1mkl.o fringestop.o util2.o read_data.o utils.o $(NOVAS) -lm
MOBJno = fft1mkl.o fringestop.nofs0.o util2.o read_data.o utils.o $(NOVAS) -lm
MOBJ2 = fringestop_fs0.o util2.o read_data.o utils.o $(NOVAS) -lm
MOBJ3 = fringestop_fsall.o util2.o read_data.o utils.o $(NOVAS) -lm
MOBJ4 = fringestop_orig.o util2.o read_data.o utils.o $(NOVAS) -lm
MOBJ5 = caldt.o  read_data.o utils.o $(NOVAS) -lm

# FLAGS:
#
# Aside from the 'obvious' (warning and optimization) flags
# there are several other options:
#
# CFLAGS:
#   -DSAVE_MEM = saves memory using NR style pointer references
#     (not official ANSI C, but works on $(CC))
#
# FFLAGS:
#   (none yet)
#
#CFLAGS = -g -O3 -Wall -DSAVE_MEM -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 #-openmp
CFLAGS = $(CFLAGS1) -g -O3 -Wall   -DSAVE_MEM -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1 -openmp
FFLAGS =  -save -O3 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_LARGEFILE_SOURCE=1  -openmp 
LIB= -L/opt/intel/fc/9.1.043/lib/ -lifcoremt -L /opt/intel/mkl/lib/32/ /opt/intel/mkl/lib/32/libmkl_p4.a #-lmkl_p4
.SUFFIXES:      .f90 .c .o


.f90.o:
	$(FC) $(FFLAGS) -c $< 

.c.o:
	$(CC) $(CFLAGS) -c $< 

# main programs
fringestop.x:  $(MOBJ)
	$(CC) $(CFLAGS) -o $@ $(MOBJ)  $(LIB)

fringestop.nofs0.x:  $(MOBJno)
	$(CC) $(CFLAGS) -o $@ $(MOBJno)  $(LIB)

fringestop_fs0.x:  $(MOBJ2)
	$(CC) -static $(CFLAGS) -o $@ $(MOBJ2)  $(LIB)

fringestop_fsall.x:  $(MOBJ3)
	$(CC) -static $(CFLAGS) -o $@ $(MOBJ3)  $(LIB)

fringestop_orig.x:  $(MOBJ4)
	$(CC) -static $(CFLAGS) -o $@ $(MOBJ4)  $(LIB)

caldt.x: $(MOBJ5)
	$(CC) -static $(CFLAGS) -o $@ $(MOBJ5)  $(LIB)

clean:
	rm *.o

