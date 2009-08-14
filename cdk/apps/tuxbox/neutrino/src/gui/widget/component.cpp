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


#include "component.h"


CComponent::CComponent()
{
	bHasFocus = false;
	bIsActivated = false;
	cNextFocusableElement = 0;
}

CComponent* CComponent::setGlobalCurrentFocusElement(CComponent* focusElement)
{
	static CComponent* currentFocusElement = 0;
	CComponent* returnVal = currentFocusElement;
	if(focusElement)
	{
		currentFocusElement = focusElement;
	}
	return returnVal;
}

void CComponent::setFocus(bool focus)
{
	//vorherig aktives Element abfragen
	CComponent* lastFocusElement = setGlobalCurrentFocusElement(this);
	if(lastFocusElement)
	{
		//und diesem den Fokus nehmen
		lastFocusElement->setFocus(false);
	}
	//dieses Element auf Fokus setzen
	bHasFocus = focus;
	if(bHasFocus)
	{
		onGainFocus();
	}
	else
	{
		onLostFocus();
	}
}

bool CComponent::hasFocus()
{
	return bHasFocus;
}

void CComponent::setNextFocusableElement(CComponent* nextFocusableElement)
{
	cNextFocusableElement = nextFocusableElement;
}

void CComponent::setNextFocus()
{
	if(cNextFocusableElement)
	{
		cNextFocusableElement->setFocus();
	}
}

void CComponent::setActivated(bool active)
{
	bIsActivated = active;
	if(active)
	{
		onGetActivated();
	}
	else
	{
		onGetDeActivated();
	}
}

bool CComponent::isActivated()
{
	return bIsActivated;
}

void CComponent::onGainFocus()
{
}

void CComponent::onLostFocus()
{		
}

void CComponent::onGetActivated()
{
}

void CComponent::onGetDeActivated()
{
}

void CComponent::paint()
{
}

void CComponent::hide()
{
}

//CDimension-----------------------------------------------------------------------------------------------
CDimension::CDimension(int width, int height)
{
	iWidth = width;
	iHeight = height;
}

int CDimension::getWidth()
{
	return iWidth;
}

int CDimension::getHeight()
{
	return iHeight;
}

void CDimension::setWidth(int width)
{
	iWidth = width;
}

void CDimension::setHeight(int height)
{
	iHeight = height;
}

void CDimension::addWidth(int width)
{
	iWidth += width;
}

void CDimension::addHeight(int height)
{
	iHeight += height;
}

//CPoint---------------------------------------------------------------------------------------------------
CPoint::CPoint(int xPos, int yPos)
{
	iXPos = xPos;
	iYPos = yPos;
}

int CPoint::getXPos()
{
	return iXPos;
}

int CPoint::getYPos()
{
	return iYPos;
}

void CPoint::setXPos(int xPos)
{
	iXPos = xPos;
}

void CPoint::setYPos(int yPos)
{
	iYPos = yPos;
}

void CPoint::addXPos(int xPos)
{
	iXPos += xPos;
}

void CPoint::addYPos(int yPos)
{
	iYPos += yPos;
}
