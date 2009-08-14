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
$Log: container.h,v $
Revision 1.4.6.3  2008/08/07 17:56:43  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.4  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef CONTAINER_H
#define CONTAINER_H

#include "zap.h"
#include "channels.h"
#include "fbClass.h"
#include "osd.h"
#include "settings.h"
#include "tuner.h"
#include "pat.h"
#include "pmt.h"
#include "eit.h"
#include "scan.h"
//#include "gui/infowindow.h"

class container
{

public:
	settings *settings_obj;
	zap *zap_obj;
	channels *channels_obj;
	fbClass *fbClass_obj;
	osd *osd_obj;
	tuner *tuner_obj;
	pat *pat_obj;
	pmt *pmt_obj;
	eit *eit_obj;
	scan *scan_obj;
/*	ber *ber_obj;
	sig *sig_obj;
	snr *snr_obj;
	pic *pic_obj;
*/

	container(zap *z, channels *c, fbClass *f, osd *o, settings *s, tuner *t, pat *pa, pmt *pm, eit *e, scan *sc/*,ber *ber, sig *sig, snr *snr, pic *pic*/);
};
#endif
