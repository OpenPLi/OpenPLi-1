/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __MOD_rcinput__
#define __MOD_rcinput__

#include <string>
#include <vector>

#define NEUTRINO_UDS_NAME "/tmp/neutrino.sock"


using namespace std;

class CRCInput
{
	private:
		struct event
		{
			uint	msg;
			uint	data;
		};

		struct timer
		{
			uint			id;
			unsigned long long	interval;
			unsigned long long	times_out;
			bool			correct_time;
		};

		uint		timerid;
		vector<timer>	timers;

		int 		fd_pipe_high_priority[2];
		int 		fd_pipe_low_priority[2];
		int         	fd_rc;
		int		fd_keyb;
		int		fd_event;

		int		fd_max;

		void open();
		void close();
		int translate(int code);

		void calculateMaxFd();

		int checkTimers();

	public:
		//rc-code definitions
		static const uint RC_MaxRC	= 0x3F;
		static const uint RC_KeyBoard	= 0x4000;
		static const uint RC_Events	= 0x80000000;
		static const uint RC_Messages	= 0x90000000;
		static const uint RC_WithData	= 0xA0000000;
		enum
		{
			RC_0=0x0, RC_1=0x1, RC_2=0x2, RC_3=0x3, RC_4=0x4, RC_5=0x5, RC_6=0x6, RC_7=0x7, RC_8=0x8, RC_9=0x9,
		    RC_right=0xA, RC_left=0xB, RC_up=0xC, RC_down=0xD, RC_ok=0xE, RC_spkr=0xF,
			RC_standby=0x10, RC_green=0x11, RC_yellow=0x12, RC_red=0x13, RC_blue=0x14, 
			RC_plus=0x15, RC_minus=0x16, RC_help=0x17, RC_setup=0x18, RC_home=0x1F, RC_page_up=0x20, RC_page_down=0x21,
			RC_top_left=27, RC_top_right=28, RC_bottom_left=29, RC_bottom_right=30,
		    RC_standby_release= RC_MaxRC+ 1,
		    RC_timeout	= 0xFFFFFFFF,
		    RC_nokey	= 0xFFFFFFFE
		};

		//only used for plugins (games) !!
		int getFileHandle()
		{
			return fd_rc;
		}
		void stopInput();
		void restartInput();

		int repeat_block;
		int repeat_block_generic;
		CRCInput();      //constructor - opens rc-device and starts needed threads
		~CRCInput();     //destructor - closes rc-device


		static bool isNumeric(unsigned int key);

		static string getKeyName(int);

		int addTimer(unsigned long long Interval, bool oneshot= true, bool correct_time= true );
		int addTimer(struct timeval Timeout);
		int addTimer(const time_t *Timeout);

		void killTimer(uint id);

		long long calcTimeoutEnd( int Timeout );
		long long calcTimeoutEnd_MS( int Timeout );

		void getMsgAbsoluteTimeout(uint *msg, uint* data, unsigned long long *TimeoutEnd, bool bAllowRepeatLR= false);
		void getMsg(uint *msg, uint* data, int Timeout, bool bAllowRepeatLR= false);     //get message, timeout in 1/10 secs :)
		void getMsg_ms(uint *msg, uint* data, int Timeout, bool bAllowRepeatLR= false);     //get message, timeout in msecs :)
		void getMsg_us(uint *msg, uint* data, unsigned long long Timeout, bool bAllowRepeatLR= false);     //get message, timeout in µsecs :)
		void postMsg(uint msg, uint data, bool Priority = true );  // push message back into buffer
		void clearRCMsg();

		int messageLoop( bool anyKeyCancels = false, int timeout= -1 );
};


#endif
