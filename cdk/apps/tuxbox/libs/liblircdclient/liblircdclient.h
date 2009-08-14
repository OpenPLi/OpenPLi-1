/*
	DBoxII-Project

	2002 by Zwen
	Homepage: http://www.dbox2.info/

	Kommentar:

	Lircd Client Klasse. Verpackung des LIRC Tools RC in client Klasse
	
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

#ifndef __liblircdclient__
#define __liblircdclient__

#include <string>

using namespace std;

class CLircdClient
{
	private:
		const char* ReadString();
		int fd;
	public:
		int  SendUsecs(string device, string command,unsigned long usecs);
		int  Send(string cmd, string device, string key);
		int  SendOnce(string device, string key);
		bool Connect();
		void Disconnect();

};

#endif
