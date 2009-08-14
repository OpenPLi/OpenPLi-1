#include <lib/gui/egauge.h>

#include <stdlib.h>

#include <lib/base/erect.h>
#include <lib/gdi/fb.h>
#include <lib/gdi/lcd.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/math.h>

/*
 * eGauge skin widget. Uses same syntax as eProgress
 *   - start parameter indicates start and end offset in degrees for the marker.
 *   - The 'start' parameter can be set in skin file
 *   - The 'pixmap' parameter can be used to specify a background pixmap
 *   - The 'sliderpixmap' parameter can be used to specify a png file as marker instead of the line.
 *     It is drawn just against the border.
 *     Note: The pixmap itself will not be rotated. So it is best to use a circular picture.
 *   - the egauge background is set to transparent
 */

eGauge::eGauge(eWidget *parent, int takefocus)
	: eProgress(parent, takefocus)
{
}

eGauge::~eGauge()
{
}

void eGauge::redrawWidget(gPainter *target, const eRect &area)
{
	if (pixmap)
	{
		target->blit(*pixmap, ePoint(0, 0), area, gPixmap::blitAlphaTest);
	}

	if (border)
	{
		/* draw border */
		target->setForegroundColor(getForegroundColor());
		target->fill(eRect(0, 0, size.width(), border));
		target->fill(eRect(0, border, border, size.height()-border));
		target->fill(eRect(border, size.height()-border, size.width()-border, border));
		target->fill(eRect(size.width()-border, border, border, size.height()-border));
	}

	int endx, endy;
	int basex = size.width() >> 1;
	int basey = size.height() >> 1;

	//target->setForegroundColor(right);
	//target->fill(eRect(border, border, size.width()-border*2, size.height()-border*2));
	// angle in degrees
	double angle = (double) start + (double) perc * (double)(360 - (start<<1)) / 100.0;
	double rads  = Radians(angle);
	
	if (!sliderPixmap)
	{
		if (direction)
		{
			endx = basex + (int) (SIN(rads) * (double)(size.width()  - (border<<1))/2.0);
			endy = basey - (int) (COS(rads) * (double)(size.height() - (border<<1))/2.0);
		}
		else 
		{
			endx = basex - (int) (SIN(rads) * (double)(size.width()  - (border<<1))/2.0);
			endy = basey + (int) (COS(rads) * (double)(size.height() - (border<<1))/2.0);
		}
		target->setForegroundColor(left);
		target->line(ePoint(basex, basey), ePoint(endx, endy));
	}
	else
	{
		if (direction)
		{
			endx = basex + (int) (SIN(rads) * (double)(size.width()  - (border<<1) - sliderPixmap->x) / 2.0) - (sliderPixmap->x >> 1);
			endy = basey - (int) (COS(rads) * (double)(size.height() - (border<<1) - sliderPixmap->y) / 2.0) - (sliderPixmap->y >> 1);
		}
		else 
		{
			endx = basex - (int) (SIN(rads) * (double)(size.width()  - (border<<1) - sliderPixmap->x) / 2.0) - (sliderPixmap->x >> 1);
			endy = basey + (int) (COS(rads) * (double)(size.height() - (border<<1) - sliderPixmap->y) / 2.0) - (sliderPixmap->y >> 1);
		}
		//eDebug("sliderPixmap %d,%d at %d,%d  size=%d,%d border=%d offset=%d", basex, basey, endx, endy, sliderPixmap->x, sliderPixmap->y, border, pixmapoffset );
		target->blit(*sliderPixmap, ePoint(endx, endy), area, gPixmap::blitAlphaTest);
	}
}

static eWidget *create_eGauge(eWidget *parent)
{
	return new eGauge(parent);
}

class eGaugeSkinInit
{
public:
	eGaugeSkinInit()
	{
		eSkin::addWidgetCreator("eGauge", create_eGauge);
	}
	~eGaugeSkinInit()
	{
		eSkin::removeWidgetCreator("eGauge", create_eGauge);
	}
};

eAutoInitP0<eGaugeSkinInit> init_eGaugeSkinInit(eAutoInitNumbers::guiobject, "eGauge");
