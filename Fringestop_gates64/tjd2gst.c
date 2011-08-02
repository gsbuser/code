/* Computes the Greenwich Sidereal Time for a list of epochs.  The input has
 * the list of TJD's.  The output has the TJD's as well as GST.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "novas-c201/novas.h"

int main(void) {

  double TJD, gst;
  double mobl, tobl, eq, dpsi, deps;

  while (scanf("%lg", &TJD)!=EOF) {
    earthtilt(TJD, &mobl, &tobl, &eq, &dpsi, &deps);
    sidereal_time(2440000.0, TJD+0.5, eq, &gst);
    printf("%14.8lf %10.8lf\n", TJD, gst/12.0*M_PI);
  }

  return(0);
}
