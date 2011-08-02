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
//#include "ipp.h"
#define FFTLEN (4096)
#define NTCHUNK 16
#define NT (32<<20) // MB  
#define FFTBLOCK 128   // should be one cache line size
#define NNODE 16        // number of acquisition nodes
#define NPOD 60
#define NCHAN 4        // number of interleaved channels
#define NCORR ((NPOD)*(NPOD+1)/2)
#define NCROSS FFTLEN*NCORR
#define MPI_BUF_CHUNK NT/2/NNODE
#define MPI_NORM_CHUNK NCHAN*FFTLEN/2/NNODE
#define NFOLD 1
#define FFTNODE (FFTLEN/NNODE)
#define NBINPDF 2000

void read2byte_mask(int fd, double sigmacut[NCORR], int size, long long int *incount, long long int *outcount, char *flag);
int compare_floats (const float * a, const float * b);
int calpdf(int fpp, double corrpdf[NCORR][NBINPDF], float fmax_tot[NCORR], int size);
void read2byte_sigmacut(int fpp, float *fbuf, double *sigmacut, int size, long long int *incount, long long int *outcount);
int readfmax(int fpp, float fmax_tot[NCORR], int size);
void read1byte(int fpp, float *fbuf, int size);
void dump1byte(int fpp, float *fbuf, int size);
void corr_driver(signed char *buf, int *ac, int len);
void xfft(char *obuf, char *ibuf, short int norm[NCHAN][FFTLEN/2]);
void xmac(int cross[FFTLEN/16][NCORR][2][8][5], char ibuf[NNODE][FFTLEN/(FFTBLOCK*NNODE*2)][NT/(FFTLEN*NCHAN)][NCHAN][FFTBLOCK],short int *foldsin, short int *foldcos);
void normalize(short int norm[NCHAN][FFTLEN/2]);
//void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODE*NCHAN][FFTLEN/2]);
//void int2float(float fcross[NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8],short int norm[NNODE*NCHAN][FFTLEN/2], int rank);
//void int2float(float fcross[NFOLD][NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8][NFOLD], int rank);

#if 0
void fft1dippirc16init(int np);
int fft1dippirc16(Ipp16s *a, Ipp16s *b, int isignp);
void fft1dippsrcinit(int np);
int fft1dippsrc(Ipp16s *a, Ipp16s *b, int isignp);
void decode(signed char*buf, char *buf1);
void int2float(float fcross[3][NCORR][FFTLEN/2][2],int icross[FFTLEN/16][NCORR][2][8][3],int rank);
#endif

#endif
