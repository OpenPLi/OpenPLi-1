#ifndef GETSET_H
#define GETSET_H
/* an enigma plugin to download Satellite Settings 
 
 $Id: getset.h,v 1.1 2004/12/01 23:05:52 essu Exp $
 
 copyright (c) 2004 by essu@yadi.org. All rights reserved
 aktuelle Versionen: $Source: /cvs/tuxbox/apps/tuxbox/plugins/enigma/getset/getset.h,v $

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 675 Mass Ave, Cambridge MA 02139, USA.
*/

#include <plugin.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <enigma_main.h>

#include <lib/base/estring.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eprogress.h>

#include <lib/system/econfig.h>
#include <lib/system/httpd.h>
#include <lib/system/info.h>
#include <lib/dvb/settings.h>
#include <lib/dvb/edvb.h>

// plugin entry point, C calling convention
extern "C" int plugin_exec( PluginParam *par );

// THX,...
class eListBoxEntryChaeck: public eListBoxEntryText
{
	gPixmap *pm;
	bool checked;
	void LBSelected(eListBoxEntry* t);
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
public:
	eListBoxEntryChaeck(eListBox<eListBoxEntryChaeck>* lb, const char* txt=0, void *key=0, int align=0, const eString &hlptxt="", int keytype = value );
	eListBoxEntryChaeck(eListBox<eListBoxEntryChaeck>* lb, const eString& txt, void* key=0, int align=0, const eString &hlptxt="", int keytype = value );
	bool getCheck() { return checked; }
	void setCheck(bool ncheck);
};

eListBoxEntryChaeck::eListBoxEntryChaeck(eListBox<eListBoxEntryChaeck>* lb, const char* txt, void *key, int align, const eString &hlptxt, int keytype )
	:eListBoxEntryText((eListBox<eListBoxEntryText>*)lb, txt, key, align, hlptxt, keytype),
	pm(eSkin::getActive()->queryImage("eListBoxEntryCheck")),
	checked(0)
{
	selectable=1;
	if (listbox)
		CONNECT(listbox->selected, eListBoxEntryChaeck::LBSelected);
}

eListBoxEntryChaeck::eListBoxEntryChaeck(eListBox<eListBoxEntryChaeck>* lb, const eString& txt, void* key, int align, const eString &hlptxt, int keytype )
	:eListBoxEntryText((eListBox<eListBoxEntryText>*)lb, txt, key, align, hlptxt, keytype),
	pm(eSkin::getActive()->queryImage("eListBoxEntryCheck")),
	checked(0)
{
	selectable=1;
	if (listbox)
		CONNECT(listbox->selected, eListBoxEntryChaeck::LBSelected);
}

void eListBoxEntryChaeck::LBSelected(eListBoxEntry* t)
{
	if (t == this)
		setCheck(checked^1);
}

const eString& eListBoxEntryChaeck::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	bool b;
	if ( (b = (state == 2)) )
		state = 0;
	eListBoxEntryText::redraw( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );
	if ( pm && checked )
	{
		eRect right = rect;
		right.setLeft( rect.right() - (pm->x + 10) );
		rc->clip(right);
		eSize psize = pm->getSize(),
					esize = rect.size();
		int yOffs = rect.top()+((esize.height() - psize.height()) / 2);
		rc->blit(*pm, ePoint(right.left(), yOffs), right, gPixmap::blitAlphaTest );
		rc->clippop();
	}
	return text;
}

void eListBoxEntryChaeck::setCheck(bool ncheck)
{
	if ( checked != ncheck )
	{
		checked=ncheck;
		listbox->invalidateCurrent();
	}
}
//... _ghost_ */

class eCheckbox;

class eProgress;

class eHTTPDownload: public eHTTPDataSource
{
	int received;
	int total;
	int fd;
	eString filename;
public:
	eHTTPDownload(eHTTPConnection *c, const char *filename);
	Signal2<void,int,int> progress; // received, total (-1 for unknown)
	~eHTTPDownload();
	void haveData(void *data, int len);
};

class eGetSettings: public eWindow
{
// listboxes
	eListBox<eListBoxEntryText> *sat_List;
	eListBox<eListBoxEntryText> *sort_List;
	eListBox<eListBoxEntryChaeck> *bouquet_List;
// buttons
	eButton *bt_satdelete, *bt_sortdelete, *bt_getset, *bt_install, *bt_ok;
// status, labels, checkboxes, progressbar
	eStatusBar *lb_selected;
	eLabel *sat_selected[5], *sort_selected[5];
	eCheckbox *Opt_Switching[6];
	eProgress *progress;
	eLabel *progresstext;

	eHTTPConnection *http[32];
	eHTTPDataSource *createDataSink(eHTTPConnection *connct, int i);
	eHTTPDownload *dl_data[32];
	int lasttime[32];
	int error[32];
	int sat_toload;
	eString opt[6], help[6], txt[6];
	eString current_url, current_file, config_dir, tar, tv, radio;
	eString bouquet_saved[32];
private:
// callbacks if  something is selected
	void SatSelected(eListBoxEntryText *item);
	void SortSelected(eListBoxEntryText *item);
	void BouquetSelected(eListBoxEntryChaeck *item);
	void Opt_Changed( int, int); 
	void init(), SatDelete(), SortDelete(), GetBouquets();

	void doDownload(int i);
	void downloadDone(int err, int i);
	void setError(int error, int i);
	void downloadProgress(int received, int total, int i);
	void finished();

	void GetSet(), keepBouquets(), tarBouquets(), restoreSaved(), installSettings();
	int readCatalog(), checkFree();
	eString getOptions();
// other
	unsigned int v_Getset_Count, option, satcounter, sortcounter;
// Sat-, Sort-List-Entries, Bouquets
	eString sat[26], sort[7], bouquet[32], bouquet_name[32];
	eString settings_uri[32], local_file[32];
public:
	eGetSettings();
	~eGetSettings();
};
#endif
