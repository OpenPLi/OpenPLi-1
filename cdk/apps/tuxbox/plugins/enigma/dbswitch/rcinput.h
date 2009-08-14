
#ifndef __RCINPUT_H__
#define __RCINPUT_H__

#define RC_DEVICE "/dev/dbox/rc0"

#include <dbox/fp.h>
#include <stdio.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>

#include <utime.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>

using namespace std;

class CRCInput
{
	private:

		int             fd;
		struct timeval  tv_prev;
		__u16           prevrccode;

		int translate(int code);

	public:
		enum
		{
			RC_0=0x00,
			RC_1=0x01,
			RC_2=0x02,
			RC_3=0x03,
			RC_4=0x04,
			RC_5=0x05,
			RC_6=0x06,
			RC_7=0x07,
			RC_8=0x08,
			RC_9=0x09,
			RC_right=0x0A,
			RC_left=0x0B,
			RC_up=0x0C,
			RC_down=0x0D,
			RC_ok=0x0E,
			RC_spkr=0x0F,
			RC_standby=0x10,
			RC_green=0x11,
			RC_yellow=0x12,
			RC_red=0x13,
			RC_blue=0x14,
			RC_plus=0x15,
			RC_minus=0x16,
			RC_help=0x17,
			RC_setup=0x18,
			RC_home=0x1F,
			RC_page_down=0x53,
			RC_page_up=0x54,
			RC_top_left=27,
			RC_top_right=28,
			RC_bottom_left=29,
			RC_bottom_right=30,
			RC_timeout=-1,
			RC_nokey=-2
		};
		
		int repeat_block;

		CRCInput(); 
		~CRCInput();

		int getKey();
};

#endif /* __RCINPUT_H__ */
