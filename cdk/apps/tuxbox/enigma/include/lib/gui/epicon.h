#ifndef __epicon_h
#define __epicon_h

#include "ewidget.h"
#include <lib/dvb/service.h>

class ePicon: public eWidget
{
	static const char *piconPaths[];

	gPixmap *picon;

protected:
	void handleServiceEvent(const eServiceEvent &event);

		/* eWidget functions */
	void redrawWidget(gPainter *paint, const eRect &area);

public:
	ePicon(eWidget *parent);
	~ePicon();

	static gPixmap *loadPicon(eString &refname, const eSize &size);
	static gPixmap *loadPicon(eServiceReference &ref, const eSize &size);
};

#endif
