/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __framebuffer__
#define __framebuffer__


#include <linux/fb.h>
#include <linux/vt.h>
#include <stdint.h>
#include <dbox/fb.h>

#include <string>

#include <gui/widget/component.h>


using namespace std;

/** Ausführung als Singleton */
class CFrameBuffer
{
	private:

		CFrameBuffer();
		~CFrameBuffer();

		struct rgbData
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		string			iconBasePath;

		int			fd, tty;
		unsigned char	*lfb;
		int		available;
		unsigned char	*background;
		int			backgroundColor;
		string			backgroundFilename;
		bool			useBackgroundPaint;
		unsigned int	xRes, yRes, stride, bpp;
		struct fb_var_screeninfo screeninfo, oldscreen;
		fb_cmap cmap;
		__u16 red[256], green[256], blue[256], trans[256];

		void paletteFade(int i, __u32 rgb1, __u32 rgb2, int level);

		int 	kd_mode;
		struct	vt_mode vt_mode;
		bool	active;
		static	void switch_signal (int);

	public:

		static CFrameBuffer* getInstance();

		void init(string fbDevice="/dev/fb/0");
		int setMode(unsigned int xRes, unsigned int yRes, unsigned int bpp);


		int getFileHandle(); //only used for plugins (games) !!

		unsigned char* getFrameBufferPointer(); //pointer to framebuffer
		unsigned int getStride(); //stride (anzahl bytes die eine Zeile im Framebuffer belegt)
		bool getActive(); //is framebuffer active

		void setTransparency( int tr = 0 );

		//Palette stuff
		void setAlphaFade(int in, int num, int tr);
		void paletteGenFade(int in, __u32 rgb1, __u32 rgb2, int num, int tr=0);
		void paletteSetColor(int i, __u32 rgb, int tr);
		void paletteSet();

		//paint functions
		void paintPixel(int x, int y, unsigned char col);

		void paintBox(int xa, int ya, int xb, int yb, unsigned char col);
		void paintBoxRel(int x, int y, int dx, int dy, unsigned char col);

		void paintLine(int xa, int ya, int xb, int yb, unsigned char col);

		void paintVLine(int x, int ya, int yb, unsigned char col);
		void paintVLineRel(int x, int y, int dy, unsigned char col);

		void paintHLine(int xa, int xb, int y, unsigned char col);
		void paintHLineRel(int x, int dx, int y, unsigned char col);


		bool paintIcon(string filename, int x, int y, unsigned char offset=1);
		bool paintIcon8(string filename, int x, int y, unsigned char offset=0);
		void loadPal(string filename, unsigned char offset=0, unsigned char endidx=255 );
		void setIconBasePath(string);

		bool loadPicture2Mem(string filename, unsigned char* memp);
		bool savePictureFromMem(string filename, unsigned char* memp);

		void setBackgroundColor(int color);
		bool loadBackground(string filename, unsigned char col = 0);
		void useBackground(bool);

		void paintBackgroundBox(int xa, int ya, int xb, int yb);
		void paintBackgroundBoxRel(int x, int y, int dx, int dy);
		void paintBackgroundBoxRel(CPoint origin, CDimension dimension);

		void paintBackground();

		void SaveScreen(int x, int y, int dx, int dy, unsigned char* memp);
		void RestoreScreen(int x, int y, int dx, int dy, unsigned char* memp);
};


#endif
