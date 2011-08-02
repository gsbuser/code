#include <stdio.h>
#include <math.h>
#include "ipp.h"
#include "ippm.h"
#include "ipps.h"
#include "xfft.h"
#define NBUF 1000000*2

static IppsFFTSpec_R_16s* pFFTSpecirc16;
static int nsrc=0;

void fft1dippirc16init(int np)
{
  int pow2,order;

  if (np == nsrc) return;
  if (np != 0) ippsFFTFree_R_16s(pFFTSpecirc16);
  nsrc=np;
  if (nsrc > NBUF/10) printf("fft1dipprc: NBUF may be too small\n");
  pow2=nsrc;
  order=log((double) nsrc)/log(2.)+0.5;
  ippsMalloc_16s(pow2);
  ippsFFTInitAlloc_R_16s(&pFFTSpecirc16,order, IPP_FFT_DIV_INV_BY_N,ippAlgHintFast);
}

void fft1dippirc16(Ipp16s *a, Ipp16s *b, int isignp){

   Ipp8u pWork[NBUF];
   IppStatus status;
   const int scale=0;


   //fft transformation
   if (isignp==1){
     status=ippsFFTFwd_RToPerm_16s_Sfs(a, b, pFFTSpecirc16, scale, pWork);
   } else {
     status=ippsFFTInv_PermToR_16s_Sfs(a, b, pFFTSpecirc16, scale, pWork);
   }
   if (status != 0) {
        printf("fft1dippirc16 error=%d isign=%d message=%s\n",status,isignp,
                ippGetStatusString(status));
        printf("a=%ld b=%ld pFFTSpecirc16=%d\n",a,b,pFFTSpecirc16);
        printf("nsrc=%d\n",nsrc);
	exit(-1);
   }
}

static IppsFFTSpec_R_32f* pFFTSpecsrc;
static int nsrcf=0;


fft1dippsrcinit(int np)
{
  int pow2,order;

  if (np == nsrcf) return;
  if (np != 0) ippsFFTFree_R_32f(pFFTSpecsrc);
  nsrcf=np;
  if (nsrcf > NBUF/10) printf("fft1dipprc: NBUF may be too small\n");
  pow2=nsrcf;
  order=log((double) nsrcf)/log(2.)+0.5;
  ippsMalloc_32f(pow2);
  ippsFFTInitAlloc_R_32f(&pFFTSpecsrc,order, IPP_FFT_DIV_INV_BY_N,ippAlgHintFast);
}

int fft1dippsrc(Ipp32f *a, Ipp32f *b, int isignp){

   Ipp8u pWork[NBUF];
   IppStatus status;

   //fft transformation
   if (isignp==1){
     status=ippsFFTFwd_RToCCS_32f(a, b, pFFTSpecsrc, pWork);
   } else {
     status=ippsFFTInv_CCSToR_32f(a, b, pFFTSpecsrc, pWork);
   }
   if (status != 0) {
        printf("fft1dippsrc error=%d isign=%d message=%s\n",status,isignp,
                ippGetStatusString(status));
        printf("a=%ld b=%ld pFFTSpecsrc=%d\n",a,b,pFFTSpecsrc);
        printf("nsrc=%d\n",nsrcf);
        exit(-1);
   }
}
