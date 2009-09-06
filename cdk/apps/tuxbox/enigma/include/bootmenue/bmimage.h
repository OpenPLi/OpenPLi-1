/*
 * $Id: bmimage.h,v 1.11 2009/06/21 13:18:04 dbluelle Exp $
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <lib/base/estring.h>
#ifdef INSTIMAGESUPPORT
#include <src/enigma.h>
#define TUXBOXDATADIR "/share/tuxbox"
#include <src/enigma_dyn_utils.h>
#endif

using namespace std;

class eImage
{
public:
	eString name;
	eString location;
};

class bmimages: public Object
{
public:
	std::vector<eImage> imageList;
	std::vector<eImage>::iterator it;
	
	bmimages() {};
	~bmimages() 
	{
		imageList.clear();
	};
	
	void setImageName(eString imageDir, eString imageName)
	{
		if (FILE *f = fopen(eString(imageDir + "/imagename").c_str(), "w"))
		{
			fprintf(f, "%s", imageName.c_str());
			fclose(f);
		}
	}
	
	eString getImageName(eString imageDir)
	{
		unsigned int pos = imageDir.find_last_of("/");
		eString imageName = imageDir.right(imageDir.length() - pos -1);
		ifstream nameFile(eString(imageDir + "/imagename").c_str());
		if (nameFile)
		{
			eString line;
			getline(nameFile, line, '\n');
			nameFile.close();
			if (line)
				imageName = line;
		}
		return imageName;
	}
	
	int load(eString mpoint, bool clearList)
	{
		struct stat s;
		eImage image;
	
		eString dir[2] = {mpoint + "/image/", mpoint + "/fwpro/"};
	
		if (clearList)
			imageList.clear();
			
		image.name = "Flash-Image";
		image.location  = "";
		imageList.push_back(image);

		for (int i = 0; i < 2; i++)
		{
			DIR *d = opendir(dir[i].c_str());
			if (d)
			{
				while (struct dirent *e = readdir(d))
				{
					if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
					{
						eString name = dir[i] + eString(e->d_name);
						stat(name.c_str(), &s);
						if (S_ISDIR(s.st_mode))
						{
							image.location = name;
							image.name = getImageName(name);
							imageList.push_back(image);
						}
					}
				}
			}
			closedir(d);
		}
		return imageList.size();
	}
	
	void discard(eString location)
	{
		system(eString("rm -rf \"" + location + "\"").c_str());
	}
	
	void rename(eString from, eString to)
	{
		eString name = to.right(to.length() - to.find_last_of("/") - 1);
		setImageName(to, name);
	}
	
#ifdef INSTIMAGESUPPORT
	int add(eString sourceImage, eString imageName, eString mountDir)
	{
		int freeSpace = 0;
		eString imageDir = mountDir + "/fwpro/" + imageName;

		eDebug("[BOOTMANAGER] unpackImage: installation device = %s, image file = %s, image dir = %s", mountDir.c_str(), sourceImage.c_str(), imageDir.c_str());
		
		// check if enough space is available
		struct statfs s;
		if (statfs(mountDir.c_str(), &s) >= 0) 
			freeSpace = (s.f_bavail * (s.f_bsize / 1024));
		eDebug("[BOOTMANAGER] unpackImage: free space on device = %d", freeSpace);
		if (freeSpace < 20000)
			return -1;
		
		if (access(mountDir.c_str(), W_OK) != 0)
			return -2;

		// check if directory is available, delete it and recreate it
		if (access(imageDir.c_str(), W_OK) == 0)
			system(eString("rm -rf " + imageDir).c_str());
		system(eString("mkdir " + imageDir + " -m777 -p").c_str());
		if (access(imageDir.c_str(), W_OK) != 0)
			return -3;
	
		// split image file
		eString squashfsPart = mountDir + "/squashfs.img";
		remove(squashfsPart.c_str());
		if (system(eString("dd if=" + sourceImage + " of=" + squashfsPart + " bs=1024 skip=1152 count=4992").c_str()) >> 8)
			return -4;
		eString cramfsPart = mountDir + "/cramfs.img";
		remove(cramfsPart.c_str());
		if (system(eString("dd if=" + sourceImage + " of=" + cramfsPart + " bs=1024 skip=0 count=1152").c_str()) >> 8)
			return -5;
		remove(sourceImage.c_str());
	
		system(eString("cp " + eString(TEMPLATE_DIR) + "instimg.tmp /tmp/instimg.sh").c_str());
		system("chmod +x /tmp/instimg.sh");
		system("touch /var/etc/.dont_restart_enigma");
		system(eString("/tmp/instimg.sh " + imageDir + " " + cramfsPart + " " + squashfsPart + " &").c_str());
		eZap::getInstance()->quit(2);
		
		return 0;
	}
#endif
};
