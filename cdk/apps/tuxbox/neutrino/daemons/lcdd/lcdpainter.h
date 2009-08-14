/*
	LCD-Daemon  -   DBoxII-Project

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

#ifndef __lcdpainter__
#define __lcdpainter__

#include <lcddisplay/lcddisplay.h>
#include <lcddisplay/fontrenderer.h>
#include <lcddclient/lcddtypes.h>


class CLCDPainter
{
	private:
		
		class FontsDef
		{
			public:
				LcdFont *channelname;
				LcdFont *time; 
				LcdFont *menutitle;
				LcdFont *menu;
		};

		CLCDDisplay			display;
		LcdFontRenderClass		*fontRenderer;
		FontsDef			fonts;

		raw_display_t		icon_lcd;
		raw_display_t		icon_setup;
		raw_display_t		icon_power;

		CLcddTypes::mode	mode;

		string				servicename;
		char				volume;
		int				lcd_brightness;
		int				lcd_standbybrightness;
		int				lcd_contrast;
		int				lcd_power;
		int				lcd_inverse;
		bool				muted;
		bool				showclock;

	public:

		CLCDPainter();
		~CLCDPainter();

		bool init();

		void show_servicename(string name);
		void show_time();
		void show_volume(char vol);
		void show_menu(int position, char* text, int highlight);

		bool set_mode(CLcddTypes::mode m, char *title);

		void setlcdparameter(int dimm, int contrast, int power, int inverse);

		void setBrightness(int);
		int getBrightness();

		void setBrightnessStandby(int);
		int getBrightnessStandby();

		void setContrast(int);
		int getContrast();

		void setPower(int);
		int getPower();

		void setInverse(int);
		int getInverse();

		void setMuted(bool);

		void update();	// applies new brightness, contrast, etc
		void CLCDPainter::pause();	// for plugins only
		void CLCDPainter::resume();	// for plugins only
};


#endif
