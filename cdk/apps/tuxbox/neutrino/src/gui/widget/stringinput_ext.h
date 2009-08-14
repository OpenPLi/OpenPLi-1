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


#ifndef __stringinput_ext__
#define __stringinput_ext__

#include <string>
#include <vector>

#include <driver/framebuffer.h>

#include "menue.h"


using namespace std;

class CExtendedInput_Item;
class CExtendedInput : public CMenuTarget
{
	protected:
		CFrameBuffer	*frameBuffer;
		int x;
		int y;
		int width;
		int height;
		int hintPosY;
		int hheight; // head font height
		int mheight; // menu font height
		int iheight; 

		vector<CExtendedInput_Item*> inputFields;
		int selectedChar;

		bool localizing;
		string	name;

		string  hint_1;
		string  hint_2;
		char*	value;
		CChangeObserver*   observ;


		virtual void paint();
		virtual void onBeforeExec(){};
		virtual void onAfterExec(){};

	public:

		CExtendedInput(string Name, char* Value, string Hint_1 = "", string Hint_2 = "", CChangeObserver* Observ = NULL, bool Localizing=true);

		void hide();
		int exec( CMenuTarget* parent, string actionKey );
		void calculateDialog();

		void addInputField( CExtendedInput_Item* );
};


class CExtendedInput_Item
{
	protected:
		CFrameBuffer	*frameBuffer;
		int ix, iy, idx, idy;
		char* data;

	public:

		virtual void setDataPointer(char* Data){data=Data;};
		virtual void init(int &x, int &y){};
		virtual void paint(int x, int y, bool focusGained){};
		virtual bool isSelectable(){return true;};

		virtual void keyPressed( int key ){};
};

class CExtendedInput_Item_Spacer : public CExtendedInput_Item
{
	protected:
		int mSpacingX;
		int mSpacingY;
	public:
		CExtendedInput_Item_Spacer(){};
		CExtendedInput_Item_Spacer(int spaceX, int spaceY=0){mSpacingX=spaceX;mSpacingY=spaceY;};
		virtual void init(int &x, int &y){x+=mSpacingX;y+=mSpacingY;};
		virtual bool isSelectable(){return false;};
};

class CExtendedInput_Item_newLiner : public CExtendedInput_Item
{
	protected:
		int mSpacingY;
	public:
		CExtendedInput_Item_newLiner(){};
		CExtendedInput_Item_newLiner(int spaceY){mSpacingY=spaceY;};
		virtual void init(int &x, int &y){x=0;y+=mSpacingY;};
		virtual bool isSelectable(){return false;};
};


class CExtendedInput_Item_Char : public CExtendedInput_Item
{
	protected:
		string allowedChars;
		bool selectable;
		
		bool isAllowedChar( char );
		int getCharID( char );

	public:
		CExtendedInput_Item_Char(string Chars="", bool Selectable=true );
		virtual ~CExtendedInput_Item_Char(){};
		void setAllowedChars( string );
		virtual void init(int &x, int &y);
		virtual void paint(int x, int y, bool focusGained);

		virtual void keyPressed( int key );
		virtual bool isSelectable(){return selectable;};
};

//----------------------------------------------------------------------------------------------------

class CIPInput : public CExtendedInput
{
	protected:
		virtual void onBeforeExec();
		virtual void onAfterExec();

	public:
		CIPInput(string Name, char* Value, string Hint_1 = "", string Hint_2 = "", CChangeObserver* Observ = NULL);
};

//----------------------------------------------------------------------------------------------------

class CDateInput : public CExtendedInput
{
   private:
		time_t* time;

	protected:
		virtual void onBeforeExec();
		virtual void onAfterExec();

	public:
		CDateInput(string Name, time_t* Time, string Hint_1 = "", string Hint_2 = "", CChangeObserver* Observ = NULL);
		~CDateInput();
		char* getValue() {return value;}
};


#endif
