#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "rc.h"

int fd=-1;

static int translate(int code)
{
	if ((code&0xFF00)==0x5C00)
	{
		switch (code&0xFF)
		{
		case 0x0C: return RC_STANDBY;
		case 0x20: return RC_HOME;
		case 0x27: return RC_DBOX;
		case 0x00: return RC_0;
		case 0x01: return RC_1;
		case 0x02: return RC_2;
		case 0x03: return RC_3;
		case 0x04: return RC_4;
		case 0x05: return RC_5;
		case 0x06: return RC_6;
		case 0x07: return RC_7;
		case 0x08: return RC_8;
		case 0x09: return RC_9;
		case 0x3B: return RC_BLUE;
		case 0x52: return RC_YELLOW;
		case 0x55: return RC_GREEN;
		case 0x2D: return RC_RED;
		case 0x54: return RC_UP_LEFT;
		case 0x53: return RC_DOWN_LEFT;
		case 0x0E: return RC_UP;
 		case 0x0F: return RC_DOWN;
		case 0x2F: return RC_LEFT;
 		case 0x2E: return RC_RIGHT;
		case 0x30: return RC_OK;
 		case 0x16: return RC_PLUS;
 		case 0x17: return RC_MINUS;
 		case 0x28: return RC_MUTE;
 		case 0x82: return RC_HELP;
		default:
			printf("unknown old rc code %x\n", code);
			return -1;
		}
	} else if (!(code&0x00))
		return code&0x3F;
	else
		printf("unknown rc code %x\n", code);
	return -1;
}


int RCOpen()
{
  fd=open("/dev/dbox/rc0", O_RDONLY);
  if (fd<0)
  {
    perror("/dev/dbox/rc0");
    return fd;
  }
  return 0;
}

int RCGet()
{
  unsigned short rc;
  if (read(fd, &rc, 2)!=2)
    return -1;
  return translate(rc);
}

int RCPoll()
{
  return -1;
}

int RCClose()
{
  if (fd>=0)
    close(fd);
  return 0;
}
