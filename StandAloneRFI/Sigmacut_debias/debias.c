#include "mpi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <fenv.h>
#include "xfft.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


#define PROFILE
#define NEPOCH 14341
//#define NEPOCH 100
#define NGATE NFOLD
#define PRANK 1
#define MAX(x,y) ((x>y)?x:y)

int main(int argc, char *argv[])
{
  static int iepoch;
  static int fdout,rank,fdin;
  static float fcross[NCROSS/NNODE];
  static double cross0[NGATE][NCROSS/NNODE];
  int size;
  int i,ii,jj;
  char fn[255],fnout[255],cmd[255];
  int rsize=NCROSS/NNODE*NGATE;
  

  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );

  
  MPI_Init(NULL,NULL);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  /* open files */
  if (rank == PRANK) {
    printf("rank=%d,size=%d\n",rank,size);
    if (argc != 4) {
      printf("argc=%d ",argc);
      for (i=0;i<argc;i++) printf("argv=%s ",argv[i]);
      fprintf(stderr,"usage: %s corrfile.out corrfile.in\n",argv[0]);
      exit(-1);
    }
  }

  if (atoi(argv[3])>=0) rank=atoi(argv[3]);
  sprintf(fn,argv[1],rank);
  printf("opening %s for read\n",fn);
  fdin=open(fn,O_RDONLY);
  if (fdin<0) {
    fprintf(stderr,"rank=%d fdin=%s\n",rank, fn);
    fflush(stderr);
    perror("open sigmacutin.dat");
    exit(-1);
  }
  sprintf(fnout,argv[2],rank);
  //sprintf(cmd,"cp %s %s\n",fn,fnout);
  sprintf(cmd,"./debiasf.x %d %d %d %s %s\n",NEPOCH,FFTNODE,NPOD,fn,fnout);
  system(cmd);
//  rename(fn, fnout);
  printf("debias %s %s \n",fn,fnout);
  exit(0) ;



  fdout=open(fnout,O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
  if (fdout<0) {printf("file %s ",fn); fflush(stdout); perror("open sigmacutout.dat");}
 
  /* debias */

  /* calculate the bias */
  for (jj=0;jj<NGATE;jj++) for(ii=0;ii<(NCROSS/NNODE);ii++) cross0[jj][ii]=0;

  for(iepoch=0;iepoch<(NEPOCH);iepoch++) {
    for (jj=0;jj<NGATE;jj++) {
      read1byte(fdin,fcross,rsize);
      for (ii=0;ii<(NCROSS/NNODE);ii++) {
	cross0[jj][ii]+=fcross[ii]/(NEPOCH);
      }
    }
  }
  printf("rank %d done bias\n",rank);

  /* rewind */
  if (lseek(fdin,0,SEEK_SET) !=0) {
    perror("lseek");
    printf("lseek error after calculating bias\n");
    fflush(stdout);
    exit(-8);
  }


  for(iepoch=0;iepoch<(NEPOCH);iepoch++) {
    for (jj=0;jj<NGATE;jj++) {
      read1byte(fdin,fcross,rsize);
      for (ii=0;ii<(NCROSS/NNODE);ii++) {
	    fcross[ii]-=cross0[jj][ii];
      }
      dump1byte(fdout,fcross,(NCROSS/NNODE)*NGATE);
    }
  }
  MPI_Finalize();
}

void read1byte(int fd, float *fbuf, int size)
{
  int i,size0;
  static char ibuf[FFTNODE];
  float fmax,ftmp;
  size0=size;

  while (size>FFTNODE-2) {
    if (i=read(fd,&fmax,sizeof(float)) != sizeof(float)) {
	perror("read1byte:read fmax");
	printf("fd=%d size=%d\n",fd,i);
	MPI_Finalize();
	exit(-1);
    }
    if (fmax>1000000) {
	printf("fmax=%f size=%d size0=%d\n",fmax,size0-size,size0);
	MPI_Finalize();
	exit(-4);
    }
    if (read(fd,ibuf,sizeof(char)*FFTNODE) != sizeof(char)*FFTNODE ) {
	perror("read1byte:read ibuf");
	MPI_Finalize();
	exit(-1);
    }
    for (i=0;i<FFTNODE;i++) {
      ftmp=ibuf[i]/127.;
      ftmp=(ibuf[i]>0)?ftmp*ftmp:-ftmp*ftmp;
      fbuf[i]=fmax*ftmp;
    }
    fbuf+=FFTNODE;
    size-=FFTNODE;
  }
}

#define MAX(x,y) ((x>y)?x:y)
void dump1byte(int fd, float *fbuf, int size)
{
  int i;
  static char obuf[FFTLEN/NNODE];
  float fmax,ftmp;

  while (size>FFTLEN/NNODE-2) {
    fmax=0;
    for (i=0;i<FFTLEN/NNODE;i++) fmax=MAX(fmax,fabs(fbuf[i]));
    for (i=0;i<FFTLEN/NNODE;i++) {
      ftmp=sqrt(fabs(fbuf[i]/fmax));
      ftmp=(fbuf[i]>0)?ftmp:-ftmp;
      obuf[i]=lroundf(127*ftmp);
    }
    if (write(fd,&fmax,sizeof(float)) != sizeof(float)) perror("dump1byte:write");
    if (write(fd,obuf,FFTLEN/NNODE) != FFTLEN/NNODE ) perror("dump1byte:write");
    fbuf+=FFTLEN/NNODE;
    size-=FFTLEN/NNODE;
  }
}


int compare_floats (const float * a, const float * b)
{
  return ( (int)(*a - *b) );
}

