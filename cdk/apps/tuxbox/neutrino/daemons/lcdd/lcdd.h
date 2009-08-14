/*
	LCD-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/



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

#ifndef __lcdd__
#define __lcdd__

#include <configfile.h>
#include <pthread.h>

#include <lcddisplay/lcddisplay.h>
#include <lcddclient/lcddMsg.h>
#include <lcddclient/lcddtypes.h>

#include "lcdpainter.h"


class CLCDD
{
	private:
		CLCDPainter		lcdPainter;
		CConfigFile		configfile;
		pthread_t		thrTime;

		bool			shall_exit;
		bool			debugoutput;

		CLCDD();
		~CLCDD();

		static void* TimeThread(void*);
		static void sig_catch(int);

		void parse_command(int connfd, CLcddMsg::Header rmsg);
		
	public:

		static CLCDD* getInstance();
		int main(int argc, char **argv);

		void saveConfig();
		void loadConfig();
};


#endif
