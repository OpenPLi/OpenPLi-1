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

#include "menue.h"
#include "stringinput.h"


bool isDigit(const char ch)
{
	if((ch>'0') && (ch<'9'))
		return true;
	else return false;
}

bool isNumber(const string& str)
{
	for (string::const_iterator i = str.begin(); i != str.end(); i++)
	{
		if (!isDigit(*i)) return false;
	}
	return true;
}


CMenuWidget::CMenuWidget(string Name, string Icon, int mwidth, int mheight, bool Localizing)
{
	frameBuffer = CFrameBuffer::getInstance();
	onPaintNotifier = NULL;
	name = Name;
	iconfile = Icon;
	selected = -1;
	width = mwidth;
	height = mheight; // height(menu_title)+10+...
	localizing = Localizing;
}

CMenuWidget::~CMenuWidget()
{
	for(unsigned int count=0;count<items.size();count++)
	{
		delete items[count];
	}
	items.clear();
}

void CMenuWidget::addItem(CMenuItem* menuItem, bool defaultselected)
{
	if (defaultselected)
		selected = items.size();
	items.insert(items.end(), menuItem);
}

void CMenuWidget::setOnPaintNotifier( COnPaintNotifier* nf )
{
	onPaintNotifier = nf;
}

int CMenuWidget::exec(CMenuTarget* parent, string)
{
	int pos;

	if (parent)
		parent->hide();

	if (onPaintNotifier)
		onPaintNotifier->onPaintNotify(name);

	paint();
	int retval = menu_return::RETURN_REPAINT;

	uint msg; uint data;
	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );

	do
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );


		if ( msg <= CRCInput::RC_MaxRC )
		{
			timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );
		}

		int handled= false;

		for (unsigned int i= 0; i< items.size(); i++)
		{
			CMenuItem* titem = items[i];
			if ( ((uint)titem->directKey!= CRCInput::RC_nokey) && ((uint)titem->directKey==msg) )
			{
				if (titem->isSelectable())
				{
					items[selected]->paint( false );
					selected= i;
					msg= CRCInput::RC_ok;
				}
				else
				{
					// swallow-key...
					handled= true;
				}
				break;
			}
		}

		if (!handled)
		{
			switch (msg)
			{

				case (CRCInput::RC_up) :
				case (CRCInput::RC_down) :
					{
						//search next / prev selectable item
						for (unsigned int count=1; count< items.size(); count++)
						{

							if (msg==CRCInput::RC_up)
							{
								pos = selected- count;
								if ( pos<0 )
									pos = items.size()-1;
							}
							else
							{
								pos = (selected+ count)%items.size();
							}

							CMenuItem* item = items[pos];

							if ( item->isSelectable() )
							{
								//clear prev. selected
								items[selected]->paint( false );
								//select new
								item->paint( true );
								selected = pos;
								break;
							}
						}
					}
					break;
				case (CRCInput::RC_ok):
					{
						//exec this item...
						CMenuItem* item = items[selected];
						int rv = item->exec( this );
						switch ( rv )
						{
							case menu_return::RETURN_EXIT_ALL:
								retval = menu_return::RETURN_EXIT_ALL;

							case menu_return::RETURN_EXIT:
								msg = CRCInput::RC_timeout;
								break;
							case menu_return::RETURN_REPAINT:
								paint();
								break;
						}
					}
					break;

				case (CRCInput::RC_home):
					msg = CRCInput::RC_timeout;
					break;

				case (CRCInput::RC_right):
					break;

				case (CRCInput::RC_left):
					msg = CRCInput::RC_timeout;
					break;

				case (CRCInput::RC_timeout):
					break;

				//close any menue on dbox-key
				case (CRCInput::RC_setup):
					{
						msg = CRCInput::RC_timeout;
						retval = menu_return::RETURN_EXIT_ALL;
					}
					break;

				default:
					if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all )
					{
						retval = menu_return::RETURN_EXIT_ALL;
						msg = CRCInput::RC_timeout;
					}
			}


			if ( msg <= CRCInput::RC_MaxRC )
			{
				// recalculate timeout für RC-Tasten
				timeoutEnd = g_RCInput->calcTimeoutEnd( g_settings.timing_menu );
			}
		}

	}
	while ( msg!=CRCInput::RC_timeout );

	hide();
	if(!parent)
	{
		g_lcdd->setMode(CLcddTypes::MODE_TVRADIO, g_Locale->getText(name));
	}

	return retval;
}

void CMenuWidget::hide()
{
	frameBuffer->paintBackgroundBoxRel(x,y-10, width,height+50 );
}

void CMenuWidget::paint()
{
	string  l_name = name;
	if(localizing)
	{
		l_name = g_Locale->getText(name);
	}
	g_lcdd->setMode(CLcddTypes::MODE_MENU, l_name);


	int neededWidth = g_Fonts->menu_title->getRenderWidth(l_name.c_str());
	if (neededWidth> width-48)
	{
		width= neededWidth+ 49;
	}

	iconOffset= 0;
	for (unsigned int i= 0; i< items.size(); i++)
	{
		if ( (items[i]->iconName!= "") ||
			 (((uint)items[i]->directKey>= CRCInput::RC_0) && ((uint)items[i]->directKey<= CRCInput::RC_9)) )
		{
			iconOffset= g_Fonts->menu->getHeight();
			break;
		}
	}

	y= ( 576 - height ) >> 1;
	x= ( ( ( g_settings.screen_EndX- g_settings.screen_StartX ) - width ) >> 1 ) + g_settings.screen_StartX;


	int hheight = g_Fonts->menu_title->getHeight();
	frameBuffer->paintBoxRel(x,y, width,hheight, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+38,y+hheight+1, width, l_name.c_str(), COL_MENUHEAD);
	frameBuffer->paintIcon(iconfile.c_str(),x+8,y+5);

	int ypos = y+hheight;

	for(unsigned int count=0;count<items.size();count++)
	{
		CMenuItem* item = items[count];
		item->init(x,ypos, width, iconOffset);
		if( (item->isSelectable()) && (selected==-1) )
		{
			ypos = item->paint(true);
			selected = count;
		}
		else
		{
			ypos = item->paint(selected==((signed int) count) );
		}
	}
	//	height = ypos - y;
}

//-------------------------------------------------------------------------------------------------------------------------------





CMenuOptionChooser::CMenuOptionChooser(string OptionName, int* OptionValue, bool Active, CChangeObserver* Observ, bool Localizing, uint DirectKey, string IconName)
{
	frameBuffer = CFrameBuffer::getInstance();
	height= g_Fonts->menu->getHeight();
	optionName = OptionName;
	active = Active;
	optionValue = OptionValue;
	observ=Observ;
	localizing= Localizing;
	directKey = DirectKey;
	iconName = IconName;
}


CMenuOptionChooser::~CMenuOptionChooser()
{
	for(unsigned int count=0;count<options.size();count++)
	{
		delete options[count];
	}
	options.clear();
}

void CMenuOptionChooser::addOption(int key, string value)
{
	keyval *tmp = new keyval();
	tmp->key=key;
	tmp->value=value;
	options.insert(options.end(), tmp);
}

void CMenuOptionChooser::removeAllOptions()
{
	for(unsigned int count=0;count<options.size();count++)
	{
		keyval* kv = options[count];
		delete kv;
	}
	options.clear();
}

void CMenuOptionChooser::setOptionValue(int val)
{
	*optionValue = val;
}

int CMenuOptionChooser::getOptionValue()
{
	return *optionValue;
}


int CMenuOptionChooser::exec(CMenuTarget*)
{
	for(unsigned int count=0;count<options.size();count++)
	{
		keyval* kv = options[count];
		if(kv->key == *optionValue)
		{
			*optionValue = options[ (count+1)%options.size() ]->key;
			break;
		}
	}
	paint(true);
	if(observ)
	{
		observ->changeNotify( optionName, optionValue );
	}
	return menu_return::RETURN_NONE;
}

int CMenuOptionChooser::paint( bool selected )
{
	unsigned char color = COL_MENUCONTENT;
	if (selected)
		color = COL_MENUCONTENTSELECTED;
	if (!active)
		color = COL_MENUCONTENTINACTIVE;

	frameBuffer->paintBoxRel(x,y, dx, height, color );

	string option = "error";

	for(unsigned int count=0;count<options.size();count++)
	{
		keyval* kv = options[count];
		if(kv->key == *optionValue)
		{
			option = kv->value;
			break;
		}
	}

	if (iconName!="")
	{
		frameBuffer->paintIcon(iconName.c_str(), x + 10, y+ ((height- 20)>>1) );
	}
	else if (((uint)directKey>= CRCInput::RC_0) && ((uint)directKey<= CRCInput::RC_9))
	{
		//number
		char tmp[10];
		sprintf((char*) tmp, "%d", directKey);

		g_Fonts->channellist_number->RenderString(x + 10, y+ height, height, tmp, color, height);
	}


	string  l_optionName = g_Locale->getText(optionName);
	string  l_option;
	if ( localizing && !isNumber(option))
		l_option = g_Locale->getText(option);
	else
		l_option = option;

	int stringwidth = g_Fonts->menu->getRenderWidth(l_option.c_str());
	int stringstartposName = x + offx + 10;
	int stringstartposOption = x + dx - stringwidth - 10; //+ offx

	g_Fonts->menu->RenderString(stringstartposName,   y+height,dx- (stringstartposName - x), l_optionName.c_str(), color);
	g_Fonts->menu->RenderString(stringstartposOption, y+height,dx- (stringstartposOption - x), l_option.c_str(), color);

	if(selected)
	{
		g_lcdd->setMenuText(0, l_optionName);
		g_lcdd->setMenuText(1, l_option);
	}

	return y+height;
}


//-------------------------------------------------------------------------------------------------------------------------------

CMenuOptionStringChooser::CMenuOptionStringChooser(string OptionName, char* OptionValue, bool Active, CChangeObserver* Observ, bool Localizing)
{
	frameBuffer = CFrameBuffer::getInstance();
	height= g_Fonts->menu->getHeight();
	optionName = OptionName;
	active = Active;
	optionValue = OptionValue;
	observ=Observ;
	localizing= Localizing;

	directKey = CRCInput::RC_nokey;
	iconName = "";
}


CMenuOptionStringChooser::~CMenuOptionStringChooser()
{
	options.clear();
}

void CMenuOptionStringChooser::addOption( string value)
{
	options.insert(options.end(), value);
}

int CMenuOptionStringChooser::exec(CMenuTarget*)
{
	bool wantsRepaint = false;
	//select next value
	for(unsigned int count=0;count<options.size();count++)
	{
		string actOption = options[count];
		if(!strcmp( actOption.c_str(), optionValue))
		{
			strcpy(optionValue, options[ (count+1)%options.size() ].c_str());
			break;
		}
	}

	paint(true);
	if(observ)
	{
		wantsRepaint = observ->changeNotify( optionName, optionValue );
	}
	if ( wantsRepaint )
		return menu_return::RETURN_REPAINT;
	else
		return menu_return::RETURN_NONE;
}

int CMenuOptionStringChooser::paint( bool selected )
{
	unsigned char color = COL_MENUCONTENT;
	if (selected)
		color = COL_MENUCONTENTSELECTED;
	if (!active)
		color = COL_MENUCONTENTINACTIVE;

	frameBuffer->paintBoxRel(x,y, dx, height, color );

	string  l_optionName = g_Locale->getText(optionName);
	string  l_option;
	if ( localizing )
		l_option = g_Locale->getText(optionValue);
	else
		l_option = optionValue;

	int stringwidth = g_Fonts->menu->getRenderWidth(l_option.c_str());
	int stringstartposName = x + offx + 10;
	int stringstartposOption = x + dx - stringwidth - 10; //+ offx

	g_Fonts->menu->RenderString(stringstartposName,   y+height,dx- (stringstartposName - x), l_optionName.c_str(), color);
	g_Fonts->menu->RenderString(stringstartposOption, y+height,dx- (stringstartposOption - x), l_option.c_str(), color);

	if(selected)
	{
		g_lcdd->setMenuText(0, l_optionName);
		g_lcdd->setMenuText(1, l_option);
	}

	return y+height;
}



//-------------------------------------------------------------------------------------------------------------------------------
CMenuForwarder::CMenuForwarder(string Text, bool Active, char* Option, CMenuTarget* Target, string ActionKey, bool Localizing, uint DirectKey, string IconName)
{
	frameBuffer = CFrameBuffer::getInstance();
	height=g_Fonts->menu->getHeight();
	text=Text;
	option = Option;
	active = Active;
	jumpTarget = Target;
	actionKey = ActionKey;
	localizing = Localizing;
	directKey = DirectKey;
	iconName = IconName;
}

int CMenuForwarder::exec(CMenuTarget* parent)
{
	if(jumpTarget)
	{
		return jumpTarget->exec(parent, actionKey);
	}
	else
	{
		return menu_return::RETURN_EXIT;
	}
}


int CMenuForwarder::paint(bool selected)
{
	string  l_text;

	if ( localizing )
		l_text = g_Locale->getText(text);
	else
		l_text = text;

	int stringstartposX = x + offx + 10;

	if(selected)
	{
		g_lcdd->setMenuText(0, l_text);

		if (option)
			g_lcdd->setMenuText(1, option);
		else
			g_lcdd->setMenuText(1, "");
	}

	unsigned char color = COL_MENUCONTENT;
	if (selected)
		color = COL_MENUCONTENTSELECTED;
	if (!active)
		color = COL_MENUCONTENTINACTIVE;

	frameBuffer->paintBoxRel(x,y, dx, height, color );
	g_Fonts->menu->RenderString(stringstartposX, y+ height, dx- (stringstartposX - x),  l_text.c_str(), color);

	if (iconName!="")
	{
		frameBuffer->paintIcon(iconName.c_str(), x + 10, y+ ((height- 20)>>1) );
	}
	else if (((uint)directKey>= CRCInput::RC_0) && ((uint)directKey<= CRCInput::RC_9))
	{
		//number
		char tmp[10];
		sprintf((char*) tmp, "%d", directKey);

		g_Fonts->channellist_number->RenderString(x + 10, y+ height, height, tmp, color, height);
	}

	if(option)
	{
		int stringwidth = g_Fonts->menu->getRenderWidth(option);
		int stringstartposOption = x + dx - stringwidth - 10; //+ offx

		g_Fonts->menu->RenderString(stringstartposOption, y+height,dx- (stringstartposOption- x),  option, color);
	}

	return y+ height;
}

//-------------------------------------------------------------------------------------------------------------------------------
CMenuSeparator::CMenuSeparator(int Type, string Text)
{
	frameBuffer = CFrameBuffer::getInstance();
	directKey = CRCInput::RC_nokey;
	iconName = "";

	height = g_Fonts->menu->getHeight();
	if(Text=="")
	{
		height = 10;
	}
	text = Text;

	if ( (Type & ALIGN_LEFT) || (Type & ALIGN_CENTER) || (Type & ALIGN_RIGHT) )
	{
		type=Type;
	}
	else
	{
		type= Type | ALIGN_CENTER;
	}
}


int CMenuSeparator::paint(bool selected)
{
	frameBuffer->paintBoxRel(x,y, dx, height, COL_MENUCONTENT );
	if(type&LINE)
	{
		frameBuffer->paintHLineRel(x+10,dx-20,y+(height>>1), COL_MENUCONTENT+3 );
		frameBuffer->paintHLineRel(x+10,dx-20,y+(height>>1)+1, COL_MENUCONTENT+1 );
	}
	if(type&STRING)
	{
		string  l_text = g_Locale->getText(text);
		int stringwidth = g_Fonts->menu->getRenderWidth(l_text.c_str());
		int stringstartposX = 0;

		if(type&ALIGN_CENTER)
		{
			stringstartposX = (x + (dx >> 1)) - (stringwidth>>1);
		}
		else if(type&ALIGN_LEFT)
		{
			stringstartposX = x + 20;
		}
		else if(type&ALIGN_RIGHT)
		{
			stringstartposX = x + dx - stringwidth - 20;
		}

		frameBuffer->paintBoxRel(stringstartposX-5, y, stringwidth+10, height, COL_MENUCONTENT );

		g_Fonts->menu->RenderString(stringstartposX, y+height,dx- (stringstartposX- x) , l_text.c_str(), COL_MENUCONTENTINACTIVE);

		if(selected)
		{
			g_lcdd->setMenuText(0, l_text);
			g_lcdd->setMenuText(1, "");
		}
	}
	return y+ height;
}

bool CPINProtection::check()
{
	char cPIN[4] = "";
	char hint[100] = "";
	do
	{
		strcpy( cPIN, "");
		CPINInput* PINInput = new CPINInput( "pinprotection.head", cPIN, 4, hint);
		PINInput->exec( getParent(), "");
		delete PINInput;
		strcpy( hint, "pinprotection.wrongcode");
	} while ((strncmp(cPIN,validPIN,4) != 0) && ( string(cPIN) != ""));
	return ( strncmp(cPIN,validPIN,4) == 0);
}


bool CZapProtection::check()
{
	int res;
	char cPIN[5] = "";
	string hint2;

	do
	{
		strcpy( cPIN, "" );

		CPLPINInput* PINInput = new CPLPINInput( "parentallock.head", cPIN, 4, hint2.c_str(), fsk );

		res = PINInput->exec( getParent(), "");
		delete PINInput;

		hint2= "pinprotection.wrongcode";
	} while ( (strncmp(cPIN,validPIN,4) != 0) &&
			  ( string(cPIN) != "" ) &&
			  ( res == menu_return::RETURN_REPAINT ) &&
			  ( fsk >= g_settings.parentallock_lockage ) );
	return ( ( strncmp(cPIN,validPIN,4) == 0 ) ||
			 ( fsk < g_settings.parentallock_lockage ) );
}

int CLockedMenuForwarder::exec(CMenuTarget* parent)
{
	Parent = parent;
	if (check())
	{
		Parent = NULL;
		return CMenuForwarder::exec(parent);
	}
	else
	{
		Parent = NULL;
		return menu_return::RETURN_REPAINT;
	}
}
