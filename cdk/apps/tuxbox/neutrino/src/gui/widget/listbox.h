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


#ifndef __listbox__
#define __listbox__

#include <string>

#include <neutrinoMessages.h>

#include <driver/framebuffer.h>

#include "menue.h"
#include "messagebox.h"


using namespace std;

class CListBox : public CMenuWidget
{
	protected:
		CFrameBuffer*	frameBuffer;
		string		caption;
		string		saveBoxCaption, saveBoxText;

		int		width;
		int		height;
		int		x;
		int		y;

		int		fheight;
		int		theight;

		unsigned int	selected;
		unsigned int	liststart;
		unsigned int	listmaxshow;
	
		unsigned int	numwidth;
		int 		ButtonHeight;

		bool		toSave;

		//----------------------------

		void setTitle( string title );

		virtual void paintItem(int pos);
		virtual void paint();
		virtual	void paintHead();
		virtual void paintFoot();
		virtual void hide();

		
		//------hier Methoden überschreiben-------

		//------Fernbedienungsevents--------------
		virtual void onRedKeyPressed(){};
		virtual void onGreenKeyPressed(){};
		virtual void onYellowKeyPressed(){};
		virtual void onBlueKeyPressed(){};
		virtual void onOkKeyPressed(){};
		virtual void onOtherKeyPressed( int key ){};

		//------gibt die Anzahl der Listenitems---
		virtual unsigned int getItemCount();

		//------malen der Items-------------------
		virtual int getItemHeight();
		virtual void paintItem(uint itemNr, int paintNr, bool selected);

		//------Benutzung von setModified---------
		void setModified( bool modified = true );
		void setSaveDialogText(string title, string text);
		virtual void onSaveData(){};



	public:
		//konstruktor UNBEDINGT aufrufen!
		CListBox(); 
		virtual int exec(CMenuTarget* parent, string actionKey);
};


#endif
