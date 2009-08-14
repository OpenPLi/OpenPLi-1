#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/video.h>

#define BSIZE           1024*16

int main(int argc, char **argv)
{
  int fd, fdo=1, i, b, x;
  struct dmxPesFilterParams flt; 
  char buffer[BSIZE];

  fd=open("/dev/ost/dvr0", O_RDONLY);
//  fdo=open(argv[1], O_CREAT|O_WRONLY|O_TRUNC);
  
  ioctl(fd, DMX_SET_BUFFER_SIZE, 1024*1024);
  
  for (i=1; i<argc; i++)
  {
    int pid, fd;
    fd=open("/dev/ost/demux0", O_RDWR);
    if (fd<0)
    {
      perror("/dev/ost/demux0");
      return -fd;
    }
    sscanf(argv[i], "%x", &pid);
    flt.pid=pid;
    flt.input=DMX_IN_FRONTEND;
    flt.output=DMX_OUT_TS_TAP;
    flt.pesType=DMX_PES_OTHER;
    flt.flags=0;
    if (ioctl(fd, DMX_SET_PES_FILTER, &flt)<0)
    {
      perror("DMX_SET_PES_FILTER");
      break;
    }
    if (ioctl(fd, DMX_START, 0)<0)
    {
      perror("DMX_SET_PES_FILTER");
      break;
    }
  }
  
  b=1024*1024;
  x=0;
  while (/* b>0*/ 1)
  {
    int r=b;
    if (r>BSIZE)
      r=BSIZE;
    r=BSIZE;
    if ((r=read(fd, buffer, r))<=0)
    {
      perror("read");
      break;
    }
    if (write(fdo, buffer, r)!=r)
    {
      perror(argv[1]);
      break;
    }
    x+=r;
    b-=r;
  }
  
  close(fd);
  if (fdo!=1)
    close(fdo);
  return 0;
}
