//#define DEBUG
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "xfft.h"
#include <omp.h>
#include <math.h>
#include <stdio.h>
#define MIN(x,y) ((x<y)?x:y)
#define SQR(x)  ((x)*(x))
/*

 Take a 4-way interleaved stream of signed char, 
 and write in transposed, compressed 4 bit format
 ready for MPI communication

 obuf[FFTLEN/FFTBLOCK/2][NT/NTCHUNK][NCHAN][NTCHUNK][FFTBLOCK]
 The first index is the block units for message passing.

 We want the last three to fit into cache.
 For 64 channels, 128 byte cache line, we can afford NTCHUNK less than 128.
 To be conservative, we will use NTCHUNK=32.

 Feb 17, 2009: added walsh switching to eliminate 4-bit bias

 */

//void xfft(int gatepow[NGATEAUTO][NFAUTO],char *obuf, char *ibuf, __m128i norm[NCHAN/2][FFTLEN/8],short int idelay[4],int nthread,double time0, double period)
void xfft(char *obuf, char *ibuf, __m128i norm[NCHAN/2][FFTLEN/8],short int idelay[4],int nthread,char walshsign[NCHAN][NWALSH2])
{
  int nbuf,k;
  __declspec(align(128)) static short int fftbuf[4][NCHAN][FFTLEN]; // cache align this

  nbuf=NT/FFTLEN/NCHAN/2;  // the last factor of two because half the data is sent off to quad cores
  omp_set_num_threads(nthread);
#pragma omp parallel for default(none) shared(obuf,ibuf,norm,nbuf,fftbuf,idelay,walshsign)  schedule(dynamic,64)
  for (k=0;k<nbuf-1;k++){
    int i,j,r32,i32,io,imp;
    short int i16,r16,igate,*ibuf16;
    register __m128i r0,r1,r2,r3,r4,r5,r6,r7;
    __m128i *fftbuf_sse;


#ifdef _OPENMP
    imp=omp_get_thread_num();
#else
    imp=0;
#endif
    
    /* we want fftbuf to stay in cache */
    for (j=0;j<NCHAN;j++) {
      for(i=0;i<FFTLEN;i++) {
	char ctmp,ctmp1;
	ctmp=ibuf[(k*FFTLEN+(i+idelay[j]))*NCHAN+j];
//	ctmp1=(ctmp & 0b10111111) | (ctmp >> 1 & 0b0100000); // clip
	fftbuf[imp][j][i]=ctmp*walshsign[j][k/NTCHUNK];
      }
      fft1dippirc16(fftbuf[imp][j],fftbuf[imp][j],1);
      fftbuf_sse=fftbuf[imp][j];
      for(i=0;i<FFTLEN/8;i++) fftbuf_sse[i]=_mm_mulhi_epi16(fftbuf_sse[i],norm[j][i]);
      r7=_mm_set1_epi8(0xf0);
      for (i=0;i<FFTLEN/2;i+=FFTBLOCK){
#if 0
	for (io=0;io<FFTBLOCK;io++){ // we process 2 numbers at once.
	  r32=fftbuf[imp][j][2*(i+io)];
//	  r32=r32*norm[j][i+io];
	  i32=fftbuf[imp][j][2*(i+io)+1];
//	  i32*=norm[j][i+io];
          obuf[io+j*FFTBLOCK+k*FFTBLOCK*NCHAN+i*(NT/(FFTLEN)/2)]=(r32 >> 16)&0x0f | (i32 >> 12)&0xf0;
	}
#else
	for (io=0;io<FFTBLOCK;io+=2*8){ // we process 32 numbers at once.
	/* bits 5-8  are extracted(?) */
	  r0=_mm_load_si128(&fftbuf[imp][j][2*(i+io)]);
	  r1=_mm_load_si128(&fftbuf[imp][j][2*(i+io)+8]);
	  r2=_mm_load_si128(&fftbuf[imp][j][2*(i+io)+16]);
	  r3=_mm_load_si128(&fftbuf[imp][j][2*(i+io)+24]);  // squeeze four 16-bit ints into 4-bit ints
#define MMSHUF _MM_SHUFFLE(3,1,2,0)   // little endian, swap i1 r1 i0 r0 -> i1 i0 r1 r0
	  r5=_mm_shufflehi_epi16(r0,MMSHUF);  
	  r6=_mm_shufflelo_epi16(r5,MMSHUF);
	  r0=_mm_shuffle_epi32(r6,MMSHUF);  // i3 i2 r3 r2 i1 i0 r1 r0 -> i3210 r3210
	  r5=_mm_shufflehi_epi16(r1,MMSHUF);
	  r6=_mm_shufflelo_epi16(r5,MMSHUF);
	  r1=_mm_shuffle_epi32(r6,MMSHUF);
	  r5=_mm_unpacklo_epi64(r0,r1);   // r0=i3210r3210, r1=i7654r7654 -> r5=r76543210
	  r6=_mm_unpackhi_epi64(r0,r1);    // r6=i76543210
	  r0=r5;
	  r1=r6;
	  
	  // now for the second set
	  r5=_mm_shufflehi_epi16(r2,MMSHUF);
	  r6=_mm_shufflelo_epi16(r5,MMSHUF);
	  r2=_mm_shuffle_epi32(r6,MMSHUF);
	  r5=_mm_shufflehi_epi16(r3,MMSHUF);
	  r6=_mm_shufflelo_epi16(r5,MMSHUF);
	  r3=_mm_shuffle_epi32(r6,MMSHUF);
	  r5=_mm_unpacklo_epi64(r2,r3);
	  r6=_mm_unpackhi_epi64(r2,r3);
	  r2=r5;  // r5 is the real part
	  r3=r6;
	  /* this part reduces the number of bits to LSB with saturate */
	  
	  r5=_mm_packs_epi16(r0,r2);  // r5=rFEDCBA9876543210, saturate
	  r0=_mm_srli_epi16(r5,4);    // in little-endian, real into LSB
	  // modified next few lines to just store MSB's.
	  r0=_mm_andnot_si128(r7,r0);//zero 4 MSB
	  r6=_mm_packs_epi16(r1,r3);  // imaginary
	  r1=_mm_and_si128(r6,r7);
	  r2=_mm_or_si128(r0,r1);
	/* write without polluting caches */
	  _mm_stream_si128(&obuf[io+j*FFTBLOCK+k*FFTBLOCK*NCHAN+i*(NT/FFTLEN/2)],r2);
	  /* the outgoing structure is obuf[FFTREST][TIME][CHAN][FFTBLOCK].
	     The BLOCK is cache friendly, the FFTREST is the MPI transpose order,
	     and we need all channels locally for the correlation.
	  */
	}
#endif
	// prefetch obuf non-persistent
      }
    }
  }
}
void normalize(short int norm[NCHAN][FFTLEN],char ibuf[NT], char*fn, int rank) {
 // __declspec(align(128)) short int fftbuf[FFTLEN];  // cache align this
  __declspec(align(128)) static float fnorm[NCHAN][FFTLEN/2];
	
  int i,j,k,nbuf,it,saturate;
  float anorm;
  int fd;

  fd=open(fn,O_CREAT|O_WRONLY|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IROTH);

  if (fd<0) {
	perror("open fnorm");
//	exit(-4);
}

  omp_set_num_threads(4);
  nbuf=NT/FFTLEN/NCHAN;
  anorm=1./(nbuf*2);
  for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) fnorm[j][i]=0;
#pragma omp parallel default(none) private(i,j) shared(ibuf,anorm,fnorm,nbuf) //reduction(+:fnorm)
  {
  __declspec(align(128)) float fnorml[NCHAN][FFTLEN/2];
  for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) fnorml[j][i]=0;
#pragma omp for
  for (k=0;k<nbuf;k++){
      /* we want fftbuf to stay in memory */
    for (j=0;j<NCHAN;j++) {
      __declspec(align(128)) short int fftbuf_norm[FFTLEN];  // cache align this
      for (i=0;i<FFTLEN;i++) fftbuf_norm[i]=ibuf[k*FFTLEN*NCHAN+i*NCHAN+j];
      fft1dippirc16(fftbuf_norm,fftbuf_norm,1);
      for(i=0;i<FFTLEN/2;i++)fnorml[j][i]+=anorm*(SQR(fftbuf_norm[2*i])+SQR(fftbuf_norm[2*i+1]));
    }
  }
#pragma omp critical
  for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) fnorm[j][i]+=fnorml[j][i];
  }
/*
 we scale norm to give a result that leaves the ceiling at two sigma.
 For a 4-bit code, the ceiling is 7*16.
 */
  saturate=0;
#pragma omp parallel for default(none) private(i,it) shared(norm,fnorm) reduction(+:saturate)
  for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) {
	it=16*65536*3./sqrt(fnorm[j][i])+0.5;
	norm[j][2*i+1]=norm[j][2*i]=MIN(it,32767);
	if (it>32767) saturate++;
  }
  if (saturate>0)   printf("rank %d norm saturate=%d\n",rank,saturate);
  write(fd,norm,NCHAN*FFTLEN*2);
  write(fd,fnorm,NCHAN*FFTLEN*4);
#if 0
  anorm=0;
  for (i=0;i<NT;i+=4){
	it=ibuf[i];
	anorm+=it*it;
  }
  printf("rank %d anorm=%f\n",rank,anorm/(NT/4));
#endif
}

#ifdef TEST
#include <sys/time.h>
#include <sys/timeb.h>

main()
{
  __declspec(align(128)) static char ibuf[NT],obuf[NT/2];
  int i,j;
  __declspec(align(128)) static short int norm[NCHAN][NT/2];
  const int nloop=10;
  struct timeb start, current;
  


  // Get the start time for profiling
  fft1dippirc16init(FFTLEN);

  for (j=0;j<NCHAN;j++) for (i=0;i<FFTLEN/2;i++) norm[j][i]=1;
  for (i=0;i<NT;i++) ibuf[i]=i%0xff;

  normalize(norm,ibuf);
  ftime( &start );
  for (i=0;i<nloop;i++)
    xfft(obuf,ibuf,norm);
  
  // Get the current time for profiling
  ftime( &current );
  double sec = (((current.time - start.time) * 1000) + (current.millitm - start.millitm)) / 1000.0;

  printf("correlated %d size buffers %d times in %lf seconds\n",NT,nloop, sec);

}
#endif
