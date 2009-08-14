
#ifndef __RCINPUT_H__
#define __RCINPUT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "libsig_comp.h"

#define RC_0		0x00
#define RC_1		0x01
#define RC_2		0x02
#define RC_3		0x03
#define RC_4		0x04
#define RC_5		0x05
#define RC_6		0x06
#define RC_7		0x07
#define RC_8		0x08
#define RC_9		0x09
#define RC_VOLUMEUP	0x0a
#define RC_VOLUMEDOWN	0x0b
#define RC_TV		0x0c
#define RC_BOUGUETUP	0x0d
#define RC_BOUGUETDOWN	0x0e
#define RC_POWER	0x0f
#define RC_MENU		0x20
#define RC_UP		0x21
#define RC_DOWN		0x22
#define RC_RIGHT	0x23
#define RC_LEFT		0x24
#define RC_OK		0x25
#define RC_AUDIO	0x26
#define RC_VIDEO	0x27
#define RC_INFO		0x28
#define RC_RED		0x40
#define RC_GREEN	0x41
#define RC_YELLOW	0x42
#define RC_BLUE		0x43
#define RC_MUTE		0x44
#define RC_TEXT		0x45
#define RC_FORWARD	0x50
#define RC_BACK		0x51
#define RC_EXIT		0x52
#define RC_TEXT_1	0x53
#define RC_HELP		0x54
#define RC_BREAK	0xff

//#define RC_DEVICE "/dev/dbox/rc0" "/dev/input/event0"
#define RC_DEVICE "/dev/rawir2"
#define BUTTON_DEVICE "/dev/dbox/fpkeys0"

#define BUTTON_UP	0x5
#define BUTTON_DOWN	0x3
#define BUTTON_EXIT	0x6
#define BUTTON_BREAK	0x7

class RcInput
{
	//rc
	int fd_rc;
	pthread_t thrRc;
	static void* ThreadRc(void*);
	//bu
	int fd_bu;
	pthread_t thrBu;
	static void* ThreadBu(void*);
public:
	static RcInput *getInstance();
	Signal1<void,unsigned short> selected;
	RcInput();
	~RcInput();
};

#endif /* __RCINPUT_H__ */
