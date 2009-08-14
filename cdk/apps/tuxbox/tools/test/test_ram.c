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

#define BSIZE 188

int CheckBlock(char *buffer)
{
	int ok, i;
	if (buffer[0]!=0x47)
	{
		printf("\nno 0x47 at beginning - sync lost!\n");
		exit(0);
	}
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=0xFF)
			ok=0;
	if (ok)
		return 1;
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=0x00)
			ok=0;
	if (ok)
		return 2;
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=0xAA)
			ok=0;
	if (ok)
		return 3;
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=0x55)
			ok=0;
	if (ok)
		return 4;
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=(0xFF-i+4))
			ok=0;
	if (ok)
		return 5;
	ok=1;
	for (i=4; i<188 && ok; i++)
		if (buffer[i]!=(i-4))
			ok=0;
	if (ok)
		return 6;
	printf("\nBIT ERROR: packet dump:\n");
	for (i=0; i<188; i++)
		printf("%02x ", buffer[i]);
	printf("\n");
	return 0;
}

int main(int argc, char **argv)
{
  int fd, i, b, x;
  struct dmxPesFilterParams flt; 
  char buffer[BSIZE];

  fd=open("/dev/ost/dvr0", O_RDONLY);
  
  ioctl(fd, DMX_SET_BUFFER_SIZE, 1024*1024);
  
  {
    int pid, fd;
    fd=open("/dev/ost/demux0", O_RDWR);
    if (fd<0)
    {
      perror("/dev/ost/demux0");
      return -fd;
    }
    flt.pid=0x1FFF;			//  stuffing
    flt.input=DMX_IN_FRONTEND;
    flt.output=DMX_OUT_TS_TAP;
    flt.pesType=DMX_PES_OTHER;
    flt.flags=0;
    if (ioctl(fd, DMX_SET_PES_FILTER, &flt)<0)
    {
      perror("DMX_SET_PES_FILTER");
      return 1;
    }
    if (ioctl(fd, DMX_START, 0)<0)
    {
      perror("DMX_SET_PES_FILTER");
      return 1;
    }
  }
  
  while (1)
  {
    int r;
    if ((r=read(fd, buffer, BSIZE))<=0)
    {
      perror("read");
      break;
    }
    if (r!=188)
    	printf("out of sync!?\n");
    else
    {
    	printf("%d", CheckBlock(buffer));
    }
  }
  
  close(fd);
  return 0;
}
