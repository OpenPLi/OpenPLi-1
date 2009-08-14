#ifndef __setuptimezone_h
#define __setuptimezone_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/statusbar.h>
#include <xmltree.h>

class eLabel;
class eComboBox;

class eZapTimeZoneSetup: public ePLiWindow
{
	int errLoadTimeZone;
	
	eLabel *ltimeZone;
	eComboBox *timeZone;
	bool showHint;
	void init_eZapTimeZoneSetup();
private:
	void okPressed();

	int eZapTimeZoneSetup::loadTimeZonesXML();
	int loadTimeZones();
	char *getTimeZoneAttr(char * Attribute);

	XMLTreeParser *parser;

public:
	eZapTimeZoneSetup(bool showHint=true);
	~eZapTimeZoneSetup();
	void setTimeZone();
};

#endif
