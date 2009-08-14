/*
 * $Id: bootmenue.cpp,v 1.28 2009/05/31 11:18:37 dbluelle Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *          based on dreamflash by mechatron
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

#ifdef HAVE_DREAMBOX_HARDWARE
#include "bootmenue.h"

#define SCRIPTFILE "/tmp/bm.sh"

extern int fh_png_getsize(const char *, int *, int *, int, int);
extern int fh_png_load(const char *, unsigned char *, int, int);
extern int fh_png_id(const char *);

stmenu *stmenu::instance = 0;
bool doexit = false;

stmenu::stmenu()
{
	instance = this;
	
	if (access("/root/platform/kernel", R_OK) != 0)
	{
		config = new bmconfig();
		img = new bmimages();
		CONNECT(RcInput::getInstance()->selected, stmenu::rc_event);
		display = new fbClass();
		lcd = new CLCDDisplay();
		CONNECT(CTimer::getInstance()->selected, stmenu::timeout);
		config->load();
		CTimer::getInstance()->start(atoi(config->timeoutValue.c_str()));
		bmgr = new bmboot();
		bmgr->getSkins(config->skinPath, config->mpoint);
		eString skin = config->skinPath + "/" + config->skinName;
		skinMax = bmgr->skinList.size() - 1;
		for (skinIndex = 0; skinIndex <= skinMax; skinIndex++)
			if (bmgr->skinList[skinIndex] == skin)
				break;
		if (loadImageList() > 1)
		{
			if (config->randomSkin == "1")
			{
				srand((unsigned)time(NULL));
				skin = bmgr->skinList[rand() % bmgr->skinList.size()];
				unsigned int pos = skin.find_last_of('/');
				config->skinName = skin.right(skin.length() - pos - 1);
				config->skinPath = skin.left(pos);
			}
			loadSkin(skin);
			eString pic = skin.replace(skin.find(".skin"), 5, ".png");
			showpic(pic);
			drawversion();
			drawmenu();
			mainloop();
		}
	
		delete display;
		delete lcd;
		delete config;
		delete img;
	}
}

stmenu::~stmenu()
{
}

void stmenu::timeout()
{
	doexit = true;
}

void stmenu::rc_event(unsigned short key)
{
	CTimer::getInstance()->start(atoi(config->timeoutValue.c_str()));
	eString skin, pic;

	switch (key)
	{
		case RC_EXIT:
		case RC_OK:
			doexit = true; 
			break;
		case RC_UP:
			if (--selentry < 0)
				selentry = maxentry;
			drawmenu();
			break;
		case RC_DOWN:
			if (++selentry > maxentry)
				selentry = 0;
			drawmenu();
			break;
		case RC_LEFT:
		case RC_RIGHT:
			skinIndex = (key == RC_LEFT) ? skinIndex - 1 : skinIndex + 1;
			if (skinIndex < 0)
				skinIndex = skinMax;
			else
			if (skinIndex > skinMax)
				skinIndex = 0;
			skin = bmgr->skinList[skinIndex];
			unsigned int pos = skin.find_last_of('/');
			config->skinName = skin.right(skin.length() - pos - 1);
			config->skinPath = skin.left(pos);
			loadSkin(skin);
			pic = skin.replace(skin.find(".skin"), 5, ".png");
			showpic(pic);
			drawversion();
			drawmenu();
			break;
		default:
			break;
	}
}

void stmenu::drawversion()
{
	eString tmp_ver = (ver_x != -1 && ver_y != -1) ? "Bootmanager - " + eString(BMVERSION) : "";
	display->RenderString(tmp_ver, ver_x + 25 + 10, ver_y, menu_xs - 25 - 10, CLCDDisplay::LEFT, ver_font, ver_r, ver_g, ver_b);
	display->Fill_buffer(menu_x - 5, menu_y - 5, menu_xs + 10, menu_ys + 10);
}

void stmenu::drawmenu()
{
	int firstentry = (selentry / 4) * 4;
	int lastentry = firstentry + 3;
	
	if (lastentry > maxentry) 
		lastentry = maxentry;
		
	display->W_buffer(menu_x - 5, menu_y - 5, menu_xs + 10, menu_ys + 10);
	lcd->draw_fill_rect (0, 0, 120, 64, CLCDDisplay::PIXEL_OFF);
	
	int a = 0;
	int h = (menu_ys / 4) - 2;

	for (int i = firstentry; i <= lastentry; i++)
	{
		lcd->RenderString(img->imageList[i].name, 0, (a * 15) + 3, 120, CLCDDisplay::CENTER, 20, CLCDDisplay::PIXEL_ON);
		int a1 = menu_y + (a * h);
		if (i == selentry)
		{
			lcd->draw_fill_rect (0, (a * 15), 120, (a * 15) + 15, CLCDDisplay::PIXEL_INV);
			//display->FillRect(menu_x + 5, a1, menu_xs + 25 + 5, h, sel_r, sel_g, sel_b);
			display->RenderCircle(menu_x + 10, a1 + 2 * (h / 3) - 2, sel_r, sel_g, sel_b);
		}
		display->RenderString(img->imageList[i].name, menu_x + 25 + 10, a1 + h, menu_xs - 25 - 10, CLCDDisplay::LEFT, h, str_r, str_g, str_b);
		a++;
	}
	lcd->update();
}

void stmenu::mainloop()
{
	while (!doexit)
		usleep(50000);
	
	config->selectedEntry = img->imageList[selentry].location;
	config->save();

	//clear menu
	lcd->draw_fill_rect (0, 0, 120, 64, CLCDDisplay::PIXEL_OFF);
	lcd->update();
	display->SetMode(720, 576, 8);

	startscript(img->imageList[selentry].location);
	goscript(img->imageList[selentry].location);
}

void stmenu::loadSkin(eString skin)
{
	if (access(skin.c_str(), R_OK) != 0)
		skin = "/share/tuxbox/enigma/boot/blank.skin";

	ver_x = ver_y = -1;
		
	ifstream skinFile(skin.c_str());
	if (skinFile)
	{
		eString line;
		while (getline(skinFile, line, '\n'))
		{
			if (line.find("first-line") == 0)
				sscanf(line.c_str(), "first-line=%d,%d,%d,%d,%d,%d", &ver_x, &ver_y, &ver_font, &ver_r, &ver_g, &ver_b);
			else
			if (line.find("menu-size") == 0)
				sscanf(line.c_str(), "menu-size=%d,%d,%d,%d", &menu_x, &menu_y, &menu_xs, &menu_ys);
			else
			if (line.find("string-color") == 0)
				sscanf(line.c_str(), "string-color=%d,%d,%d", &str_r, &str_g, &str_b);
			else
			if (line.find("select-color") == 0)
				sscanf(line.c_str(), "select-color=%d,%d,%d", &sel_r, &sel_g, &sel_b);
		}
		skinFile.close();
	}
}

int stmenu::loadImageList()
{
	int numberOfImages = 0;
	if ((numberOfImages = img->load(config->mpoint, true)) > 0)
	{
		maxentry = numberOfImages - 1;
		for (int i = 0; i <= maxentry; i++)
		{
			if (img->imageList[i].location == config->selectedEntry)
			{
				selentry = i;
				break;
			}
		}
	}
	return numberOfImages;
}

void stmenu::showpic(eString pic)
{
	if (access(pic.c_str(), R_OK) != 0)
		pic = "/share/tuxbox/enigma/boot/blank.png";

	if (fh_png_id(pic.c_str()) == 1)
	{
		int x, y;
		fh_png_getsize(pic.c_str(), &x, &y, INT_MAX, INT_MAX);
		unsigned char *buffer = (unsigned char *)malloc(x * y * 3);

		if (fh_png_load(pic.c_str(), buffer, x, y) == 0)
		{
			display->SetSAA(atoi(config->videoFormat.c_str())); //rgb = 0, fbas = 1, svideo = 2, component = 3;
			display->SetMode(720, 576, 16);
			display->fb_display(buffer, NULL, x, y, 0, 0, 0, 0);
		}

		free(buffer);
	}
}

void stmenu::startscript(eString image)
{
	if (FILE *f = fopen(SCRIPTFILE, "w"))
	{
		fprintf(f, "#!/bin/sh\n");
		if (image != "")
		{
			fprintf(f, "killall -9 smbd\n");
			fprintf(f, "killall -9 nmbd\n");
			fprintf(f, "killall -9 inetd\n");
			fprintf(f, "killall -9 rcS\n");
			fprintf(f, "killall -9 init\n");
			fprintf(f, "chroot %s ../go\n", image.c_str());
		}
		else
			fprintf(f, "echo booting flash image...\n");

		fclose(f);
		system("chmod 755 "SCRIPTFILE);
	}
}

void stmenu::goscript(eString image)
{
	eString go = image + "/go";
	if (FILE *f = fopen(go.c_str(), "w"))
	{
		fprintf(f, "#!/bin/sh\n\n");
		fprintf(f, "mount -t devfs dev /dev\n");
		fprintf(f, "/etc/init.d/rcS&\n");
		fclose(f);
		system(eString("chmod 755 " + go).c_str());
	}
}

int main(int argc, char **argv)
{
	stmenu::getInstance();
	return 0;
}
#else
int main(int argc, char **argv)
{
	return 0;
}
#endif
