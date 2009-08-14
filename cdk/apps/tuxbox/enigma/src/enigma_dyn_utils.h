/*
 * $Id: enigma_dyn_utils.h,v 1.20 2005/11/12 19:44:59 digi_casi Exp $
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

#ifndef __enigma_dyn_utils_h
#define __enigma_dyn_utils_h

#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/serviceplaylist.h>
#include <lib/system/http_dyn.h>
#include <src/enigma_dyn_colors.h>

#define TEMPLATE_DIR TUXBOXDATADIR+eString("/enigma/templates/")
#define TEMPLATE_DIR2 CONFIGDIR+eString("/enigma/templates/")
#define HTDOCS_DIR TUXBOXDATADIR+eString("/enigma/htdocs/")
#define HTDOCS_DIR2 CONFIGDIR+eString("/enigma/htdocs/")

#define CHARSETMETA "<META http-equiv=Content-Type content=\"text/html; charset=UTF-8\">\n"
#define WINDOWCLOSE "<html><head><META http-equiv=Content-Type content=\"text/html; charset=UTF-8\"></head><body><script  language=\"javascript\">close();</script></body></html>"
#define WINDOWBACK "<html><head><META http-equiv=Content-Type content=\"text/html; charset=UTF-8\"></head><body><script  language=\"javascript\">back();</script></body></html>"

eString getAttribute(eString filename, eString attribute);
eString readFile(eString filename);
eString button(int width, eString buttonText, eString buttonColor, eString buttonRef, eString color="", bool xml=false, int height=22, eString bgrepeat="repeat-x", int border=0, int margin_right=1, int margin_bottom=0, eString font_wight="");
eString getTitle(eString title);
int getHex(int c);
eString httpUnescape(const eString &string);
eString filter_string(eString string);
eString httpEscape(const eString &string);
std::map<eString, eString> getRequestOptions(eString opt, char delimiter);
eString ref2string(const eServiceReference &r);
eServiceReference string2ref(const eString &service);
eString closeWindow(eHTTPConnection *content, eString msg, int wait);
eString htmlChars(eString);
eString unHtmlChars(eString);
eString getIP(void);
off64_t getMovieSize(eString);
eString getWebifVersion();

extern eString getRight(const eString&, char); // implemented in timer.cpp
extern eString getLeft(const eString&, char);  // implemented in timer.cpp

#endif /* __enigma_dyn_utils_h */
