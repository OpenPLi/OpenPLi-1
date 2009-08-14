/*
 * $Id: chttpd.cpp,v 1.3 2005/10/18 11:30:19 digi_casi Exp $
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
#define CHTTPD_VERSION "0.1"

#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <config.h>
#include "webserver.h"
#include "debug.h"
#include <chttpd/chttpdconfig.h>

using namespace std;

CWebserver *webserver;
chttpdConfig cfg;

void sig_catch(int msignal)
{
	switch (msignal)
	{
		case SIGHUP:
			aprintf("got signal HUP, reading config\n");
			cfg.load();
			break;
		default:
			aprintf("stop requested...\n");
			webserver->Stop();
			delete(webserver);
			exit(0);
	}
}

int main(int argc, char **argv)
{
	bool debug = false;
	bool do_fork = true;

	int i;

	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{

			if (strncmp(argv[i], "-d", 2) == 0)
			{
				CDEBUG::getInstance()->Debug = true;
				do_fork = false;
			}
			else 

			if (strncmp(argv[i], "-f", 2) == 0)
			{
				do_fork = false;
			}
			else if (strncmp(argv[i],"--version", 9) == 0) 
			{
				printf("chttp - Webserver\n");
				printf("Version: %s\n", CHTTPD_VERSION);
				return 0;
			}
			else if ((strncmp(argv[i], "--help", 6) == 0) || (strncmp(argv[i], "-h", 2) == 0))
			{
				printf("chttpd parameters:\n");
				printf("-d\t\tdebug\n");
				printf("-f\t\tdo not fork\n");
				printf("--version\tversion\n");
				printf("--help\t\tthis text\n\n");
				return 0;
			}
		}
	}

	signal(SIGINT,sig_catch);
	signal(SIGHUP,sig_catch);
	signal(SIGTERM,sig_catch);

	aprintf("HTTP-Server starting..\n");
	
	if (do_fork)
	{
		switch (fork())
		{
		case -1:
			dperror("fork");
			return -1;
		case 0:
			break;
		default:
			return 0;
		}

		if (setsid() == -1)
		{
			dperror("[chttpd] Error setsid");
			return -1;
		}
	}

	if ((webserver = new CWebserver(debug)) != NULL)
	{
			if (webserver->Start())
			{
				webserver->DoLoop();
				webserver->Stop();
			}
	}
	else
	{
		aprintf("Error initializing chttpd\n");
		return -1;
	}

	return 0; 
}
#else
int main(int argc, char **argv)
{
	return 0;
}
#endif

