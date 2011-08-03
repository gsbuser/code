#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <string.h>

int main (int argc, char **argv)
{
  struct mtop Arg;
  pid_t pid = getpid ();
  
  if (argc != 3){
    printf ("%d: USAGE: %s MTDEV CMD \n",pid,argv[0]);
    return 1;
  }

  int fd;
  while (1){
    if (strcmp (argv[2],"EOM") == 0){
      Arg.mt_op = MTEOM;
      Arg.mt_count=(int)NULL;
      break;
    }
  }

  fd = open (argv[1], O_RDONLY);
  ioctl (fd, MTIOCTOP, &Arg);
  close (fd);
  return 0;
}
