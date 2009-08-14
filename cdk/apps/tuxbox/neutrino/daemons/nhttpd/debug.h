/*
	Timer-Daemon  -   DBoxII-Project

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

#ifndef __nhttpd_debug__
#define __nhttpd_debug__

#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>

#include "request.h"

class CDEBUG
{
	private:
		pthread_mutex_t Log_mutex;
		char *buffer;
		FILE *Logfile;

		CDEBUG();
		~CDEBUG();

	public:
		bool Debug;
		bool Log;
		bool Verbose;

		static CDEBUG* getInstance();
		void printf( const char *fmt, ... );
		void debugprintf( const char *fmt, ... );
		void logprintf( const char *fmt, ... );
		void LogRequest(CWebserverRequest *Request);


};

#define aprintf(fmt, args...) {CDEBUG::getInstance()->printf( "[nhttpd] " fmt, ## args);}
#define dprintf(fmt, args...) {CDEBUG::getInstance()->debugprintf( "[nhttpd] " fmt, ## args);}
#define lprintf(fmt, args...) {CDEBUG::getInstance()->logprintf( "[nhttpd] " fmt, ## args);}
#define dperror(str) {perror("[nhttpd] " str);}

#endif
