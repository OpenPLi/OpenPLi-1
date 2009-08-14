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


#include <stdio.h>

#include <lcddclient/lcddMsg.h>
#include <lcddclient/lcddclient.h>

bool CLcddClient::send(const unsigned char command, char* data = NULL, const unsigned int size = 0)
{
	CLcddMsg::Header msgHead;
	msgHead.version = CLcddMsg::ACTVERSION;
	msgHead.cmd     = command;

	open_connection(LCDD_UDS_NAME);

	if (!send_data((char*)&msgHead, sizeof(msgHead)))
	    return false;
	
	if (size != 0)
	    return send_data(data, size);

	return true;
}


void CLcddClient::setMode(char mode, std::string head)
{
	CLcddMsg::commandMode msg2;

	msg2.mode = mode;
	strcpy( msg2.text, head.substr(0, sizeof(msg2.text)-2).c_str() );

	send(CLcddMsg::CMD_SETMODE, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CLcddClient::setMenuText(char pos, std::string text, char highlight)
{
	CLcddMsg::commandMenuText msg2;

	msg2.position = pos;
	msg2.highlight = highlight;
	strcpy( msg2.text, text.substr(0, sizeof(msg2.text)-2).c_str() );

	send(CLcddMsg::CMD_SETMENUTEXT, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CLcddClient::setServiceName(std::string name)
{
	CLcddMsg::commandServiceName msg2;

	strcpy( msg2.servicename, name.substr(0, sizeof(msg2.servicename)-2).c_str() );

	send(CLcddMsg::CMD_SETSERVICENAME, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CLcddClient::setMute(bool mute)
{
	CLcddMsg::commandMute msg2;

	msg2.mute = mute;

	send(CLcddMsg::CMD_SETMUTE, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CLcddClient::setVolume(char volume)
{
	CLcddMsg::commandVolume msg2;

	msg2.volume = volume;

	send(CLcddMsg::CMD_SETVOLUME, (char*)&msg2, sizeof(msg2));

	close_connection();
}

void CLcddClient::setContrast(int contrast)
{
	CLcddMsg::commandSetBrightness msg2;

	msg2.brightness = contrast;

	send(CLcddMsg::CMD_SETLCDCONTRAST, (char*)&msg2, sizeof(msg2));

	close_connection();
}

int CLcddClient::getContrast()
{
	CLcddMsg::responseGetBrightness msg2;

	send(CLcddMsg::CMD_GETLCDCONTRAST);

	receive_data((char*)&msg2, sizeof(msg2));

	close_connection();

	return msg2.brightness;
}

void CLcddClient::setBrightness(int brightness)
{
	CLcddMsg::commandSetBrightness msg2;

	msg2.brightness = brightness;

	send(CLcddMsg::CMD_SETLCDBRIGHTNESS, (char*)&msg2, sizeof(msg2));

	close_connection();
}

int CLcddClient::getBrightness()
{
	CLcddMsg::responseGetBrightness msg2;

	send(CLcddMsg::CMD_GETLCDBRIGHTNESS);

	receive_data((char*)&msg2, sizeof(msg2));

	close_connection();

	return msg2.brightness;
}

void CLcddClient::setBrightnessStandby(int brightness)
{
	CLcddMsg::commandSetBrightness msg2;

	msg2.brightness = brightness;

	send(CLcddMsg::CMD_SETSTANDBYLCDBRIGHTNESS, (char*)&msg2, sizeof(msg2));

	close_connection();
}

int CLcddClient::getBrightnessStandby()
{
	CLcddMsg::responseGetBrightness msg2;

	send(CLcddMsg::CMD_GETSTANDBYLCDBRIGHTNESS);

	receive_data((char*)&msg2, sizeof(msg2));

	close_connection();

	return msg2.brightness;
}

void CLcddClient::setPower(bool power)
{
	CLcddMsg::commandPower msg2;

	msg2.power = power;

	send(CLcddMsg::CMD_SETLCDPOWER, (char*)&msg2, sizeof(msg2));

	close_connection();
}

bool CLcddClient::getPower()
{
	CLcddMsg::commandPower msg2;

	send(CLcddMsg::CMD_GETLCDPOWER);

	receive_data((char*)&msg2, sizeof(msg2));

	close_connection();

	return msg2.power;
}

void CLcddClient::setInverse(bool inverse)
{
	CLcddMsg::commandInverse msg2;

	msg2.inverse = inverse;

	send(CLcddMsg::CMD_SETLCDINVERSE, (char*)&msg2, sizeof(msg2));

	close_connection();
}

bool CLcddClient::getInverse()
{
	CLcddMsg::commandInverse msg2;

	send(CLcddMsg::CMD_GETLCDINVERSE);

	receive_data((char*)&msg2, sizeof(msg2));

	close_connection();

	return msg2.inverse;
}

void CLcddClient::shutdown()
{
	setMode(CLcddTypes::MODE_SHUTDOWN, "");

	send(CLcddMsg::CMD_SHUTDOWN);

	close_connection();
}

void CLcddClient::update()
{
	send(CLcddMsg::CMD_UPDATE);
	close_connection();
}

void CLcddClient::pause()
{
	send(CLcddMsg::CMD_PAUSE);
	close_connection();
}

void CLcddClient::resume()
{
	send(CLcddMsg::CMD_RESUME);
	close_connection();
}

