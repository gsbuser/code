/* raw correlation header structure in output from correlator */
struct CorrHeader {
        int lcount;
        struct timeval tv;
        int nfreq;
        int nfold;
        int totalsize;
} ;

/* Antenna coordinates structure: contains location and timelag info
 * for one antenna.
 */
typedef struct {
  double x,y,z,t;
} ANTCOORDS;

/* Visibility information structure: contains information required
 * to access a particular visibility or set of visibilities.
 *
 * Visibility files are ordered first in time, then baseline (00, 01,
 * .. 0{np-1}, 11, 12, etc.  Within each baseline the frequencies are
 * stored as complex*8=float[2].
 */
typedef struct {

  /* Number of files, and the times at which they start */
  long nfile;
  long *t_filestart;

  /* File containing visibilities */
  char *DataFiles;

  /* Number of pods, frequencies, times, cross-baselines,
   * and all-baselines, respectively.  Formulas:
   *
   * nb = np*(np-1)/2 and nba = np*(np+1)/2.
   *
   * nbyte_v = number of bytes in file per visibility
   * nbyte_t = number of bytes in file per timestamp = nbyte_v * nf * nba
   *
   * ndata = # of reals per timestamp = 2 * nf * nba
   */
   long np, nf, nt, nb, nba, nbyte_v, nbyte_t, ndata;

  /* Memory index for looking up a particular baseline:
   * the i-j baseline is in the ind[i]+j part of the file.
   */
  long *ind;

  /* Mask on each baseline: use (1) or not (0) in calibration. */
  unsigned short int *mask;

  /* Offsets on each baseline */
  double *offset;

  /* Buffering parameters */
  long t_buffer_min;
  long t_buffer_max;
  void *buffer;

  /* Flag on which baselines to use for calibration */
  short int *usecal;
} VIS;

/* Source structure.  Will add more to this soon. */
typedef struct {
  /* Position variables */
  double ra, dec;

  /* Intensity at central frequency. */
  double I;

  /* Source model -- right now elliptical Gaussian */
  double a2, b2, pa;
} RADSRC;

/* The beam structure is currently a place-holder */
typedef struct {
  int placeholder;
} BEAM;
