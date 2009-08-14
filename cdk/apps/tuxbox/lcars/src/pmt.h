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
$Log: pmt.h,v $
Revision 1.6  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.5  2002/06/13 01:35:48  TheDOC
NVOD should work now

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef PMT_H
#define PMT_H

#include <map>

class pmt_data
{
public:
	int pmt;
	int sid;
	int pid_counter;
	int type[10];
	int PID[10];
	int subtype[10]; // 0 - nichts, 1 - vtxt, 2 - ac3
	int component[10];
	int ecm_counter;
	int CAID[10];
	int ECM[10];
	int PCR;
};

class pmt
{

public:
	pmt_data readPMT(int pmt_pid);
};

#endif
