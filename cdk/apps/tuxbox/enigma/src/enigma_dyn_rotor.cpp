/*
 * $Id: enigma_dyn_rotor.cpp,v 1.11 2005/10/12 20:46:27 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
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

#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <enigma.h>
#include <enigma_main.h>
#include <enigma_standby.h>
#include <timer.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/record.h>
#include <lib/dvb/serviceplaylist.h>
#include <lib/dvb/frontend.h>

#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_rotor.h>
#include <configfile.h>

using namespace std;

eString getConfigRotor(void)
{
	eString result = readFile(TEMPLATE_DIR + "rotor.tmp");
	eString positions;
	eTransponder *tp = NULL;
	eString satPos, satName, motorPos, hexmotorPos, current;
	bool gotoX = false;
	
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi && sapi->transponder)
		tp = sapi->transponder;
	
	for (std::list<eLNB>::iterator it(eTransponderList::getInstance()->getLNBs().begin()); it != eTransponderList::getInstance()->getLNBs().end(); it++)
	{
		// eDebug("[webif] getConfigRotor: useGotoX=%d", it->getDiSEqC().useGotoXX);
		if ((it->getDiSEqC().useGotoXX & 1) == 0)
		{
			// go thru all satellites...
			for (ePtrList<eSatellite>::iterator s (it->getSatelliteList().begin()); s != it->getSatelliteList().end(); s++)
			{
				eString tmp  = readFile(TEMPLATE_DIR + "rotorSat.tmp");
				if (tp && s->getOrbitalPosition() == tp->satellite.orbital_position)
					current = "on.gif";
				else
					current = "trans.gif";
				satName = s->getDescription();
				satPos = eString().sprintf("%d", s->getOrbitalPosition());
				int motorPosition = it->getDiSEqC().RotorTable[s->getOrbitalPosition()];
				motorPos = eString().sprintf("%d", motorPosition);
				hexmotorPos = eString().sprintf("%X", motorPosition);

				tmp.strReplace("#CURRENT#", current);
				tmp.strReplace("#SATPOS#", satPos);
				tmp.strReplace("#SATNAME#", satName);
				tmp.strReplace("#MOTORPOS#", motorPos);
				tmp.strReplace("#GOTOBUTTON#", button(100, "Goto", GREEN, "javascript:motor('gotostoredpos', '" + hexmotorPos + "')", "#FFFFFF"));
				tmp.strReplace("#STOREBUTTON#", button(100, "Store", RED, "javascript:motor('storetopos', '" + hexmotorPos + "')", "#FFFFFF"));
				positions += tmp;
			}
		}
		else
			gotoX = true;
	}
	if (!positions)
	{
		if (!gotoX)
			positions = "<tr><td colspan=\"6\">No motor position table available.</td></tr>";
		else
			positions = "<tr><td colspan=\"6\">GotoX active.</td></tr>";
	}
	result.strReplace("#MOTORPOSITIONS#", positions);

	return result;
}

static eString sendDiSEqCCmd(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString addr = opt["addr"];
	eString cmd = opt["cmd"];
	eString params = opt["params"];
	eString frame = opt["frame"];
	if (!frame)
		frame = "E0";
	
	int iaddr, icmd, iframe;
	sscanf(addr.c_str(), "%x", &iaddr);
	sscanf(cmd.c_str(), "%x", &icmd);
	sscanf(frame.c_str(), "%x", &iframe);

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	eDebug("[enigma_dyn_rotor] sending DiSeqCCmd: %x %x %s %x", iaddr, icmd, params.c_str(), iframe);
	eFrontend::getInstance()->sendDiSEqCCmd(iaddr, icmd, params, iframe);
	
	return closeWindow(content, "", 500);
}

void ezapRotorInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/sendDiSEqCCmd", sendDiSEqCCmd, lockWeb);
}
#endif
