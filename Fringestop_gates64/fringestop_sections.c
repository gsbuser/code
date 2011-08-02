#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <fenv.h>
#include <libgen.h>
#include "mpi.h"

#include "constants.h"
#include "structures.h"
#include "routines.h"
#include "params.h"

/*
        accumulate to float fcross[NCORR][FFTLEN/2][2]
 */
void shuffle(int nfold, int ncorr, int fftlen, int npod, int nnode, float *fcross, float *icross)
{
	shuffle_(&nfold,&ncorr,&fftlen,&npod,&nnode,fcross,icross);
}
//#define DEBIAS2(x) (((x>0)?(x+0.5)*(x+0.5):(x-0.5)*(x-0.5))/(127.*127.))
#define DEBIAS2(x) ((x*x)/(127.*127.))
#ifdef FOLDSIN
#define NFOLD (NGATE+2)
#else
#define NFOLD NGATE
#endif
#define NPAD 2

int main(int argc, char* argv[]) {


  //  double PNT_RA, PNT_DEC;
  FILE *fp;
  static int fdin, fdout, fdonegate, fdoutac, rank;
  char fname[80],fntimes[80],fnin[80];
  ANTCOORDS coords[NPOD];
  int ii,jj, re, is, bi, iepoch, t_index;
  int fstart=0, nf=(NF/NNODE), tstart=0;  
  double TJD[NEPOCH_READ], GST[NEPOCH_READ];
  double RA_pt[NEPOCH_READ], Dec_pt[NEPOCH_READ];
  char fn[80],fnout[80],fnonegate[80], fntmp[80];
  static float fcross[NGATE][NCORR][FFTLEN/NNODE/2][2];
  static float fcross_cum[NGATE][NCORR+NPAD][FFTLEN/NNODE/2][2];
  static float cross[NGATE][NCROSS];
  static double cross0[NGATE][NCROSS];
  static float cross_onegate[ONEGATE][NCROSS];
  static float fcross_onegate[ONEGATE][NCORR][FFTLEN/NNODE/2][2];
  ssize_t size;
  int ierr=0;
    char lmap[NFOLD][FFTLEN/16/NNODE];

  struct timeval4
  {
    int tv_sec;
    int tv_usec;
  };

  //ierr=feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW );
  //printf("ierr=%d exception=%d\n",ierr,fegetexcept ());

  MPI_Init(NULL,NULL);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);
  if (rank == 0) {
    printf("rank=%d,size=%d NEPOCH=%d OBS_EPOCH=%f\n",rank,size,NEPOCH,OBS_EPOCH);
   if (NNODE != size) exit(-1);
   if (NGATE*(NCORR+NPAD)%NNODE!=0) {
	fprintf(stderr,"error: nnode must divide NGATE*NCORR\n");
	exit(-1);
   }
    if (argc != 3) {
      printf("argc=%d ",argc);
      for (ii=0;ii<argc;ii++) printf("argv=%s ",argv[ii]);
      fprintf(stderr,"usage: %s corrfile.out corrfile.in\n",argv[0]);
      exit(-1);
    }
  }

  sprintf(fn,"%s.node%%d",argv[1]);
  sprintf(fnin,fn,rank);
  fdin=open(fnin,O_RDONLY);
  if (fdin<0) {
    fprintf(stderr,"rank=%d fdin=%s\n",rank, fn);
    fflush(stderr);
    perror("open corrin.dat");
    exit(-1);
  }

  /* determine output files */
  sprintf(fn,argv[2]);
#ifdef WRITELOCAL
  char *fnbase="/mnt/d/pen/%s.node%d.fsall";  
  sprintf(fnout,fnbase,basename(fn),rank);
#else
  char *fnbase="/mnt/d/pen/%s.fsall.rebin";  
  sprintf(fnout,fnbase,basename(fn));
#endif

  char *fnbase2="%s.node%d.fs0";
  sprintf(fnonegate,fnbase2,fn,rank);

  fdout=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdout<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}

  fdonegate=open(fnonegate,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdonegate<0) {printf("file %s ",fnonegate); fflush(stdout); perror("open corrout.dat");}

#ifdef FOLDSIN
  fnbase="/mnt/c/pen/%s.node%d.acfold";
  sprintf(fnout,fnbase,basename(fn),rank);   
  fdoutac=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdoutac<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}
#endif


  sprintf(fntimes,"/mnt/code/pen/Nov_2007/Fringestop_gates/%s/times.dat",basename(fn));
  if (rank==0) printf("fntimes=%s\n",fntimes);
  get_tjd_gst(fntimes, NEPOCH_READ, TJD, GST);
  get_antenna_coords("/mnt/code/tchang/Dec2007/Fringestop_gates/data/coords60_dec2.dat", coords, NPOD);

  if (rank==0) printf("infile=%s outfile=%s timefile=%s\n",fnin,fnout,fntimes);

  for (iepoch=0;iepoch<NEPOCH_READ;iepoch++) {
    RA_pt[iepoch] = PNT_RA;
    Dec_pt[iepoch] = PNT_DEC;
  }

  for(ii=0;ii<NGATE;ii++) for (jj=0;jj<NCROSS;jj++) cross[ii][jj]=0;
  for (jj=0;jj<NCROSS;jj++) cross_onegate[0][jj]=0;
  for(is=0;is<NGATE;is++) for (jj=0;jj<NCORR+NPAD;jj++) for (ii=0;ii<FFTLEN/NNODE/2;ii++)
   for (re=0;re<2;re++) fcross_cum[is][jj][ii][re]=0;


  /*frequency subset of this node */
  fstart=nf*(rank);


#if 1
   struct CorrHeader head;

   if (read(fdin,&head,sizeof(head)) != sizeof(head)) perror("fringestop: read");
   if (rank == 0 ) {
	if (head.nfold != NFOLD) {
	printf("nfold from file=%d != NFOLD\n",head.nfold);
	exit(-1);
	}
	if (NFOLD != NGATE) printf("using FOLDSIN\n");
   }
  lseek(fdin,0,SEEK_SET);
  
  /*subtract the bias */
  //  lseek(fdin,seeksize,SEEK_SET);

  for (jj=0;jj<NGATE;jj++) for(ii=0;ii<NCROSS;ii++) cross0[jj][ii]=0;

  for (iepoch=(tstart);iepoch<=(NEPOCH);iepoch++) {

    static char buf[2][NFOLD][NCROSS];
    float fmax[NCORR];
    int i1;

//        printf("rank %d b4 parallel section\n",rank);
//    omp_set_num_threads(2);
#pragma omp parallel sections default(shared) num_threads(2)
    {
//        printf("rank %d inside parallel section\n",rank);
#pragma omp section
    {
//        printf("rank %d start read\n",rank);
        if ( iepoch<NEPOCH) readbuf2(fdin,buf[iepoch%2],fmax,lmap);
    }
#pragma omp section
    {
//    omp_set_num_threads(8);
//        printf("rank %d in loop ",rank);
    if (iepoch>0)
    for (jj=0;jj<NGATE;jj++) {
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross0,jj,buf,iepoch) num_threads(7)
	for (i1=0;i1<FFTLEN/16/NNODE;i1++) 
	    if (lmap[jj][i1]) 
		for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
      		double tmp=DEBIAS2(buf[1-iepoch%2][jj][ii])*fmax[(ii/16)%NCORR];
      		tmp=(buf[1-iepoch%2][jj][ii]>0)?tmp:-tmp;
      		cross0[jj][ii]+=tmp/(NEPOCH);
    	}
    }
    }
    }
  }

  printf("rank %d done bias\n",rank);
  lseek(fdin,0,SEEK_SET);

#endif



  /* each timestamp */
  for(iepoch=(tstart);iepoch<=(NEPOCH);iepoch++) {

    int i1;
    float fmax[NCORR];
    char buf[2][NFOLD][NCROSS];
    t_index=iepoch+EPOCH_START;

//    omp_set_num_threads(2);
#pragma omp parallel sections default(shared) num_threads(2)
    {
#pragma omp section
    {
        if ( iepoch<NEPOCH) readbuf2(fdin,buf[iepoch%2],fmax,lmap);
    }
#pragma omp section
    {
    if (iepoch>0) {
//    omp_set_num_threads(7);
#ifdef FOLDSIN
    for (jj=NGATE;jj<NFOLD;jj++){
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross,cross_onegate,jj,buf,cross0,iepoch) num_threads(7)
        for (i1=0;i1<FFTLEN/16/NNODE;i1++)
            if (lmap[jj][i1])
                for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
        int cross_tmp=0;
        cross_tmp=DEBIAS2(buf[1-iepoch%2][jj][ii])*fmax[(ii/16)%NCORR];
        cross_tmp=(buf[1-iepoch%2][jj][ii]>0)?cross_tmp:-cross_tmp;
        cross[jj-NGATE][ii]=cross_tmp;
    }
    }
    dump1byte(fdoutac,cross[0],2*NCROSS*ONEGATE);
#endif

    for (jj=0;jj<NGATE;jj++){
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross,cross_onegate,jj,buf,cross0,iepoch) num_threads(7)
	for (i1=0;i1<FFTLEN/16/NNODE;i1++) 
	    if (lmap[jj][i1]) 
		for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
        int cross_tmp=0;
	cross_tmp=DEBIAS2(buf[1-iepoch%2][jj][ii])*fmax[(ii/16)%NCORR];
	cross_tmp=(buf[1-iepoch%2][jj][ii]>0)?cross_tmp:-cross_tmp;
	cross_tmp-=cross0[jj][ii];
	cross[jj][ii]=cross_tmp;
	cross_onegate[0][ii]+=cross_tmp;
      }
    }

    /* convert data from int to float and do transpose to the regular order as in fcross */
    shuffle(NGATE, NCORR, FFTLEN, NPOD, NNODE, fcross,cross);
    shuffle(ONEGATE, NCORR, FFTLEN, NPOD, NNODE, fcross_onegate,cross_onegate);

#if 0

    /* fringestop ONEGATE */

    /* loop over baselines */
#pragma omp parallel for default(none) shared(t_index,coords,fstart,nf,TJD,GST,RA_pt,Dec_pt,fcross,fcross_onegate)
    for(bi=0;bi<NPOD;bi++) {
      int df_index, bindex, igate, bj, i, j;
      double data_onegate[ONEGATE][2*nf];
      double dt, dt_2pi, freq;
      double csphase[2], cs_dphase[2], datacorrect[2], thisdata[2];
      double suppression, dGST_fringe, half_fringe_angle;

      /* auto correlation */
#if 0
      for (igate=0;igate<ONEGATE;igate++)
	for(df_index=(F_CUTOFF); df_index<(nf-F_CUTOFF); df_index++)
	  for (i=0;i<2;i++)
	    fcross_onegate[0][bindex][df_index][i]=0;
#endif

      for(bj=bi;bj<NPOD;bj++) {

	bindex=NPOD*bi-bi*(bi+1)/2+bj;

	/* time delay -- to the first timestamp*/
	//dt = get_delta_t(RA_pt[EPOCH_START]-GST[EPOCH_START], Dec_pt[EPOCH_START], bi, bj,
	//		 coords);
	/* time delay */
	dt = get_delta_t(RA_pt[t_index]-GST[t_index], Dec_pt[t_index], bi, bj,
                         coords);

	dt_2pi = dt * 2. * M_PI;

	freq = CORR_FREQ0 + CORR_DFREQ*(fstart+F_CUTOFF);
	csphase[0]   = cos(freq*dt_2pi);
	csphase[1]   = sin(freq*dt_2pi);
	cs_dphase[0] = cos(CORR_DFREQ*dt_2pi);
	cs_dphase[1] = sin(CORR_DFREQ*dt_2pi);

#if 1
	dGST_fringe = 0.5*CORR_NBLOCK*CORR_LENGTH*EARTH_OMEGA;
	half_fringe_angle = M_PI*freq*(get_delta_t(PNT_RA-GST[EPOCH_START]
						   +dGST_fringe,
						   PNT_DEC,bi, bj, coords)
				       - get_delta_t(PNT_RA-GST[EPOCH_START]
						     -dGST_fringe,
						     PNT_DEC, bi, bj, coords));
	suppression = (1.-fabs(dt/CORR_LENGTH)) * SINC(half_fringe_angle);
	csphase[0] /= suppression;
	csphase[0] /= suppression;
#endif

	/* apply fringestop */
	for(i=0;i<FFTLEN/NNODE/2;i++) 
	  for(j=0;j<2;j++) data_onegate[0][i*2+j]=fcross_onegate[0][bindex][i][j];

	for(df_index=(F_CUTOFF); df_index<(nf-F_CUTOFF); df_index++) {

	  datacorrect[0]=0.;
	  datacorrect[1]=0.;
	  thisdata[0] = data_onegate[0][df_index*2];
	  thisdata[1] = data_onegate[0][df_index*2+1];
	  CAFLOP(datacorrect, thisdata, csphase);
            
	  for (i=0;i<2;i++)
	    fcross_onegate[0][bindex][df_index][i]=datacorrect[i];

	  double temp[2];
	  temp[0]=temp[1]=0;
	  C_COPY(temp,csphase);
	  CWFLOP(csphase, temp, cs_dphase);

	  /* plot one single frequency */
	  freq += CORR_DFREQ;
	}
	bindex++;
      }
    }
#endif

    /* write out the onegate data in 1 byte*/
    dump1byte(fdonegate,fcross_onegate,NCROSS*ONEGATE);


#if 1
    
    /* fringestop NGATE */
    /* loop over baselines */
#pragma omp parallel for default(none) shared(t_index,coords,fstart,nf,TJD,GST,RA_pt,Dec_pt,fcross,fcross_cum,lmap) num_threads(7)
    for(bi=0;bi<NPOD;bi++) {
      int df_index,bindex,igate,bj,i,j;
      double data[NGATE][2*nf];
      double dt, dt_2pi, freq;
      double csphase[2], cs_dphase[2], datacorrect[2], thisdata[2];
      double suppression, dGST_fringe, half_fringe_angle;

      /* auto correlation */
#if 0
      for (igate=0;igate<NGATE;igate++)
	for(df_index=(F_CUTOFF); df_index<(nf-F_CUTOFF); df_index++)
	  for (i=0;i<2;i++)
	    fcross[igate][bindex][df_index][i]=0;
#endif


      for(bj=bi;bj<NPOD;bj++) {
        double csphasetable[nf][2];

	bindex=NPOD*bi-bi*(bi+1)/2+bj;

	/* time delay */
	dt = get_delta_t(RA_pt[t_index]-GST[t_index], Dec_pt[t_index], bi, bj,
			 coords);
	dt_2pi = dt * 2. * M_PI;

	freq = CORR_FREQ0 + CORR_DFREQ*(fstart+F_CUTOFF);
	csphase[0]   = cos(freq*dt_2pi);
	csphase[1]   = sin(freq*dt_2pi);
	cs_dphase[0] = cos(CORR_DFREQ*dt_2pi);
	cs_dphase[1] = sin(CORR_DFREQ*dt_2pi);

#if 1
	dGST_fringe = 0.5*CORR_NBLOCK*CORR_LENGTH*EARTH_OMEGA;
	half_fringe_angle = M_PI*freq*(get_delta_t(PNT_RA-GST[t_index]
						   +dGST_fringe,
						   PNT_DEC,bi, bj, coords)
				       - get_delta_t(PNT_RA-GST[t_index]
						     -dGST_fringe,
						     PNT_DEC, bi, bj, coords));
	suppression = (1.-fabs(dt/CORR_LENGTH)) * SINC(half_fringe_angle);
	csphase[0] /= suppression;
	csphase[1] /= suppression;
#endif

	for(df_index=(F_CUTOFF); df_index<(nf-F_CUTOFF); df_index++) {
	  double temp[2];
	  csphasetable[df_index][0]=csphase[0];
	  csphasetable[df_index][1]=csphase[1];
	  temp[0]=temp[1]=0;
	  C_COPY(temp,csphase);
	  CWFLOP(csphase, temp, cs_dphase);
        }
        /*NGATES*/

	for (igate=0;igate<NGATE;igate++) {
	  for(df_index=(F_CUTOFF); df_index<(nf-F_CUTOFF); df_index++) {
	    if (lmap[igate][df_index/8]){
	      datacorrect[0]=0.;
	      datacorrect[1]=0.;
	      thisdata[0] = fcross[igate][bindex][df_index][0];
	      thisdata[1] = fcross[igate][bindex][df_index][1];
	      CAFLOP(datacorrect, thisdata, csphasetable[df_index]);
	      for (i=0;i<2;i++)
		fcross_cum[igate][bindex][df_index][i]+=datacorrect[i];
	    }
	  }
	}
      }
    }
#endif
    if ((iepoch+1) % NTBIN == 0 ) {
	float mpi_cross[NNODE][NGATE*(NCORR+NPAD)/NNODE][FFTLEN/NNODE];
	float obuf[NGATE*NCORR/NNODE][FFTLENOUT];
	int i;

    	ierr=MPI_Alltoall(fcross_cum,NGATE*(NCORR+NPAD)*FFTLEN/NNODE/NNODE,MPI_FLOAT,mpi_cross,NGATE*(NCORR+NPAD)*FFTLEN/NNODE/NNODE,MPI_REAL,MPI_COMM_WORLD);
//		printf("rank %d done mpi alltoall\n",rank);
#pragma omp parallel for default(none) shared(obuf,mpi_cross,rank) num_threads(7)
	for (i=0;i<NGATE*(NCORR+NPAD)/NNODE;i++) {
		float buf[FFTLEN];
		int inode,ifreq;
		for (inode=0;inode<NNODE;inode++)
			for (ifreq=0;ifreq<FFTLEN/NNODE;ifreq++){
				buf[ifreq+inode*FFTLEN/NNODE]=mpi_cross[inode][i][ifreq];
		}
		int fftlen=FFTLEN;
		int minusone=-1;
                fft1_(buf,buf,&fftlen,&minusone);
		for (ifreq=0;ifreq<FFTLENOUT/2;ifreq++){
			obuf[i][ifreq]=buf[ifreq];
			obuf[i][FFTLENOUT-ifreq-1]=buf[FFTLEN-ifreq-1];
		}
		int plusone=1;
		fftlen=FFTLENOUT;
                fft1_(obuf[i],obuf[i],&fftlen,&plusone);
	}
#ifdef WRITELOCAL
        size=write(fdout,obuf,sizeof(float)*(NCORR+NPAD)*NGATE*FFTLENOUT/NNODE);
#else
	static float obufall[NGATE][(NCORR+NPAD)*FFTLENOUT];
	ierr=MPI_Gather(obuf,NGATE*(NCORR+NPAD)/NNODE*FFTLENOUT,MPI_FLOAT,obufall,NGATE*(NCORR+NPAD)/NNODE*FFTLENOUT,MPI_FLOAT,0,MPI_COMM_WORLD);
	if (ierr != 0) fprintf(stderr,"mpi_gather: ierr=%d\n",ierr);
	if (rank == 0)
           for (i=0;i<NGATE;i++)
       		if (write(fdout,obufall[i],sizeof(float)*NCORR*FFTLENOUT)<sizeof(float)*NCORR*FFTLENOUT) perror("write corr");
#endif

      //      size=write(fdout,fcross_cum,sizeof(float)*NCROSS*NGATE);
      // if (size<sizeof(float)*NCROSS*NGATE) perror("write corr");
      /* write out the onegate data in 1 byte*/
#if 0
      dump2byte(fdout,fcross_cum,NCROSS*NGATE);
#endif

      for(is=0;is<NGATE;is++) for (jj=0;jj<NCORR;jj++) for (ii=0;ii<FFTLEN/NNODE/2;ii++)
	for (re=0;re<2;re++) fcross_cum[is][jj][ii][re]=0;
      if (rank == 0) printf("done epoch=%d\n",iepoch);
    }
  }
  }
  }
  }
#ifdef WRITELOCAL
        size=write(fdout,cross0,sizeof(double)*NCROSS*NGATE);
#else
  	static double mpi_cross0[NNODE][NGATE][NCROSS];
        ierr=MPI_Gather(cross0,NGATE*NCROSS,MPI_DOUBLE,mpi_cross0,NGATE*NCROSS,MPI_DOUBLE,0,MPI_COMM_WORLD);
        if (ierr != 0) fprintf(stderr,"mpi_gather: ierr=%d\n",ierr);
        if (rank == 0)
       if (write(fdout,mpi_cross0,sizeof(double)*NGATE*NCROSS*NNODE)<sizeof(float)*NGATE*NCROSS*NNODE) perror("write corr0");

#endif

  close(fdonegate);
  close(fdout);

  MPI_Finalize();

  return(0);

}

#define MAX(x,y) ((x>y)?x:y)
void dump1byte(int fpp, float *fbuf, int size)
{
  int i;
#define MAXBUF (NCROSS*ONEGATE)
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
    if (write(fpp,&fmax,sizeof(float)) != sizeof(float)) perror("dump1byte:write");
    if (write(fpp,obuf,FFTLEN/NNODE) != FFTLEN/NNODE ) perror("dump1byte:write");
    fbuf+=FFTLEN/NNODE;
    size-=FFTLEN/NNODE;
  }
}
void dump2byte(int fpp, float *fbuf, int size)
{
  int i;
  static short int obuf[FFTLEN/NNODE];
  float fmax,ftmp;

  while (size>FFTLEN/NNODE-2) {
    fmax=0;
    for (i=0;i<FFTLEN/NNODE;i++) fmax=MAX(fmax,fabs(fbuf[i]));
    for (i=0;i<FFTLEN/NNODE;i++) {
      ftmp=sqrt(fabs(fbuf[i]/fmax));
      ftmp=(fbuf[i]>0)?ftmp:-ftmp;
      obuf[i]=lroundf(32767*ftmp);
    }
    if (write(fpp,&fmax,sizeof(float)) != sizeof(float)) perror("dump1byte:write");
    if (write(fpp,obuf,sizeof(short int)*FFTLEN/NNODE) != sizeof(short int)*FFTLEN/NNODE ) perror("dump2byte:write");
    fbuf+=FFTLEN/NNODE;
    size-=FFTLEN/NNODE;
  }
}



void readbuf2(int fd,char *buf,float *fmax,char *lmap) 
{
	int imap[NFOLD*FFTLEN/16/NNODE];
	int i, icount;
	struct CorrHeader head;
	
	if (read(fd,&head,sizeof(head)) != sizeof(head)) perror("readbuf:read");
	if (read(fd,lmap,NFOLD*FFTLEN/16/NNODE) != NFOLD*FFTLEN/16/NNODE) perror("readbuf:read");
	icount=0;
        for (i=0;i<NFOLD*FFTLEN/16/NNODE;i++) if (lmap[i]>0) imap[icount++]=i;

	if (read(fd,fmax,NCORR*sizeof(float)) != NCORR*sizeof(float)) perror("readbuf:read");
	for (i=0;i<head.lcount;i++)
	if (read(fd,buf+imap[i]*NCORR*2*8,NCORR*2*8) != NCORR*2*8) perror("readbuf:read");
}
