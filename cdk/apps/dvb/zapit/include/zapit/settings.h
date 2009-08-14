/*
 * $Header: /cvs/tuxbox/apps/dvb/zapit/include/zapit/settings.h,v 1.3.2.1 2002/10/24 00:01:59 obi Exp $
 *
 * zapit's settings - d-box2 linux project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
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

#ifndef __zapit__settings_h__
#define __zapit__settings_h__


#ifdef HAVE_CONFIG_H
#include                "config.h"
#else
#define CONFIGDIR       "/var/tuxbox/config"
#endif


#define CONFIGFILE      CONFIGDIR "/zapit/zapit.conf"
#define SERVICES_XML    CONFIGDIR "/zapit/services.xml"
#define BOUQUETS_XML    CONFIGDIR "/zapit/bouquets.xml"

#define CABLES_XML      "/etc/cables.xml"
#define SATELLITES_XML  "/etc/satellites.xml"

#define AUDIO_DEVICE    "/dev/dvb/card0/audio0"
#define DEMUX_DEVICE    "/dev/dvb/card0/demux0"
#define FRONTEND_DEVICE "/dev/dvb/card0/frontend0"
#define SEC_DEVICE      "/dev/dvb/card0/sec0"
#define VIDEO_DEVICE    "/dev/dvb/card0/video0"


#define CAMD_UDS_NAME  "/tmp/camd.socket"


#endif /* __zapit__settings_h__ */
