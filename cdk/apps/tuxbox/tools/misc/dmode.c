#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/video.h>

#define VERSION "0.1"
void help(char *prog_name) {
  printf("Version %s\n",VERSION);
  printf("Usage: %s <switches> <number/name/keyword>\n\n",prog_name);
  printf("Switches:\n"
      "-h, --help             help\n"
      "-p, --pan-scan         pan scan mode\n"
      "-l, --letter-box       letter box mode\n"
      "-c, --center-cut-out   center cut out mode\n"
      "-1                     4:3 mode\n"
      "-2                     16:9 mode\n"
      "-3                     20:9 mode\n");
  exit(0);
}

int main(int argc, char **argv)
{
	int fd;
	int count=1;
	videoDisplayFormat_t vdf = VIDEO_PAN_SCAN;
	videoFormat_t vf = VIDEO_FORMAT_4_3;
	int mode;

	if (argc < 2)
	{
		help(argv[0]);
	}

    if ((strcmp("-h",argv[count]) == 0) || (strcmp("--help",argv[count]) == 0)) {
      help(argv[0]);
	  return 0;
    }
    else if ((strcmp("-p",argv[count]) == 0) || (strcmp("--pan-scan",argv[count]) == 0)) {
	  vdf = VIDEO_PAN_SCAN;
	  mode = 1;
    }
    else if ((strcmp("-l",argv[count]) == 0) || (strcmp("--letter-box",argv[count]) == 0)) {
	  vdf = VIDEO_LETTER_BOX;
	  mode = 1;
    }
    else if ((strcmp("-c",argv[count]) == 0) || (strcmp("--center-cut-out",argv[count]) == 0)) {
	  vdf = VIDEO_CENTER_CUT_OUT;
	  mode = 1;
    }
    else if ( strcmp("-1",argv[count]) == 0 ) {
	  vf = VIDEO_FORMAT_4_3;
	  mode = 2;
    }
    else if ( strcmp("-2",argv[count]) == 0 ) {
	  vf = VIDEO_FORMAT_16_9;
	  mode = 2;
    }
    else if ( strcmp("-3",argv[count]) == 0 ) {
	  vf = VIDEO_FORMAT_20_9;
	  mode = 2;
    }
	else {
      help(argv[0]);
	  return 0;
	}

	if((fd = open("/dev/dvb/card0/video0",O_RDWR|O_NONBLOCK)) < 0){
		perror("VIDEO DEVICE: ");
		return -1;
	}

	if (mode==1) {

	if ( (ioctl(fd,VIDEO_SET_DISPLAY_FORMAT,vdf) < 0)){
		perror("VIDEO_SET_DIPLAY_FORMAT: ");
		return -1;
	}

	} else if (mode==2){

	if ( (ioctl(fd,VIDEO_SET_FORMAT,vf) < 0)){
		perror("VIDEO_SET_DIPLAY_FORMAT: ");
		return -1;
	}

	}

	close(fd);
	return 0;
}
