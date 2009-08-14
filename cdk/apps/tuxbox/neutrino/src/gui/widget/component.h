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


#ifndef __component__
#define __component__


class CComponent
{
	protected:
		bool		bHasFocus;
		bool		bIsActivated;
		CComponent*	cNextFocusableElement;

		//statische Funktion um das letzte Element das Fokus hatte zu ermitteln
		static CComponent* setGlobalCurrentFocusElement( CComponent* ); 

	public:
		CComponent();
	
		//Funktionen zum setzen des Fokus
		void setFocus(bool focus = true);
		void setNextFocusableElement(CComponent*);
		void setNextFocus();

		//Abfrage des Fokus
		bool hasFocus();

		//überschreibbare Fokus-Events
		virtual void onGainFocus();
		virtual void onLostFocus();

		//Aktivieren des Elements
		void setActivated(bool active=true);
		bool isActivated();

		//überschreibbare Aktivierungs-Events
		virtual void onGetActivated();
		virtual void onGetDeActivated();

		//Paint-Funktionen
		virtual void paint();
		virtual void hide();
};


class CDimension
{
	private:
		int iWidth;
		int iHeight;
	public:
		CDimension(int width=0, int height=0);

		int getWidth();
		int getHeight();

		void setWidth(int width);
		void setHeight(int height);

		void addWidth(int width);
		void addHeight(int height);
};


class CPoint
{
	private:
		int iXPos;
		int iYPos;
	public:
		CPoint(int xPos=0, int yPos=0);

		int getXPos();
		int getYPos();

		void setXPos(int xPos);
		void setYPos(int yPos);

		void addXPos(int xPos);
		void addYPos(int yPos);
};


#endif
