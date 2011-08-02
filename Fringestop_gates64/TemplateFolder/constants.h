/* Physical/astronomical constants:
 * SPEED_OF_LIGHT in meters/microsec
 * EARTH_OMEGA in radians/microsec
 */
#define SPEED_OF_LIGHT 299.792458
#define EARTH_OMEGA (7.288739858e-11)

/* Location of telescope, radians */
#define TEL_LAT 0.333299709
#define TEL_LON 1.292416311

/* Angular units */
#define DEGREE 0.017453292519943295769236907684886
#define ARCSEC (4.848136811e-6)

/* Reference catalog and observing epochs */
#define REF_EPOCH 11544.5
//#define OBS_EPOCH 13750.0
//#define OBS_EPOCH  14292.97  // Feb 20
//#define OBS_EPOCH  14236.986
#ifndef OBS_EPOCH
#define OBS_EPOCH 14321.84
#endif

/* Correlation file format */
/* Number of bytes in a correlation real number */
#define SIZE_CORRFLOAT 4
/* Corresponding data type in C */
#define DATA_CORRFLOAT float

/* System parameters */
#define MAX_FILENAME_LEN 1024

/* Correlation parameters:
 * FREQ0 in MHz (frequency of 1st bin)
 * DFREQ in MHz (frequency step between bins)
 * LENGTH in microsec (length of an individual block in FFT)
 * NBLOCK (# blocks per sample -- dimensionless)
 */
//#define CORR_FREQ0 156.0
#define CORR_FREQ0 156.0
//#define CORR_FREQ0 1170.0
//#define CORR_FREQ0 1280.0
//#define CORR_DFREQ (0.0040690104166666666666666666666667*2.)
#define CORR_DFREQ (-0.0040690104166666666666666666666667*2.)
#define CORR_LENGTH (245.76/2.)
#define CORR_NBLOCK (1024*2.)



/* Parameters for RAM buffering of visibility data: */
/* put #define BUFFER_ON here if we want to buffer */
#define BUFFER_ON
/* number of timestamps to buffer */
#define NBUFFER_VIS 128

/* Gain control parameters */
#define GAINMAX 0.1

/* Weight on each visibility (essentially 1/sigma^2 for each component).
 * In theory this is 1/(maximum correlation)^2(#subsamples per epoch)
 *
 * Also includes offset in Real part of visibilitiy (depends on rounding
 * in digital correlator).
 */
#define VISIBILITY_WEIGHT 16384.0
#define VISIBILITY_OFFSET 0.007526649338

/* RNG seed -- use #define RNG_SEED_FORTRAN to get same sequence
 * of RNGs in Gibbs sampler.
 */
#define RNG_SEED_FORTRAN 10000

/* Default values for closure errors */
#define CLOSURE_SATURATED 1.0e+49

/* Macros for complex arithmetic */
/* x=y */
#define C_COPY(x,y)   (x)[0]=(y)[0], (x)[1]=(y)[1]
/* x+=y*z */
#define CAFLOP(x,y,z) (x)[0]+=(y)[0]*(z)[0]-(y)[1]*(z)[1], (x)[1]+=(y)[1]*(z)[0]+(y)[0]*(z)[1]
/* x=y*z */
#define CWFLOP(x,y,z) (x)[0]=(y)[0]*(z)[0]-(y)[1]*(z)[1], (x)[1]=(y)[1]*(z)[0]+(y)[0]*(z)[1]
/* x+=y*conjugate(z) */
#define CCFLOP(x,y,z) (x)[0]+=(y)[0]*(z)[0]+(y)[1]*(z)[1], (x)[1]+=(y)[1]*(z)[0]-(y)[0]*(z)[1]
/* x =y*conjugate(z) */
#define CCWFLOP(x,y,z) (x)[0]=(y)[0]*(z)[0]+(y)[1]*(z)[1], (x)[1]=(y)[1]*(z)[0]-(y)[0]*(z)[1]
/* |x|^2 */
#define SQABS(x) ((x)[0]*(x)[0]+(x)[1]*(x)[1])

/* Function macros */
#define SINC(x) (fabs(x)>1.0e-12? sin(x)/(x): 1.-(x)*(x)/6.)
