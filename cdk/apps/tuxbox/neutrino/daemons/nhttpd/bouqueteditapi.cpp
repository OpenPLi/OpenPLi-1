/*
	webserver  -   DBoxII-Project

	Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

	$Id: bouqueteditapi.cpp,v 1.7 2002/10/15 20:39:47 woglinde Exp $

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


#include "bouqueteditapi.h"
#include "debug.h"


//-------------------------------------------------------------------------
bool CBouqueteditAPI::Execute(CWebserverRequest* request)
{
	unsigned operation = 0;
	const char *operations[9] = {
		"main",
		"add",
		"move",
		"delete",
		"save",
		"rename",
		"edit",
		"editchannels",
		"set"
	};

	dprintf("ExecuteBouquetEditor %s\n",request->Filename.c_str());

	while (operation < 9) {
		if (request->Filename.compare(operations[operation]) == 0) {
			break;
		}
		operation++;
	}

	if (operation > 8) {
		request->Send404Error();	// if nothing matches send 404 error .)
		return false;
	}

	if(request->Method == M_HEAD) {
		request->SendPlainHeader("text/html");
		return true;
	}
	switch(operation)
	{

		case 0 :	return showBouquets(request); break;
		case 1 :	return addBouquet(request); break;
		case 2 :	return moveBouquet(request); break;
		case 3 :	return deleteBouquet(request); break;
		case 4 :	return saveBouquet(request); break;
		case 5 :	return renameBouquet(request); break;
		case 6 :	return editBouquet(request); break;
		case 7 :	return changeBouquet(request); break;
		case 8 :	return setBouquet(request); break;
		default:	request->Send404Error();
	}		
	return false;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::showBouquets(CWebserverRequest* request)
{
	int selected = -1;
	request->SendPlainHeader("text/html");
	request->SendHTMLHeader("Bouquet-Editor Main");
	request->SocketWrite("<H2>Bouquet-Editor</H2>\n");
	if (request->ParameterList["saved"] == "1")
	{
		request->SocketWrite("Bouquets gespeichert . . .<BR>");
		request->SocketWriteLn("<SCRIPT LANGUAGE=\"JavaScript\">\n<!--\ntop.bouquets.location.reload();//-->\n</SCRIPT>");
	}
	request->SocketWrite("<P><A HREF=\"add\">[add Bouquet]</A> <A HREF=\"save\">[save]</A> </P>");
	if (request->ParameterList["selected"] != "")
	{
		selected = atoi(request->ParameterList["selected"].c_str());
	}


	CZapitClient::BouquetList AllBouquetList;					// List of all bouquets
	AllBouquetList.clear();
	Parent->Zapit->getBouquets(AllBouquetList, true); 
	CZapitClient::BouquetList::iterator bouquet = AllBouquetList.begin();
	
	unsigned int bouquetSize = AllBouquetList.size();
//	request->printf("Bouquets: %i<BR>\n", bouquetSize);
	
	request->SocketWrite("<TABLE WIDTH=\"90%\">");
	for(; bouquet != AllBouquetList.end();bouquet++)
	{
		char classname;

		if ((bouquet->bouquet_nr + 1) % 2 == 1)
			classname='a';
		else
			classname='b';
		
		request->printf("<TR CLASS=\"%c\">\n<TD>",(selected == (int) bouquet->bouquet_nr + 1)?'c':classname);
		if (selected == (int) (bouquet->bouquet_nr + 1))
			request->SocketWrite("<A NAME=\"akt\"></A>");
		// lock/unlock
		if (bouquet->locked)
			request->printf("<CENTER><A HREF=\"set?selected=%i&action=unlock#akt\"><IMG src=\"../images/lock.gif\" ALT=\"Bouquet entsperren\"></IMG></A></CENTER></TD>\n", bouquet->bouquet_nr + 1);
		else
			request->printf("<CENTER><A HREF=\"set?selected=%i&action=lock#akt\"><IMG src=\"../images/unlock.gif\" ALT=\"Bouquet sperren\"></IMG></A></CENTER></TD>\n", bouquet->bouquet_nr + 1);

		// hide/show
		if (bouquet->hidden)
			request->printf("<TD><CENTER><A HREF=\"set?selected=%i&action=show#akt\"><IMG src=\"../images/hidden.gif\" ALT=\"Bouquet entsperren\"></IMG></A></CENTER></TD>\n", bouquet->bouquet_nr + 1);
		else
			request->printf("<TD><CENTER><A HREF=\"set?selected=%i&action=hide#akt\"><IMG src=\"../images/visible.gif\" ALT=\"Bouquet entsperren\"></IMG></A></CENTER></TD>\n", bouquet->bouquet_nr + 1);

		request->printf("<TD><A HREF=\"edit?selected=%i&name=%s\">%s</A></TD>", bouquet->bouquet_nr + 1, bouquet->name, bouquet->name);
		request->printf("<TD WIDTH=\"100\"><NOBR><A HREF=\"rename?selected=%i&name=%s\"><IMG SRC=\"../images/modify.gif\" ALT=\"Bouquet umbenennen\"></IMG></a>&nbsp;\n",bouquet->bouquet_nr + 1, bouquet->name);
		request->printf("<A HREF=\"delete?selected=%i&name=%s\"><IMG src=\"../images/remove.gif\" ALT=\"Bouquet löschen\"></IMG></A>&nbsp;\n",
			bouquet->bouquet_nr + 1, bouquet->name);
		

		// move down
		if (bouquet->bouquet_nr + 1 < bouquetSize)
			request->printf("<A HREF=\"move?selected=%i&action=down#akt\"><IMG src=\"../images/arrowdown.gif\" ALT=\"nach unten\"></A>&nbsp;\n", bouquet->bouquet_nr + 1);

		//move up
		if (bouquet->bouquet_nr > 0)
			request->printf("<A HREF=\"move?selected=%i&action=up#akt\"><IMG src=\"../images/arrowup.gif\" ALT=\"nach oben\"></A>\n", bouquet->bouquet_nr + 1);

		request->SocketWrite("</NOBR></TD></TR>\n");
	}
	request->SocketWrite("</TABLE>");

	request->SendHTMLFooter();
	return true;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::addBouquet(CWebserverRequest* request)
{
	if (request->ParameterList["name"] == "") {
		request->SendPlainHeader("text/html");
		request->SendHTMLHeader("Bouquet-Editor");
		request->SocketWrite("<H2>Bouquet-Editor</H2><BR><H3>Neues Bouquet</H3>\n");
		request->SocketWrite("<FORM ACTION=\"add\" METHOD=\"POST\" ENCTYPE=\"x-www-form-urlencoded\">\n");
		request->SocketWrite("Bouquetname: <INPUT TYPE=\"Text\" SIZE=\"30\" NAME=\"name\">");
		request->SocketWrite("<INPUT TYPE=\"submit\" VALUE=\"add\">\n");
		request->SocketWrite("</FORM>\n");
		request->SendHTMLFooter();
	}
	else
	{
		if (Parent->Zapit->existsBouquet(request->ParameterList["name"]) == -1) {
			Parent->Zapit->addBouquet(request->ParameterList["name"]);
			request->Send302("/bouquetedit/main#akt");
		} else {
			request->SendPlainHeader("text/html");
			request->SendHTMLHeader("Bouquet-Editor");
			request->SocketWrite("Have to add:");
			request->URLDecode(request->ParameterList["name"]);
			request->SocketWrite(request->ParameterList["name"].c_str());
			request->SocketWrite("<BR>Error! Bouquet already exists!\n");
			request->SocketWrite("<BR><A HREF=\"main#akt\">back</A>\n");
			request->SendHTMLFooter();
		}
	}
	return true;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::moveBouquet(CWebserverRequest* request)
{
	if (request->ParameterList["selected"] != "" && (request->ParameterList["action"] == "up" || request->ParameterList["action"] == "down")) {
		int selected = atoi(request->ParameterList["selected"].c_str());
		if (request->ParameterList["action"] == "up") {
			Parent->Zapit->moveBouquet(selected - 1, (selected - 1) - 1);
			selected--;
		} else {
			Parent->Zapit->moveBouquet(selected - 1, (selected + 1) - 1);
			selected++;
		}
		char redirbuff[100];
		sprintf(redirbuff, "main?selected=%i", selected);
		request->Send302(redirbuff);
	} else {
		request->SendPlainHeader("text/html");
		request->SendHTMLHeader("Bouquet-Editor");
		request->SocketWrite("Error!");
		request->SendHTMLFooter();
	}
	return true;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::deleteBouquet(CWebserverRequest* request)
{
	int selected = -1;
	if (request->ParameterList["selected"] != "") {
		selected = atoi(request->ParameterList["selected"].c_str());
	}
	
	if (request->ParameterList["sure"] != "yes") {
		request->SendPlainHeader("text/html");
		request->SendHTMLHeader("Bouquet-Editor");
		request->SocketWrite("<H2>Bouquet-Editor</H2><BR><H3>Bouquet löschen</H3>\n");
		request->SocketWrite("<B>Delete ");
		request->SocketWrite(request->ParameterList["name"]);
		request->SocketWrite("</B><BR>\n");
		
		request->printf("Sure? <A HREF=\"delete?selected=%i&sure=yes\">[Yep!]</A> <A HREF=\"main\">[no way!!!]</A>", selected);
		request->SendHTMLFooter();
	} else {
		Parent->Zapit->deleteBouquet(selected - 1);
		request->Send302("/bouquetedit/main#akt");
	}
	return true;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::saveBouquet(CWebserverRequest* request)
{
	Parent->Zapit->saveBouquets();
	Parent->Zapit->commitBouquetChange();
	Parent->UpdateBouquets();
	request->Send302("/bouquetedit/main?saved=1");
	return true;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::renameBouquet(CWebserverRequest* request)
{
	if (request->ParameterList["selected"] != "") 
	{
		if (request->ParameterList["nameto"] == "") {
			request->SendPlainHeader("text/html");
			request->SendHTMLHeader("Bouquet-Editor");
			request->SocketWrite("<H2>Bouquet-Editor</H2><BR><H3>Bouquet umbenennen</H3>\n");
			request->SocketWrite("<FORM ACTION=\"rename\" METHOD=\"POST\" ENCTYPE=\"x-www-form-urlencoded\">\n");
			request->printf("Bouquetname: <INPUT TYPE=\"Text\" SIZE=\"30\" NAME=\"nameto\" value=\"%s\">\n", request->ParameterList["name"].c_str());
			request->printf("<INPUT TYPE=\"hidden\" NAME=\"selected\" VALUE=\"%s\">\n", request->ParameterList["selected"].c_str());
			request->SocketWrite("<INPUT TYPE=\"submit\" VALUE=\"rename\">\n");
			request->SocketWrite("</FORM>\n");
			request->SendHTMLFooter();
		}
		else
		{
			Parent->Zapit->renameBouquet(atoi(request->ParameterList["selected"].c_str()) - 1, request->ParameterList["nameto"].c_str());
			request->Send302((char*)("/bouquetedit/main?selected=" + request->ParameterList["selected"] + "#akt").c_str());
		}
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------


bool CBouqueteditAPI::editBouquet(CWebserverRequest* request)
{
CZapitClient::BouquetChannelList BChannelList;
CZapitClient::BouquetChannelList::iterator channels;

	if (request->ParameterList["selected"] != "") {
		int selected = atoi(request->ParameterList["selected"].c_str());
		request->SendPlainHeader("text/html");
		request->SendHTMLHeader("Bouquet-Editor");
		request->SocketWrite("<Script language=\"Javascript\" src=\"/channels.js\">\n</script>\n");
		request->printf("<H2>Bouquet-Editor</H2><BR><H3>Bouquet %s bearbeiten</H3>\n",request->ParameterList["name"].c_str());
		request->SocketWrite("<FORM ACTION=\"editchannels\" METHOD=\"POST\" NAME=\"channels\" ENCTYPE=\"x-www-form-urlencoded\">\n");
		request->SocketWrite("<INPUT TYPE=\"HIDDEN\" NAME=\"selected\" VALUE=\"");
		request->SocketWrite(request->ParameterList["selected"].c_str());
		request->SocketWrite("\">\n");
		request->SocketWrite("<TABLE CELLSPACING=5><TR><TD>\n");

		// List channels in bouquet
		request->SocketWrite("<SELECT MULTIPLE SIZE=\"20\" NAME=\"bchannels\">\n");
		Parent->Zapit->getBouquetChannels(selected - 1, BChannelList, CZapitClient::MODE_CURRENT);
		channels = BChannelList.begin();
		for(; channels != BChannelList.end();channels++)
		{
			request->printf("<OPTION VALUE=\"%i\">%s</OPTION>\n", channels->channel_id,channels->name);
		}
		request->SocketWrite("</SELECT>\n");

		request->SocketWrite("</TD><TD><CENTER>\n");
		request->SocketWrite("<INPUT TYPE=\"Button\" VALUE=\"up\" onClick=\"poschannel(document.channels.bchannels, 0);\"><BR><BR>\n");
		request->SocketWrite("<INPUT TYPE=\"Button\" VALUE=\"down\" onClick=\"poschannel(document.channels.bchannels, 1);\"><BR><BR>\n");
		request->SocketWrite("<INPUT TYPE=\"Button\" VALUE=\">>>\" onClick=\"movechannels(document.channels.bchannels, document.channels.achannels);\"><BR><BR>\n");		
		request->SocketWrite("<INPUT TYPE=\"Button\" VALUE=\"<<<\" onClick=\"movechannels(document.channels.achannels, document.channels.bchannels);\"><BR><BR>\n");
		request->SocketWrite("</CENTER></td><td>\n");

		// List all channels
		request->SocketWrite("<SELECT multiple SIZE=\"20\" NAME=\"achannels\">\n");
		Parent->Zapit->getChannels(BChannelList, CZapitClient::MODE_CURRENT, CZapitClient::SORT_ALPHA);
		channels = BChannelList.begin();
		for(; channels != BChannelList.end();channels++)
		{
			if (!Parent->Zapit->existsChannelInBouquet(selected - 1, channels->channel_id)){
				request->printf("<OPTION VALUE=\"%i\">%s</OPTION>\n", channels->channel_id,channels->name);
			}
		}
		request->SocketWrite("</SELECT>\n");

		request->SocketWrite("</TD></TR></TABLE>\n");
		request->SocketWrite("<INPUT TYPE=\"button\" VALUE=\"Fertig\" onClick=\"fertig();\">\n");
		request->SocketWrite("</FORM>\n");
		request->SendHTMLFooter();
	}
	else
	{
		request->Send302("/bouquetedit/main#akt");
	}
	return true;
}

//-------------------------------------------------------------------------


bool CBouqueteditAPI::changeBouquet(CWebserverRequest* request)
{
	if (request->ParameterList["selected"] != "") {
		int selected = atoi(request->ParameterList["selected"].c_str());
		CZapitClient::BouquetChannelList BChannelList;
		Parent->Zapit->getBouquetChannels(selected - 1, BChannelList, CZapitClient::MODE_CURRENT);
		CZapitClient::BouquetChannelList::iterator channels = BChannelList.begin();
		for(; channels != BChannelList.end();channels++)
		{
			Parent->Zapit->removeChannelFromBouquet(selected - 1, channels->channel_id);
		}
		string bchannels = request->ParameterList["bchannels"];
		int pos;
		while ((pos = bchannels.find(',')) >= 0) {
			string bchannel = bchannels.substr(0, pos);
			bchannels = bchannels.substr(pos+1, bchannels.length());
			Parent->Zapit->addChannelToBouquet(selected - 1, atoi(bchannel.c_str()));
			
		}
		if (bchannels.length() > 0)
			Parent->Zapit->addChannelToBouquet(selected - 1, atoi(bchannels.c_str()));
		Parent->Zapit->renumChannellist();
		Parent->UpdateBouquets();
		request->Send302((char*)("/bouquetedit/main?selected=" + request->ParameterList["selected"] + "#akt").c_str());
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------

bool CBouqueteditAPI::setBouquet(CWebserverRequest* request)
{
	if (request->ParameterList["selected"] != "") {
		int selected = atoi(request->ParameterList["selected"].c_str());
		if(request->ParameterList["action"].compare("hide") == 0)
			Parent->Zapit->setBouquetHidden(selected - 1,true);
		else if(request->ParameterList["action"].compare("show") == 0)
			Parent->Zapit->setBouquetHidden(selected - 1,false);
		else if(request->ParameterList["action"].compare("lock") == 0)
			Parent->Zapit->setBouquetLock(selected - 1,true);
		else if(request->ParameterList["action"].compare("unlock") == 0)
			Parent->Zapit->setBouquetLock(selected - 1,false);
//		request->Send302("/bouquetedit/main#akt");
		request->Send302((char*)("/bouquetedit/main?selected=" + request->ParameterList["selected"] + "#akt").c_str());

		return true;
	}
	return false;
}

