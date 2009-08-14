/*
 * $Id: debug.cpp,v 1.4 2005/10/18 19:20:34 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
  * based on nhttpd (C) 2001/2002 Dirk Szymanski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef ENABLE_EXPERT_WEBIF
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include "debug.h"
#include <chttpd/chttpdconfig.h>

extern chttpdConfig cfg;

CDEBUG *CDEBUG::instance = NULL;

CDEBUG *CDEBUG::getInstance(void)
{
	if (!instance)
		instance = new CDEBUG();

	return instance;
}

CDEBUG::CDEBUG(void)
{
	Debug = false;
	cfg.Log = false;
	cfg.Verbose = false;
	Logfile = NULL;
	buffer = new char[1024 * 4];
	pthread_mutex_init(&Log_mutex, NULL);
}

CDEBUG::~CDEBUG(void)
{
	if (Logfile)
	{
		fclose(Logfile);
		Logfile = NULL;
	}

	delete[] buffer;
}

void CDEBUG::LogRequest(CWebserverRequest *Request)
{
	if ((cfg.Log) || (cfg.Verbose))
	{
		if ((cfg.Log) && (!Logfile))
			Logfile = fopen("/tmp/httpd_log","a");

		pthread_mutex_lock(&Log_mutex);
		std::string method;

		switch (Request->Method) {
		case M_GET:
			method = "GET";
			break;
		case M_POST:
			method = "POST";
			break;
		case M_HEAD:
			method = "HEAD";
			break;
		default:
			method = "unknown";
			break;
		}

		struct tm *time_now;
		time_t now = time(NULL);
		char zeit[80];

		time_now = localtime(&now);
		strftime(zeit, 80, "[%d/%b/%Y:%H:%M:%S]", time_now);

		::sprintf(buffer,"%s %s %s %d %s %s\n",
			Request->Client_Addr.c_str(),
			zeit,
			method.c_str(),
			Request->HttpStatus,
			Request->URL.c_str(),
			//Request->ContentType.c_str(),
			Request->Param_String.c_str());

		if ((cfg.Log) && (Logfile))
			::fprintf(Logfile, "%s", buffer);

		if (cfg.Verbose)
			::printf("%s",buffer);

		pthread_mutex_unlock(&Log_mutex);
	}
}

void CDEBUG::debugprintf ( const char *fmt, ... )
{
	if (Debug)
	{
		pthread_mutex_lock( &Log_mutex );

		va_list arglist;
		va_start( arglist, fmt );
		vsprintf( buffer, fmt, arglist );
		va_end(arglist);

		::printf(buffer);

		pthread_mutex_unlock( &Log_mutex );
	}
}

void CDEBUG::logprintf ( const char *fmt, ... )
{
	if (Debug)
	{
		pthread_mutex_lock( &Log_mutex );

		va_list arglist;
		va_start( arglist, fmt );
		vsprintf( buffer, fmt, arglist );
		va_end(arglist);

		::printf(buffer);

		pthread_mutex_unlock( &Log_mutex );
	}
}

void CDEBUG::printf ( const char *fmt, ... )
{
	pthread_mutex_lock( &Log_mutex );

	va_list arglist;
	va_start( arglist, fmt );
	if(arglist)
		vsprintf( buffer, fmt, arglist );
	va_end(arglist);

	::printf(buffer);

	pthread_mutex_unlock( &Log_mutex );
}
#endif



