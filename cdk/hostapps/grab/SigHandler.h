/*
 * $Id: SigHandler.h,v 1.1 2001/12/22 17:14:52 obi Exp $
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
 * $Log: SigHandler.h,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 

#ifndef SigHandler_h
#define SigHandler_h

#include <csignal>
#include <cerrno>
#include <cstring>
#include <cstdio>

class SigHandler {

    protected:
	int num;

	struct sigaction action;
	struct sigaction old_action;

	bool activated;

    public:

	bool is_active() const { return activated; }

	SigHandler(void) : num(0)
	{
		action.sa_handler = 0;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		
		activated = false;
	}

	SigHandler(int _num) : num(_num)
	{
		action.sa_handler = 0;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0;
		
		activated = false;
	}
	~SigHandler();

	int activate();
	int deactivate();

	void set_num(int _num) { num = _num; }
	void add_mask(int _num) { sigaddset(&action.sa_mask, _num); }
	void del_mask(int _num) { sigdelset(&action.sa_mask, _num); }
	bool test_mask(int _num) const
	{
	    if (sigismember(&action.sa_mask, _num))
		return true;
	    else
		return false;
	}

	void set_function(void (*function) (int)) { action.sa_handler = function; }
	void set_default() { action.sa_handler = SIG_DFL; }
	void set_ignore() { action.sa_handler = SIG_IGN; }
	void set_flags(int flags) { action.sa_flags = flags; }

};

#endif /* SigHandler_h */
