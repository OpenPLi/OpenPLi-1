#ifndef __src_enigma_epg
#define __src_enigma_epg

#include <lib/gui/ewidget.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/dvb/dvb.h>

class EITEvent;
class eStatusBar;

class eZapEPG: public eWidget
{
	gFont timeFont, titleFont, descrFont;
	gColor entryColor, entryFgColor, entryColorSelected, baseColor, timeBgCol, timeFgCol,
		serviceBgCol, serviceFgCol, timelineColor, timelineNowColor, descriptionBgCol, descriptionFgCol;

	unsigned int offs, focusColumn, hours, numservices;
	int pig, dopicon, doEPGCat;
	int ViewSx, ViewSy, ViewEx, ViewEy, PigW, PigH, SNWidth; // viewport definition, pig width/hieght (assume pig is based at ViewSx,ViewSy)
	std::vector<gPixmap*> picons;

	ePtrList<eLabel> timeLine;
	eStatusBar *sbar;
	eWidget *eventWidget;
	int NowTimeLineXPos;
	time_t start, end;
	struct serviceentry;
	struct entry: public eWidget
	{
		gFont &timeFont, &titleFont, &descrFont;
		gColor entryColor, entryFgColor, entryColorSelected;
		eWidget *sbar;
		eTextPara *para;
		eString title;
		unsigned int xOffs, yOffs;
		void redrawWidget(gPainter *target, const eRect &area);
		void gotFocus();
		void lostFocus();
	public:
		Signal0<void> redrawed;
		struct serviceentry *service;
		static gPixmap *inTimer;
		static gPixmap *inTimerRec;
		void setActive(int active);
		void setColor(gColor bgcolor, gColor fgcolor);
		const EITEvent *event;
		entry(eWidget *parent, gFont &timeFont, gFont &titleFont, gFont &descrFont, gColor entryColor, gColor entryFgColor, gColor entryColorSelected, eWidget *sbar );
		~entry();
	};
	enum {ScrollUp, ScrollDown, ScrollLeft, ScrollRight, ScrollNone};
	
	struct serviceentry
	{
		eRect pos;
		eLabel *header;
		eServiceReferenceDVB service;
		ePtrList<entry> entries;
		ePtrList<entry>::iterator current_entry;
		serviceentry() : header(0), current_entry(entries.end()) { }
		~serviceentry() { delete header; }
	};
	std::list<eServiceReferenceDVB> services;
	std::list<eServiceReferenceDVB>::iterator curS, curE;
	std::list<serviceentry> serviceentries;
	std::list<serviceentry>::iterator current_service;
	int eventHandler(const eWidgetEvent &event);
	void buildService(serviceentry &service);
	void selService(int dir);
	void selEntry(int dir);
	void drawTimeLines();
	void buildServices();
	void buildPage(int direction = ScrollNone);
	void init_eZapEPG();
public:
	std::list<serviceentry>::iterator& getCurSelected() { return current_service; }
	void addToList( const eServiceReference& ref );
	eZapEPG();
	~eZapEPG();
	
	/* eWidget functions */
	void show();
};

#endif
