#include <lib/gui/eprogress.h>

#include <stdlib.h>

#include <lib/base/erect.h>
#include <lib/gdi/fb.h>
#include <lib/gdi/lcd.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

/*
 * eProgress skin widget
 *
 * Skin properties:
 *   - 'start' defines offset for the progress position
 *   - 'pixmap' parameter can be used to specify a background pixmap
 *   - 'sliderpixmap' parameter can be used to specify a png file as marker instead of the color bar
 *   - 'direction' parameter switches between horizontal/vertical
 *   - 'alphatest': with this parameter the 'leftColor' area can be made transparent
 */

eProgress::eProgress(eWidget *parent, int takefocus)
	: eWidget(parent, takefocus)
{
	init_eProgress();
}
void eProgress::init_eProgress()
{
	left = eSkin::getActive()->queryScheme("eProgress.left");
	right = eSkin::getActive()->queryScheme("eProgress.right");
	perc = start = 0;
	border = 2;
	direction = 0;
	sliderPixmap = NULL;
	setForegroundColor(eSkin::getActive()->queryScheme("eProgress.border"));
}

eProgress::~eProgress()
{
}

bool eProgress::setParams( int _start, int _perc )
{
	if ( perc != _perc || start != _start )
	{
		perc = _perc;
		start = _start;
		invalidate();
		return true;
	}
	return false;
}

void eProgress::setPerc(int p)
{
	if (perc != p)
	{
		perc=p;
		invalidate();
	}
}

void eProgress::setStart(int p)
{
	if (start != p)
	{
		start=p;
		invalidate();
	}
}

void eProgress::redrawWidget(gPainter *target, const eRect &area)
{
	switch (direction)
	{
		case 0:
		{
			int range = size.width() - border * 2;
			if (sliderPixmap) range -= sliderPixmap->x;

			int st = start * range / 100;
			if (st < 0) st = 0;
			if (st > range) st = range;

			int dh = perc * range / 100;
			if (dh < 0) dh = 0;
			if ((dh + st) > range) dh = range - st;

			if (pixmap)
			{
				target->blit(*pixmap, ePoint(0, 0), eRect(0, 0, border + dh + st, size.height()), gPixmap::blitAlphaTest);
			}

			if (sliderPixmap)
			{
				int x = border + st + dh;
				int y = (size.height() - sliderPixmap->y) / 2;
				if (x < 0) x = 0;
				if (y < 0) y = 0;
				if (x > size.width() - sliderPixmap->x) x = size.width() - sliderPixmap->x;
				if (y > size.height() - sliderPixmap->y) y = size.height() - sliderPixmap->y;
				target->blit(*sliderPixmap, ePoint(x, y), area, gPixmap::blitAlphaTest);
			}
			else
			{
				if (left >= 0)
				{
					target->setForegroundColor(left);
					target->fill(eRect(border + st, border, dh, size.height() - border * 2));
				}
				target->setForegroundColor(right);
				target->fill(eRect(border + dh + st, border, size.width() - border * 2 - dh - st, size.height() - border * 2));
				if (st)
				{
					target->fill(eRect(border, border, st, size.height() - border * 2));
				}
			}
			break;
		}
		case 1:
		{
			int range = size.height() - border * 2;
			if (sliderPixmap) range -= sliderPixmap->y;

			int st = start * range / 100;
			if (st < 0) st = 0;
			if (st > range) st = range;

			int dh = perc * range / 100;
			if (dh < 0) dh = 0;
			if ((dh + st) > range) dh = range - st;

			if (pixmap)
			{
				target->blit(*pixmap, ePoint(0, 0), eRect(0, 0, size.width(), border + dh + st), gPixmap::blitAlphaTest);
			}

			if (sliderPixmap)
			{
				int x = (size.width() - sliderPixmap->x) / 2;
				int y = border + st + dh;
				if (x < 0) x = 0;
				if (y < 0) y = 0;
				if (x > size.width() - sliderPixmap->x) x = size.width() - sliderPixmap->x;
				if (y > size.height() - sliderPixmap->y) y = size.height() - sliderPixmap->y;
				target->blit(*sliderPixmap, ePoint(x, y), area, gPixmap::blitAlphaTest);
			}
			else
			{
				if (left >= 0)
				{
					target->setForegroundColor(left);
					target->fill(eRect(border, border+st, size.width() - border * 2, dh));
				}
				target->setForegroundColor(right);
				target->fill(eRect(border, border+dh+st, size.width() - border * 2, size.height() - border * 2 - dh - st));
				if (st)
				{
					target->fill(eRect(border, border, size.width() - border * 2, st));
				}
			}
			break;
		}
	}
	if (border)
	{
		/* draw border */
		target->setForegroundColor(getForegroundColor());
		target->fill(eRect(0, 0, size.width(), border));
		target->fill(eRect(0, border, border, size.height() - border));
		target->fill(eRect(border, size.height() - border, size.width() - border, border));
		target->fill(eRect(size.width() - border, border, border, size.height() - border));
	}
}

int eProgress::setProperty(const eString &prop, const eString &value)
{
	if (prop == "leftColor")
		left = eSkin::getActive()->queryColor(value);
	else if (prop == "rightColor")
		right = eSkin::getActive()->queryColor(value);
	else if (prop == "start")
		start = atoi(value.c_str());
	else if (prop == "border")
		border = atoi(value.c_str());
	else if (prop == "direction")
		direction = atoi(value.c_str());
	else if (prop == "alphatest" && value == "on")
		left = gColor(-1); /* make 'left' transparent */
	else if (prop == "sliderpixmap")
		setSliderPixmap(eSkin::getActive()->queryImage(value));
	else
		return eWidget::setProperty(prop, value);
	return 0;
}

static eWidget *create_eProgress(eWidget *parent)
{
	return new eProgress(parent);
}

class eProgressSkinInit
{
public:
	eProgressSkinInit()
	{
		eSkin::addWidgetCreator("eProgress", create_eProgress);
	}
	~eProgressSkinInit()
	{
		eSkin::removeWidgetCreator("eProgress", create_eProgress);
	}
};

eAutoInitP0<eProgressSkinInit> init_eProgressSkinInit(eAutoInitNumbers::guiobject, "eProgress");
