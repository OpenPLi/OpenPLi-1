#ifndef RC_INPUT_H
#define RC_INPUT_H

#include <config.h>
#ifdef HAVE_DBOX_HARDWARE
	#include "input_fake.h"
	#define RC_0			KEY_0
	#define RC_1			KEY_1
	#define RC_2			KEY_2
	#define RC_3			KEY_3
	#define RC_4			KEY_4
	#define RC_5			KEY_5
	#define RC_6			KEY_6
	#define RC_7			KEY_7
	#define RC_8			KEY_8
	#define RC_9			KEY_9
	#define RC_RIGHT		KEY_RIGHT
	#define RC_LEFT			KEY_LEFT
	#define RC_UP			KEY_UP
	#define RC_DOWN			KEY_DOWN
	#define RC_OK			KEY_OK
	#define RC_SPKR			KEY_MUTE
	#define RC_STANDBY		KEY_POWER
	#define RC_GREEN		KEY_GREEN
	#define RC_YELLOW		KEY_YELLOW
	#define RC_RED			KEY_RED
	#define RC_BLUE			KEY_BLUE
	#define RC_PLUS			KEY_VOLUMEUP
	#define RC_MINUS		KEY_VOLUMEDOWN
	#define RC_HELP			KEY_HELP
	#define RC_SETUP		KEY_SETUP
	#define RC_HOME			KEY_HOME
	#define RC_PAGE_DOWN		KEY_PAGEDOWN
	#define RC_PAGE_UP		KEY_PAGEUP

/* SAGEM remote controls have the following additional keys */

	#ifndef KEY_TOPLEFT
		#define KEY_TOPLEFT	0x1a2
	#endif

	#ifndef KEY_TOPRIGHT		
		#define KEY_TOPRIGHT	0x1a3
	#endif

	#ifndef KEY_BOTTOMLEFT
		#define KEY_BOTTOMLEFT	0x1a4
	#endif

	#ifndef KEY_BOTTOMRIGHT
		#define KEY_BOTTOMRIGHT	0x1a5
	#endif
#else
	#define RC_0			0x00
	#define RC_1			0x01
	#define RC_2			0x02
	#define RC_3			0x03
	#define RC_4			0x04
	#define RC_5			0x05
	#define RC_6			0x06
	#define RC_7			0x07
	#define RC_8			0x08
	#define RC_9			0x09
	#define RC_RIGHT		0x0A
	#define RC_LEFT			0x0B
	#define RC_UP			0x0C
	#define RC_DOWN			0x0D
	#define RC_OK			0x0E
	#define RC_SPKR			0x0F
	#define RC_STANDBY		0x10
	#define RC_GREEN		0x11
	#define RC_YELLOW		0x12
	#define RC_RED			0x13
	#define RC_BLUE			0x14
	#define RC_PLUS			0x15
	#define RC_MINUS		0x16
	#define RC_HELP			0x17
	#define RC_SETUP		0x18
	#define RC_HOME			0x1F
	#define RC_PAGE_DOWN		0x53
	#define RC_PAGE_UP		0x54
#endif // HAVE_DREAMBOX_HARDWARE

extern	void			RcGetActCode( void );
extern	int			RcInitialize( int extfd );
extern	void			RcClose( void );

#endif  // RC_INPUT_H
