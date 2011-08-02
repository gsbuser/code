#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#define NBIN 2
#define MASK 0x20 // 0x80=nyquist, 0x40=real zero freq, 0x20=imagin
#define NT (32<<20)
struct timeval4
  {
    int tv_sec;
    int tv_usec;
  };
struct CorrHeader {
        int lcount;
        struct timeval4 tv;
        int nfreq;
        int nfold;
        int totalsize;
} ;
int main(int argc, char *argv[])
{
	int fd,bit,i,cum=0,j;
	FILE *fp;
	char ibuf[NT/8];
	double time0;
        struct timeval tv; //struct timezone tz;
        struct timeval4 tv4;
	FILE *fdts;
	char ts[80];
        int  icount;
        struct CorrHeader head;



	if (argc != 3) {
		printf("usage: %s bitstreamfile timestampfile\n",argv[0]);
		exit(0);
	}
	fd=open(argv[1],O_RDONLY);
	if (fd<0) perror("open");
	fdts=fopen(argv[2],"w");
	if (fdts==NULL) perror("fopen");
	for (i=0;1;i++){
		//if (read(fd,ibuf,NT/8)!=NT/8) exit(0);
                if (read(fd,&head,sizeof(head)) != sizeof(head)) {
			perror("readbuf:read");
			exit(-1);
		}
		if (lseek(fd,head.totalsize-sizeof(head),SEEK_CUR)<0) exit(0);
                tv4=head.tv;
                tv.tv_sec=tv4.tv_sec;tv.tv_usec=tv4.tv_usec;
                printf("rank %d usec=%d %s",0,tv.tv_usec,asctime(gmtime(&tv.tv_sec)));
                                if (i==0) {
                                        sprintf(ts,"%sUTC %f\n",asctime(gmtime(&tv.tv_sec)),tv.tv_usec/1000000.);
                                        ts[24]=' ';
                                        for (j=0;j<4;j++) ts[j+24]=ts[j+20];
                                        ts[20]='U';
                                        ts[21]='T';
                                        ts[22]='C';
                                        ts[23]=' ';
                                        fprintf(fdts,"%s",ts);
                                        time0=tv.tv_sec+tv.tv_usec/1000000.;
                                }
                                fprintf(fdts,"%f\n",tv.tv_sec+tv.tv_usec/1000000.-time0);
                                fflush(fdts);
	}
}


