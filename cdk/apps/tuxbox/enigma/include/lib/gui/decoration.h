#ifndef __lib_gui_decoration_h
#define __lib_gui_decoration_h

#include <lib/base/erect.h>
#include <lib/base/estring.h>
class gPixmap;
class gPainter;

class eDecoration
{
	gPixmap *iTopLeft, *iTop,
			*iTopRight, *iLeft, *iRight, 
			*iBottomLeft, *iBottom, *iBottomRight;

	eString	basename;
public:
	operator bool() { return iTopLeft || iTop || iTopRight || iLeft || iRight || iBottomLeft || iBottom || iBottomRight; }
	
	eDecoration();
  
	bool load(const eString& basename);

	void drawDecoration(gPainter *target, ePoint size );
	int borderTop, borderLeft, borderBottom, borderRight;
};

#endif
