#include "mpi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "xfft.h"
#include <stdio.h>
#include "omp.h"


//#define DUMP1BIT
#define PROFILE
#define MIN(x,y) ((x<y)?x:y)
#define MAX(x,y) ((x>y)?x:y)
#define MPIWAITCHUNK 1
#define GATEPOWOUT 10
/* 
   revised August 2, 2007
   for two independent sets of send and receive nodes.
   Implicitly assumes that there are more correlation than acquisition nodes.
   revised August 9, 2009
   revised to write to a third set of disk nodes
*/

void corr_driver(signed char *buf, int len, MPI_Comm mpi_acq_comm)
{
  static double timestart;

  static char walshsign[2][NNODEACQ][NCHAN][NWALSH2];
  static char dumpcorrbuf[2][NFOLD*NCROSS/NNODECORR+sizeof(struct CorrHeader)+sizeof(int)+sizeof(float)*NCORR+sizeof(char)*NFOLD*FFTLEN/16/NNODECORR];
  static char dump1bitbuf[2][(NT*NNODEACQ)/(8*NNODECORR)+sizeof(tv)];
  
  static int iteration=0;
  static int fdout,ierr,rank,nproc,fdout1bit,fdgpow;
//  ssize_t size;

  __declspec(align(128)) static char obuf[3][NT/4]; // 4-bit FFT output data, half the buffer
  __declspec(align(128)) static char obuf8[2][NT/2]; // half the raw data
  __declspec(align(128)) static char xbuf[2][NT/2]; // 4-bit on xmac nodes
  __declspec(align(128)) static int cross[NFOLD][NCROSS];
  __declspec(align(128)) static short int norm[NCHAN][FFTLEN];
  
  int i,j,k,m;
  struct timeb start, current;
  double sec,delaymin,time0;
  static struct timeval tvhist[3];

  static short int idelay[NCHAN];
  /* for gating of incoherent total power */
  static int gatepow[NGATEAUTO][NFAUTO];
  static double gatepowd[NGATEAUTO][NFAUTO],gatepowsum[NGATEAUTO][NFAUTO];
  FILE *fp;
  char fn[80],fn1[80];
  static short int cosac[NDIMGATE],sinac[NDIMGATE],gate[NDIMGATE];
  static int dmoffset[FFTLEN/2/NNODECORR];

  //=== B2045-16 ==========================================================
  // double dm=11.5100;
  
  //double period=1.96151568503502085; //B2045-16  Feb 19 2011  11:00 am
  //double period=1.96151253619228510; //B2045-16  Feb 20 2011  11:00 am
  //double period=1.96150940625743078; //B2045-16  Feb 21 2011  11:00 am
  //double period=1.96149708689115459; //B2045-16  Feb 25 2011  11:00 am
  //double period=1.96149406336696484; //B2045-16  Feb 26 2011  11:00 am
  //double period=1.96149106567084846; //B2045-16  Feb 27 2011  11:00 am
  
  //double period=1.96142463758628014; //B2045-16  Jun 07 2011  05:00 am

  //double period=1.96144234120984675; //B2045-16  Jun 15 2011 06:00 am
  //double period=1.96144387255867991; //B2045-16  Jun 16 2011 05:00 am
  // double period=1.96144726296342674; //B2045-16 Jun 17 2011 6:30 am 
  // double period=1.96144990488186818; //b2045-16 Jun 18 2011 07:00 am 
  // double period=1.96145165924587786; //B2045-16 Jun 19 2011 06:00 am
  //double period=1961.45407812236658; //B2045-16 Jun 20 2011 06:00 am   
  // double period=1.96145580961633004; //B2045-16 Jun 21 2011 05:00 am 
  // double period=1.96145902471242402; //B2045-26 Jun 22 2011 06:00 am
  // double period=1.96146403301785426; //B2045-16 Jun 24 2011 14:00 
  //double period=1.96146703568338921; //B2045-16 Jun 26 2011 03:00 am
  // double period=1.96147168379738423; //B2045-16 Jun 27 2011 05:30 am
  // double period=1.96147438918628382; //B2045-16 Jun 28 2011 05:30 am
  // double period=1.96149159528723226; //B2045-16 Jul 04 2011 06:00 am
   //double period=1.96152925488573942; //B2045-16 Jul 17 2011 03:30 am 
  //double period=1.96153220640506311 //B2045-16 Jul 18 2011 03:15 am 
  //double period=1.96153220640506311 //B2045-16 Jul 19 2011 04:00 am  
  // double period=1.96153220640506311 //B2045-16 Jul 19 2011 04:00 am times value of 16
	
   //double period=22.0	//rfi period (no specific one)
//  double period=22.0 //RFI Jul 19 2011 04:00 am long period to reduce the number of timestamps and data taken.

// === B0329+54 ======================================================
  //double dm=26.77600 

    //  double period=0.7145609330291934; //B0329+54 Jul 15 2011 5:30 am
   //    double period=0.71447629092426303; //B0329+54 Jul 19 2011 6:30 am
  //   double period=0.71447111880694115; //B0329+54 Jul 19 2011 6:30 am
     //double period=0.71447054570290220; //B0329+54 Jul 30 2011 8:00 am
       
// === B2310+42 ======================================================
    //double dm=17.30000 

//  double period=0.34940961203309831  //B2310+42 Jul 17 2011 5:00 am
    //double period=0.34940966676111776 //B2310+42 Jul 18 2011 4:30 am

// === B0823+26 ======================================================

  //double dm=19.4751;  //B0823+26  Fe// b 5 0:30am, 2010
  //double period=0.53067346118238606;
  //double period=0.53067528201798336;  //B0823+26  Feb 7 00:30am, 2010
  //double period=0.53067708814396337;  //B0823+26  Feb 9 00:30am, 2010

  //double period=0.53069093861762281;  //B0823+26  Feb 25 22:30pm, 2010
  //double period=0.53069169928524343;  //B0823+26  Feb 26 22:30pm, 2010
  //double period=0.53069245047792606;  //B0823+26  Feb 27 22:30pm, 2010
  //double period=0.53069319239145318;  //B0823+26  Feb 28 22:30pm, 2010
  //double period=0.53069392517370136;  //B0823+26  Mar 01 22:30pm, 2010
  //double period=0.53069464887368326;  //B0823+26  Mar 02 22:30pm, 2010



  //double dm=19.4751;  //B0823+26  Dec17 3am, 2008 
  //double period=0.53062961469460993;
  //
  //double dm=19.4751;  //B0823+26  Dec20 3am, 2008 
  //double period=0.53063189802572958;
  //double dm=19.4751;  //B0823+26  Dec25 3am, 2008 
  //double period=0.53063590904417106;
  //double dm=19.4751;  //B0823+26  Jan 8 3am, 2008 
  //double period=0.53064814808364963;
  //double dm=19.4751;  //B0823+26  Jan 9 3am, 2008 
  //double period=0.53064905705275385;
  //double dm=19.4751;  //B0823+26  Jan 11 3am, 2008 
  //double period=0.53065088340109503;
  //double dm=19.4751;  //B0823+26  Jan 12 3am, 2008 
  //double period=0.53065180089504747;
  //double dm=0.0;  //B0823+26  Jan 13 3am, 2008 
  //double period=0.53065272143187462;
  //double dm=19.4751;  //B0823+26  Nov 07 8am, 2009 
  //double period=0.53061073240086114;
 
  //double dm=19.4751;  //B0823+26  Nov 08 8am, 2009 
  //double period=0.53061093708021178;

  //double dm=19.4751;  //B0823+26  Nov 09 8am, 2009 
  //double period=0.53061115723497880;

  //double dm=19.4751;  //B0823+26  Dec 18 5am, 2009 
  //double period=0.53063065844704829;
  
  //double dm=19.4751;  //B0823+26  Dec 19 5am, 2009 
  //double period=0.53063142480852866;

  //double dm=19.4751;  //B0823+26  Dec 20 5am, 2009 
  //double period=0.53063220088970081;

  // === B2217+47 ======================================================
  //

      double dm=43.516;
      double period=0.53844607128335201; //B2217+47 Aug 03 2011, 0730
      //   double period=0.53844571850954708; //B2217+47 Aug 02 2011, 7am
      //	double period=0.53844477621453655; // B2217+47 Jul 30 2011, 7am 
 //  double period=0.53844452557889917; //B2217+47 Jul 28 2010, 8am
  //double period=0.53844275273615415; // B2217+47  Jul 25 2010, 2am
  //double period=0.53844302282813851; // B2217+47  Jul 26 2010, 2am
  //double period=0.53844330027401986; // B2217+47  Jul 27 2010, 2am
  //double period=0.53844417692597142; // B2217+47  Jul 30 2010, 2am
  //double period=0.53844448402163368; // B2217+47  Jul 31 2010, 2am
  //double period=0.53844479862399430; // B2217+47  Aug 01 2010, 2am
  //double period=0.53844512078232401; // B2217+47  Aug 02 2010, 2am
  //double period=0.53844545055375124; // B2217+47  Aug 03 2010, 2am
  //double period=0.53844578800439115; // B2217+47  Aug 04 2010, 2am
  //double period=0.53844613320656583; // B2217+47  Aug 05 2010, 2am
  //double period=0.53844648623059891; // B2217+47  Aug 06 2010, 2am

  //double period=0.53844876766559662; // B2217+47  Aug 12 2010, 2am
  //double period=0.53844917347203045; // B2217+47  Aug 13 2010, 2am
  
  //double period=0.53845788744036463; // B2217+47  Sep 01 2010, 2am
  //double period=0.53845808398659972; // B2217+47  Sep 02 2010, 12am

  //double dm=0.0;  //B2217+47  Dec 19 1700, 2009
  //double period=0.53850380442322660; //Dec 19, 2009 

  //double dm=43.516;  //B2217+47  Aug 14 01am, 2009
  //double period=0.53845831900166650; //Sep 01, 2009 (Provided by Jayanta)

  //double period=0.53845781078144557; //Aug 31, 2009 (Provided by Jayanta)
  //double period=0.53845730605249707; //Aug 30, 2009 (Provided by Jayanta)
  // double period=0.53845680492701479; //Aug 29, 2009 (Provided by Jayanta)
  // double period=0.53845630752863997; //Aug 28, 2009 (Provided by Jayanta)
  // double period=0.53845532446441212; //Aug 27 01 am, 2009
  //double period=0.53845158089265306; //Aug 19 01 am, 2009
  //double period=0.53845113930391119; //Aug 18 01 am, 2009
  //double period=0.53845070452913933; //Aug 17 01 am, 2009
  //double period=0.538449855779; //Aug 15 01 am, 2009
  //double period=0.53844944181201026;  // Aug 14 01am, 2009
  //
  //double dm=43.516;  //B2217+47  June 29 04am, 2009
  //double period=0.53843885097630653;  // June 29 04am, 2009  
  //double dm=43.516;  //B2217+47  June 30 04am, 2009
  //double period=0.53843904567176833;  // June 30 04am, 2009  
  //double dm=43.516;  //B2217+47  July 01 04am, 2009
  //double period=0.53843896847426902;  // July 01 04am, 2009  
  //double dm=43.516;  //B2217+47  July 01 04am, 2009
  //double period=0.53844812745031140;  // July 01 04am, 2009  
  //double dm=43.516;  //B2217+47  Aug 18 11pm, 2008
  //double period=0.53845132855582085;  // Aug 18 11pm, 2008  
  //double period=0.53845207234365068;  //B2217+47  Aug 20 1am, 2008
  //double period=0.53845252186433322;  //Aug 21 1am, 2008
  //double period=0.53845297697284241;  //Aug 22 1am, 2008
  //double period=0.53845343775724348;  //Aug 23 1am, 2008
  //double period=0.53845603844148366;  //Aug 28 12:30am, 2008
  // === B0823+26 ======================================================
  //double dm=19.4751;  //B0823+26 Aug20 7am, 2008 
  //double period=0.53063898642355628;
  // ==== 2007 =========================================================
  //double dm=3.17600 ;   // B1929+10
  //double period=226.51581287877815/1000.;
  //double period=226.52455415509880/1000.; // Aug 8, 10PM, 2007
  //double dm=14.176 ;   // B2016+28
  //double period=557.95353755889391/1000.; // Aug 9, 12:30AM, 2007
  //double dm=43.516 ;   // B2219+47
  //double dm=12.889 ;   // B0834+06
  //double dm=19.4751 ;   // B0823+26
  //double dm=43.516;  //B2217+47
  //double dm=43.516;  //B2217+47 on May 20, 2008, 4am 
  //double dm=5.75130;   // B0809+74
  //double dm=36.6 ;   // J0843+0719
  //double period=538.44701517651617/1000.; // Aug 9, 8:45PM, 2007
  //double period=538.44700113885358/1000.; // Aug 9, 12:30AM, 2007
  //double period=538.44739535373128/1000.; // Aug 10, 9:00PM, 2007
  //double period=538.448196484749064/1000.; // Aug 12, 12:00AM, 2007
  //double period=538.448397818021673/1000.; // Aug 13, 12:00AM, 2007
  //double period=538.448941929355329/1000.; // Aug 14, 12:00AM, 2007
  //double period=1.2737682915785; // b0834
  //double period=1.277366458210704;// b0834 dec 4
  //double period=0.5306220979499;// b0823+26 dec 6
  //double period=0.53062339799224662;// b0823+26 3AM dec 9
  //double period=0.53062405943537465;// b0823+26 3AM dec 10
  //double period=0.53062490103624748;// b0823+26 3:48 AM dec 11 (transit)
  //double period=0.53062557388143478;// b0823+26 3:44 AM dec 12 (transit)
  //double period=0.53062695447312717;// b0823+26 3:36 AM dec 14 (transit)
  //double period=0.53062766136351593; // b0823+26 3:32 AM dec 15 (transit)
  //double period=0.53062837999848625;  // b0823+26 3:28:22 AM dec 16 (transit)
  //double period=0.53062910758558803; // b0823+26 3:24:27 AM dec 17 (transit)
  //double period=0.53062984457453592; // b0823+26 3:20 AM dec 18 (transit)
  //double period=0.53850370380092704;  //B2217+47 19:00 dec 17 (beam scan)
  //double period=0.53850391648296886; //B2217+47 19:00 dec 19 (bandpass)
  //double period=0.53844406749291124; //B2217+47 4am, May 20, 2008
  //double period=1.29220186954528003;// b0809+74 dec 6
  //double period=1.36586; // J0843+0719
  //double dm=26.77600 ;   // B0329
  //double period=714.46573783550343/1000.; // Aug 9, 1:20 AM, 2007
  //double period=714.46606694532477/1000.; // Aug 10, 7:45 AM, 2007
  //double dm=73.77 ;   // B0740-28
  //double period=166.76781634965982/1000.;
  //double freq0=1170;
  //double freq0=248; // MHz  ! lo1=310 lo4=62
  //=======================================================================

  

  double freq0=156; // MHz  ! lo1=218 lo4=62
  int isign=-1; 	// -1 for inverted frequencies


  // check to see that we have a full buffer
  if (len != NT) {
    printf("did not get full buffer, rank: %d\n",rank);
    exit(43);
  }

  if (iteration == 0) {
    // Initialize MPI
    ierr=MPI_Comm_size(MPI_COMM_WORLD,&nproc);
    if (ierr != 0) printf("error finding comm size=%d\n",ierr);
    if (nproc != NNODEACQ+2*NNODECORR) {
      printf("number nodes compiled %d != number nodes executed %d\n",NNODEACQ+2*NNODECORR,nproc);
      exit(42);
    }
    ierr=MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if (ierr != 0) printf("error finding comm rank=%d\n",ierr);
    //printf("rank %d in rtcorr\n",rank);
    // initialize fft 
    fft1dippirc16init(FFTLEN);
    fp=fopen("/mnt/code/gsbuser/EoR/Correlate_dualwrite64/walsh.char","r");
    if (fp==NULL) {
      perror("open walsh.char");
      exit(-1);
    }
    if (fread(walshsign,sizeof(char),(NNODEACQ*NCHAN*NWALSH2*2),fp) !=(NNODEACQ*NCHAN*NWALSH2*2)) {perror("read walsh");exit(-1);}
    for (j=0;j<3;j++) for (i=0;i<NT/4;i++) obuf[j][i]=0;
    //if (rank >= NNODECORR || rank < NNODEACQ) {
    if (rank < NNODEACQ+NNODECORR) {
      fp=fopen("/mnt/code/gsbuser/EoR/Correlate_dualwrite64/delay.2011jul01.dat","r");
      //fp=fopen("/mnt/code/gsbuser/EoR/Correlate_dualwrite64/antsys.hdr.microseconds","r");
    if (fp==NULL) {
      perror("open delay.dat");
      exit(-1);
    }
    printf("rank %d delay=",rank);
    delaymin=1.e20;
    for (i=0;i<NNODEACQ*NCHAN;i++) {
      double delay;
      fscanf(fp,"%lf\n",&delay);
      delay=-delay;
      delaymin=MIN(delaymin,delay);
      if (i/NCHAN==rank%NNODECORR) idelay[i%NCHAN]=delay/0.03;
    }
    for (i=0;i<NCHAN;i++){
      idelay[i]-=delaymin/0.03;
      printf("%d ",idelay[i]);
    }
    printf("\n");
    fclose(fp);
    }
#ifdef GATEDPOW
    for (i=0;i<NGATEAUTO;i++) for(j=0;j<NFAUTO;j++) gatepowd[i][j]=gatepow[i][j]=0;
    if (rank == 0) {
        sprintf(fn,outfile,rank,"gatepow");
	fdgpow=open(fn,O_TRUNC|O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);
    if (fdgpow<0) {
      printf("rank %d: ",rank);
      perror("open gpow file");
    }
    }
#endif
    if (rank >= NNODECORR) {
      //printf("rank %d test2\n",rank);
      if (rank>=NNODECORR+NNODEACQ) {
#define NBUF 50
	//const int NBUF=10, MAXBUF=100000;
	const int MAXBUF=NLOOPS;
	int wsize[MAXBUF],i=0,iw=0,j;
	static char dumpcorrbuf[NBUF][NFOLD*NCROSS/NNODECORR+sizeof(struct CorrHeader)+sizeof(int)+sizeof(float)*NCORR+sizeof(char)*NFOLD*FFTLEN/16/NNODECORR];
            //printf("rank %d entering region common\n",rank);
    if (rank-NNODECORR-NNODEACQ< NNODEACQ && rank-NNODECORR-NNODEACQ>=0) {
      MPI_Status status;
      MPI_Recv(norm,FFTLEN*8,MPI_CHAR,rank-NNODEACQ,100,MPI_COMM_WORLD,&status);
    sprintf(fn,outfile,rank-NNODECORR-NNODEACQ,".norm");
    fdout=open(fn,O_TRUNC|O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IROTH);
    if (fdout<0) {
      printf("rank %d: ",rank);
      perror("open norm output file");
    }
    write(fdout,norm,FFTLEN*8);
    close(fdout);
    }

  omp_set_num_threads(2);
#pragma omp parallel sections default(shared)
	{
#pragma omp section
	  { /* MPI section */
            //printf("rank %d entering region MPI\n",rank);
	    for (i=0;i<NLOOPS;i++) {
	      MPI_Status status;
              int count;
  MPI_Bcast(&tv,sizeof(struct timeval),MPI_CHAR,MASTERNODE,MPI_COMM_WORLD);
                if (i<4) continue;
                wsize[i]=i;
	        //printf("rank %d waiting for %d\n",rank,rank-NNODECORR-NNODEACQ);	
	        MPI_Recv(wsize+i,4,MPI_CHAR,rank-NNODECORR-NNODEACQ,150,MPI_COMM_WORLD,&status);
                //MPI_Get_count(&status,MPI_CHAR,&count);
	        //printf("rank %d wsize=%d max=%d,count=%d\n",rank,wsize[i],NFOLD*NCROSS/NNODECORR+sizeof(struct CorrHeader)+sizeof(int)+sizeof(float)*NCORR+sizeof(char)*NFOLD*FFTLEN/16/NNODECORR,count);	
	        MPI_Recv(dumpcorrbuf[i%NBUF],wsize[i],MPI_CHAR,rank-NNODECORR-NNODEACQ,150,MPI_COMM_WORLD,&status);
                //MPI_Get_count(&status,MPI_CHAR,&count);
	        //printf("rank %d rec count=%d\n",rank,count);	
	      while (i-iw>NBUF-3) {
		printf("rank %d waiting for write at buf=%d iw=%d\n",rank,i,iw);
		usleep(100000);
	      }
	    }
	  }
#pragma omp section
	  {  /* write disk section */
            int maxlag=0;
	    // open output data file
            //printf("rank %d entering region write\n",rank);
	    sprintf(fn,outfile,rank-NNODECORR-NNODEACQ,"");
	    fdout=open(fn,O_TRUNC|O_CREAT|O_WRONLY|O_SYNC,S_IRUSR|S_IWUSR|S_IROTH);
    if (fdout<0) {
      printf("rank %d: ",rank);
      perror("open output file");
    }
	    for (iw=4;iw<MAXBUF;iw++) {
	      while(i<=iw) usleep(1000);
              maxlag=MAX(maxlag,i-iw);
              //printf("rank %d write wsize=%d\n",rank,wsize[iw]);
	      if (write(fdout,dumpcorrbuf[iw%NBUF],wsize[iw]) != wsize[iw]) perror("write");
              if (iw == MAXBUF-10) printf("rank %d write maxlag=%d\n",rank,maxlag);
	    }
	  }
       
        } // end parallel region
      }else{
	for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) norm[j][i]=1;
	sprintf(fn,outfile,rank,".norm");
	normalize(norm,buf,"/tmp/beamnorm.dat",rank);
	MPI_Send(norm,FFTLEN*8,MPI_CHAR,rank+NNODEACQ,100,MPI_COMM_WORLD);
        MPI_Send(norm,FFTLEN*8,MPI_CHAR,rank-NNODECORR,100,MPI_COMM_WORLD);
      }
    } else {
    MPI_Status status;
      //printf("rank %d test3\n",rank);
    for (i=0;i<NCROSS;i++) for (j=0;j<NFOLD;j++) cross[j][i]=0;
    if (rank < NNODEACQ) {
      MPI_Recv(norm,FFTLEN*8,MPI_CHAR,rank+NNODECORR,100,MPI_COMM_WORLD,&status)
;
    }

#ifdef DUMP1BIT
    sprintf(fn1,outfile,rank,".bit");
    fdout1bit=open(fn1,O_TRUNC|O_CREAT|O_WRONLY|O_SYNC,S_IRUSR|S_IWUSR|S_IROTH);
    if (fdout<0) {
      printf("rank %d: ",rank);
      perror("open 1bit file");
    }
#endif
    }
  } // firstime==1
  MPI_Bcast(&tv,sizeof(struct timeval),MPI_CHAR,MASTERNODE,MPI_COMM_WORLD);
#if 0
  if (iteration == 0) timestart=tv.tv_sec+tv.tv_usec/1000000.;
#else
	//timestart=1186686126+165118/1000000.;  // ca 12:30 AM Aug 10, 2007, start of 2217_5
	//timestart=1197338375+850306/1000000.;  // ca 7:30 AM Dec 11, 2007, start of 0823 11_9
//	timestart=1280504647.190610;  // Jul 30 2010
	timestart=1298091701.580934;  // feb 19, 2011
#endif
    if (iteration == 0 && rank==0) printf("current time is %lf, using start time of %lf\n",tv.tv_sec+tv.tv_usec/1000000.,timestart);
#ifdef DUMP1BIT
  omp_set_num_threads(4);
#else
  omp_set_num_threads(3);
#endif
#pragma omp parallel sections default(shared) private(start,current,sec) 
//  {
//#pragma omp sections nowait
    {
/* sections 	1: FFT.  Single thread.  almost all nodes
		2: xmac. only compute nodes
		3: communication.  All nodes
		4: 1bit disk write.  compute nodes
   the acquisition nodes should run without hyperthreading.
*/


#pragma omp section
      {  /* FFT section */
#ifdef PROFILE
      ftime( &start );
#endif
      if (rank>=NNODECORR) {
	printf("rank %d ",rank);
	for (int i=0;i<8;i++) printf("%d ",buf[i]);
	printf("\n");
	// fft data in buf -> obuf
        time0=(tv.tv_sec+tv.tv_usec/1000000.)-timestart;
#ifdef GATEDPOW
	for (i=0;i<NGATEAUTO;i++) for(j=0;j<NFAUTO;j++) {
		gatepowd[i][j]+=gatepow[i][j];
		gatepow[i][j]=0;
	}
#endif
	xfft(obuf[iteration%3],buf,norm,idelay,3,walshsign[0][rank-NNODECORR]);
#if 0
        int fdx=open("/mnt/raid0/pen/fftdump.dat",O_TRUNC|O_CREAT|O_WRONLY|O_SYNC,S_IRUSR|S_IWUSR|S_IROTH);
	write(fdx,obuf[iteration%3],NT/4);
        fdx=open("/mnt/raid0/pen/ibufdump.dat",O_TRUNC|O_CREAT|O_WRONLY|O_SYNC,S_IRUSR|S_IWUSR|S_IROTH);
	write(fdx,buf,NT/2);
	exit(-1);
#endif
#ifdef PROFILE
	ftime( &current );
	sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
	if (sec>0.25)
	printf("rank %d fft %d size buffers in %lf seconds\n",rank,NT, sec);
#endif
      } else if (iteration>0 ) {
        int nrecv,itarget,isource;
        if (rank<NNODEACQ) {
        time0=(tvhist[(iteration+2)%3].tv_sec+tvhist[(iteration+2)%3].tv_usec/1000000.)-timestart;
	xfft(obuf[(iteration+2)%3],obuf8[iteration%2],norm,idelay,1,walshsign[1][rank]);
//	for (i=0;i<1000;i++) printf("%hhx",obuf[(iteration+2)%3][i]);
	ftime( &current );
	sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
//	printf("rank %d fft %d size buffers in %lf seconds\n",rank,NT, sec);
        }
#ifdef DUMP1BIT
        if (iteration>2) {
		dump1bit(dump1bitbuf[iteration%2],xbuf[iteration%2],tvhist[iteration%3]);
	}
#endif
#ifdef GATEDPOW
	for (i=0;i<NGATEAUTO;i++) for(j=0;j<NFAUTO;j++) {
		gatepowd[i][j]+=gatepow[i][j];
		gatepow[i][j]=0;
	}
#endif
	ftime( &current );
	sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
        if (sec>0.25)
	printf("rank %d fft+dump %d size buffers in %lf seconds\n",rank,NT, sec);
      }
      }
#pragma omp section
      { 
	if (rank<NNODECORR && iteration > 2){

        ftime( &start );

        time0=(tvhist[iteration%3].tv_sec+tvhist[iteration%3].tv_usec/1000000.)-timestart;
        foldpulsar(cosac,sinac,dmoffset,gate,time0,dm,period,freq0,isign,rank);
  	xmac(cross,xbuf[iteration%2],sinac,cosac,dmoffset,gate,walshsign[0]);
  	xmac(cross,xbuf[iteration%2]+NTCORR/2,sinac+NSIZEGATE/2,cosac+NSIZEGATE/2,dmoffset,gate+NSIZEGATE/2,walshsign[1]);

//	dumpcorr(fdout,cross,tvhist[iteration%3]);

	dumpcorr(dumpcorrbuf[iteration%2],cross,tvhist[iteration%3]);
	ftime( &current );
	sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
	if (sec>0.25)
	printf("rank %d xmac+convert %d size buffers in %lf seconds\n",rank, NTCORR*2,sec);
        }
      }
#pragma omp section
      {

  // all the communication
  // pass work units
	  //	  const int MPIWAITCHUNK=1;
	  //NNODECORR;
	  int mpi_size,nrecv;
	  int isource,mpisource,itarget;
	  mpi_size=MPI_BUF_CHUNK/2;

	  ftime( &start );
	  if (rank>=NNODECORR) {  /* if we are an acquisition node: */

		/* send half the raw data off */
		//printf("rank %d start send\n",rank);
	    MPI_Send(buf+NT/2,NT/2,MPI_CHAR,rank-NNODECORR,100,MPI_COMM_WORLD);
		//printf("rank %d done send\n",rank);

	  if (iteration>1) {  /* send the FFTd buffers */
            nrecv=0;
	    for (i=0;i<NNODECORR;i++) {
              MPI_Request request[MPIWAITCHUNK];
              MPI_Status stat[MPIWAITCHUNK];

	      itarget=(i+rank)%NNODECORR;
	      MPI_Isend(obuf[(iteration+1)%3]+itarget*mpi_size,mpi_size,MPI_CHAR,itarget,200+i,MPI_COMM_WORLD,request+nrecv++);
		//printf("rank %d done isend size=%d\n",rank,size);
/* for NNODECORR=8, NNODE=16, we have the following:
	i=0:  rank=0: itarget=0, isource=0,8.
	i=1:  rank=0: itarget=1, isource=7,15
 */
	      if ((i+1)%MPIWAITCHUNK==0){
//		printf("rank %d start waitall isend nrecv=%d i=%d itarget=%d\n",rank,nrecv,i,itarget);
		MPI_Waitall(nrecv,request,stat);
		nrecv=0;
		//printf("rank %d done waitall isend i=%d\n",rank,i);
	      }
	    }
	    }
	  } else {
	    MPI_Request request[2];
	    MPI_Status stat[2];
            int nrecv=0;
            if (rank<NNODEACQ){  /* first group of nodes gets 1/2 buffer to FFT */
	       MPI_Status status;
	       //MPI_Recv(obuf8[1-iteration%2],NT/2,MPI_CHAR,rank+NNODECORR,100,MPI_COMM_WORLD,&status);
	       MPI_Irecv(obuf8[1-iteration%2],NT/2,MPI_CHAR,rank+NNODECORR,100,MPI_COMM_WORLD,request+nrecv++);
	    }
	    if (iteration>3) {
                struct CorrHeader *dumpheaderp;

                ftime( &start );
                dumpheaderp=dumpcorrbuf[1-iteration%2];
                int wsize=dumpheaderp->totalsize; // NCROSS*NFOLD/NNODECORR+sizeof(tv)+sizeof(int);
                //printf("rank %d sending %d to %d\n",rank,wsize,rank+NNODECORR+NNODEACQ);      
                MPI_Send(&wsize,4,MPI_CHAR,rank+NNODECORR+NNODEACQ,150,MPI_COMM_WORLD);
                MPI_Isend(dumpcorrbuf[1-iteration%2],wsize,MPI_CHAR,rank+NNODECORR+NNODEACQ,150,MPI_COMM_WORLD,request+nrecv++);

            }
	    MPI_Waitall(nrecv,request,stat);
		/* first get the FFTd data from the acquisition nodes */
	  if (iteration>1) {
	    nrecv=0;
	    for (i=0;i<NNODECORR;i++) {
	      MPI_Request request[MPIWAITCHUNK];
	      MPI_Status stat[MPIWAITCHUNK];

	      isource=(NNODECORR-i+rank)%NNODECORR;
	      if (isource<NNODEACQ) {
		mpisource=isource+NNODECORR;
		MPI_Irecv(xbuf[1-iteration%2]+isource*mpi_size,mpi_size,MPI_CHAR,mpisource,200+i,MPI_COMM_WORLD,request+nrecv++);
		//printf("rank %d done irecv size=%d\n",rank,size);
	      }
	      if ((i+1)%MPIWAITCHUNK==0 && nrecv>0) {
//		printf("rank %d start waitall nrecv=%d\n",rank,nrecv);
		MPI_Waitall(nrecv,request,stat);
		nrecv=0;
//		printf("rank %d done waitall\n",rank);
	      }
            }
		/* now swap FFT'd data with computation nodes which did half the FFTs */
#if 1
	    nrecv=0;
            for (i=0;i<NNODECORR;i++) {
	      static MPI_Request request[2*MPIWAITCHUNK];
	      static MPI_Status stat[2*MPIWAITCHUNK];

              itarget=(i+rank)%NNODECORR;
	      if (rank<NNODEACQ)
              MPI_Isend(obuf[(iteration+1)%3]+itarget*mpi_size,mpi_size,MPI_CHAR,itarget,100+i,MPI_COMM_WORLD,request+nrecv++);
              isource=(NNODECORR-i+rank)%NNODECORR;
	      if (isource < NNODEACQ) 
              MPI_Irecv(xbuf[1-iteration%2]+isource*mpi_size+NTCORR/2,mpi_size,MPI_CHAR,isource,100+i,MPI_COMM_WORLD,request+nrecv++);
              if ((i+1)%MPIWAITCHUNK==0) {
		//printf("rank %d start 2nd waitall nrecv=%d isource=%d itarget=%d i=%d\n",rank,nrecv,isource,itarget,i);
                MPI_Waitall(nrecv,request,stat);
#if 0
		for (int ii=0;ii<mpi_size&&xbuf[1-iteration%2][isource*mpi_size+ii+NTCORR/2]==0;ii++) 
			if (ii>mpi_size-2 && iteration>3) {
				printf("zero buffer, mpi_size=%d isource=%d, rank=%d iteration=%d\n",mpi_size,isource,rank,iteration);
//				exit(-1);
			}
#endif
	      }
//		printf("rank %d done 2nd waitall nrecv=%d i=%d\n",rank,nrecv,i);
                nrecv=0;
            }
#endif
	  }
	  
	}
	ftime( &current );
	sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
	if (sec>0.27)
	printf("rank %d mpi %d size buffers in %lf seconds\n",rank, NT, sec);

      }
#ifdef DUMP1BIT
#pragma omp section
      {
	if (rank<NNODECORR && iteration>3) {
       		ftime( &start );
		int wsize=(NT*NNODEACQ)/(8*NNODECORR)+sizeof(tv);
		if(write(fdout1bit,dump1bitbuf[1-iteration%2],wsize)!=wsize) perror("write1bit");
		ftime( &current );
		sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;
		if (sec>0.25)
		printf("rank %d write %d size 1bit buffers in %lf seconds\n",rank, wsize, sec);
	}
      }
#endif
    } // end parallel sections

  
//  }
//  if (rank ==0) printf("tv=%d\n",tv.tv_sec);
#ifdef GATEDPOW
	if ((iteration+1)%GATEPOWOUT==0) {
	if (rank==0) for (i=0;i<NGATEAUTO;i++) for(j=0;j<NFAUTO;j++) gatepowsum[j][i]=0;
	printf("rank %d calling MPI_Reduce\n",rank);
	if (iteration>0) MPI_Reduce(gatepowd,gatepowsum,NGATEAUTO*NFAUTO,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	for (i=0;i<NGATEAUTO;i++) for(j=0;j<NFAUTO;j++) gatepowd[j][i]=0;
	printf("rank %d done MPI_Reduce\n",rank);
	if (rank==0) {
		if (write(fdgpow,gatepowsum,NGATEAUTO*NFAUTO*sizeof(double)) != NGATEAUTO*NFAUTO*sizeof(double)) {
			perror("write gatepow");
			printf("write: fdgpow=%d\n",fdgpow);
		}
	}
	}
#endif
  tvhist[iteration%3]=tv;
  iteration++;
  start=current;
  //printf("rank %d exit corr_driver\n",rank);
}
