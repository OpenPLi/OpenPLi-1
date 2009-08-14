/*
 * $Id: SigFlags.h,v 1.1 2001/12/22 17:14:52 obi Exp $
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
 * $Log: SigFlags.h,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 

#ifndef SigFlags_h
#define SigFlags_h

#include "SigHandler.h"

class SigFlags {

    public:

	SigFlags(void);

	static void generic_handler(int signum)
	{
		if (signum >= 0 && signum < _NSIG)
			flags[signum] = true;
	}

	static void core_dump_handler(int signum)
	{
		if (signum >= 0 && signum < _NSIG)
			flags[signum] = true;

		*((unsigned long*)1) = 0;
	}
	
	static void reset_all()
	{
		for (int i = 0; i < _NSIG; i++)
			flags[i] = false;
	}

	static void reset(int signum)
	{
		if (signum >= 0 && signum < _NSIG)
			flags[signum] = false;
	}
	
	static bool flags[_NSIG];
	static bool initialized;
	static SigHandler handlers[_NSIG];

};

#endif /* SigFlags_h */
