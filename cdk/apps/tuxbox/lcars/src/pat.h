/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: pat.h,v $
Revision 1.5  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef PAT_H
#define PAT_H

#include <map>

struct pat_entry
{
	int TS;
	int ONID;
	int SID;
	int PMT;
};

class pat
{
	int oldpatTS;
	std::multimap<int, struct pat_entry> pat_list;
public:
	bool readPAT();
	int getTS() { return (*pat_list.begin()).second.TS; }
	int getPMT(int SID);
};

#endif
