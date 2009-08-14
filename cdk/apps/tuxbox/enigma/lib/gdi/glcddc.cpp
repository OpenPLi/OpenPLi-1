#ifndef DISABLE_LCD

#include <lib/gdi/glcddc.h>

gLCDDC *gLCDDC::instance;

gLCDDC::gLCDDC(eLCD *lcd): lcd(lcd)
{
	init_gLCDDC();
}
void gLCDDC::init_gLCDDC()
{
	instance=this;
	
	update=1;

	pixmap=new gPixmap();
	pixmap->x=lcd->size().width();
	pixmap->y=lcd->size().height();
	pixmap->bpp=8;
	pixmap->bypp=1;
	pixmap->stride=lcd->stride();
	pixmap->data=lcd->buffer();

	pixmap->clut.colors=256;
	pixmap->clut.data=0;
}

gLCDDC::~gLCDDC()
{
	delete pixmap;
	instance=0;
}

void gLCDDC::exec(gOpcode *o)
{
	switch (o->opcode)
	{
	case gOpcode::flush:
	case gOpcode::end:
		if (update)
			lcd->update();
	default:
		gPixmapDC::exec(o);
		break;
	}
}

gLCDDC *gLCDDC::getInstance()
{
	return instance;
}

void gLCDDC::setUpdate(int u)
{
	update=u;
}

#endif //DISABLE_LCD
