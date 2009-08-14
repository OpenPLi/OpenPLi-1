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
$Log: container.cpp,v $
Revision 1.4.6.3  2008/08/07 17:56:43  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include "container.h"

container::container(zap *z, channels *c, fbClass *f, osd *o, settings *s, tuner *t, pat *pa, pmt *pm, eit *e, scan *sc/*,ber *ber, sig *sig, snr *snr, pic *pic*/)
{
	zap_obj = z;
	channels_obj = c;
	fbClass_obj = f;
	osd_obj = o;
	settings_obj = s;
	tuner_obj = t;
	pat_obj = pa;
	pmt_obj = pm;
	eit_obj = e;
	scan_obj = sc;
/*	ber_obj = ber;
	sig_obj = sig;
	snr_obj = snr;
	pic_obj = pic;
*/
}
