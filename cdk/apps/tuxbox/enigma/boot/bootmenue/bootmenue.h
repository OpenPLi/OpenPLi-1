/*
 * $Id: bootmenue.h,v 1.14 2005/11/26 14:40:19 digi_casi Exp $
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

#include "my_fb.h"
#include "my_lcd.h"
#include "my_rc.h"
#include "my_timer.h"
#include <bootmenue/bmconfig.h>
#include <bootmenue/bmimage.h>
#include <bootmenue/bmboot.h>

#define BMVERSION "0.5"

class stmenu: public Object
{
	static stmenu *instance;
	fbClass *display;
	CLCDDisplay *lcd;
	bmconfig *config;
	bmimages *img;
	bmboot *bmgr;

	int ver_x, ver_y, ver_font, ver_r, ver_g, ver_b;
	int menu_x, menu_y, menu_xs, menu_ys;
	int str_r, str_g, str_b;
	int sel_r, sel_g, sel_b;
	int skinIndex, skinMax;

	int selentry, maxentry;

	void rc_event(unsigned short key);
	void mainloop();
	void loadSkin(eString skin);
	int loadImageList();
	void timeout();

	void startscript(eString image);
	void goscript(eString image);

	void drawversion();
	void drawmenu();
	void showpic(eString pic);

 public:
	static stmenu *getInstance() {return (instance) ? instance : instance = new stmenu();}
	stmenu();
	~stmenu();
 private:
 	static void timeout(int);
};
