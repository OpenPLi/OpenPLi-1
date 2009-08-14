#ifndef __APPS_ENIGMA_CHANNELINFO__
#define __APPS_ENIGMA_CHANNELINFO__

#include <lib/gui/ewidget.h>
#include <lib/gui/elabel.h>
#include <lib/dvb/si.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/service.h>

class eChannelInfo : public eDecoWidget
{	
	eLabel ctime, cname, copos, cdescr, cdolby, cstereo, cformat, cscrambled;
	eString name, descr, genre, starttime;
	int cflags;

	static const char *genresTableShort[];

	void getServiceInfo( const eServiceReferenceDVB& );
	int LayoutIcon(eLabel *, int,  int );
	void ParseEITInfo(EITEvent *e);
	eServiceReferenceDVB current;

	void redrawWidget(gPainter *, const eRect &);
	int eventHandler(const eWidgetEvent &event);
	EIT *eit;
	void EITready(int err);
	void closeEIT();
	eString DescriptionForEPGSearch; // EPG search
	void init_eChannelInfo();
public:
	enum
	{
		cflagStereo = 1,
		cflagDolby = 2,
		cflagWide = 4,
		cflagScrambled = 8
	};

	eChannelInfo( eWidget*, const char* deco="eStatusBar" );
	~eChannelInfo()
	{
		closeEIT();
		delete eit;
	}
	void update( const eServiceReferenceDVB& );
	void clear();
	static eString getGenre(int index) {return genresTableShort[index];}
	eString GetDescription();  	// EPG search
};

#endif // __APPS_ENIGMA_CHANNELINFO__

