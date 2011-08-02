/* NOVAS routines */
#ifndef NOVAS_ROUTINES_H
#define NOVAS_ROUTINES_H
#include "novas-c201/novas.h"
#include "novas-c201/novascon.h"
#include "novas-c201/solarsystem.h"
#endif

/* Header file for read_data.c routines */
#ifndef ROUTINES_H
#define ROUTINES_H

void readbuf2(int fd,char *buf,float *fmax,char *lmap);

void dump1byte(int fpp, float *fbuf, int size);
void dump1bytefsall(int fpp, float *fbuf, int size);
void dump2byte(int fpp, float *fbuf, int size);

void int2float(int nfold, int ncorr, int fftlen, int npod, int nnode, float *fcross, int *icross);

/* read_data */

void get_tjd_gst(char FileName[], long nepochs, double *TJD, double *GST);

void get_antenna_coords(char *FileName, ANTCOORDS *coords, long npods);

void new_visibility(VIS *v, unsigned long flag, char *FileName);

void kill_visibility(VIS *v);

void read_data_vector(VIS *v, double *data, long i, long j,
  long tsample, unsigned long flag);

void set_offset(VIS *v);

void set_fixed_offset(VIS *v);

void read_src_file(char *FileName, RADSRC *src, long Nsrc);

/* utils */

double get_delta_t(double H, double Dec, long i, long j,
  ANTCOORDS *coords);

double ant_separation(long i, long j, ANTCOORDS *coords);

void get_uv(double H, double Dec, long i, long j, ANTCOORDS *coords,
  double *ul, double *vl);

void get_dist_pa(double Dec0, double Dec, double delta_RA, double *dist, 
  double *pa);

void get_beam_coords(double H_src, double Dec_src, double H_beam,
  double Dec_beam, double *X, double *Y, double *phi);

double tjd2gst(double TJD);

void print_bad_baselines(FILE *fp, long np, double *closure, VIS *v,
  double amp_err_min, double phase_err_min);

void get_elaz(double H, double Dec, double *el, double *az);

/* beam_model */

void get_beam_amplitude(double x, double y, BEAM *bi, BEAM *bj, long nf,
  double fi, double df, double *out, double TJD, double el, double az);

void get_stokes(RADSRC *src, double freq, double *IQUV, double ul,
  double vl);

void get_template(BEAM *b, long i, long j, RADSRC *src, double *TJD,
  double *GST, double *RA_ctr, double *Dec_ctr, long nsrc, long fstart,
  long nf, long t_index, ANTCOORDS *coords, double *predict,
  unsigned long flag);

double get_vt_tt(BEAM *b, long i, long j, RADSRC *src, double *TJD,
  double *GST, double *RA_ctr, double *Dec_ctr, long nsrc, long fstart,
  long nf, long t_index, ANTCOORDS *coords, double *vt, double *tt,
  VIS *v, unsigned long flag);

/* sampling */

double gibbs_gain(BEAM *b, RADSRC *src, double *TJD,
  double *RA_ctr, double *Dec_ctr, long nsrc, long np, long fstart,
  long nf, long tstart, long nt, ANTCOORDS *coords, double *gain,
  VIS *v, long n_iter, double *closure, unsigned long flag);

/* froutines */

#include "froutines.h"

#endif

