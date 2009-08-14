#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <dbox/saa7126_core.h>

#define SAA7126_DEVICE "/dev/dbox/saa0"

#define VERSION "0.2pre"
void help(char *prog_name) {
  printf("Version %s\n",VERSION);
  printf("Usage: %s <options>\n\n",prog_name);
  printf("Switches:\n"
     "-h, --help            help\n"
     "-o, --power-save <X>  power save mode\n"
     "                      none get power save state\n"
     "                      0    power save off\n"
     "                      1    power save on\n"
     "-r, --rgb             rgb mode\n"
     "-f, --fbas            fbas mode\n"
     "-s, --svideo          svideo mode\n"
     "-p, --pal             pal mode\n"
     "-n, --ntsc            ntsc mode\n"
     "-i, --input <X>       input control\n"
     "                      MP1      = 1\n"
     "                      MP2      = 2\n"
     "                      CSYNC    = 4\n"
     "                      DEMOFF   = 8\n"
     "                      SYMP     = 16\n"
     "                      COLORBAR = 128\n"
	 " -w, --wss <x>        widescreen signaling\n"
	 "                      0    4:3 full format\n"
	 "                      1    14:9 center letterbox\n"
	 "                      2    14:9 top letterbox\n"
	 "                      3    16:9 center letterbox\n"
	 "                      4    16:9 top letterbox\n"
	 "                      5    >16:9 center letterbox\n"
	 "                      6    4:3 with 14:9 center letterbox\n"
	 "                      7    16:9 full format (anamorphic)\n");
  exit(0);
}

/** */
int vps()
{
	char data[6];

	memset(data,0,6);

	/* vps on */
	data[0] = 1;

	/* 0: ??? 1: mono 2: stereo 3: dual sound */
	data[1] |= 2;
	return 0;
}


/** */
int read_powersave()
{
	int arg=0;
    int fd;

	if((fd = open(SAA7126_DEVICE,O_RDWR|O_NONBLOCK)) < 0){
		perror("SAA DEVICE: ");
		return -1;
	}

	if ( (ioctl(fd,SAAIOGPOWERSAVE,&arg) < 0)){
		perror("IOCTL: ");
		close(fd);
		return -1;
	}

	printf("SAA7126 POWER STATE: ");

	if(arg)
		printf("ON\n");
	else
		printf("OFF\n");

	close(fd);

	return 0;
}

/** */
int main(int argc, char **argv)
{
	int fd;
	int count=1;
    int mode;
	int arg;

	if (argc < 2)
	{
		help(argv[0]);
	}

    if ((strcmp("-h",argv[count]) == 0) || (strcmp("--help",argv[count]) == 0)) {
      help(argv[0]);
	  return 0;
    }
    else if ((strcmp("-r",argv[count]) == 0) || (strcmp("--rgb",argv[count]) == 0)) {
	  arg = SAA_MODE_RGB;
	  mode = SAAIOSMODE;
    }
    else if ((strcmp("-f",argv[count]) == 0) || (strcmp("--fbas",argv[count]) == 0)) {
	  arg = SAA_MODE_FBAS;
	  mode = SAAIOSMODE;
    }
    else if ((strcmp("-s",argv[count]) == 0) || (strcmp("--svideo",argv[count]) == 0)) {
	  arg = SAA_MODE_SVIDEO;
	  mode = SAAIOSMODE;
    }
    else if ((strcmp("-p",argv[count]) == 0) || (strcmp("--pal",argv[count]) == 0)) {
	  arg = SAA_PAL;
	  mode = SAAIOSENC;
    }
    else if ((strcmp("-n",argv[count]) == 0) || (strcmp("--ntsc",argv[count]) == 0)) {
	  arg = SAA_NTSC;
	  mode = SAAIOSENC;
    }
    else if ((strcmp("-i",argv[count]) == 0) || (strcmp("--input",argv[count]) == 0)) {
      if(argc<3)
	  {
		help(argv[0]);
		return 0;
	  }

	  arg = atoi(argv[count+1]);
	  mode = SAAIOSINP;
    }
    else if ((strcmp("-o",argv[count]) == 0) || (strcmp("--power-save",argv[count]) == 0)) {
      if(argc<3)
	  {
		read_powersave();
		return 0;
	  }
      else
	  {
	  	arg = atoi(argv[count+1]);
	  	mode = SAAIOSPOWERSAVE;
	  }
    }
	else if ((strcmp("-w",argv[count]) == 0) || (strcmp("--wss",argv[count]) == 0) ) {
		arg = atoi(argv[count+1]);
		mode = SAAIOSWSS;
	}
	else {
      help(argv[0]);
	  return 0;
	}

	if((fd = open(SAA7126_DEVICE,O_RDWR|O_NONBLOCK)) < 0){
		perror("SAA DEVICE: ");
		return -1;
	}

	if ( (ioctl(fd,mode,&arg) < 0)){
		perror("IOCTL: ");
		return -1;
	}

	close(fd);
	return 0;
}
