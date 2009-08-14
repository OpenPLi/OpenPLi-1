#include <lib/gui/epixmap.h>
#include <lib/gui/eskin.h>
#include <lib/gui/init.h>
#include <lib/gui/init_num.h>

ePixmap::ePixmap(eWidget *parent): eWidget(parent)
{
	position=ePoint(0, 0);
	setBackgroundColor(getForegroundColor());
}

ePixmap::~ePixmap()
{
}

void ePixmap::redrawWidget(gPainter *paint, const eRect &area)
{
	if (pixmap)
		paint->blit(*pixmap, position);
}

void ePixmap::eraseBackground(gPainter *target, const eRect &area)
{
}

static eWidget *create_ePixmap(eWidget *parent)
{
	return new ePixmap(parent);
}

class ePixmapSkinInit
{
public:
	ePixmapSkinInit()
	{
		eSkin::addWidgetCreator("ePixmap", create_ePixmap);
	}
	~ePixmapSkinInit()
	{
		eSkin::removeWidgetCreator("ePixmap", create_ePixmap);
	}
};

eAutoInitP0<ePixmapSkinInit> init_ePixmapSkinInit(eAutoInitNumbers::guiobject, "ePixmap");
