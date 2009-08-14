#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dbox/fp.h>

int main(int argc, char **argv)
{
  int fd=open("/dev/dbox/fp0", O_RDWR);
  if (fd<0)
  {
    perror("/dev/dbox/fp0");
    return 1;
  }
  
  if (argc==1)
  {
  	int val;
  	if (ioctl(fd, FP_IOCTL_GET_WAKEUP_TIMER, &val)<0)
  	{
  		perror("FP_IOCTL_GET_WAKEUP_TIMER");
  		return -1;
  	}
  	printf("wakeup-timer: %d minutes.\n", val);
  } else if (argc==2)
  {
  	int val=atoi(argv[1]);
  	if (ioctl(fd, FP_IOCTL_SET_WAKEUP_TIMER, &val)<0)
  	{
  		perror("FP_IOCTL_SET_WAKEUP_TIMER");
  		return -1;
  	}
  } else
  {
  	printf("usage: wakeup [<minutes>]\n");
  }
  
  close(fd);
  return 0;
}
