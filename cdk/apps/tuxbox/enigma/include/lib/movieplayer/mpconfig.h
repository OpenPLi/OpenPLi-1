/*
 * $Id: mpconfig.h,v 1.3 2005/12/23 17:00:07 digi_casi Exp $
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
 
#ifndef __lib_mpconfig_h
#define __lib_mpconfig_h

#include <vector>
#include <lib/base/estring.h>

#define CONFFILE0 "/share/tuxbox/enigma/templates/movieplayer.xml"
#define CONFFILE1 "/var/tuxbox/config/movieplayer.xml"

struct serverConfig
{
	eString serverIP;
	eString webifPort;
	eString streamingPort;
	eString vlcUser;
	eString vlcPass;
	eString startDir;
	eString CDDrive;
};

struct videoTypeParms
{
	eString name;
	eString extension;
	eString videoRate;
	eString audioRate;
	eString videoCodec;
	eString videoRatio;
	bool transcodeVideo;
	bool transcodeAudio;
	eString fps;
	bool soutadd;
};

struct codecs
{
	eString mpeg1;
	eString mpeg2;
	eString audio;
};

class eMPConfig
{
	struct serverConfig serverConf;
	std::vector<struct videoTypeParms> videoParmList;
	struct codecs avcodecs;
public:
	eMPConfig();
	~eMPConfig();
	bool load();
	void save();
	struct videoTypeParms getVideoParms(eString name, eString extension);
	struct serverConfig getServerConfig();
	struct codecs getAVCodecs();
	void setVideoParms(struct videoTypeParms videoParms);
	void setServerConfig(struct serverConfig serverConf);
	void setAVCodecs(struct codecs avCodecs);
};
#endif
