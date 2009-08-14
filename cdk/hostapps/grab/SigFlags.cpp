/*
 * $Id: SigFlags.cpp,v 1.1 2001/12/22 17:14:52 obi Exp $
 *
 * Copyright (C) 2001 Peter Niemayer et al.
 * See AUTHORS for details.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Log: SigFlags.cpp,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */

#include "SigFlags.h"

bool SigFlags::flags[_NSIG];
bool SigFlags::initialized = false;
SigHandler SigFlags::handlers[_NSIG];

SigFlags::SigFlags()
{
	int i;
	
	if (!initialized)
	{
		initialized = true;
		reset_all();
	}
	
	for (i = 0; i < _NSIG; i++)
	{
		handlers[i].set_num(i);
		handlers[i].set_function(SigFlags::generic_handler);
	}
	
	handlers[SIGILL].set_function(SigFlags::core_dump_handler);
	if (handlers[SIGILL].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGILL");

	handlers[SIGBUS].set_function(SigFlags::core_dump_handler);
	if (handlers[SIGBUS].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGBUS\n");

	handlers[SIGIO].set_function(SigFlags::core_dump_handler);
	if (handlers[SIGIO].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGIO\n");

	handlers[SIGXCPU].set_function(SigFlags::core_dump_handler);
	if (handlers[SIGXCPU].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGXCPU\n");

	handlers[SIGXFSZ].set_function(SigFlags::core_dump_handler);
	if (handlers[SIGXFSZ].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGXFSZ\n");

	if (handlers[SIGINT].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGINT\n");

	if (handlers[SIGTERM].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGTERM\n");

	if (handlers[SIGHUP].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGHUP\n");

	if (handlers[SIGCHLD].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGCHLD\n");

	if (handlers[SIGALRM].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGALRM\n");

	if (handlers[SIGPIPE].activate())
		fprintf(stderr,"SigFlags: unable to handle SIGPIPE\n");
}

