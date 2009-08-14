/*
 * $Id: bmboot.h,v 1.3 2005/11/26 14:40:20 digi_casi Exp $
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

#include <iomanip>
#include <iostream>
#include <fstream>

using namespace std;

#include <lib/base/estring.h>

class bmboot: public Object
{
public:
	void mountJFFS2()
	{
		system("umount /tmp/jffs2");
		system("mkdir /tmp/jffs2");
		system("mount -t jffs2 /dev/mtdblock/1 /tmp/jffs2");
	}

	void unmountJFFS2()
	{
		system("umount /tmp/jffs2");
		system("rm -rf /tmp/jffs2");
	}

	bmboot() {};
	~bmboot() {};
	
	void activateMenu(eString menu)
	{
		bool bm = (menu == "BM");
		bool fw = (menu == "FW");
		bool found = false;
		bool active = false;
		bool initChanged = false;
		eString line;
		eString file;
	
		mountJFFS2();
	
		ifstream initFile ("/tmp/jffs2/etc/init");
		if (initFile)
		{
			while (getline(initFile, line, '\n'))
			{
				if (line.find("fwpro") != eString::npos)
				{
					int pos = line.find_first_not_of(" ");
					active = (line[pos] != '#' && line[pos] != ':');
					if (fw)
					{
						if (!active)
						{
							line[pos] = ' ';
							initChanged = true;
						}
						found = true;
					}
					else
					{
						if (active)
						{
							if (pos > 1)
								line[pos - 2] = ':';
							else
								line = ": " + line;
							initChanged = true;	
						}
					}
				}
				if (line.find("bm.sh") != eString::npos)
				{
					int pos = line.find_first_not_of(" ");
					active = (line[pos] != '#' && line[pos] != ':');
					if (bm)
					{
						if (!active)
						{
							line[pos] = ' ';
							initChanged = true;
						}
						found = true;
					}
					else
					{
						if (active)
						{
							if (pos > 1)
								line[pos - 2] = ':';
							else
								line = ": " + line;	
							initChanged = true;
						}
					}
				}
				if (file)
					file += "\n" + line;
				else
					file = line;
			}
		}
		initFile.close();
	
		if (!found)
		{
			if (bm)
			{
				file += "\n/bin/bootmenue && /tmp/bm.sh";
				initChanged = true;
			}
		}
		
		if (initChanged)
		{
			FILE *out = fopen("/tmp/jffs2/etc/init", "w");
			fprintf(out, file.c_str());
			fclose(out);
			system("chmod +x /tmp/jffs2/etc/init");
		}
	
		unmountJFFS2();
	}
	
	std::vector<eString> skinList;
	
	int getSkins(eString skinPath, eString mountPoint)
	{
		eString dirs[2];
		eString dir;

		skinList.clear();

		dirs[0] = skinPath;
		dirs[1] = mountPoint + "/boot/skins";
	
		for (int i = 0; i < 2; i++)
		{
			dir = dirs[i];
			if (i == 0)
			{
				mountJFFS2();
				if (dirs[i].find("/var") == 0)
					dir = "/tmp/jffs2" + dirs[i].right(dirs[i].length() - 4);
			}
			DIR *d = opendir(dir.c_str());
			if (d)
			{
				while (struct dirent *e = readdir(d))
				{
					if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
					{
						eString name = eString(e->d_name);
						if (name.right(5) == ".skin")
							skinList.push_back(eString(dirs[i] + "/" + name));
					}
				}
				closedir(d);
			}
			if (i == 0)
				unmountJFFS2();
		}
		return skinList.size();
	}
};
