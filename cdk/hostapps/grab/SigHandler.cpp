/*
 * $Id: SigHandler.cpp,v 1.1 2001/12/22 17:14:52 obi Exp $
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
 * $Log: SigHandler.cpp,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 

#include "SigHandler.h"

SigHandler::~SigHandler()
{
	if (deactivate())
		fprintf(stderr,"SigHandler::~SigHandler unable to deactivate\n");
}

int SigHandler::activate()
{
	
	if (activated)
		return 0;
	
	if (sigaction(num, &action, &old_action) < 0)
	{
		fprintf(stderr,"SigHandler::activate %s\n", strerror(errno));
		return -1;
	}
	
	activated = true;
	return 0;
}

int SigHandler::deactivate(void) {
	
	if (!activated)
		return 0;
	
	if (sigaction(num, &old_action, 0) < 0)
	{
		fprintf(stderr,"SigHandler::deactivate %s\n", strerror(errno));
		return -1;
	}
	
	activated = false;
	return 0;
}
