#ifndef __evideowidget_h
#define __evideowidget_h

#include "ewidget.h"

class eVideoWidget: public eWidget
{
protected:
	int pigDevice;
	int borderWidth;
	eSize pigSize;
	eSize visiblePigSize;

	void calculatePigSizes(eSize limits);

	/* eWidget functions */
	int eventHandler(const eWidgetEvent &event);
	void redrawWidget(gPainter *target, const eRect &area);
	int setProperty(const eString &prop, const eString &value);

public:
	eVideoWidget(eWidget *parent);
	~eVideoWidget();
};

#endif
