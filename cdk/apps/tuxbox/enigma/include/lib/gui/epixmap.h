#ifndef __epixmap_h
#define __epixmap_h

#include "ewidget.h"
class gPixmap;

class ePixmap: public eWidget
{
	ePoint position;
public:
	ePixmap(eWidget *parent);
	~ePixmap();
	
	void redrawWidget(gPainter *paint, const eRect &area);
	void eraseBackground(gPainter *target, const eRect &area);
};

#endif
