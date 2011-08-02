#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#include "constants.h"
#include "structures.h"
#include "routines.h"

/* Reads in the TJD and GST.
 *   
 * FileName = which file to read
 * nepochs = number of TJD/GST pairs in the file
 * TJD = write TJDs here
 * GST = write GSTs here
 */  
void get_tjd_gst(char FileName[], long nepochs, double *TJD, double *GST) 
{
  FILE *fp;
  long i;
  fp = fopen(FileName, "r");
  for(i=0;i<nepochs;i++) {
    fscanf(fp, "%lg %lg", TJD+i, GST+i);
//    TJD[i]-=0.16/6.3;
//    GST[i]-=0.16;
  }   
  fclose(fp);
}

/* Reads an antenna coordinate file.  Each line is one antenna, the
 * order of variables is x,y,z and then the timelag.
 */
void get_antenna_coords(char *FileName, ANTCOORDS *coords, long npods) {
  long i;
  FILE *fp;

  fp = fopen(FileName, "r");
  for(i=0;i<npods;i++) {
    fscanf(fp, "%lg %lg %lg %lg", &(coords[i].x), &(coords[i].y),
      &(coords[i].z), &(coords[i].t));
  }
  fclose(fp);
}

/* Initializes a visibility structure.  flag is currently a place-holder
 * for new functionality.  FileName is the file to be read.
 */
void new_visibility(VIS *v, unsigned long flag, char *FileName) {

  char *current_file;
  char temp[MAX_FILENAME_LEN];
  FILE *fp;
  long np, nf, nt;
  long i;
  int status, len;

  /* FileName format has np (pods), nf (frequencies), nt (times), nfile (files)
   * on the first line.  After this it has alternating file names and starting
   * t-indices (starting with 0).
   */
  fp = fopen(FileName, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error: new_visibility: failed to open %s.\n", FileName);
    exit(1);
  }
  if (fgets(temp, MAX_FILENAME_LEN-1, fp)==NULL) {
    fprintf(stderr, "Error: new_visibility: unexpected EOF %s\n", FileName);
    exit(1);
  }
  status = sscanf(temp, "%ld %ld %ld %ld", &np, &nf, &nt, &(v->nfile));
  if (status != 4 || np<=0 || nf<=0 || nt<=0 || v->nfile<=0) {
    fprintf(stderr, "Error: new_visibility: illegal np,nf,nt,nfile.\n");
    exit(1);
  }

  /* Following lines give the list of files and the t-indices at which they start. */
  v->DataFiles = (char*)malloc((size_t)(v->nfile*MAX_FILENAME_LEN*sizeof(char)));
  v->t_filestart = (long*)malloc((size_t)(v->nfile*sizeof(long)));
  for(i=0; i<v->nfile; i++) {
    current_file = v->DataFiles+MAX_FILENAME_LEN*i;
    if (fgets(current_file, MAX_FILENAME_LEN-1, fp)==NULL) {
      fprintf(stderr, "Error: new_visibility: unexpected EOF %s\n", FileName);
      exit(1);
    }
    len = strlen(current_file);
    strcpy(current_file + len - 1, "\0"); /* Removes trailing \n */
    if (fgets(temp, MAX_FILENAME_LEN-1, fp)==NULL) {
      fprintf(stderr, "Error: new_visibility: unexpected EOF %s\n", FileName);
      exit(1);
    }
    status = sscanf(temp, "%ld", v->t_filestart+i);
    if (status != 1 || v->t_filestart[i]<0 || v->t_filestart[i]>=nt) {
      fprintf(stderr, "Error: new_visibility: illegal t_filestart.\n");
      exit(1);
    }
  }
  fclose(fp);

  /* Set up numbers of everything */
  v->np = np;
  v->nf = nf;
  v->nt = nt;
  v->nb = (np*(np-1))/2; 
  v->nba= (np*(np+1))/2; 

  v->nbyte_v = 2 * SIZE_CORRFLOAT;
  v->nbyte_t = v->nbyte_v * v->nba * nf;  // 2 polarizations
  v->ndata = 2 * nf * v->nba;

  /* Allocate memory */
  v->ind = (long*)malloc((size_t)(np*sizeof(long))); 
  v->mask = (unsigned short int*)malloc((size_t)
    (v->nba*sizeof(unsigned short int)));
  v->offset = (double*)malloc((size_t)(v->ndata*sizeof(double)));
  v->usecal = (short int*)malloc((size_t)(v->nba*sizeof(short int)));

  /* Set calibration array to all 1's -- use */
  for(i=0; i<v->nba; i++) v->usecal[i] = 1;

  /* Compute the memory indices */
  for(i=0; i<np; i++) v->ind[i] = np*i - (i*(i+1))/2;
  //for(i=0; i<np; i++) v->ind[i] = np*i - (i*(i+1))/2 + np*i - (i*(i-1))/2;
  // accommodating two polarizations

  /* Buffering capability.  -1 indicates the buffer doesn't contain
   * anything.
   */
#ifdef BUFFER_ON
  v->t_buffer_min = -1;
  v->t_buffer_max = -1;
  /*  v->buffer = (void*)malloc((size_t)(NBUFFER_VIS*v->nbyte_t)); */
#endif
}

/* Destroys a visibility structure */
void kill_visibility(VIS *v) {
  free((char*)v->ind        );
  free((char*)v->mask       );
  free((char*)v->offset     );
  free((char*)v->DataFiles  );
  free((char*)v->t_filestart);
  free((char*)v->usecal     );
#ifdef BUFFER_ON
  free((char*)v->buffer     );
#endif
}

/* Reads the tsample record, baseline i-j from the visibility file.
 * If i<0, reads all baselines.  The vector data must have length
 * 2*nf for the case of reading an individual baseline, and ndata
 * for reading all baselines.
 *
 * flag bits:
 *        0x1: add visibility to the output instead of over-writing.
 *        0x2: subtract offset.  Must call set_offset first.
 */
void read_data_vector(VIS *v, double *data, long i, long j,
  long tsample, unsigned long flag) {

  DATA_CORRFLOAT *filedata;
  FILE *fp;
  double *offset_ptr;
  long ifile, t_index;
  long nbytes, firstbyte, nbytes_read;
  long ivar, nvar;
  off64_t firstbyte_buffer;
  long nbytes_buffer;
  int status;
  static long fd=-2;
  static char *filep;

  /* First find the correct file and timestamp */
  ifile = v->nfile-1;
  while ( v->t_filestart[ifile]>tsample ) ifile--;
  t_index = tsample - v->t_filestart[ifile];

  /* Number of bytes to read */
  nbytes = v->nbyte_v * v->nf;
  if (i<0) nbytes *= v->nba;

  /* Index of first byte */
  firstbyte = v->nbyte_t * t_index;
  if (i>=0) {
    if (j<i || i>=v->np || j>=v->np) {
      fprintf(stderr, "Error: read_data_vector: illegal baseline i=%ld j=%ld\n", i, j);
      exit(1);
    }
    firstbyte += v->nbyte_v * v->nf * (v->ind[i]+j);
  }

#if 0
  /* Verbose output */
  printf("timestamp %ld, baseline %ld,%ld\n", tsample, i, j);
  printf("file %s, start @ %ld, read %ld bytes\n",
    v->DataFiles+MAX_FILENAME_LEN*ifile, firstbyte, nbytes);
#endif

  /* If we're buffering, and we want to read something that's outside the
   * currently buffered range, replace the buffer
   */
#ifdef BUFFER_ON
  if (tsample<v->t_buffer_min || tsample>v->t_buffer_max) {
    firstbyte_buffer = v->nbyte_t *  t_index;
    v->t_buffer_min = tsample;

    /* Figure out maximum time sample that's in this file */
    v->t_buffer_max = v->t_buffer_min + NBUFFER_VIS - 1;
    if (ifile==v->nfile-1) {
      if (v->t_buffer_max>=v->nt)
        v->t_buffer_max = v->nt-1;
    }
    else {
      if (v->t_buffer_max>=v->t_filestart[ifile+1])
        v->t_buffer_max = v->t_filestart[ifile+1]-1;
    }
    nbytes_buffer = v->nbyte_t * (v->t_buffer_max-v->t_buffer_min+1);

    /* Read the data */
    /*    fp = fopen(v->DataFiles+MAX_FILENAME_LEN*ifile, "rb");

    if (fp==NULL) {
      fprintf(stderr, "Error: read_data_vector: can't open %s\n",
        v->DataFiles+MAX_FILENAME_LEN*ifile);
      perror("fopen");
      exit(1);
    }
    */
    if (fd ==-2) {
      int start=0;
      size_t filelength;
      char cmd[80]="wc -c ";
      fd=open(v->DataFiles+MAX_FILENAME_LEN*ifile,O_RDONLY);
      if (fd<0) {
	perror("fopen");
	fprintf(stderr,"name=%s fd=%ld\n",v->DataFiles+MAX_FILENAME_LEN*ifile,fd);
	exit(1);
      }
      fp= popen(strcat(cmd,v->DataFiles+MAX_FILENAME_LEN*ifile),"r");
      if (fp==NULL) {
	fprintf(stderr, "Error: can't wc -c %s\n",
		v->DataFiles+MAX_FILENAME_LEN*ifile);
	perror("fopen");
	exit(1);
    }
      if (1!=fscanf(fp,"%ld",&filelength)) {
	perror("fscanf");
	exit(1);
      }
      if (pclose(fp)<0) {perror("pclose");exit(1);}
      filelength=1800000000;
      fprintf(stderr,"mmap: filelength=%ld",filelength);
      filep=mmap(0,filelength, PROT_READ, MAP_SHARED, fd, 0);
      if (filep == MAP_FAILED) {
	perror("mmap");
	exit(1);
      }
    }

    v->buffer=filep+firstbyte_buffer;
    /*   memcpy(v->buffer,filep+firstbyte_buffer,nbytes_buffer); */
    /*
    status = fseek(fp, firstbyte_buffer, SEEK_SET);
    if (status) {
      fprintf(stderr, "Error: read_data_vector: can't set %s to %ld bytes\n",
        v->DataFiles+MAX_FILENAME_LEN*ifile, firstbyte_buffer);
      perror("fseek");
      exit(1);
    }

    nbytes_read = (long)fread(v->buffer, (size_t)1, (size_t)nbytes_buffer, fp);
    if (nbytes_read!=nbytes_buffer) {
      fprintf(stderr, "Error: read_data_vector: only read %ld/%ld bytes\n",
        nbytes_read, nbytes_buffer);
      printf("fread: bi=%ld, bj=%ld\n",i,j);
      perror("fread");
      exit(1);
    }
    fclose(fp);
    */
    
    
#if 0
    fprintf(stderr, "Set buffer t=[%ld,%ld], byte=%ld, nbyte=%ld <%s>\n",
      v->t_buffer_min, v->t_buffer_max, firstbyte_buffer, nbytes_buffer,
      v->DataFiles+MAX_FILENAME_LEN*ifile);
#endif
  }

  /* Set filedata to the correct part of the stream */
  filedata = (DATA_CORRFLOAT*)v->buffer;
  filedata += (firstbyte - v->nbyte_t*(v->t_buffer_min-v->t_filestart[ifile]))
    /sizeof(DATA_CORRFLOAT);
#endif

  /* Read the data directly if we're not buffering */
#ifndef BUFFER_ON
  filedata = (DATA_CORRFLOAT*)malloc(nbytes);
  fp = fopen(v->DataFiles+MAX_FILENAME_LEN*ifile, "rb");
  if (fp==NULL) {
    fprintf(stderr, "Error: read_data_vector: can't open %s\n",
      v->DataFiles+MAX_FILENAME_LEN*ifile);
    exit(1);
  }
  status = fseek(fp, firstbyte, SEEK_SET);
  if (status) {
    fprintf(stderr, "Error: read_data_vector: can't set %s to %ld bytes\n",
      v->DataFiles+MAX_FILENAME_LEN*ifile, firstbyte);
    exit(1);
  }
  nbytes_read = (long)fread((void*)filedata, (size_t)1, (size_t)nbytes, fp);
  if (nbytes_read!=nbytes) {
    fprintf(stderr, "Error: read_data_vector: only read%ld/%ld bytes\n",
      nbytes_read, nbytes);
    exit(1);
  }
  fclose(fp);
#endif

  /* Copy the data to data, clear memory.  This step is necessary if the
   * file data are at lower precision.
   */
  nvar = 2 * v->nf;
  if (i<0) nvar *= v->nba;
  if (flag & 0x1) {
    for(ivar=0; ivar<nvar; ivar++)
      data[ivar]+=(double)filedata[ivar];
  } else {
    for(ivar=0; ivar<nvar; ivar++)
      data[ivar] =(double)filedata[ivar];
  }

  /* If the offset option is turned on, figure out which part of the offset
   * vector to read.  Note that it has length ndata.
   */
  if (flag & 0x2) {
    offset_ptr = v->offset + 2 * v->nf * (v->ind[i]+j);
    for(ivar=0; ivar<nvar; ivar++)
      data[ivar]-=offset_ptr[ivar];
  }

#ifndef BUFFER_ON
  free((char*)filedata);
#endif
}

/* Computes the offsets, i.e. time-average of visibilities, and stores
 * them in the 'offset' vector of v.
 */
void set_offset(VIS *v) {

  long nvar = 2 * v->nf * v->nba;  
  long tsample, ivar;

  for(ivar=0; ivar<nvar; ivar++) v->offset[ivar] = 0.;
  for(tsample=0; tsample<v->nt; tsample++)
    read_data_vector(v, v->offset, -1, -1, tsample, 0x1);
  for(ivar=0; ivar<nvar; ivar++) v->offset[ivar] /= v->nt;
  //printf("offset = %e %e %e\n", v->offset[0], v->offset[1], v->offset[2]);
}

/* Sets offsets to fixed value (otherwise same as set_offset, but
 * usually faster).
 */
void set_fixed_offset(VIS *v) {

  long nvar = 2 * v->nf * v->nba;  
  long ivar;

  for(ivar=0; ivar<nvar; ivar++) {
    v->offset[ivar] = ivar%2? VISIBILITY_OFFSET: 0.;
  }
}

/* Reads a radio source file.  Right now the source file format is
 * ra, dec, I.  The sources are stored in an array
 * RADSRC[Nsrc].  The top Nsrc sources are read.
 *
 * Source RA/Dec are precessed from REF_EPOCH to OBS_EPOCH (as defined
 * in the headers).
 */
void read_src_file(char *FileName, RADSRC *src, long Nsrc) {

  FILE *fp;
  long i;
  double ra,dec,stokesI;
  double x_ref[3], x_obs[3];
  double N_obs[3], E_obs[3], N_ref[3], E_ref[3];
  double a,b,pa, precess_angle;
  double fwhm_to_sigma = 0.4246609;

  fp = fopen(FileName, "r");
  for(i=0;i<Nsrc;i++) {
    fscanf(fp, "%lg %lg %lg %lg %lg %lg\n", &ra, &dec, &stokesI, &a, &b, &pa);

    /* Calculate new source position, corrected for precession */
    x_ref[0] = cos(dec*DEGREE)*cos(ra*DEGREE);
    x_ref[1] = cos(dec*DEGREE)*sin(ra*DEGREE);
    x_ref[2] = sin(dec*DEGREE);
    precession(REF_EPOCH, x_ref, OBS_EPOCH, x_obs);
    src[i].ra = atan2(x_obs[1], x_obs[0]);
    if (src[i].ra<0) src[i].ra+=2*M_PI;
    src[i].dec = atan2(x_obs[2],sqrt(x_obs[0]*x_obs[0]+x_obs[1]*x_obs[1]));

    /* Get effect of precession on PA.  Here precess_angle is the
     * PA of the *reference* NCP in the *observed* equinox.  Should
     * be close to zero.
     */
    N_obs[0] = -sin(src[i].dec)*cos(src[i].ra);
    N_obs[1] = -sin(src[i].dec)*sin(src[i].ra);
    N_obs[2] =  cos(src[i].dec);
    E_obs[0] = -sin(src[i].ra);
    E_obs[1] =  cos(src[i].ra);
    E_obs[2] =  0.;
    precession(OBS_EPOCH, N_obs, REF_EPOCH, N_ref);
    precession(OBS_EPOCH, E_obs, REF_EPOCH, E_ref);
    precess_angle = atan2(E_ref[2], N_ref[2]);

    /* Intensities */
    src[i].I = stokesI;

    /* Major/minor axes */
    a *= ARCSEC*fwhm_to_sigma;
    b *= ARCSEC*fwhm_to_sigma;
    src[i].a2 = a*a;
    src[i].b2 = b*b;
    src[i].pa = pa*DEGREE - precess_angle;
  }
  fclose(fp);
}
