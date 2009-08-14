/*
 * $Id: enigma_dyn_utils.cpp,v 1.23 2005/11/12 19:44:59 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <lib/base/estring.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn.h>

using namespace std;

extern int pdaScreen;

eString getWebifVersion()
{
	return WEBIFVERSION;
}

eString htmlChars(eString s)
{
	s.strReplace("\'", "&#39;");
	s.strReplace("\"", "&quot;");
	return s;
}
eString unHtmlChars(eString s)
{
	s.strReplace("&#39;", "\'");
	s.strReplace("&quot;", "\"");
	return s;
}
eString getAttribute(eString filename, eString attribute)
{
	eString result = "&nbsp;";

	ifstream infile(filename.c_str());
	if (infile)
	{
		eString buffer;
		while (getline(infile, buffer, '\n'))
		{
			if (buffer.find(attribute + "=") == 0)
			{
				result = getRight(buffer, '=');
				if (result == "")
					result = "&nbsp;";
				break;
			}
		}
	}
	return result;
}

eString readFile(eString filename)
{
	eString result;
	eString line;
	
	if (strstr(filename.c_str(), (TEMPLATE_DIR).c_str()) != 0)
	{
		char *pch = strrchr(filename.c_str(), '/');
		eString filename2 = TEMPLATE_DIR2 + eString(strdup(pch + 1));
		if (access(filename2.c_str(), R_OK) == 0)
			filename = filename2;
	}	
			
	if (strstr(filename.c_str(), (HTDOCS_DIR).c_str()) != 0)
	{
		char *pch = strrchr(filename.c_str(), '/');
		eString filename2 = HTDOCS_DIR2 + eString(strdup(pch + 1));
		if (access(filename2.c_str(), R_OK) == 0)
			filename = filename2;
	}

	ifstream infile(filename.c_str());
	if (infile)
		while (getline(infile, line, '\n'))
			result += line + "\n";

	return result;
}


eString button(int width, eString buttonText, eString buttonColor, eString buttonRef, eString color, bool xml, int height, eString bgrepeat, int border, int margin_right, int margin_bottom, eString font_weight)
{
	eString ref1, ref2;

	std::stringstream result;
	
	if (pdaScreen == 0)
	{
		if (buttonRef.find("javascript") == eString::npos)
		{
			ref1 = "self.location.href='";
			ref2 = "'";
		}

		result << "<input name=\"" << buttonText << "\""
			" type=\"button\" style=\"width:" << width << "px;"
			"border:" << border << "px;"
			"margin-right:" << margin_right << "px;"
			"margin-bottom:" << margin_bottom << "px;"
			"height:" << height << "px;";
		if (font_weight != "")
			result << "font-weight:" << font_weight << ";";
		if (buttonColor)
		{
			if (buttonColor.find("#") == eString::npos)
				result << "background-image:url(/" << buttonColor << ");background-repeat:" << bgrepeat << ";";
			else
				result << "background-color:" << buttonColor << ";";
		}
		if (color)
			result << "color: " << color << ";";
			
		eString ending = (xml) ? " />" : ">";
		
		result << "\" value=\"" << buttonText << "\" onclick=\"" << ref1 << buttonRef << ref2 << "\"" << ending;
	}
	else
		result << "<a href=\"" << buttonRef << "\"><span class=\"button\">" << buttonText << "</span></a>&nbsp;";

	return result.str();
}

eString getTitle(eString title)
{
	eString result;
	if (pdaScreen == 0)
	{
		result = "<script>if(parent.setTitle) {parent.setTitle('" + title + "')} " +
			"else {document.title='" + title + "'}</script>\n";
	}
	else
	{
		result = "<h1>" + title + "</h1>\n";
	}
	return result;
}

eString filter_string(eString string)
{
	string.strReplace("\xc2\x86","");
	string.strReplace("\xc2\x87","");
	string.strReplace("\xc2\x8a"," ");
	string.strReplace("\"", "'");
	return string;
}

int getHex(int c)
{
	c = toupper(c);
	c -= '0';
	if (c < 0)
		return -1;
	if (c > 9)
		c -= 'A' - '0' - 10;
	if (c > 0xF)
		return -1;
	return c;
}

eString httpUnescape(const eString &string)
{
	eString result;
	for (unsigned int i = 0; i < string.length(); ++i)
	{
		int c = string[i];
		switch (c)
		{
		case '%':
		{
			int value = '%';
			if ((i + 1) < string.length())
				value = getHex(string[++i]);
			if ((i + 1) < string.length())
			{
				value <<= 4;
				value += getHex(string[++i]);
			}
			result += value;
			break;
		}
		case '+':
			result += ' ';
			break;
		default:
			result += c;
			break;
		}
	}
	return result;
}

eString httpEscape(const eString &string)
{
	eString result;
	for (unsigned int i = 0; i < string.length(); ++i)
	{
		int c = string[i];
		int valid = 0;
		if ((c >= 'a') && (c <= 'z'))
			valid = 1;
		else if ((c >= 'A') && (c <= 'Z'))
			valid = 1;
		else if (c == ':')
			valid = 1;
		else if ((c >= '0') && (c <= '9'))
			valid = 1;

		if (valid)
			result += c;
		else
			result += eString().sprintf("%%%x", c);
	}
	return result;
}

eString ref2string(const eServiceReference &r)
{
	return httpEscape(r.toString());
}

eServiceReference string2ref(const eString &service)
{
	eString str = httpUnescape(service);
	return eServiceReference(str);
}

std::map<eString, eString> getRequestOptions(eString opt, char delimiter)
{
	std::map<eString, eString> result;

	if (opt[0] == '?')
		opt = opt.mid(1);

	while (opt.length())
	{
		unsigned int e = opt.find("=");
		if (e == eString::npos)
			e = opt.length();
		unsigned int a = opt.find(delimiter, e);
		if (a == eString::npos)
			a = opt.length();
		eString n = opt.left(e);

		unsigned int b = opt.find(delimiter, e + 1);
		if (b == eString::npos)
			b = (unsigned)-1;
		eString r = httpUnescape(opt.mid(e + 1, b - e - 1));
		result.insert(std::pair<eString, eString>(n, r));
		opt = opt.mid(a + 1);
	}
	return result;
}

eString closeWindow(eHTTPConnection *content, eString msg, int wait)
{
	eString result;

	if (pdaScreen == 0)
	{
		content->code = 204;
		content->code_descr = "No Content";
		result = "";
	}
	else
	{
		content->local_header["Content-Type"] = "text/html; charset=utf-8";
		result = readFile(TEMPLATE_DIR + "pdaResponse.tmp");
		result.strReplace("#WAIT#", eString().sprintf("%d", wait));
		result.strReplace("#MSG#", msg);
	}
	return result;
}

eString getIP()
{
	int sd;
	struct ifreq ifr;
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd < 0)
		return "?.?.?.?-socket-error";
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_addr.sa_family = AF_INET; // fixes problems with some linux vers.
	strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name));
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
		return "?.?.?.?-ioctl-error";
	close(sd);

	return eString().sprintf("%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

off64_t getMovieSize(eString filename)
{
	off64_t size = 0;
	int slice = 0;
	struct stat64 s;
	while (!stat64((filename + (slice ? eString().sprintf(".%03d", slice) : eString(""))).c_str(), &s))
	{
		size += s.st_size;
		++slice;
	}
	eDebug("[GETMOVIESIZE] %s: %lld", filename.c_str(), size);
	return size;
}

