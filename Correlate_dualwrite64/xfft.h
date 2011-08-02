#ifndef XFFT_H
#define XFFT_H
/*

 Take a 4-way interleaved stream of signed char, 
 and write in transposed, compressed 4 bit format
 ready for MPI communication

 obuf[FFTLEN/FFTBLOCK/2][NT/NTCHUNK][NCHAN][NTCHUNK][FFTBLOCK]
 The first index is the block units for message passing.

 We want the last three to fit into cache.
 For 64 channels, 128 byte cache line, we can afford NTCHUNK less than 128.
 To be conservative, we will use NTCHUNK=32.

 */
#include "ipp.h"
#include "mpi.h"
#include <emmintrin.h>
#include <sys/time.h>
#define FFTLEN 4096
#define NTCHUNK 32
#define NT (32 << 20 ) // 32 MB 
#define FFTBLOCK 64   // should be one cache line size
#define NNODEACQ 16        // number of acquisition nodes
#define NNODECORR 16       // number of correlation nodes
#define MASTERNODE NNODECORR
#define NCHAN 4        // number of interleaved channels
#define NCORR ((NCHAN*NNODEACQ)*(NCHAN*NNODEACQ+1)/2)
#define NCROSS (FFTLEN*NCORR)
#define MPI_BUF_CHUNK (NT/2/NNODECORR)
#define MPI_NORM_CHUNK (NCHAN*FFTLEN/2/NNODECORR)
#define NREDGATE NTCHUNK // granularity of gates, should be smaller than NTCHUNK
#define NT2 (NT/2)
#define NTCORR (NT2*NNODEACQ/NNODECORR)
#define NSIZEGATE (2*NT2/(FFTLEN*NCHAN*NREDGATE)) // length of gate tables
#define NDIMGATE (12*NSIZEGATE) // dimension of gate weights
#define NDIMGATEPOW (2*NT/(FFTLEN*NCHAN)) // dimension of gate weights
//#define FOLDSIN		// fold only on sin
//#define FOLDSINCOS	// fold on sin and cos
#define NGATE 16
#define NWALSH2 32  // the half time length of a walsh sequence
#ifdef FOLDSIN
#define NFOLD (NGATE+1)
#else
#define NFOLD NGATE
#endif
//#define GATEDPOW
#define NGATEAUTO 32
#define NFAUTO	32

#define NLOOPS (3600*4)

char *outfile;
struct timeval tv; struct timezone tz;

struct CorrHeader {
        int lcount;
        struct timeval tv;
        int nfreq;
        int nfold;
        int totalsize;
} ;

void dumpcorr(char *obuf, int *corr,struct timeval tv);
void dump1bit(char *obuf,char *ibuf,struct timeval tv);
void corr_driver(signed char *buf, int len, MPI_Comm mpi_acq_group);
void xfft(char *obuf, char *ibuf, __m128i norm[NCHAN/2][FFTLEN/8],short int 
	idelay[4],int nthread,char walshsign[NCHAN][NWALSH2]);
void xmac(int cross[FFTLEN/16][NCORR][2][8][NFOLD], char ibuf[NNODECORR][FFTLEN/(FFTBLOCK*NNODECORR*2)][NT2/(FFTLEN*NCHAN)][NCHAN][FFTBLOCK],short int *foldsin, short int *foldcos, int *dmoffset, short int *gate,char walshsign[NNODEACQ*NCHAN][NWALSH2]);

void normalize(short int norm[NCHAN][FFTLEN/2],char ibuf[NT],char*fn,int rank);
//void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODECORR*NCHAN][FFTLEN/2]);
void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODECORR*NCHAN][FFTLEN/2], int rank);
void fft1dippirc16init(int np);
void fft1dippirc16(Ipp16s *a, Ipp16s *b, int isignp);

void fitac(float acwt[2], int *zerofreq);
void  foldac(short int foldcos[NT/65536],short int foldsin[NT/65536],float wt[2]);
void  foldpulsar(short int foldcos[NDIMGATE],short int foldsin[NDIMGATE],int dmoffet[FFTLEN/2], short int gate[NDIMGATE], double time0, double dm, double period, double freq0, int isign, int rank);
void  foldpulsarpow(int dmoffset[FFTLEN/2], short int gate[NDIMGATE], double time0, double dm, double period, double freq0, int isign, int ngate);


void fft(float *a, int n);

#endif
