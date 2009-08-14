#ifndef __basicserver__
#define __basicserver__

/*
 * $Header: /cvs/tuxbox/apps/dvb/zapit/include/zapit/Attic/basicserver.h,v 1.3 2002/10/18 11:55:09 thegoodguy Exp $
 *
 * Basic Server Class (Neutrino) - DBoxII-Project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
 *
 * License: GPL
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

#include <string>

#include <zapit/client/basicmessage.h>

class CBasicServer
{
 private:
	int sock_fd;
	std::string name;

 public:
	bool prepare(const char* socketname);
	void run(bool (parse_command)(CBasicMessage::Header &rmsg, int connfd), const CBasicMessage::t_version version);
};

#endif
