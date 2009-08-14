/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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


#include <global.h>
#include <neutrino.h>

#include <driver/fontrenderer.h>
#include <driver/rcinput.h>

#include <gui/color.h>

#include "lcdcontroler.h"
#include "messagebox.h"

#define BRIGHTNESSFACTOR 2.55
#define CONTRASTFACTOR 0.63


CLcdControler::CLcdControler(string Name, CChangeObserver* Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	observer = Observer;
	name = Name;
	width = 360;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight+ mheight* 3;
	x=((720-width) >> 1) -20;
	y=(576-height)>>1;

	contrast = g_lcdd->getContrast();
	brightness = g_lcdd->getBrightness();
	brightnessstandby = g_lcdd->getBrightnessStandby();
}

void CLcdControler::setLcd()
{
	printf("contrast: %d brightness: %d brightness standby: %d\n", contrast, brightness, brightnessstandby);
	g_lcdd->setBrightness(brightness);
	g_lcdd->setBrightnessStandby(brightnessstandby);
	g_lcdd->setContrast(contrast);
	g_lcdd->update();
}

int CLcdControler::exec(CMenuTarget* parent, string)
{
	int res = menu_return::RETURN_REPAINT;
	if (parent)
	{
		parent->hide();
	}
	unsigned int contrast_alt= contrast;
	unsigned int brightness_alt = brightness;
	unsigned int brightnessstandby_alt= brightnessstandby;


	setLcd();
	paint();
//	setLcd();

	int selected = 0;

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	bool loop=true;
	while (loop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd, true );

		if ( msg <= CRCInput::RC_MaxRC )
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

		switch ( msg )
		{
			case CRCInput::RC_down:
				{
					int max = 2;

					if(selected<max)
					{
						paintSlider(x+10, y+ hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", false);
						paintSlider(x+10, y+ hheight+ mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", false);
						paintSlider(x+ 10, y+ hheight+ mheight* 2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", false);
						selected++;
						switch (selected)
						{
							case 0:
								paintSlider(x+ 10, y+ hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", true);
								break;
							case 1:
								paintSlider(x+ 10, y+ hheight+ mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", true);
								break;
							case 2:
								paintSlider(x+ 10, y+ hheight+ mheight* 2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", true);
								break;
						}
					}
					break;
    	        }
			case CRCInput::RC_up:
				if(selected>0)
				{
					paintSlider(x+10, y+hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", false);
					paintSlider(x+10, y+hheight+mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", false);
					paintSlider(x+10, y+hheight+mheight*2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", false);
					selected--;
					switch (selected)
					{
						case 0:
							paintSlider(x+10, y+hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", true);
							break;
						case 1:
							paintSlider(x+10, y+hheight+mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", true);
							break;
						case 2:
							paintSlider(x+10, y+hheight+mheight*2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", true);
							break;
					}
				}
				break;

			case CRCInput::RC_right:
				switch (selected)
				{
					case 0:
						if (contrast<63)
						{
							contrast+=5;
							paintSlider(x+10, y+hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", true);
							setLcd();
						}
						break;
					case 1:
						if (brightness<245)
						{
							brightness+=10;
							paintSlider(x+10, y+hheight+mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", true);
							setLcd();
						}
						break;
					case 2:
						if (brightnessstandby<245)
						{
							brightnessstandby+=10;
							paintSlider(x+10, y+hheight+mheight*2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", true);
							setLcd();
						}
						break;
				}
				break;

			case CRCInput::RC_left:
				switch (selected)
				{
					case 0:
						if (contrast>0)
						{
							contrast-=5;
							paintSlider(x+10, y+hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", true);
							setLcd();
						}
						break;
					case 1:
						if (brightness>0)
						{
							brightness-=10;
							paintSlider(x+10, y+hheight+mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", true);
							setLcd();
						}
						break;
					case 2:
						if (brightnessstandby>0)
						{
							brightnessstandby-=10;
							paintSlider(x+10, y+hheight+mheight*2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby", true);
							setLcd();
						}
						break;
				}
				break;

			case CRCInput::RC_home:
				if ( ( (contrast != contrast_alt) || (brightness != brightness_alt) || (brightnessstandby != brightnessstandby_alt) ) &&
			    	 ( ShowMsg(name, g_Locale->getText("messagebox.discard"), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel, "", 380 ) == CMessageBox::mbrCancel ) )
					break;

				// sonst abbruch...
				g_lcdd->setContrast(contrast_alt);
				g_lcdd->setBrightness(brightness_alt);
				g_lcdd->setBrightnessStandby(brightnessstandby_alt);

			case CRCInput::RC_timeout:
			case CRCInput::RC_ok:
				loop = false;
				break;

			default:
				if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
				{
					loop = false;
					res = menu_return::RETURN_EXIT_ALL;
				}
		}
	}

	hide();

	if(observer)
		observer->changeNotify(name, NULL);

	return res;
}

void CLcdControler::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void CLcdControler::paint()
{
	frameBuffer->paintBoxRel(x,y, width,hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+hheight, width, g_Locale->getText(name).c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x,y+hheight, width,height-hheight, COL_MENUCONTENT);

	paintSlider(x+10, y+hheight, contrast, CONTRASTFACTOR, g_Locale->getText("lcdcontroler.contrast"),"contrast", true);
	paintSlider(x+10, y+hheight+mheight, brightness, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightness"),"brightness", false);
	paintSlider(x+10, y+hheight+mheight*2, brightnessstandby, BRIGHTNESSFACTOR, g_Locale->getText("lcdcontroler.brightnessstandby"),"brightnessstandby",false);

}

void CLcdControler::paintSlider(int x, int y, unsigned int spos, float factor, string text, string iconname, bool selected)
{
	int startx = 200;
	if (!spos)
		return;
	frameBuffer->paintBoxRel(x + startx ,y,120,mheight, COL_MENUCONTENT);
	frameBuffer->paintIcon("volumebody.raw",x + startx,y+2+mheight/4);
	string iconfile = "volumeslider2";
	if (selected)
		iconfile += "blue";
	iconfile +=".raw";
	frameBuffer->paintIcon(iconfile,(int)(x+ (startx+3) +(spos / factor)),y+mheight/4);

	g_Fonts->menu->RenderString(x,y+mheight, width, text.c_str(), COL_MENUCONTENT);
}
