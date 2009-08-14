#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <dbox/fp.h>

int main(int argc, char **argv) {
int fd;
char buf[2];
 
 fd=open("/dev/dbox/rc0",O_RDONLY);
 if (fd < 0) {
  fprintf(stderr,"couldn't open /dev/dbox/rc0 for reading\n");
  exit(1);
 }
 if ((argc > 1) && (strcmp(argv[1],"-b") == 0))
  ioctl(fd,RC_IOCTL_BCODES,1);
 while(1) {
  read(fd,buf,2);
  printf("RC: %02X %02X\n",buf[0],buf[1]);
 }
 return 0;
}
