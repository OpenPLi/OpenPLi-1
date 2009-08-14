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

#include "colorchooser.h"
#include "messagebox.h"


CColorChooser::CColorChooser(string Name, unsigned char *R, unsigned char *G, unsigned char *B, unsigned char* Alpha, CChangeObserver* Observer)
{
	frameBuffer = CFrameBuffer::getInstance();
	observer = Observer;
	name = Name;
	width = 360;
	hheight = g_Fonts->menu_title->getHeight();
	mheight = g_Fonts->menu->getHeight();
	height = hheight+ mheight* 4;
	x=((720-width) >> 1) -20;
	y=(576-height)>>1;

	r = R;
	g = G;
	b = B;
	alpha = Alpha;
}

void CColorChooser::setColor()
{
	int color = convertSetupColor2RGB(*r,*g, *b);
	if(!alpha)
	{
		frameBuffer->paletteSetColor(254, color, 0);
		frameBuffer->paletteSet();
	}
	else
	{
		int tAlpha = convertSetupAlpha2Alpha( *alpha );
		frameBuffer->paletteSetColor(254, color, tAlpha);
		frameBuffer->paletteSet();
	}
	/*
	char colorstr[30];
	sprintf((char*)&colorstr, "%02x.%02x.%02x", *r, *g, *b);
	frameBuffer->paintBoxRel(x+218,y+107, 80, 20, COL_MENUCONTENT);
	fonts->epg_date->RenderString(x+218,y+120, 80, colorstr, COL_MENUCONTENT);
	*/
}

int CColorChooser::exec(CMenuTarget* parent, string)
{
	int res = menu_return::RETURN_REPAINT;
	if (parent)
	{
		parent->hide();
	}
	unsigned char r_alt= *r;
	unsigned char g_alt= *g;
	unsigned char b_alt= *b;
	unsigned char a_alt = 0;
	if (alpha)
		a_alt= *alpha;


	setColor();
	paint();
	setColor();

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
					int max = 3;
					if (alpha==NULL)
						max=2;

					if(selected<max)
					{
						paintSlider(x+10, y+ hheight, r, g_Locale->getText("colorchooser.red"),"red", false);
						paintSlider(x+10, y+ hheight+ mheight, g, g_Locale->getText("colorchooser.green"),"green", false);
						paintSlider(x+ 10, y+ hheight+ mheight* 2, b, g_Locale->getText("colorchooser.blue"),"blue", false);
						paintSlider(x+ 10, y+ hheight+ mheight* 3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",false);
						selected++;
						switch (selected)
						{
							case 0:
								paintSlider(x+ 10, y+ hheight, r, g_Locale->getText("colorchooser.red"),"red", true);
								break;
							case 1:
								paintSlider(x+ 10, y+ hheight+ mheight, g, g_Locale->getText("colorchooser.green"),"green", true);
								break;
							case 2:
								paintSlider(x+ 10, y+ hheight+ mheight* 2, b, g_Locale->getText("colorchooser.blue"),"blue", true);
								break;
							case 3:
								paintSlider(x+ 10, y+ hheight+ mheight* 3, alpha, g_Locale->getText("colorchooser.alpha"),"alpha", true);
								break;
						}
					}
					break;
    	        }
			case CRCInput::RC_up:
				if(selected>0)
				{
					paintSlider(x+10, y+hheight, r,g_Locale->getText("colorchooser.red"),"red", false);
					paintSlider(x+10, y+hheight+mheight, g,g_Locale->getText("colorchooser.green"),"green", false);
					paintSlider(x+10, y+hheight+mheight*2, b,g_Locale->getText("colorchooser.blue"),"blue", false);
					paintSlider(x+10, y+hheight+mheight*3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",false);
					selected--;
					switch (selected)
					{
						case 0:
							paintSlider(x+10, y+hheight, r,g_Locale->getText("colorchooser.red"),"red", true);
							break;
						case 1:
							paintSlider(x+10, y+hheight+mheight, g,g_Locale->getText("colorchooser.green"),"green", true);
							break;
						case 2:
							paintSlider(x+10, y+hheight+mheight*2, b,g_Locale->getText("colorchooser.blue"),"blue", true);
							break;
						case 3:
							paintSlider(x+10, y+hheight+mheight*3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",true);
							break;
					}
				}
				break;

			case CRCInput::RC_right:
				switch (selected)
				{
					case 0:
						if (*r<100)
						{
							*r+=5;
							paintSlider(x+10, y+hheight, r,g_Locale->getText("colorchooser.red"),"red", true);
							setColor();
						}
						break;
					case 1:
						if (*g<100)
						{
							*g+=5;
							paintSlider(x+10, y+hheight+mheight, g,g_Locale->getText("colorchooser.green"),"green", true);
							setColor();
						}
						break;
					case 2:
						if (*b<100)
						{
							*b+=5;
							paintSlider(x+10, y+hheight+mheight*2, b,g_Locale->getText("colorchooser.blue"),"blue", true);
							setColor();
						}
						break;
					case 3:
						if (*alpha<100)
						{
							*alpha+=5;
							paintSlider(x+10, y+hheight+mheight*3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",true);
							setColor();
						}
						break;
				}
				break;

			case CRCInput::RC_left:
				switch (selected)
				{
					case 0:
						if (*r>0)
						{
							*r-=5;
							paintSlider(x+10, y+hheight, r,g_Locale->getText("colorchooser.red"),"red", true);
							setColor();
						}
						break;
					case 1:
						if (*g>0)
						{
							*g-=5;
							paintSlider(x+10, y+hheight+mheight, g,g_Locale->getText("colorchooser.green"),"green", true);
							setColor();
						}
						break;
					case 2:
						if (*b>0)
						{
							*b-=5;
							paintSlider(x+10, y+hheight+mheight*2, b,g_Locale->getText("colorchooser.blue"),"blue", true);
							setColor();
						}
						break;
					case 3:
						if (*alpha>0)
						{
							*alpha-=5;
							paintSlider(x+10, y+hheight+mheight*3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",true);
							setColor();
						}
						break;
				}
				break;

			case CRCInput::RC_home:
				if ( ( (*r != r_alt) || (*g != g_alt) || (*b != b_alt) || ( (alpha) && (*alpha != a_alt) ) ) &&
			    	 ( ShowMsg(name, g_Locale->getText("messagebox.discard"), CMessageBox::mbrYes, CMessageBox::mbYes | CMessageBox::mbCancel, "", 380 ) == CMessageBox::mbrCancel ) )
					break;

				// sonst abbruch...
				*r = r_alt;
				*g = g_alt;
				*b = b_alt;
				if (alpha)
					*alpha= a_alt;

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

void CColorChooser::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y, width,height);
}

void CColorChooser::paint()
{
	frameBuffer->paintBoxRel(x,y, width,hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+hheight, width, g_Locale->getText(name).c_str(), COL_MENUHEAD);
	frameBuffer->paintBoxRel(x,y+hheight, width,height-hheight, COL_MENUCONTENT);

	paintSlider(x+10, y+hheight, r,g_Locale->getText("colorchooser.red"),"red", true);
	paintSlider(x+10, y+hheight+mheight, g,g_Locale->getText("colorchooser.green"),"green", false);
	paintSlider(x+10, y+hheight+mheight*2, b,g_Locale->getText("colorchooser.blue"),"blue",false);
	paintSlider(x+10, y+hheight+mheight*3, alpha,g_Locale->getText("colorchooser.alpha"),"alpha",false);

	//color preview
	frameBuffer->paintBoxRel(x+220,y+hheight+5,    mheight*4,   mheight*4-10,   COL_MENUHEAD);
	frameBuffer->paintBoxRel(x+222,y+hheight+2+5,  mheight*4-4 ,mheight*4-4-10, 254);
}

void CColorChooser::paintSlider(int x, int y, unsigned char *spos, string text, string iconname, bool selected)
{
	if (!spos)
		return;
	frameBuffer->paintBoxRel(x+70,y,120,mheight, COL_MENUCONTENT);
	frameBuffer->paintIcon("volumebody.raw",x+70,y+2+mheight/4);
	string iconfile = "volumeslider2";
	if (selected)
		iconfile += iconname;
	iconfile +=".raw";
	frameBuffer->paintIcon(iconfile,x+73+(*spos),y+mheight/4);

	g_Fonts->menu->RenderString(x,y+mheight, width, text.c_str(), COL_MENUCONTENT);
}
