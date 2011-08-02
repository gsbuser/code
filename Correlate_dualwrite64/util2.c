#include <math.h>
#include <stdio.h>
#include "xfft.h"
/*
	accumulate to float fcross[NCORR][FFTLEN/2][2]
 */
void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODE*NCHAN][FFTLEN/2/NNODE], int rank)
{
  int i,j,iot,io,jo,jl,ic,jc;
  iot=0;
//	printf("rank %d start %d end %d\n",rank,rank*FFTLEN/NNODE/2,(rank+1)*FFTLEN/NNODE/2);
  for (io=0;io<NCHAN*NNODE;io++) for (jo=io;jo<NCHAN*NNODE;jo++) {
    for(j=0;j<FFTLEN/NNODE/2;j++) for (i=0;i<2;i++) {
      jl=j+rank*FFTLEN/NNODE/2;
      fcross[iot][jl][i]=icross[j/8][iot][i][j%8];
      fcross[iot][jl][i]/=norm[io][j];
      fcross[iot][jl][i]/=norm[jo][j];
      icross[j/8][iot][i][j%8]=0;
    }
    iot++;
  }
}



/*
 write the correlation in compressed form.
 1 byte per complex number.  2 bit scaled exponent, 3 bits mantissa per number.

 or, 2 bytes per complex number.  cross correlation coefficient.
 */
void dumpcorr(int fdout, float fcross[NCORR][FFTLEN/2][2])
{
  float auto[NCHAN*NNODE][FFTLEN/2/NNODE];
  int i,j,iot,io,jo,jl,ic,jc,k;
  iot=0;
  for (io=0;io<NCHAN*NNODE;io++) for (jo=io;jo<NCHAN*NNODE;jo++) {
    if (jo==io)
    for(j=0;j<FFTLEN/NNODE/2;j++) {      
      jl=j+rank*FFTLEN/NNODE/2;
      auto[io][j]=fcross[iot][jl][0];
    }
    iot++;
  }
  /* write auto */
  if (write(fdout,auto,NCHAN*NNODE*FFTLEN/NNODE/2*sizeof(float))<0) perror("write");
  /* inverse sqrt auto */
  for(j=0;j<NCHAN*NNODE;j++) for (i=0;i<FFTLEN/NNODE)  
    auto[j][i]=1/sqrt(auto[j][i]);

  iot=0;
  for (io=0;io<NCHAN*NNODE;io++) for (jo=io+1;jo<NCHAN*NNODE;jo++) {
    for(j=0;j<FFTLEN/NNODE/2;j++) for (i=0;i<2;i++) {      
      jl=j+rank*FFTLEN/NNODE/2;
      ftmp=fcross[iot][jl][i]*auto[io][j]*auto[jo][j];
      itmp[iot][j][i]=ftmp*127;
    }
    iot++;
  }
  /* write itmp */
  i=write(fdout,itmp,NCHAN*NNODE*(NCHAN*NNODE-1)/2*FFTLEN/NNODE);
  if (i<0) perror("write");
}


void dump1bit(int fdout,char *ibuf,int n)
{
  char a,b,c,d;
  int i;
  static char obuf[n/4];
  

  for (i=0;i<n;i+=4){
    a=ibuf[i];
    b=ibuf[i+1];
    c=ibuf[i+2];
    d=ibuf[i+3];
    obuf[i]=a&0x80|(a&0x08)<<3|(b&0x80)>>2|(b&0x08)<<1|
	(c&0x80)>>4|(c&0x08)>>1|(d&0x80)>>6|(d&0x08)>>3;
  }
  /* write obuf to disk */
  i=write(fdout,obuf,n/4);
  if (i<0) perror("write");
}
