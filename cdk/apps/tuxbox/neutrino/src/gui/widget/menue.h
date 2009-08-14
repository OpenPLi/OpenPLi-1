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


#ifndef __MENU__
#define __MENU__

#include <string>
#include <vector>

#include <driver/framebuffer.h>
#include <driver/rcinput.h>


using namespace std;

struct menu_return
{
	enum
		{
			RETURN_NONE	= 0,
			RETURN_REPAINT 	= 1,
			RETURN_EXIT 	= 2,
			RETURN_EXIT_ALL = 4
		};
};

class CChangeObserver
{
	public:
		virtual ~CChangeObserver(){}
		virtual bool changeNotify(string OptionName, void *Data)
		{
			return false;
		}
};

class COnPaintNotifier
{
	public:
		virtual ~COnPaintNotifier(){}
		virtual bool onPaintNotify(string MenuName)
		{
			return false;
		}
};

class CMenuTarget
{
	public:

		CMenuTarget(){}
		virtual ~CMenuTarget(){}
		virtual void hide(){}
		virtual int exec(CMenuTarget* parent, string actionKey)
		{
			return 0;
		}
};


class CMenuItem
{
	protected:
		CFrameBuffer	*frameBuffer;
		int x, y, dx, offx;
		bool		active;
	public:
		int		directKey;
		string	iconName;

		CMenuItem()
		{
			directKey = -1;
			iconName = "";
		}
		virtual ~CMenuItem(){}

		virtual void init(int X, int Y, int DX, int OFFX)
		{
			x=X;
			y=Y;
			dx=DX;
			offx=OFFX;
		}
		virtual int paint(bool selected=false)
		{
			return -1;
		}
		virtual int getHeight()
		{
			return -1;
		}
		virtual bool isSelectable()
		{
			return false;
		}

		virtual int exec(CMenuTarget* parent)
		{
			return 0;
		}
		virtual void setActive( bool Active)
		{
			active = Active;
			paint();
		};
};

class CMenuSeparator : public CMenuItem
{
		int		height;
		int		type;
		string		text;

	public:
		enum
		{
		    EMPTY =	0,
		    LINE =	1,
		    STRING =	2,
		    ALIGN_CENTER = 4,
		    ALIGN_LEFT =   8,
		    ALIGN_RIGHT = 16
		};


		CMenuSeparator(int Type=0, string Text="");

		int paint(bool selected=false);
		int getHeight()
		{
			return height;
		}
};

class CMenuForwarder : public CMenuItem
{
		int		height;
		string		text;
		char*		option;
		CMenuTarget*	jumpTarget;
		string		actionKey;
		bool		localizing;
	public:

		CMenuForwarder(string Text, bool Active=true, char *Option=NULL, CMenuTarget* Target=NULL, string ActionKey="", bool Localizing= true, uint DirectKey= CRCInput::RC_nokey, string IconName= "");
		int paint(bool selected=false);
		int getHeight()
		{
			return height;
		}
		int exec(CMenuTarget* parent);
		bool isSelectable()
		{
			return active;
		}
};

class CMenuOptionChooser : public CMenuItem
{
		struct keyval
		{
			int key;
			string value;
		};

		vector<keyval*>    options;
		int                height;
		string             optionName;
		int*               optionValue;
		CChangeObserver*   observ;
		bool               localizing;

	public:
		CMenuOptionChooser(){}
		CMenuOptionChooser(string OptionName, int* OptionValue, bool Active = false, CChangeObserver* Observ = NULL, bool Localizing= true, uint DirectKey= CRCInput::RC_nokey, string IconName= "");
		~CMenuOptionChooser();


		void addOption(int key, string value);
		void removeAllOptions();
		void setOptionValue(int val);
		int getOptionValue();

		int paint(bool selected);
		int getHeight()
		{
			return height;
		}
		bool isSelectable()
		{
			return active;
		}

		int exec(CMenuTarget* parent);
};

class CMenuOptionStringChooser : public CMenuItem
{
		vector<string>	options;
		int				height;
		string			optionName;
		char*			optionValue;
		CChangeObserver*	observ;
		bool               localizing;

	public:
		CMenuOptionStringChooser(){}
		CMenuOptionStringChooser(string OptionName, char* OptionValue, bool Active = false, CChangeObserver* Observ = NULL, bool Localizing= true);
		~CMenuOptionStringChooser();

		void addOption( string value);
		int paint(bool selected);
		int getHeight()
		{
			return height;
		}
		bool isSelectable()
		{
			return active;
		}

		int exec(CMenuTarget* parent);
};


class CMenuWidget : public CMenuTarget
{
	protected:
		CFrameBuffer		*frameBuffer;
		COnPaintNotifier*	onPaintNotifier;
		vector<CMenuItem*>	items;
		string			name;
		string			iconfile;
		bool			localizing;

		int			width;
		int			height;
		int			x;
		int			y;
		int			selected;
		int 			iconOffset;

	public:
		CMenuWidget()
		{
			name="";
			iconfile="";
			selected=-1;
			onPaintNotifier=NULL;
			iconOffset= 0;
		};
		CMenuWidget(string Name, string Icon="", int mwidth=400, int mheight=390, bool Localizing=true);
		~CMenuWidget();

		virtual void addItem(CMenuItem* menuItem, bool defaultselected=false);
		virtual void paint();
		virtual void hide();
		virtual int exec(CMenuTarget* parent, string actionKey);

		void setOnPaintNotifier( COnPaintNotifier* );
		void setName(string Name)
		{
			name=Name;
		}
		void setIcon(string Icon)
		{
			iconfile=Icon;
		}
};

class CPINProtection
{
	protected:
		char* validPIN;
		bool check();
		virtual CMenuTarget* getParent() = NULL;
	public:
		CPINProtection( char* validpin){ validPIN = validpin;};
};

class CZapProtection : public CPINProtection
{
	protected:
		virtual CMenuTarget* getParent() { return( NULL);};
	public:
		int	fsk;

		CZapProtection( char* validpin, int	FSK ) : CPINProtection(validpin){ fsk= FSK; };
		bool check();
};

class CLockedMenuForwarder : public CMenuForwarder, public CPINProtection
{
	CMenuTarget* Parent;

	protected:
		virtual CMenuTarget* getParent(){ return Parent;};
	public:
		CLockedMenuForwarder(string Text, char* validPIN, bool Active=true, char *Option=NULL,
		                     CMenuTarget* Target=NULL, string ActionKey="", bool Localizing= true,
		                     uint DirectKey= CRCInput::RC_nokey, string IconName= "")

		                     : CMenuForwarder(Text, Active, Option, Target, ActionKey, Localizing,
		                     DirectKey, IconName) ,
		                       CPINProtection( validPIN){};

		virtual int exec(CMenuTarget* parent);
};


#endif
