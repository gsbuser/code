/* Assorted math & astrometric operations */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "constants.h"
#include "structures.h"
#include "routines.h"

/* Operation to compute the time delay between two antennas at a point on the sky.
 * H = hour angle @ Prime Meridian = RA-GST
 * Dec = declination
 * i,j = antenna numbers.
 */

double get_delta_t(double H, double Dec, long i, long j, ANTCOORDS *coords) {

  double dxbl, dybl, dzbl;
  double xs, ys, zs;

  dxbl = coords[i].x-coords[j].x;
  dybl = coords[i].y-coords[j].y;
  dzbl = coords[i].z-coords[j].z;

  xs = cos(Dec) * cos(H);
  ys = cos(Dec) * sin(H);
  zs = sin(Dec);

  return((xs*dxbl+ys*dybl+zs*dzbl)/SPEED_OF_LIGHT + coords[i].t - coords[j].t);
}

/* Separation in meters between two antennas */
double ant_separation(long i, long j, ANTCOORDS *coords) {
  double dxbl, dybl, dzbl;

  dxbl = coords[i].x-coords[j].x;
  dybl = coords[i].y-coords[j].y;
  dzbl = coords[i].z-coords[j].z;

  return(sqrt(dxbl*dxbl+dybl*dybl+dzbl*dzbl));
}

/* Separation in meters between two antennas */
double u_separation(double H, double Dec, long i, long j, ANTCOORDS *coords) {

  double ul, vl;

  get_uv(H, Dec, i, j, coords, &ul, &vl);

  return(ul);
}

/* Operation to construct the u*lambda, v*lambda vector.  This is baseline
 * length projected onto the sky.  'u' is projection onto north-pointing vector,
 * 'v' is onto east-pointing.  Units are microseconds.
 */

/* chris' get_uv */

void get_uv(double H, double Dec, long i, long j, ANTCOORDS *coords,
	    double *ul, double *vl) {

  double dxbl, dybl, dzbl;
  double xs, ys, zs;

  /* Convert H to hour angle at Greenwich (note that this does not
  over-write H variable in calling routine) */ 
  
  H -= TEL_LON;  // convert to LST
  H += TEL_LON;  // convert back to GST

  dxbl = coords[i].x-coords[j].x;
  dybl = coords[i].y-coords[j].y;
  dzbl = coords[i].z-coords[j].z;

  /* u-vector */
  xs = -sin(Dec) * cos(H);
  ys = -sin(Dec) * sin(H);
  zs = cos(Dec);
  *ul = (xs*dxbl+ys*dybl+zs*dzbl)/SPEED_OF_LIGHT;


  /* v-vector */
  xs = -sin(H);
  ys = cos(H);
 /* zs=0 so not used here */
  *vl = (xs*dxbl+ys*dybl)/SPEED_OF_LIGHT;

}


/*
  void get_uv(double H, double Dec, long i, long j, ANTCOORDS *coords,
	    double *ul, double *vl) {

  double dxbl, dybl, dzbl;
  double xs, ys, zs;
*/

  /* Convert H to hour angle at Greenwich (note that this does not
  over-write H variable in calling routine) */ 
  //  H += TEL_LON;   // convert GST to LST  (why plus?)
/*
  H -= TEL_LON;   // convert GST to LST

  dxbl = coords[i].x-coords[j].x;
  dybl = coords[i].y-coords[j].y;
  dzbl = coords[i].z-coords[j].z;
*/

  /* u-vector */
/*
  xs = sin(H);
  ys = cos(H);
  *ul = (xs*dxbl+ys*dybl)/SPEED_OF_LIGHT;
  */   

  /* v-vector */
/*
  xs = -sin(Dec)*cos(H);
  ys = sin(Dec)*sin(H);
  zs = cos(Dec);
*/

 /* zs=0 so not used here */
//  *vl = (xs*dxbl+ys*dybl+zs*dzbl)/SPEED_OF_LIGHT;

//}



/* Routine to compute distance and PA to point (Dec, Delta_RA) from point (Dec0,0). */

void get_dist_pa(double Dec0, double Dec, double delta_RA, double *dist, double *pa) {

  double x, y, z;
  double cc = cos(Dec)*cos(delta_RA);
  double sindec = sin(Dec);
  double cosdec0 = cos(Dec0);
  double sindec0 = sin(Dec0);

  x = cosdec0*sindec - sindec0*cc;
  y = cos(Dec)*sin(delta_RA);
  z = sindec0*sindec + cosdec0*cc;

  *dist = atan2(sqrt(x*x+y*y), z);
  *pa = atan2(y, x);
}

/* Operation to get the beam coordinates corresponding to a particular Dec_src/H,
 * in terms of the beam center Dec_beam/H.
 *
 * beam "X" = up
 * beam "Y" = left
 * phi = RA/Dec position angle of "X" (radians E of N)
 */

void get_beam_coords(double H_src, double Dec_src, double H_beam, double Dec_beam,
  double *X, double *Y, double *phi) {
#define MIN_THETA (1.e-7)

  double dH;
  double alpha, beta1, beta2, theta, junk, r;

  /* Hour angle difference from beam to source */
  dH = H_src - H_beam;

  /* Get separation and rotation angles */
  get_dist_pa(Dec_src, Dec_beam, dH, &theta, &alpha); /* src --> beam */
  get_dist_pa(Dec_beam, Dec_src, dH, &theta, &beta1);  /* beam --> src */
  get_dist_pa(Dec_beam, TEL_LAT, -H_beam, &junk, &beta2); /* beam --> zenith */

  /* EAA coordinates of source in beam frame */
  r = 2.*sin(0.5*theta);
  *X = r*cos(beta1-beta2);
  *Y = r*sin(beta1-beta2);

  /* Position angle */
  *phi = theta<MIN_THETA? beta2: M_PI - alpha - beta1 + beta2;
  if (*phi>2*M_PI) *phi-=2*M_PI;
  if (*phi<0) *phi+=2*M_PI;
#undef MIN_THETA
}

/* Operation to get the GST given TJD */
double tjd2gst(double TJD) {
  double gst, mobl, tobl, eq, dpsi, deps;

  earthtilt(TJD, &mobl, &tobl, &eq, &dpsi, &deps);
  sidereal_time(2440000.0, TJD+0.5, eq, &gst);
  gst *= M_PI/12.0; /* hours --> radians */
  return(gst);
}

/* Prints bad closure results to fp.  np = # antennas,
 * closure = closure errors (length 2*np*np), v=visibilities.
 * Only prints baselines used in calibration.  The amplitude and
 * phase minimum errors are unitless and in radians respectively.
 */
void print_bad_baselines(FILE *fp, long np, double *closure, VIS *v,
  double amp_err_min, double phase_err_min) {

  double amp_err, phase_err;
  long i,j;

  for(i=0; i<np; i++)
    for(j=i+1; j<np; j++)
      if (v->usecal[v->ind[i]+j]) {
        amp_err   = closure[2*(np*j+i)]-1;
        phase_err = closure[2*(np*j+i)+1];
        if (fabs(amp_err)>amp_err_min || fabs(phase_err)>phase_err_min)
          fprintf(fp, "BL %4ld %4ld: amp_err=%9.6lf phase_err=%9.6lf\n",
            i, j, amp_err, phase_err);
      }
}

/* Gets elevation and azimuth for given hour angle and declination.
 * Works by getting the coordinates of the point in the xyz frame
 * where z->NCP, y->due E, x-> intersection of meridian and equator.
 */
void get_elaz(double H, double Dec, double *el, double *az) {

  static int is_init = 0;
  static double sinlat, coslat;
  double x, y, z;

  if (!is_init) {
    sinlat = sin(TEL_LAT);
    coslat = cos(TEL_LAT);
    is_init = 1;
  }

  x = -sinlat*cos(Dec)*cos(H) + coslat*sin(Dec);
  y = cos(Dec)*sin(H);
  z = coslat*cos(Dec)*cos(H) + sinlat*sin(Dec);

  *el = atan2(z, sqrt(x*x+y*y));
  *az = atan2(-y,-x) + M_PI;
}
