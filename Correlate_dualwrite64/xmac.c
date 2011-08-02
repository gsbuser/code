#include "xfft.h"
#include <emmintrin.h>
#include "omp.h"
/*
  correlate the compressed fourier sequence
  result cross[FFTLEN/16][NCORR][2][8]
  gating: arrays dmoffset, gate
*/
void xmac(int cross[NFOLD][FFTLEN/16/NNODECORR][NCORR][2][8], char ibuf[NNODEACQ][FFTLEN/(FFTBLOCK*NNODECORR*2)][NT2/(FFTLEN*NCHAN)][NCHAN][FFTBLOCK],short int *foldsin,short int *foldcos, int *dmoffset, short int *gate,char walshsign[NNODEACQ*NCHAN][NWALSH2])
{
// register variables
  register __m128i accumr,accumi,prod1,prod2,vr1,vi1,vr2,vi2;
  __m128i vrsse,visse;
  short int *vr16,*vi16;
  // L1 cache buffers
//  short int v1[NCHAN*NNODE][NTCHUNK][2][8],accum[2][8];
// each SSE2 registers stores 8 short ints.
  __declspec(align(128)) __m128i v1[NCHAN*NNODECORR][NTCHUNK][2],accum[2];
// align with cache line
  // we keep accum in register space while iterating over NTCHUNK
  // L2 cache buffers
  __declspec(align(128)) static short int v[FFTBLOCK/8][NCHAN*NNODECORR][2][NTCHUNK][8];

  /* 
     We want to keep chunks of corr[FFTBLOCK][NCORR] to fit in L2.
     This results in an effective copy to a different cache line
     to prevent aliasing.
  */
  int i,j,it,ifout,io,jo,i8,ifblock,iot,itchunk,ichan,inode;

  omp_set_num_threads(4);
  
#pragma omp parallel default(none) shared(ibuf,cross,foldsin,foldcos,dmoffset,gate,walshsign) private(accumr,accumi,prod1,prod2,vr1,vi1,vr2,vi2,vrsse,visse,vr16,vi16,v1,accum,v,i,j,it,ifout,io,jo,i8,ifblock,iot,itchunk,ichan,inode)
{
  vr16=(short int*) &vrsse;
  vi16=(short int*) &visse;
#pragma omp for //schedule(dynamic,64)
  for (ifout=0;ifout<FFTLEN/(2*NNODECORR*FFTBLOCK);ifout++) 
    for (it=0;it<NT2/(FFTLEN*NCHAN);it+=NTCHUNK){// we insert this loop to keep corr local
      __m128i walshsse[NNODEACQ*NCHAN];
      // load v
      //if (ifout+it==0) printf("ifout=%d it%d\n",ifout,it);
      accumr=_mm_setzero_si128();
      for (inode=0;inode<NNODEACQ*NCHAN;inode++) walshsse[inode]=_mm_set1_epi16(walshsign[inode][it/NTCHUNK]);
      //for (inode=0;inode<NNODEACQ*NCHAN;inode++) walshsse[inode]=_mm_set1_epi16(1);
      for (ifblock=0;ifblock<FFTBLOCK/8;ifblock++) {
      //printf("ifblock=%d \n",ifblock);
	// load v1.  This may need to be blocked for large numbers of nodes
	// to fit into L1.
        for (inode=0;inode<NNODEACQ;inode++)
      	for (itchunk=0;itchunk<NTCHUNK;itchunk+=1) 
        for (ichan=0;ichan<NCHAN;ichan++){
          accumi=_mm_loadu_si128(&ibuf[inode][ifout][it+itchunk][ichan][ifblock*8]); // half the loads are aligned
	        vr2=_mm_unpacklo_epi8(accumr,accumi);  // convert to 16 bit
          vi2=_mm_slli_epi16(vr2,4);   // shift the real from 4LSB to 16-bit MSB
          vi1=_mm_srai_epi16(vr2,12);   // back to 16-bit LSB
          vr1=_mm_srai_epi16(vi2,12);   // the real part are the 4 LSB
//      if(ifout+it+ifblock+inode+itchunk+ichan==0) printf("ichan,inode,itchunk=%d %d %d\n",ichan,inode,itchunk);
//	        v1[ichan+inode*NCHAN][itchunk][0]=vr1;
	        v1[ichan+inode*NCHAN][itchunk][0]=_mm_mullo_epi16(vr1,walshsse[ichan+inode*NCHAN]);
//      if(ifout+it+ifblock+inode+itchunk+ichan==0) printf("ichan,inode,itchunk=%d %d %d\n",ichan,inode,itchunk);
//      printf("ichan=%d \n",ichan);
//	        v1[ichan+inode*NCHAN][itchunk][1]=vi1;
	        v1[ichan+inode*NCHAN][itchunk][1]=_mm_mullo_epi16(vi1,walshsse[ichan+inode*NCHAN]);
/*
	  vr2=_mm_unpackhi_epi8(accumr,accumi);  // convert to 16 bit
          vi2=_mm_slli_epi16(vr2,4);   // the real part are the 4 LSB
          vi1=_mm_srai_epi16(vr2,12);   
          vr1=_mm_srai_epi16(vi2,12);  
	  v1[ichan+inode*NCHAN][itchunk+1][0]=vr1;
	  v1[ichan+inode*NCHAN][itchunk+1][1]=vi1;
 */
        }
//_mm_load_ps(float * p )
        iot=0;
	      for (io=0;io<NCHAN*NNODEACQ;io++)
	      for (jo=io;jo<NCHAN*NNODEACQ;jo++){
	    /* inner loop:
	       two SSE2 vector loads from L1, two mult, two add.
	       Performs correlation for 8 numbers (32 operations).
	       We use 6 SSE2 registers.
	    */
//	    accumi=accumr=0;
	      accumi=accumr=_mm_setzero_si128();
	      for (i=0;i<NTCHUNK;i++){
	        vr1=v1[io][i][0];
	        vi1=v1[io][i][1];
	        vr2=v1[jo][i][0];
	        vi2=v1[jo][i][1];
//	      accumr+=vr1*vr2+vi1*vi2;
//	      accumi+=-vr1*vi2+vi1*vr2;
	        prod1=_mm_mullo_epi16(vr2,vr1);
	        accumr=_mm_add_epi16(prod1,accumr);
	        prod2=_mm_mullo_epi16(vr1,vi2);
	        accumi=_mm_sub_epi16(accumi,prod2);
	        prod1=_mm_mullo_epi16(vi1,vi2);
	        accumr=_mm_add_epi16(accumr,prod1);
	        prod2=_mm_mullo_epi16(vi1,vr2);
	        accumi=_mm_add_epi16(accumi,prod2);
	      }
//	    cross[ifblock][iot++][]+=accum;
/* we are hoping to keep the relevant section of cross in cache */
	      vrsse=accumr;
	      visse=accumi;
//      printf("iot=%d \n",iot);
	      for(i8=0;i8<8;i8++) {
			int offset,igate,isin,icos,ifreq;
			ifreq=ifout*FFTBLOCK/8+ifblock;
			offset=dmoffset[ifreq*8+i8];
			igate=gate[it/NREDGATE+offset];
#if 0
			if (igate > NFOLD || igate<0) {
				printf("igate=%d\n",igate);
				exit(-10);
			}
			if (abs(cross[ifreq][iot][1][i8][igate])>1000000) {
				printf("cross=%d inc=%d\n", cross[ifreq][iot][1][i8][igate], vi16[i8]);
				exit(-10);
			}
#endif
		      cross[igate][ifreq][iot][0][i8]+=vr16[i8];
		      cross[igate][ifreq][iot][1][i8]+=vi16[i8];
#ifdef FOLDSIN
			isin=foldsin[it/NREDGATE+offset];
			icos=foldcos[it/NREDGATE+offset];
		      cross[NFOLD-2][ifreq][iot][0][i8]+=vr16[i8]*icos/32768;
		      cross[NFOLD-2][ifreq][iot][1][i8]+=vi16[i8]*icos/32768;
		      cross[NFOLD-1][ifreq][iot][0][i8]+=vr16[i8]*isin/32768;
		      cross[NFOLD-1][ifreq][iot][1][i8]+=vi16[i8]*isin/32768;
#endif
 	      }
//      printf("iot=%d \n",iot);
        iot++;
	    }
    }
  }
} // omp end parallel
}

#ifdef TEST
#include <sys/time.h>
#include <sys/timeb.h>

main()
{
  __declspec(align(128)) char obuf[NT/2];
  const int NCROSS=FFTLEN*NCHAN*(NCHAN+1)/2;
  __declspec(align(128)) int cross[NCROSS];
  int i,nbuf;
  const int nloop=10;
  struct timeb start, current;


  nbuf=NT/FFTLEN/NCHAN;
  // Get the start time for profiling

  for (i=0;i<NCORR;i++) cross[i]=0;
  ftime( &start );

  for (i=0;i<nloop;i++)
    xmac(cross,obuf);

  // Get the current time for profiling
  ftime( &current );
  double sec = (((current.time - start.time) * 1000) + (current.millitm - start.
millitm)) / 1000.0;

  printf("correlated %d size buffers %d times in %lf seconds\n",NT,nloop, sec);

}     
#endif
