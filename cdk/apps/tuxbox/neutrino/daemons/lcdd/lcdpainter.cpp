/*
	LCD-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean',
                      2002 thegoodguy
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


#include <config.h>

#include <dbox/fp.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

#include "bigclock.h"
#include "newclock.h"
#include "lcdpainter.h"

CLCDPainter::CLCDPainter()
{
}

CLCDPainter::~CLCDPainter()
{
}

bool CLCDPainter::init()
{
	fontRenderer = new LcdFontRenderClass( &display );
	fontRenderer->AddFont(FONTDIR "/micron.ttf");
	fontRenderer->InitFontCache();

	#define FONTNAME "Micron"
	fonts.channelname=fontRenderer->getFont(FONTNAME, "Regular", 15);
	fonts.time=fontRenderer->getFont(FONTNAME, "Regular", 14);
	fonts.menutitle=fontRenderer->getFont(FONTNAME, "Regular", 15);
	fonts.menu=fontRenderer->getFont(FONTNAME, "Regular", 12);

	setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
	display.setIconBasePath( DATADIR "/lcdd/icons/");

	if(!display.isAvailable())
	{
		printf("exit...(no lcd-support)\n");
		return false;
	}

	if (!display.paintIcon("neutrino_setup.raw",0,0,false))
	{
		printf("exit...(no neutrino_setup.raw)\n");
		return false;
	}
	display.dump_screen(&icon_setup);

	if (!display.paintIcon("neutrino_power.raw",0,0,false))
	{
		printf("exit...(no neutrino_power.raw)\n");
		return false;
	}
	display.dump_screen(&icon_power);

	if (!display.paintIcon("neutrino_lcd.raw",0,0,false))
	{
		printf("exit...(no neutrino_lcd.raw)\n");
		return false;
	}
	display.dump_screen(&icon_lcd);

	mode = CLcddTypes::MODE_TVRADIO;
	show_servicename("Booting...");
	showclock=true;
	return true;
}

void CLCDPainter::show_servicename( string name )
{
	servicename = name;
	if (mode != CLcddTypes::MODE_TVRADIO)
		return;

	display.draw_fill_rect (0,14,120,48, CLCDDisplay::PIXEL_OFF);

	if (fonts.channelname->getRenderWidth(name.c_str(), true) > 120)
	{
		int pos;
		string text1 = name;
		do
		{
			pos = text1.find_last_of("[ .]+"); // <- characters are UTF-encoded!
			if (pos != -1)
				text1 = text1.substr( 0, pos );
		} while ( ( pos != -1 ) && ( fonts.channelname->getRenderWidth(text1.c_str(), true) > 120 ) );
		
		if ( fonts.channelname->getRenderWidth(text1.c_str(), true) <= 120 )
			fonts.channelname->RenderString(1,29+16, 130, name.substr(text1.length()+ 1).c_str(), CLCDDisplay::PIXEL_ON, 0, true); // UTF-8
		else
		{
			string text1 = name;
			while (fonts.channelname->getRenderWidth(text1.c_str(), true) > 120)
				text1= text1.substr(0, text1.length()- 1);
			
			fonts.channelname->RenderString(1,29+16, 130, name.substr(text1.length()).c_str(), CLCDDisplay::PIXEL_ON, 0, true); // UTF-8
		}
		
		fonts.channelname->RenderString(1,29, 130, text1.c_str(), CLCDDisplay::PIXEL_ON, 0, true); // UTF-8
	}
	else
	{
		fonts.channelname->RenderString(1,37, 130, name.c_str(), CLCDDisplay::PIXEL_ON, 0, true); // UTF-8
	}
	display.update();
}

void CLCDPainter::show_time()
{
	char timestr[50];
	struct timeb tm;
	//printf("[lcdd] clock event\n");
	if (showclock)
	{
		ftime(&tm);
		strftime((char*) &timestr, 20, "%H:%M", localtime(&tm.time) );

		if(mode!=CLcddTypes::MODE_STANDBY)
		{
			display.draw_fill_rect (77,50,120,64, CLCDDisplay::PIXEL_OFF);
			int pos = 122 - fonts.time->getRenderWidth(timestr);
			fonts.time->RenderString(pos,62, 50, timestr, CLCDDisplay::PIXEL_ON);
		}
		else
		{
			//big clock
			struct tm *t = localtime(&tm.time);

			display.draw_fill_rect (-1,-1,120,64, CLCDDisplay::PIXEL_OFF);
			ShowNewClock(&display, t->tm_hour, t->tm_min, t->tm_wday, t->tm_mday, t->tm_mon);
//			showBigClock(&display, t->tm_hour,t->tm_min);
			/*
			fonts.menutitle->RenderString(60,62, 60, timestr, CLCDPainterisplay::PIXEL_ON);
			*/
		}
		display.update();
	}
}

void CLCDPainter::show_volume(char vol)
{
	volume = vol;
	if ((mode==CLcddTypes::MODE_TVRADIO) || (mode==CLcddTypes::MODE_SCART))
	{
		display.draw_fill_rect (11,53,73,61, CLCDDisplay::PIXEL_OFF);
		//strichlin
		if (muted)
		{
			display.draw_line (12,55,73,60, CLCDDisplay::PIXEL_ON);
		}
		else
		{
			int dp = int( vol/100.0*61.0+12.0);
			display.draw_fill_rect (11,54,dp,60, CLCDDisplay::PIXEL_ON);
		}

		display.update();
	}
}

void CLCDPainter::show_menu(int position, char* text, int highlight )
{
	if (mode != CLcddTypes::MODE_MENU)
	{
		return;
	}
	// reload specified line
	display.draw_fill_rect(-1,35+14*position,120,35+14+14*position, CLCDDisplay::PIXEL_OFF);
	fonts.menu->RenderString(0,35+11+14*position, 140, text , CLCDDisplay::PIXEL_INV, highlight);
	display.update();
}

bool CLCDPainter::set_mode(CLcddTypes::mode m, char *title)
{
	bool shall_exit = false;
	switch (m)
	{
		case CLcddTypes::MODE_TVRADIO:
			setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
			//printf("[lcdd] mode: tvradio\n");
			display.load_screen(&icon_lcd);
			mode = m;
			showclock = true;
			show_volume(volume);
			show_servicename(servicename);
			show_time();
			display.update();
			break;
		case CLcddTypes::MODE_SCART:
			setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
			//printf("[lcdd] mode: scart\n");
			display.load_screen(&icon_lcd);
			mode = m;
			showclock = true;
			show_volume(volume);
			show_time();
			display.update();
			break;
		case CLcddTypes::MODE_MENU:
		case CLcddTypes::MODE_MENU_UTF8:
			setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
			//printf("[lcdd] mode: menu\n");
			mode = m;
			showclock = false;
			display.load_screen(&icon_setup);
			fonts.menutitle->RenderString(-1,28, 140, title, CLCDDisplay::PIXEL_ON, 0, m == CLcddTypes::MODE_MENU_UTF8);
			display.update();
			break;
		case CLcddTypes::MODE_SHUTDOWN:
			setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
			//printf("[lcdd] mode: shutdown\n");
			mode = m;
			showclock = false;
			display.load_screen(&icon_power);
			display.update();
			shall_exit = true;
			break;
		case CLcddTypes::MODE_STANDBY:
			//printf("[lcdd] mode: standby\n");
			setlcdparameter(lcd_standbybrightness, lcd_contrast, lcd_power, lcd_inverse);
			mode = m;
			showclock = true;
			display.draw_fill_rect (-1,0,120,64, CLCDDisplay::PIXEL_OFF);
			show_time();
			display.update();
			break;

		default:
			printf("[lcdd] Unknown mode: %i\n", m);
	}
	return shall_exit;
}

void CLCDPainter::setlcdparameter(int dimm, int contrast, int power, int inverse)
{
	int fp, fd;
	if (power==0)
		dimm=0;

	if ((fp = open("/dev/dbox/fp0",O_RDWR)) <= 0)
	{
		perror("[lcdd] pen '/dev/dbox/fp0' failed!");
	}

	if ((fd = open("/dev/dbox/lcd0",O_RDWR)) <= 0)
	{
		perror("[lcdd] open '/dev/dbox/lcd0' failed!");
	}

	if (ioctl(fp,FP_IOCTL_LCD_DIMM, &dimm) < 0)
	{
		perror("[lcdd] set dimm failed!");
	}
	
	if (ioctl(fd,LCD_IOCTL_SRV, &contrast) < 0)
	{
		perror("[lcdd] set contrast failed!");
	}

	if (ioctl(fd,LCD_IOCTL_ON, &power) < 0)
	{
		perror("[lcdd] set power failed!");
	}

	if (ioctl(fd,LCD_IOCTL_REVERSE, &inverse) < 0)
	{
		perror("[lcdd] set invert failed!");
	}

	close(fp);
	close(fd);
}

void CLCDPainter::setBrightness(int bright)
{
	lcd_brightness = bright;
}

int CLCDPainter::getBrightness()
{
	return lcd_brightness;
}

void CLCDPainter::setBrightnessStandby(int bright)
{
	lcd_standbybrightness = bright;
}

int CLCDPainter::getBrightnessStandby()
{
	return lcd_standbybrightness;
}

void CLCDPainter::setContrast(int contrast)
{
	lcd_contrast = contrast;
}

int CLCDPainter::getContrast()
{
	return lcd_contrast;
}

void CLCDPainter::setPower(int power)
{
	lcd_power = power;
}

int CLCDPainter::getPower()
{
	return lcd_power;
}

void CLCDPainter::setInverse(int inverse)
{
	lcd_inverse = inverse;
}

int CLCDPainter::getInverse()
{
	return lcd_inverse;
}

void CLCDPainter::setMuted(bool mu)
{
	muted = mu;
	show_volume(volume);
}

void CLCDPainter::update()
{
	setlcdparameter(lcd_brightness, lcd_contrast, lcd_power, lcd_inverse);
}

void CLCDPainter::resume()
{
	display.resume();
}

void CLCDPainter::pause()
{
	display.pause();
}
