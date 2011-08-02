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
#include <omp.h>
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
//#define NUMTHREADS 4
#define DEBIAS2(x) ((x*x)/(127.*127.))
#define SQR(x)  ((x)*(x))
#define MAX(x,y) ((x>y)?x:y)
#define MIN(x,y) ((x<y)?x:y)
#ifdef FOLDSIN
#define NFOLD (NGATE+1)
#else
#define NFOLD NGATE
#endif
#define NPAD 0
#ifndef FSALLNODE
#define FSALLNODE 0
#endif

int main(int argc, char* argv[]) {


  //  double PNT_RA, PNT_DEC;
  FILE *fp;
  static int fdin, fdout, fdonegate, fdoutac, fdoutvar, rank;
  char fname[80],fntimes[80],fnin[80];
  ANTCOORDS coords[NPOD];
  int i,ii,jj, re, is, bi, iepoch, t_index;
  int fstart=0, nf=(NF/NNODE), tstart=0;  
  double TJD[NEPOCH_READ], GST[NEPOCH_READ];
  double RA_pt[NEPOCH_READ], Dec_pt[NEPOCH_READ];
  char fn[80],fnrebin[80],fnout[80],fnonegate[80], fntmp[80];
  static float fcross[NGATE][NCORR][FFTLEN/NNODE/2][2];
  static float fcross_cum[NGATE][NCORR+NPAD][FFTLEN/NNODE/2][2];
  static float cross[NGATE][NCROSS];
  static double cross0[NGATE][NCROSS];
  static int count_cross0[NGATE][NCROSS];
  static float cross_onegate[ONEGATE][NCROSS];
  static float fcross_onegate[ONEGATE][NCORR][FFTLEN/NNODE/2][2];
  static float fcross_tmp[NCORR*FFTLEN/NNODE];
  static float fcross_var[NCORR*FFTLENOUT/NNODE/2];
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
    if (argc != 4) {
      printf("argc=%d ",argc);
      for (ii=0;ii<argc;ii++) printf("argv=%s ",argv[ii]);
      fprintf(stderr,"usage: %s corrfile.in corrfile.out fsall.rebin.out\n",argv[0]);
      exit(-1);
    }
  }
  omp_set_num_threads(4);

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
  char *fnbase="%s.node%d.fsall";  
  sprintf(fnout,fnbase,fn,rank);
  fdout=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdout<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}
#else
  char *fnbase="%s.fsall.rebin";
  if (rank == 0) {
  sprintf(fnout,fnbase,argv[3]);
  fdout=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdout<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}
  fnbase="%s.var.rebin";  
  sprintf(fnout,fnbase,argv[3]);
  fdoutvar=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdoutvar<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}
  }
#endif


  char *fnbase2="%s.node%d.fs0";
  sprintf(fnonegate,fnbase2,fn,rank);

  fdonegate=open(fnonegate,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdonegate<0) {printf("file %s ",fnonegate); fflush(stdout); perror("open corrout.dat");}

#ifdef FOLDSIN
  fnbase="%s.node%d.acfold";
  sprintf(fnout,fnbase,fn,rank);   
  fdoutac=open(fnout,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
  if (fdoutac<0) {printf("file %s ",fnout); fflush(stdout); perror("open corrout.dat");}
#endif


  sprintf(fntimes,"%s/times.dat",basename(fn));
  if (rank==0) printf("fntimes=%s\n",fntimes);
  get_tjd_gst(fntimes, NEPOCH_READ, TJD, GST);
  get_antenna_coords("/mnt/code/tchang/Aug2008/Fringestop_gates/data/coords60_aug19.dat", coords, NPOD);

  if (rank==0) printf("infile=%s outfile=%s timefile=%s\n",fnin,fnout,fntimes);

  for (iepoch=0;iepoch<NEPOCH_READ;iepoch++) {
    RA_pt[iepoch] = PNT_RA;
    Dec_pt[iepoch] = PNT_DEC;
  }

  for(ii=0;ii<NGATE;ii++) for (jj=0;jj<NCROSS;jj++) cross[ii][jj]=0;
  for (jj=0;jj<NCROSS;jj++) cross_onegate[0][jj]=0;
  for(is=0;is<NGATE;is++) for (jj=0;jj<NCORR+NPAD;jj++) for (ii=0;ii<FFTLEN/NNODE/2;ii++)   for (re=0;re<2;re++) fcross_cum[is][jj][ii][re]=0;
  for (i=0;i<NCORR*FFTLENOUT/NNODE/2;i++) fcross_var[i] = 0;
  
  
  /*frequency subset of this node */
  fstart=nf*(rank);


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
#if 1
  
  /*subtract the bias */

  for (jj=0;jj<NGATE;jj++) for(ii=0;ii<NCROSS;ii++) cross0[jj][ii]=count_cross0[jj][ii]=0;

  for (iepoch=(tstart);iepoch<(NEPOCH);iepoch++) {

    static char buf[NFOLD][NCROSS];
    float fmax[NCORR];
    int i1;
    
    readbuf2(fdin,buf,fmax,lmap);
    for (jj=0;jj<NGATE;jj++) {
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross0,jj,buf,iepoch,count_cross0)
      for (i1=0;i1<FFTLEN/16/NNODE;i1++) 
	if (lmap[jj][i1]) 
	  for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
	    double tmp=DEBIAS2(buf[jj][ii])*fmax[(ii/16)%NCORR];
	    tmp=(buf[jj][ii]>0)?tmp:-tmp;
	    cross0[jj][ii]+=tmp;//   /(NEPOCH);
	    count_cross0[jj][ii]++;
	  }
    }
  }

  printf("rank %d done bias\n",rank);
  lseek(fdin,0,SEEK_SET);

#endif



  /* each timestamp */
  for(iepoch=(tstart);iepoch<(NEPOCH);iepoch++) {
    
    int i1;
    float fmax[NCORR];
    char buf[NFOLD][NCROSS];
    t_index=iepoch+EPOCH_START;

    readbuf2(fdin,buf,fmax,lmap);

#ifdef FOLDSIN
    for (jj=NGATE;jj<NFOLD;jj++){
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross,cross_onegate,jj,buf,cross0) 
      for (i1=0;i1<FFTLEN/16/NNODE;i1++)
	if (lmap[jj][i1])
	  for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
	    int cross_tmp=0;
	    cross_tmp=DEBIAS2(buf[jj][ii])*fmax[(ii/16)%NCORR];
	    cross_tmp=(buf[jj][ii]>0)?cross_tmp:-cross_tmp;
	    cross[jj-NGATE][ii]=cross_tmp;
	  }
    }
    shuffle((NFOLD-NGATE), NCORR, FFTLEN, NPOD, NNODE, fcross,cross);
    dump1byte(fdoutac,fcross,(NFOLD-NGATE)*NCROSS*ONEGATE);
#endif

    for (jj=0;jj<NGATE;jj++){
#pragma omp parallel for default(none) private(ii) shared(lmap,fmax,cross,cross_onegate,jj,buf,cross0,count_cross0) 
      for (i1=0;i1<FFTLEN/16/NNODE;i1++) 
	if (lmap[jj][i1]) 
	  for(ii=i1*NCROSS*NNODE*16/FFTLEN;ii<(i1+1)*NCROSS*NNODE*16/FFTLEN;ii++)  {
	    int cross_tmp=0;
	    cross_tmp=DEBIAS2(buf[jj][ii])*fmax[(ii/16)%NCORR];
	    cross_tmp=(buf[jj][ii]>0)?cross_tmp:-cross_tmp;
	    cross_tmp-=cross0[jj][ii]/count_cross0[jj][ii];
	    cross[jj][ii]=cross_tmp;
	    //cross_onegate[0][ii]+=cross_tmp;
	  }
    }
    
    /* convert data from int to float and do transpose to the regular order as in fcross */
    shuffle(NGATE, NCORR, FFTLEN, NPOD, NNODE, fcross,cross);
    //shuffle(ONEGATE, NCORR, FFTLEN, NPOD, NNODE, fcross_onegate,cross_onegate);


    /* write out the onegate data in 1 byte*/
    //dump1byte(fdonegate,fcross_onegate,NCROSS*ONEGATE);

    /* compute noise on rebinned grid */
    //if (iepoch%2==0) {
    //	for(i=0;i<NCORR*FFTLEN/NNODE;i++) fcross_tmp[i]=((float *)fcross_onegate)[i];
    //} else {
    //	for (i=0;i<NCORR*FFTLEN/NNODE;i++)
    //		fcross_var[i*FFTLENOUT/FFTLEN/2] += SQR(fcross_tmp[i]-((float *)fcross_onegate)[i]);
    //}


#if 1
    
    /* fringestop NGATE */
    /* loop over baselines */
#pragma omp parallel for default(none) shared(t_index,coords,fstart,nf,TJD,GST,RA_pt,Dec_pt,fcross,fcross_cum,lmap)
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
      float obuf_han_ave[NGATE*(NCORR+NPAD)/NNODE][FFTLENOUT/2][2];
      int i;

      
      ierr=MPI_Alltoall(fcross_cum,NGATE*(NCORR+NPAD)*FFTLEN/NNODE/NNODE,MPI_FLOAT,mpi_cross,NGATE*(NCORR+NPAD)*FFTLEN/NNODE/NNODE,MPI_FLOAT,MPI_COMM_WORLD);
      //		printf("rank %d done mpi alltoall\n",rank);
#pragma omp parallel default(none) shared(obuf_han_ave,mpi_cross,rank) private(i)
      {
	/* reorder the matrix so that all frequencies are together */
#pragma omp for 
      for (i=0;i<NGATE*(NCORR+NPAD)/NNODE;i++) {
        float buf[FFTLEN];
	typedef float cfloat[FFTLEN/FFTLENOUT/2][2];
        cfloat *cbuf;
	int inode,ifreq;
	int j,k,l;
	cbuf=(cfloat *)buf;
	for (j=0;j<FFTLENOUT/2;j++)
	  obuf_han_ave[i][j][0]=obuf_han_ave[i][j][1]=0;
	for (inode=0;inode<NNODE;inode++)
	  for (ifreq=0;ifreq<FFTLEN/NNODE;ifreq++){
	    buf[ifreq+inode*FFTLEN/NNODE]=mpi_cross[inode][i][ifreq];
	  }
      /*initialize obuf_han_ave */
      /* do hanning smoothing here, then do frequency rebin */
	for (k=0;k<FFTLENOUT/2;k++)
	  for (j=0;j<(FFTLEN/FFTLENOUT);j++)
	    for (l=0;l<2;l++)
	    obuf_han_ave[i][k][l]+=(cbuf[k][(k==0)?MAX(0,j-1):j-1][l]*0.25+
				 cbuf[k][j][l]*0.5+
				 cbuf[k][(k==FFTLENOUT/2-1)?MIN(j+1,FFTLEN/FFTLENOUT-1):j+1][l]*0.25)/(FFTLEN/FFTLENOUT);
      }     
      }
#ifdef WRITELOCAL
      /* write out the onegate data in 1 byte*/
      dump1byte(fdout,obuf_han_ave,(NCORR+NPAD)*NGATE*FFTLENOUT/NNODE);
      //      size=write(fdout,obuf,sizeof(float)*(NCORR+NPAD)*NGATE*FFTLENOUT/NNODE);
#else
      static float obufall[NGATE][(NCORR+NPAD)*FFTLENOUT];
      ierr=MPI_Gather(obuf_han_ave,NGATE*(NCORR+NPAD)*FFTLENOUT/NNODE,MPI_FLOAT,obufall,NGATE*(NCORR+NPAD)*FFTLENOUT/NNODE,MPI_FLOAT,FSALLNODE,MPI_COMM_WORLD);
      if (ierr != 0) fprintf(stderr,"mpi_gather: ierr=%d\n",ierr);
      if (rank == FSALLNODE)
	for (i=0;i<NGATE;i++)
	  dump1byte(fdout,obufall[i], NCORR*FFTLENOUT);
	  //  if (write(fdout,obufall[i],sizeof(float)*NCORR*FFTLENOUT)<sizeof(float)*NCORR*FFTLENOUT) perror("write corr");
/* write noise variance */
      //static float obufvar[NNODE][NCORR*FFTLENOUT/NNODE/2];
      //ierr=MPI_Gather(fcross_var,NCORR*FFTLENOUT/2/NNODE,MPI_FLOAT,obufvar,NCORR*FFTLENOUT/2/NNODE,MPI_FLOAT,FSALLNODE,MPI_COMM_WORLD);
      //if (ierr != 0) fprintf(stderr,"mpi_gather: ierr=%d\n",ierr);
      //if (rank == FSALLNODE)
      //if (write(fdoutvar,obufvar,sizeof(float)*NCORR*FFTLENOUT/2)!=sizeof(float)*NCORR*FFTLENOUT/2) perror("write corr");
      MPI_Barrier(MPI_COMM_WORLD);
      //for (i=0;i<NCORR*FFTLENOUT/NNODE/2;i++) fcross_var[i] = 0;
#endif
      
      //      size=write(fdout,fcross_cum,sizeof(float)*NCROSS*NGATE);
      // if (size<sizeof(float)*NCROSS*NGATE) perror("write corr");
      /* write out the onegate data in 1 byte*/
#if  0
      dump2byte(fdout,fcross_cum,NCROSS*NGATE);
#endif

      for(is=0;is<NGATE;is++) for (jj=0;jj<NCORR;jj++) for (ii=0;ii<FFTLEN/NNODE/2;ii++)
	for (re=0;re<2;re++) fcross_cum[is][jj][ii][re]=0;
      if ( (rank == 0) && ( (iepoch+1) % 64 == 0) ) printf("done epoch=%d\n",iepoch);
    }
  }
#ifdef WRITELOCAL
  size=write(fdout,cross0,sizeof(double)*NCROSS*NGATE);
#else
  static double mpi_cross0[NNODE][NGATE][NCROSS];
  ierr=MPI_Gather(cross0,NGATE*NCROSS,MPI_DOUBLE,mpi_cross0,NGATE*NCROSS,MPI_DOUBLE,0,MPI_COMM_WORLD);
  if (ierr != 0) fprintf(stderr,"mpi_gather: ierr=%d\n",ierr);
  if (rank == 0)
    if (write(fdout,mpi_cross0,sizeof(double)*NGATE*NCROSS*NNODE)<sizeof(double)*NGATE*NCROSS*NNODE) perror("write corr0");
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

void dump1byte_test(int fpp, float *fbuf, int size)
{
#define MAXBUF (NCROSS*ONEGATE)
  int i;
  struct {float fmax; char obuf[FFTLEN/NNODE]} outbuf[size/(FFTLEN/NNODE)];
  static char obuf[FFTLEN/NNODE];
  float fmax,ftmp;
  int k=0;

  while (size>FFTLEN/NNODE-2) {
    fmax=0;
    for (i=0;i<FFTLEN/NNODE;i++) fmax=MAX(fmax,fabs(fbuf[i]));
    outbuf[k].fmax=fmax;
    for (i=0;i<FFTLEN/NNODE;i++) {
      ftmp=sqrt(fabs(fbuf[i]/fmax));
      ftmp=(fbuf[i]>0)?ftmp:-ftmp;
      outbuf[k].obuf[i]=lroundf(127*ftmp);
    }
    k++;
    fbuf+=FFTLEN/NNODE;
    size-=FFTLEN/NNODE;
  }
  if (write(fpp,outbuf,sizeof(outbuf))!=sizeof(outbuf)) perror("dump1byte:write");
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
