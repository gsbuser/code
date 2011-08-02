#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include "xfft.h"
#include <string.h>


#define MAX(x,y) ((x>y)?x:y)
/*
	accumulate to float fcross[NCORR][FFTLEN/2][2]
 */
void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODECORR*NCHAN][FFTLEN/2/NNODECORR], int rank)
{
  int i,j,iot,io,jo,jl,ic,jc;
  iot=0;
//	printf("rank %d start %d end %d\n",rank,rank*FFTLEN/NNODECOR/2,(rank+1)*FFTLEN/NNODECORR/2);
  for (io=0;io<NCHAN*NNODECORR;io++) for (jo=io;jo<NCHAN*NNODECORR;jo++) {
    for(j=0;j<FFTLEN/NNODECORR/2;j++) for (i=0;i<2;i++) {
      jl=j+rank*FFTLEN/NNODECORR/2;
      fcross[iot][jl][i]=icross[j/8][iot][i][j%8];
      fcross[iot][jl][i]/=norm[io][j];
      fcross[iot][jl][i]/=norm[jo][j];
      icross[j/8][iot][i][j%8]=0;
    }
    iot++;
  }
}
#ifdef OLDDUMP
void dumpcorr(char *obuf, int *corr, struct timeval tv)
{
	int i,j,count,dmax;
//	static float fmap[NCROSS*NFOLD/NNODECORR];
	double tmp;

	count=NCROSS*NFOLD/NNODECORR;

	dmax=0;
	for (i=0;i<count;i++) 
		dmax=MAX(abs(corr[i]),dmax);
	if (dmax>1000000) {
		printf("dmax=%d\n",dmax);
		exit(-1);
	}
	memcpy(obuf,&dmax,sizeof(int));
	obuf+=sizeof(int);
	memcpy(obuf,&tv,sizeof(tv));
	obuf+=sizeof(tv);
	for (i=0;i<count;i++) {
		tmp=corr[i];
		tmp/=dmax;
		tmp=sqrt(fabs(tmp));
		tmp=(corr[i]>0)?tmp:-tmp;
		obuf[i]=127*tmp;
		corr[i]=0;
	}

	return;
}

#else
void dumpcorr(char *obuf, int corr[NFOLD*FFTLEN/16/NNODECORR][NCORR*2*8], struct timeval tv)
{
/* for gated output, write only the gates that are not empty.
   For slow pulsars, this is a substantial space gain.
   */
        int i,j,count,dmax,lcount,i1;
        char lmap[NFOLD*FFTLEN/16/NNODECORR];
        int imap[NFOLD*FFTLEN/16/NNODECORR];
	float fmap[NCORR];
        double tmp;
	struct CorrHeader dumpheader;

        
#pragma omp parallel for default(none) private(j) shared(lmap,corr)
        for (i=0;i<NFOLD*FFTLEN/16/NNODECORR;i++) {
		lmap[i]=0;
		for (j=0;j<NCORR*2*8;j++) {
			lmap[i]|= (corr[i][j]!=0);
		}
	}
	lcount=0;
	for (i=0;i<NCORR;i++) fmap[i]=0;
        for (i=0;i<NFOLD*FFTLEN/16/NNODECORR;i++) {
		if (lmap[i]>0) {
			imap[lcount]=i;
			lcount++;
#pragma omp parallel for default(none) shared(fmap,corr,i)
			for (j=0;j<NCORR*2*8;j++) {
				fmap[j/16]=MAX(fmap[j/16],abs(corr[i][j])+1);
			}
		}
	}

        count=NCROSS*NFOLD/NNODECORR;
	dumpheader.lcount=lcount;
	dumpheader.tv=tv;
	dumpheader.nfreq=FFTLEN/NNODECORR;
	dumpheader.nfold=NFOLD;
	dumpheader.totalsize=sizeof(dumpheader)+sizeof(float)*NCORR+sizeof(char)*NFOLD*FFTLEN/16/NNODECORR+lcount*NCORR*2*8*sizeof(char);
        memcpy(obuf,&dumpheader,sizeof(dumpheader));
        obuf+=sizeof(dumpheader);
        memcpy(obuf,lmap,sizeof(char)*NFOLD*FFTLEN/16/NNODECORR);
        obuf+=sizeof(char)*NFOLD*FFTLEN/16/NNODECORR;
        memcpy(obuf,fmap,sizeof(float)*NCORR);
        obuf+=sizeof(float)*NCORR;
#pragma omp parallel for default(none) private(j,tmp,i) shared(lmap,obuf,corr,fmap,imap,lcount)
        for (i1=0;i1<lcount;i1++) {
		i=imap[i1];
                for (j=0;j<NCORR*2*8;j++) {
                	tmp=corr[i][j]/fmap[j/16];
                	tmp=sqrt(fabs(tmp));
                	tmp=(corr[i][j]>0)?tmp:-tmp;
                	obuf[j+i1*NCORR*8*2]=lrint(127*tmp);
                	corr[i][j]=0;
		}
        }
        return;
}
#endif

void dump1bit(char *obuf,char *ibuf,struct timeval tv)
{
  char a,b,c,d;
  int i,imax;
  imax=(NT*NNODEACQ)/(2*NNODECORR);
  for (i=0;i<imax;i+=4){
    a=ibuf[i];
    b=ibuf[i+1];
    c=ibuf[i+2];
    d=ibuf[i+3];
    obuf[i/4]=a&0x80|(a&0x08)<<3|(b&0x80)>>2|(b&0x08)<<1|
	(c&0x80)>>4|(c&0x08)>>1|(d&0x80)>>6|(d&0x08)>>3;
  }
  memcpy(obuf+imax/4,&tv,sizeof(tv));
}


#define NFFTAC 65536
#define SQR(x)   ((x)*(x))
void fitac(float acwt[2], int *zerofreq)
{
        static float voltage[NFFTAC];
        float peak,theta,power;
        int i,ipeak;


        if (NT/65536>NFFTAC) fprintf(stderr,"fitac: NFFTAC<NSAMPLE");
        power=0;
        for (i=0;i<NT/(65536);i++) {
                voltage[i]=zerofreq[i];
                power+=zerofreq[i];
        }
        power/=NT/65536;
        for (i=NT/65536;i<NFFTAC;i++) voltage[i]=power;
        fft(voltage,NFFTAC);
        peak=0;
        for (i=10;i<NFFTAC/16;i+=2) {
                power=SQR(voltage[i])+SQR(voltage[i+1]);
                if (power>peak) {
                        peak=power;
                        ipeak=i;
                }
        }
        printf("ipeak=%d power=%f\n",ipeak,power);
        theta=atan2(voltage[ipeak+1],voltage[ipeak]);
        acwt[0]=2*ipeak*M_PI/NFFTAC; // i is twice the peak frequency
        acwt[1]=theta;
}

void  foldac(short int foldcos[NT/65536],short int foldsin[NT/65536],float wt[2])
{
        int i;
        for (i=0;i<NT/65536;i++) {
                foldcos[i]=32767*cos(wt[0]*i+wt[1]);
                foldsin[i]=32767*sin(wt[0]*i+wt[1]);
        }
}

void  foldpulsar(short int foldcos[NDIMGATE],short int foldsin[NDIMGATE],int dmoffset[FFTLEN/2], short int gate[NDIMGATE], double time0, double dm, double period, double freq0, int isign, int rank)
{
        int i;//,isign=1; // -1 for decreasing RF freq with increasing baseband freq
        //double freq0=156; // MHz
        //double freq0=1170; // MHz
        double periodAC=0.01; // AC power period
        double deltat,freq,time,dtime,deltatmax;
        int ngate=NFOLD;
        int ifo,iphase;

#ifdef FOLDSIN
	ngate=NFOLD-1;
#endif
#ifdef FOLDSINCOS
	ngate=NFOLD-2;
#endif

//        printf("time0=%f\n",time0);
        freq=freq0+isign*16.6666666666666667;
        deltatmax=4149.6*dm*(1/SQR(freq)-1/SQR(freq0));
        deltatmax=MAX(deltatmax,0);
        for (i=0;i<FFTLEN/2/NNODECORR;i++) {
                dtime=3.e-8*FFTLEN*NREDGATE;
                ifo=i+rank*FFTLEN/2/NNODECORR;
                freq=freq0+isign*ifo*16.6666666666666667/(FFTLEN/2);// ! MHz
                deltat=4149.6*dm*(1/SQR(freq)-1/SQR(freq0));// sec, always positive
                dmoffset[i]=(deltatmax-deltat)/dtime+1;
                if (dmoffset[i]>NDIMGATE-NTCORR*2/(FFTLEN*NCHAN*NREDGATE)-2) {
                        printf("dmoffset error: deltat=%f i=%d freq=%f dm=%f\n",deltat,i,freq,dm);
                        exit(-1);
                }
        }

        for (i=0;i<NDIMGATE;i++) {
                dtime=3.e-8*FFTLEN*NREDGATE*i;
                time=time0+dtime;
                iphase=time*ngate/period;
                gate[i]=iphase%ngate;
                foldcos[i]=32767*cos(2*M_PI*time/periodAC);
                foldsin[i]=32767*sin(2*M_PI*time/periodAC);
        }
}



void  foldpulsarpow(int dmoffset[FFTLEN/2], short int gate[NDIMGATEPOW], double time0, double dm, double period, double freq0, int isign, int ngate)
{
        double deltat,freq,time,dtime,deltatmax;
        int i,ifo;
	long long int iphase;

        freq=freq0+isign*16.6666666666666667;
        deltatmax=4149.6*dm*(1/SQR(freq)-1/SQR(freq0));
        deltatmax=MAX(deltatmax,0);
        for (i=0;i<FFTLEN/2;i++) {
                dtime=3.e-8*FFTLEN;
                ifo=i;
                freq=freq0+isign*ifo*16.6666666666666667/(FFTLEN/2);// ! MHz
                deltat=4149.6*dm*(1/SQR(freq)-1/SQR(freq0));// sec, always positive
                dmoffset[i]=(deltatmax-deltat)/dtime+1;
                if (dmoffset[i]>NDIMGATEPOW-NT/(FFTLEN*NCHAN)-2) {
                        printf("dmoffset error: deltat=%f i=%d freq=%f dm=%f\n",deltat,i,freq,dm);
                        exit(-1);
                }
        }

        for (i=0;i<NDIMGATEPOW;i++) {
                dtime=3.e-8*FFTLEN*i;
                time=time0+dtime;
                iphase=time*ngate/period;
                gate[i]=iphase%ngate;
        }
}






void fft(float *a, int n)
{
//        fft1dippsrcinit(n);
//        fft1dippsrc(a,a,1);
        fft_(a,a,&n);
}

