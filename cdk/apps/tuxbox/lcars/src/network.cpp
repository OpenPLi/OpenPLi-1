/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: network.cpp,v $
Revision 1.18.6.2  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.18.6.1  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.18  2003/01/05 02:41:53  TheDOC
lcars supports inputdev now

Revision 1.17  2002/10/14 01:19:15  woglinde


mostly compiler warnings, but I got not all

Revision 1.16  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.15  2002/06/08 20:21:09  TheDOC
adding the cam-sources with slight interface-changes

Revision 1.14  2002/06/08 15:11:47  TheDOC
autostart in yadd added

Revision 1.13  2002/06/08 14:40:23  TheDOC
made lcars tuxvision-compatible

Revision 1.12  2002/06/02 22:05:53  TheDOC
http-network-stuff again

Revision 1.11  2002/06/02 21:25:33  TheDOC
http-network-stuff again

Revision 1.10  2002/06/02 21:06:27  TheDOC
http-network-stuff

Revision 1.9  2002/06/02 14:23:36  TheDOC
some fixes and changes

Revision 1.8  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.7  2002/05/21 04:37:42  TheDOC
http-update... new web-frontend in http://dbox/file/start.htm... will be main index soon

Revision 1.6  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.7  2001/12/20 00:31:38  tux
Plugins korrigiert

Revision 1.6  2001/12/17 14:00:41  tux
Another commit

Revision 1.5  2001/12/17 03:52:42  tux
Netzwerkfernbedienung fertig

Revision 1.4  2001/12/17 02:36:05  tux
Fernbedienung ueber's Netzwerk, 1. Schritt

Revision 1.3  2001/12/17 01:55:28  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#include "network.h"
#include "channels.h"
#include "pat.h"
#include "pmt.h"

network::network(zap *z, channels *c1, fbClass *f, osd *o, settings *s, tuner *t, pat *pa, pmt *pm, eit *e, scan *sc, rc *r, control *c, variables *v)
{
	rc_obj = r;
	control_obj = c;
	vars = v;
	zap_obj = z;
	channels_obj = c1;
	fb_obj = f;
	setting = s;
	tuner_obj = t;
	pat_obj = pa;
	pmt_obj = pm;
	eit_obj = e;
	scan_obj = sc;
}

std::string network::replace_vars(std::string tmp_string)
{
	bool quit = false;
	std::string work_string = tmp_string;
	int position = 0;

	do
	{
		std::string::size_type pos = work_string.find("%");
		if (std::string::npos == pos)
			quit = true;
		else
		{
			position += pos;
			work_string = work_string.substr(pos);
			std::string::size_type pos = work_string.find(" ");
			if (std::string::npos == pos)
				quit = true;
			else
			{
				std::string var = work_string.substr(0, pos);
				if (var == "%CHANNELLIST")
				{
					std::string http1 = getfile(DATADIR "/lcars/http/channels1.htm");
					std::string http2 = getfile(DATADIR "/lcars/http/channels2.htm");
					std::string http3 = getfile(DATADIR "/lcars/http/channels3.htm");
					std::string http4 = getfile(DATADIR "/lcars/http/channels4.htm");
					std::stringstream iss;
					for (int count = 0; count < channels_obj->numberChannels(); count++)
					{
						iss << http1 << count << http2 << "/channels/zapto/" << count << http3 << channels_obj->getServiceName(count) << http4;
					}
					iss << std::ends;
					tmp_string.replace(tmp_string.find(var), var.length(), iss.str());
					work_string = work_string.substr(pos);
					position += pos;
				}
				else if (vars->isavailable(var))
				{
					tmp_string.replace(tmp_string.find(var), var.length(), vars->getvalue(var));
					work_string = work_string.substr(pos);
					position += pos;
				}
				else
				{
					work_string = work_string.substr(1);
					position++;
				}
				std::cout << work_string << std::endl;
			}
		}
	} while(!quit);
	return tmp_string;
}

void network::startThread()
{
	pthread_create(&thread, 0, &network::startlistening, this);
}

void network::writetext(std::string text)
{
	write(inbound_connection, text.c_str(), text.length());
}

std::string network::getfile(std::string name)
{
	std::ifstream inFile;
	std::string httpfile;

	inFile.open(name.c_str());

	std::string tmp_string;
	while(getline(inFile, tmp_string))
		httpfile.append(tmp_string);

	return httpfile;
}

void *network::startlistening(void *object)
{
	network *n = (network *) object;

	int file_descriptor, inbound_connection, read_size;
	struct sockaddr_in server_address, inbound_address;
	socklen_t inbound_address_size;
	unsigned char *address_holder;
	char buffer[1024];

	file_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (file_descriptor < 0)
	{
		perror("socket()");
		exit(1);
	}

	memset((void*)&server_address, 0, sizeof(server_address));
	server_address.sin_port = htons(PORT);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind(file_descriptor, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
	{
		perror("bind()");
		exit(2);
	}

	while(1)
	{

		if (listen(file_descriptor, 5) < 0)
		{
			perror("listen()");
			exit(3);
		}
		memset((void*) &inbound_address, 0, sizeof(inbound_address));
		inbound_address.sin_family = AF_INET;
		inbound_address_size = sizeof(inbound_address);
		inbound_connection = accept(file_descriptor, (struct sockaddr*) &inbound_address, &inbound_address_size);
		if (inbound_connection < 0)
		{
			perror("accept()");
			exit(4);
		}

		n->inbound_connection = inbound_connection;

		address_holder = (unsigned char*) &inbound_address.sin_addr.s_addr;
		printf("Connection from %d.%d.%d.%d\n", address_holder[0], address_holder[1], address_holder[2], address_holder[3]);

		char command[20][10000];
		if ((read_size = read(inbound_connection, buffer, 10000)) < 0)
		{
			perror("read()");
			exit(5);
		}
		printf("Bytes read: %d\n", read_size);

		buffer[read_size] = '\0';
		printf("%s", buffer);

		int parm_count = 0;
		int act_pos = 0;
		for (int i = 0; i < read_size; i++)
		{
			if (buffer[i] != '\n')
			{
				command[parm_count][act_pos] = buffer[i];
				act_pos++;
			}
			else
			{
				if (buffer[i] == '\n')
				{
					command[parm_count][act_pos] = '\0';
					act_pos = 0;
					parm_count++;
				}
			}
		}
		command[parm_count][act_pos - 1] = '\0';
		parm_count++;

		printf ("%d Parameter\n", parm_count);
		for (int i = 0; i < parm_count; i++)
		{
			printf("Parameter %d: %s\n", i, command[i]);
		}

		std::string headerok = "HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/html\n\r\n";
		std::string headertextok = "HTTP/1.0 200 OK \nContent-Type: text/plain\n\n";
		std::string headerfailed = "HTTP/1.1 404 Not found\nConnection: close\nContent-Type: text/html\n\r\n";
		char writebuffer[1024];
		bool post = false;
		std::string postline;
		for (int i = 0; i < parm_count; i++)
		{
			std::string line(command[i]);
			std::string parm[20];

			std::istringstream iss(line);
			int counter = 0;
			while(std::getline(iss, parm[counter++], ' '));


			if (parm[0] == "GET")
			{
				std::istringstream iss2(parm[1]);
				std::string path[20];
				int counter2 = 0;
				while(std::getline(iss2, path[counter2++], '/'));


				if (path[1] == "" && counter2 == 2)
				{
					printf("GET root\n");
					write(inbound_connection, headerok.c_str(), headerok.length());
					write(inbound_connection, (*n->setting).getVersion().c_str(), (*n->setting).getVersion().length());
					strcpy(writebuffer, "<br><br><a href=\"/channels/gethtmlchannels\">Channellist</a>");
					write(inbound_connection, writebuffer, strlen(writebuffer));
					n->writetext("<br><br><a href=\"command\">Command-Parser</a>");
					n->writetext("<br><br><a href=\"rc/frame\">Remote Control</a>");
					n->writetext("<br><br><a href=\"/epg/now\">EPG Now</a>");
					n->writetext("<br><br><a href=\"/epg/next\">EPG Next</a>");
					strcpy(writebuffer, "<br><br><a href=\"/channels/lcars.dvb\">Channellist in DVB2000-format</a>");
					write(inbound_connection, writebuffer, strlen(writebuffer));
					path[1] = "file";
					path[2] = "start.htm";
					counter2 = 4;
				}

				

				if (path[1] == "command")
				{
					std::string response = "<html><body><form action=\"/command\" method=post><input type=text name=command size=80><br><input type=submit name=submit value=\"Befehl ausfuehren\"><br></form><form action=\"http://192.168.40.4/command\" method=post><input type=text name=sub size=80><br><input type=submit name=submit value=\"Sub starten\"></form></body></html>";
					write(inbound_connection, headerok.c_str(), headerok.length());
					write(inbound_connection, response.c_str(), response.length());
				}
				else if (path[1] == "control")
				{
					if (path[2] == "channellist")
					{
						write(inbound_connection, headertextok.c_str(), headertextok.length());
						for (int count = 0; count < (*n->channels_obj).numberChannels(); count++)
						{
							channel tmp_chan = (*n->channels_obj).getChannelByNumber(count);
							sprintf(writebuffer, "%d %s\n", count + 1, (*n->channels_obj).getServiceName(count).c_str());
							write(inbound_connection, writebuffer, strlen(writebuffer));
						}
					}
					if (path[2] == "info?streaminfo")
					{
						write(inbound_connection, headertextok.c_str(), headertextok.length());
						sprintf(writebuffer, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 1, 2, 3, 4, 5, 6, 7);
						write(inbound_connection, writebuffer, strlen(writebuffer));
					}
					else if (path[2] == "zapto" && path[2].length() == 5)
					{
						write(inbound_connection, headertextok.c_str(), headertextok.length());
						sprintf(writebuffer, "%d", (*n->channels_obj).getCurrentChannelNumber() + 1);
						write(inbound_connection, writebuffer, strlen(writebuffer));
					}
					else if (path[2].substr(0, 5) == "zapto")
					{
						int number = atoi(path[2].substr(6).c_str());
						if (path[2].substr(6) == "getpids")
						{
							write(inbound_connection, headertextok.c_str(), headertextok.length());
							sprintf(writebuffer, "%d\n%d\n", (*n->channels_obj).getCurrentVPID(), (*n->channels_obj).getCurrentAPID());
							write(inbound_connection, writebuffer, strlen(writebuffer));
						}
						else if (number > 0)
						{
							(*n->channels_obj).setCurrentChannel(number - 1);
							
							(*n->channels_obj).zapCurrentChannel();
							(*n->channels_obj).setCurrentOSDProgramInfo();
							n->eit_obj->gotNow = false;
							(*n->channels_obj).receiveCurrentEIT();
							(*n->channels_obj).setCurrentOSDProgramEIT();
							(*n->channels_obj).updateCurrentOSDProgramAPIDDescr();
							
							write(inbound_connection, headerok.c_str(), headerok.length());

						}
					}
					else if (path[2].substr(0, 3) == "epg")
					{
						write(inbound_connection, headertextok.c_str(), headertextok.length());

						event tmp_event = n->eit_obj->waitForNow();
						sprintf(writebuffer, "%s\n%s\n%s\n", tmp_event.event_name, tmp_event.event_short_text, tmp_event.event_extended_text);
						write(inbound_connection, writebuffer, strlen(writebuffer));

					}
					
				}
				else if (path[1] == "file" && counter2 > 3)
				{
					std::string ending = path[2].substr(path[2].find("."));
					std::string filename = DATADIR "/lcars/http/" + path[2];
					if (ending == ".htm" || ending == ".html" || ending == ".css")
					{
						std::ifstream inFile;
						std::string httpfile;
						inFile.open(filename.c_str());
						if (!inFile)
						{
								write(inbound_connection, headerfailed.c_str(), headerfailed.length());
						}
						else
						{
							std::string tmp_string;
							while(getline(inFile, tmp_string))
								httpfile.append(tmp_string);
							httpfile = n->replace_vars(httpfile);
							write(inbound_connection, httpfile.c_str(), httpfile.length());
						}
					}
					else
					{
						int fd = open(filename.c_str(), O_RDONLY);
						if (fd == -1)
						{
							write(inbound_connection, headerfailed.c_str(), headerfailed.length());
						}
						else
						{
							std::string header = "HTTP/1.1 200 OK\nConnection: close\nAccept-Ranges: bytes\nContent-Type: application/octet-stream\n\r\n";
							write(inbound_connection, header.c_str(), header.length());
							char c;
							while(read(fd, &c, 1))
								write(inbound_connection, &c, 1);
						}
						close(fd);
					}


				}
				else if (path[1] == "version")
				{
					write(inbound_connection, headerok.c_str(), headerok.length());
					write(inbound_connection, (*n->setting).getVersion().c_str(), (*n->setting).getVersion().length());
				}
				else if (path[1] == "channels")
				{
					if (path[2] == "getcurrent")
					{
						write(inbound_connection, headerok.c_str(), headerok.length());
						sprintf(writebuffer, "%d", (*n->channels_obj).getCurrentChannelNumber());
						write(inbound_connection, writebuffer, strlen(writebuffer));
					}
					else if (path[2] == "numberchannels")
					{
						write(inbound_connection, headerok.c_str(), headerok.length());
						sprintf(writebuffer, "%d", (*n->channels_obj).numberChannels());
						write(inbound_connection, writebuffer, strlen(writebuffer));
					}
					else if (path[2] == "gethtmlchannels")
					{
						write(inbound_connection, headerok.c_str(), headerok.length());
						write(inbound_connection, "<table>", 7);
						for (int count = 0; count < (*n->channels_obj).numberChannels(); count++)
						{
							sprintf(writebuffer, "<tr><td>%d</td><td><a href=\"zapto/%d\">%s</a></td></tr>\n", count, count, (*n->channels_obj).getServiceName(count).c_str());
							write(inbound_connection, writebuffer, strlen(writebuffer));
						}
						write(inbound_connection, "</table>", 8);
					}
					else if (path[2] == "getchannels")
					{
						std::cout << "Sending Channellist" << std::endl;
						write(inbound_connection, headerok.c_str(), headerok.length());
						for (int count = 0; count < (*n->channels_obj).numberChannels(); count++)
						{
							channel tmp_chan = (*n->channels_obj).getChannelByNumber(count);
							sprintf(writebuffer, "%d %s<br>\n", count, tmp_chan.serviceName);
							write(inbound_connection, writebuffer, strlen(writebuffer));
						}
					}
					else if (path[2] == "lcars.dvb")
					{
						std::string header = "HTTP/1.1 200 OK\nConnection: close\nAccept-Ranges: bytes\nContent-Type: application/octet-stream\n\r\n";
						write(inbound_connection, header.c_str(), header.length());
						int fd = open(CONFIGDIR "/lcars/lcars.dvb", O_RDONLY);
						char c;
						while(read(fd, &c, 1))
							write(inbound_connection, &c, 1);
						close(fd);
					}
					else if (path[2] == "scan")
					{
						(*n->scan_obj).scanChannels(NORMAL);
					}
					else if (path[2] == "zapto")
					{
						int number = atoi(path[3].c_str());
						
						(*n->channels_obj).setCurrentChannel(number);
						
						(*n->channels_obj).zapCurrentChannel();
						(*n->channels_obj).setCurrentOSDProgramInfo();
						(*n->channels_obj).receiveCurrentEIT();
						(*n->channels_obj).setCurrentOSDProgramEIT();
						(*n->channels_obj).updateCurrentOSDProgramAPIDDescr();
						
						write(inbound_connection, headerok.c_str(), headerok.length());
						write(inbound_connection, "Done!<br>\n", 10);

						if ((*n->channels_obj).getCurrentAPIDcount() == 1)
							sprintf(writebuffer, "VPID: %x APID: %x<br>\n", (*n->channels_obj).getCurrentVPID(), (*n->channels_obj).getCurrentAPID(0));
						else
							sprintf(writebuffer, "VPID: %x APID: %x APID: %x<br>\n", (*n->channels_obj).getCurrentVPID(), (*n->channels_obj).getCurrentAPID(0), (*n->channels_obj).getCurrentAPID(1));
						write(inbound_connection, writebuffer, strlen(writebuffer));

						n->writetext("<br><hr>\n");
						event tmp_event;
						char text[100];
						for (int i = 0; i < 2; i++)
						{
							tmp_event = (i == 0) ? (*n->eit_obj).getNow() : (*n->eit_obj).getNext();

							sprintf(text, "%s - <a href=/epg/%s>%s</a><br>\n", ctime(&tmp_event.starttime), (i == 0) ? "now" : "next", tmp_event.event_name);
							n->writetext(text);

						}
						n->writetext("<hr><br>\n");

						strcpy(writebuffer, "<br><a href=\"/video/stop\">Video stop</a>");
						write(inbound_connection, writebuffer, strlen(writebuffer));
					}
				}
				else if (path[1] == "video")
				{
					if (path[2] == "stop")
					{
						(*n->zap_obj).dmx_stop();

						write(inbound_connection, headerok.c_str(), headerok.length());
						write(inbound_connection, "Done!", 6);
					}
					else if (path[2] == "start")
					{
						(*n->zap_obj).dmx_start();

						write(inbound_connection, headerok.c_str(), headerok.length());
						write(inbound_connection, "Done!", 6);
					}

				}

				else if (path[1] == "epg")
				{
					char text[1000];
					n->writetext(headerok);
					event tmp_event;
					tmp_event = (path[2] == "now") ? (*n->eit_obj).getNext() : (*n->eit_obj).getNow();
					sprintf(text, "Starttime: %s<br>\n", ctime(&tmp_event.starttime));
					n->writetext(text);
					sprintf(text, "Dauer: %d min<br>\n", (int)(tmp_event.duration / 60));
					n->writetext(text);
					sprintf(text, "<b><h3>%s</h3></b><br>\n", tmp_event.event_name);
					n->writetext(text);
					sprintf(text, "%s<br>\n", tmp_event.event_short_text);
					n->writetext(text);
					sprintf(text, "<textarea cols=80 rows=10 readonly>%s</textarea><br>\n", tmp_event.event_extended_text);
					n->writetext(text);
					sprintf(text, "Running Status: 0x%x - EventID: 0x%x<br>\n", tmp_event.running_status, tmp_event.eventid);
					n->writetext(text);

				}
				else if (path[1] == "rc")
				{
					if (path[2] == "frame")
					{
						write(inbound_connection, headerok.c_str(), headerok.length());
						n->writetext("<frameset  rows=\"0,*\"><frame name=\"command\" src=\"\" marginwidth=\"0\" marginheight=\"0\" scrolling=\"no\" frameborder=\"0\"><frame name=\"main\" src=\"/rc/rc\" marginwidth=\"0\" marginheight=\"0\" scrolling=\"auto\" frameborder=\"0\"></frameset>");
					}
					else if (path[2] == "rc")
					{
						write(inbound_connection, headerok.c_str(), headerok.length());
						std::stringstream ostr;
						ostr << "<map name=\"rc\">";
						ostr << "<area target=\"command\" alt=\"1\" shape=\"circle\" coords=\"103,67,8\" href=\"1\">";
						ostr << "<area target=\"command\" alt=\"2\" shape=\"CIRCLE\" coords=\"123,64,8\" href=\"2\">";
						ostr << "<area target=\"command\" alt=\"3\" shape=\"CIRCLE\" coords=\"143,63,8\" href=\"3\">";
						ostr << "<area target=\"command\" alt=\"4\" shape=\"circle\" coords=\"105,88,8\" href=\"4\">";
						ostr << "<area target=\"command\" alt=\"5\" shape=\"CIRCLE\" coords=\"125,85,9\" href=\"5\">";
						ostr << "<area target=\"command\" alt=\"6\" shape=\"CIRCLE\" coords=\"142,81,8\" href=\"6\">";
						ostr << "<area target=\"command\" alt=\"7\" shape=\"circle\" coords=\"102,108,9\" href=\"7\">";
						ostr << "<area target=\"command\" alt=\"8\" shape=\"circle\" coords=\"121,106,8\" href=\"8\">";
						ostr << "<area target=\"command\" alt=\"9\" shape=\"circle\" coords=\"145,101,9\" href=\"9\">";
						ostr << "<area target=\"command\" alt=\"0\" shape=\"circle\" coords=\"102,131,9\" href=\"0\">";
						ostr << "<area target=\"command\" alt=\"standby\" coords=\"54,59,86,75\" href=\"standby\">";
						ostr << "<area target=\"command\" alt=\"home\" coords=\"55,80,86,95\" href=\"home\">";
						ostr << "<area target=\"command\" alt=\"dbox\" coords=\"56,99,86,117\" href=\"dbox\">";
						ostr << "<area target=\"command\" alt=\"blue\" shape=\"circle\" coords=\"143,143,9\" href=\"blue\">";
						ostr << "<area target=\"command\" alt=\"yellow\" shape=\"circle\" coords=\"102,156,12\" href=\"yellow\">";
						ostr << "<area target=\"command\" alt=\"green\" shape=\"circle\" coords=\"80,174,11\" href=\"green\">";
						ostr << "<area target=\"command\" alt=\"red\" shape=\"circle\" coords=\"66,193,11\" href=\"red\">";
						ostr << "<area target=\"command\" alt=\"ok\" shape=\"CIRCLE\" coords=\"106,238,18\" href=\"ok\">";
						ostr << "<area target=\"command\" alt=\"up\" coords=\"83,200,134,217\" href=\"up\">";
						ostr << "<area target=\"command\" alt=\"down\" coords=\"86,255,129,275\" href=\"down\" shape=\"RECT\">";
						ostr << "<area target=\"command\" alt=\"right\" coords=\"128,218,149,262\" href=\"right\" shape=\"RECT\">";
						ostr << "<area target=\"command\" alt=\"left\" coords=\"72,218,90,256\" href=\"left\" shape=\"RECT\">";
						ostr << "<area target=\"command\" alt=\"plus\" shape=\"circle\" coords=\"62,266,8\" href=\"plus\">";
						ostr << "<area target=\"command\" alt=\"minus\" shape=\"circle\" coords=\"74,289,10\" href=\"minus\">";
						ostr << "<area target=\"command\" alt=\"mute\" shape=\"CIRCLE\" coords=\"104,314,8\" href=\"mute\">";
						ostr << "<area target=\"command\" alt=\"help\" shape=\"CIRCLE\" coords=\"146,329,11\" href=\"help\">";
						ostr << "</map>";
						ostr << "<img src=\"/datadir/rc.jpg\" width=\"200\" height=\"500\"border=\"0\" usemap=\"#rc\">";
						ostr << std::ends;
						n->writetext(ostr.str());
					}
					else if (path[2] == "1")
					{
						n->rc_obj->cheat_command(RC_1);
					}
					else if (path[2] == "2")
					{
						n->rc_obj->cheat_command(RC_2);
					}
					else if (path[2] == "3")
					{
						n->rc_obj->cheat_command(RC_3);
					}
					else if (path[2] == "4")
					{
						n->rc_obj->cheat_command(RC_4);
					}
					else if (path[2] == "5")
					{
						n->rc_obj->cheat_command(RC_5);
					}
					else if (path[2] == "6")
					{
						n->rc_obj->cheat_command(RC_6);
					}
					else if (path[2] == "7")
					{
						n->rc_obj->cheat_command(RC_7);
					}
					else if (path[2] == "8")
					{
						n->rc_obj->cheat_command(RC_8);
					}
					else if (path[2] == "9")
					{
						n->rc_obj->cheat_command(RC_9);
					}
					else if (path[2] == "0")
					{
						n->rc_obj->cheat_command(RC_0);
					}
					else if (path[2] == "standby")
					{
						n->rc_obj->cheat_command(RC_STANDBY);
					}
					else if (path[2] == "home")
					{
						n->rc_obj->cheat_command(RC_HOME);
					}
					else if (path[2] == "menu")
					{
						n->rc_obj->cheat_command(RC_MENU);
					}
					else if (path[2] == "blue")
					{
						n->rc_obj->cheat_command(RC_BLUE);
					}
					else if (path[2] == "yellow")
					{
						n->rc_obj->cheat_command(RC_YELLOW);
					}
					else if (path[2] == "green")
					{
						n->rc_obj->cheat_command(RC_GREEN);
					}
					else if (path[2] == "red")
					{
						n->rc_obj->cheat_command(RC_RED);
					}
					else if (path[2] == "up")
					{
						n->rc_obj->cheat_command(RC_UP);
					}
					else if (path[2] == "down")
					{
						n->rc_obj->cheat_command(RC_DOWN);
					}
					else if (path[2] == "right")
					{
						n->rc_obj->cheat_command(RC_RIGHT);
					}
					else if (path[2] == "left")
					{
						n->rc_obj->cheat_command(RC_LEFT);
					}
					else if (path[2] == "ok")
					{
						n->rc_obj->cheat_command(RC_OK);
					}
					else if (path[2] == "mute")
					{
						n->rc_obj->cheat_command(RC_MUTE);
					}
					else if (path[2] == "plus")
					{
						n->rc_obj->cheat_command(RC_VOLPLUS);
					}
					else if (path[2] == "minus")
					{
						n->rc_obj->cheat_command(RC_VOLMINUS);
					}
					else if (path[2] == "help")
					{
						n->rc_obj->cheat_command(RC_HELP);
					}


				}

				else if (path[1] == "datadir")
				{
					std::string filename = DATADIR "/lcars/" + path[2];
					int fd = open(filename.c_str(), O_RDONLY);
					char c;
					while(read(fd, &c, 1))
						write(inbound_connection, &c, 1);
					close(fd);

				}
				else
				{
					write(inbound_connection, headerfailed.c_str(), headerfailed.length());
				}
			}
			else if (parm[0] == "POST")
			{
				post = true;
				postline = command[i];
				std::cout << "Postline: " << postline << std::endl;
			}
		}
		if (post)
		{
			std::string line(postline);
			std::string parm[20];

			std::istringstream iss(line);
			int counter = 0;
			while(std::getline(iss, parm[counter++], ' '));

			std::istringstream iss2(parm[1]);
			std::string path[20];
			int counter2 = 0;
			while(std::getline(iss2, path[counter2++], '/'));

			std::cout << "PATH[1]: " << path[1] << std::endl;
			if (path[1] == "command")
			{
				std::string parameters(command[parm_count - 1]);
				std::stringstream iss(parameters);
				std::string tmp_string;
				std::vector<std::string> var_list;
				std::string command;
				var_list.clear();
				while(std::getline(iss, tmp_string, '&'))
					var_list.insert(var_list.end(), tmp_string);
				for (int i = 0; (unsigned int) i < var_list.size(); i++)
				{
					std::cout << "command2" << std::endl;
					std::stringstream iss2(var_list[i]);
					std::getline(iss2, tmp_string, '=');
					if (tmp_string == "command")
					{
						std::getline(iss2, command, '=');
						break;
					}
					else if (tmp_string == "sub")
					{
						std::getline(iss2, command, '=');
						break;
					}
				}

				if (tmp_string == "command")
				{
					std::replace(command.begin(), command.end(), '+', ' ');
					std::cout << "Command: " << command << std::endl;
					n->control_obj->runCommand(n->control_obj->parseCommand(command));
				}
				else if (tmp_string == "sub")
				{
					std::replace(command.begin(), command.end(), '+', ' ');
					std::cout << "Command: " << command << std::endl;
					n->control_obj->runSub(command);
				}
				std::string response = "<html><body><form action=\"http://192.168.40.4/command\" method=post><input type=text name=command size=80><br><input type=submit name=submit value=\"Befehl ausfuehren\"><br></form><form action=\"http://192.168.40.4/command\" method=post><input type=text name=sub size=80><br><input type=submit name=submit value=\"Sub starten\"></form></body></html>";
				write(inbound_connection, headerok.c_str(), headerok.length());
				write(inbound_connection, response.c_str(), response.length());

			}
			else if (path[1] == "SID2")
			{
				std::string request(buffer);
				std::string xml = request.substr(request.find("\r\n\r\n"));
				std::cout << "Parsing\n" << std::endl;
				std::cout << "End Parsing\n" << std::endl;

				std::cout << xml << std::endl;
				std::cout << "Making HTTP\n" << std::endl;
				std::stringstream ostr;
				ostr.clear();

				ostr << "HTTP/1.1 200 OK\r\n";
				ostr << "Connection: close\r\n";
				ostr << "Content-Length: " << xml.length() << "\r\n";
				ostr << "Content-Type: text/xml\r\n";
				ostr << "Date: Fri, 17 Jul 1998 19:55:08 GMT\r\n";
				ostr << "Server: LCARS Webserver\r\n\r\n";
				ostr << xml << std::ends;

				std::string send = ostr.str();
				std::cout << "Sending now XML\n" << std::endl;
				std::cout << send << std::endl;
				write(inbound_connection, send.c_str(), send.length());
			}

		}

		close(inbound_connection);
	}
}
