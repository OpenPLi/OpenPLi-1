/*
 * $Id: bmconfig.h,v 1.6 2005/11/26 14:40:20 digi_casi Exp $
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
 
#define BMCONFIGFILE "/tuxbox/config/enigma/bootmenue.conf"
#define SKINDIR "/var/tuxbox/config/enigma/boot"

#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;

#include <lib/base/estring.h>

class bmconfig: public Object
{
 public:
	bmconfig() {};
	~bmconfig() {};
	
	eString mpoint, selectedEntry, randomSkin, timeoutValue, videoFormat, skinPath, skinName;
	
	void load()
	{
		timeoutValue = "10";
		videoFormat = "1";
		selectedEntry = "";
		skinPath = SKINDIR;
		skinName = "blank.skin";
		mpoint = "/media/usb";
		randomSkin = "0";
		
		eString file;
		if (access("/tmp/jffs2", R_OK) == 0)
			file = "/tmp/jffs2"BMCONFIGFILE;
		else
			file = "/var"BMCONFIGFILE;
		ifstream configFile(file.c_str());
		eString line;
		if (configFile)
		{
			while (getline(configFile, line, '\n'))
			{
				if (line.find("timeout") == 0)
					timeoutValue = line.right(line.length() - 8);
				else 
				if (line.find("videoformat") == 0)
					videoFormat = line.right(line.length() - 12);
				else 
				if (line.find("selentry") == 0)
					selectedEntry = line.right(line.length() - 9);
				else 
				if (line.find("skin-path") == 0)
					skinPath = line.right(line.length() - 10);
				else 
				if (line.find("skin-name") == 0)
					skinName = line.right(line.length() - 10);
				else 
				if (line.find("mountpoint") == 0)
					mpoint = line.right(line.length() - 11);
				else 
				if (line.find("randomskin") == 0)	
					randomSkin = line.right(line.length() - 11);
			}
			configFile.close();
		}
		else
			save();
	}

	void save()
	{
		eString file;
		if (access("/tmp/jffs2", R_OK) == 0)
			file = "/tmp/jffs2"BMCONFIGFILE;
		else
			file = "/var"BMCONFIGFILE;
		if (FILE *f = fopen(file.c_str(), "w"))
		{
			fprintf(f, "#BootManager-Config\n");
			fprintf(f, "mountpoint=%s\n", mpoint.c_str());
			fprintf(f, "selentry=%s\n", selectedEntry.c_str());
			fprintf(f, "randomskin=%s\n", randomSkin.c_str());
			fprintf(f, "timeout=%s\n", timeoutValue.c_str());
			fprintf(f, "videoformat=%s\n", videoFormat.c_str());
			fprintf(f, "skin-path=%s\n", skinPath.c_str());
			fprintf(f, "skin-name=%s\n", skinName.c_str());
			fclose(f);
		}
	}
};
