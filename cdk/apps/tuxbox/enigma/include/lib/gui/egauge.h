#ifndef __egauge_h
#define __egauge_h

#include <lib/gui/eprogress.h>
#include <lib/gdi/grc.h>

/**
 * \brief A progressbar.
 *
 * Useful for displaying a progress or stuff.
 */
class eGauge: public eProgress
{
public:
	eGauge(eWidget *parent, int takeFocus=0);
	~eGauge();
	
	void redrawWidget(gPainter *target, const eRect &area);
};

#endif
