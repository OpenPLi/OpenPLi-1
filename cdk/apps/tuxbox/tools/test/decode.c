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
  int video, audio, vpid=0x1FFF, apid=0x1FFF;
  struct dmxPesFilterParams flt; 

  sscanf(argv[1], "%x", &vpid);
  sscanf(argv[2], "%x", &apid);
  
  printf("showing you: VPID %04x APID %04X PCRPID %04x ;)\n", vpid, apid, vpid);

  video=open("/dev/ost/demux0", O_RDWR);
  if (video<0)
  {
    perror("/dev/ost/demux0");
    return -video;
  }
  flt.pid=vpid;
  flt.input=DMX_IN_FRONTEND;
  flt.output=DMX_OUT_DECODER;
  flt.pesType=DMX_PES_VIDEO;
  flt.flags=0;
  if (ioctl(video, DMX_SET_PES_FILTER, &flt)<0)
  {
    perror("DMX_SET_PES_FILTER");
    return 1;
  }
  if (ioctl(video, DMX_START, 0)<0)
  {
    perror("DMX_SET_PES_FILTER");
    return 1;
  }

  audio=open("/dev/ost/demux0", O_RDWR);
  if (video<0)
  {
    perror("/dev/ost/demux0");
    return -video;
  }
  flt.pid=apid;
  flt.input=DMX_IN_FRONTEND;
  flt.output=DMX_OUT_DECODER;
  flt.pesType=DMX_PES_AUDIO;
  flt.flags=0;
  if (ioctl(audio, DMX_SET_PES_FILTER, &flt)<0)
  {
    perror("DMX_SET_PES_FILTER");
    return 1;
  }
  if (ioctl(audio, DMX_START, 0)<0)
  {
    perror("DMX_SET_PES_FILTER");
    return 1;
  }
  
  getchar();
  
  close(video);
  close(audio);
  return 0;
}
