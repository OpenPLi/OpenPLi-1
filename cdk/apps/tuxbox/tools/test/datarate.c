/*
	ein kleines tool um euch zum verzweifeln zu bringen:
	
	"insmod shit.o"
	"../streamhack/ecmgen > /dev/sh/ecmdata &"
	"pzap taquilla xx &"
	"./datarate"

	(bei bedarf tuts auch jeder andere sender, aber das entspannt dann nicht so)	
	
	man beachte den unterschied zwischen laufenden avia-queue und nicht-laufender.
	
	auf gtx evtl. alles problemlos?
	AUF PHILIPS/ENX JEDENFALLS NICHT!!
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <memory.h>
#include <linux/fb.h>
#include <sys/times.h>
#include <time.h>
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 100

char buffer[1024*1024];
unsigned char *lfb;
int size;

void Check(char *text, int what);
void Do(int what);

int main()
{
	int fd;
	struct fb_fix_screeninfo fix;
	int available;
	
	fd=open("/dev/fb/0", O_RDWR);
	if (fd<0)
	{
		perror("/dev/fb/0");
		return -fd;
	}
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("FBIOGET_FSCREENINFO");
    return 0;
	}

	size=available=fix.smem_len;
	printf("%dk video mem\n", available/1024);

	lfb=(unsigned char*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
	if (!lfb)
	{
		perror("mmap");
		return 0;
	}
	
	Check("write", 0);
	Check("read", 1);
	Check("compare", 2);
	Check("sdram write", 3);
	
  munmap(lfb, available);
  close(fd);
}

void Check(char *text, int what)
{
	int i, delta;
	
	struct tms begin, end;
	
	long bm=0;
	
	times(&begin);
	
	for (i=0; i<4; i++)
	{
		Do(what);
		bm+=size;
	}
	
	times(&end);
	
	delta=end.tms_utime-begin.tms_utime;
	bm/=delta;
	bm*=CLOCKS_PER_SEC;
	printf("%s: %d kb/s\n", text, bm/1024);
}

void Do(int what)
{
	int i;
	switch (what)
	{
	case 0:
		memset(lfb, 0x55, size);
		break;
	case 1:
		memcpy(buffer, lfb, size);
		break;
	case 2:
#if 0
		if (memcmp(buffer, lfb, size))
		{
			printf("COMPARE FAILED!\n");
		}
#else
		for (i=0; i<size; i++)
			if (buffer[i]!=lfb[i])
			{
				printf("miscompare at %x\n",  i);
			}
#endif
		break;
	case 3:
		memset(buffer, 0x55, size);
		break;
	}
}
