#include <lib/gui/ewindow.h>
#include <lib/gdi/grc.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/gdi/epng.h>
#include <lib/gui/elabel.h>
#include <lib/gui/guiactions.h>
#include <lib/gdi/font.h>

int eWindow::globCancel = eWindow::ON;

eWindow::eWindow(int takefocus)
	:eWidget(0, takefocus)
{
	init_eWindow();
}
void eWindow::init_eWindow()
{
	deco.load("eWindow");

	titleBarColor=eSkin::getActive()->queryScheme("eWindow.titleBar");
	fontColor=eSkin::getActive()->queryScheme("eWindow.titleBarFont");

	borderLeft=eSkin::getActive()->queryValue("eWindow.borderLeft", deco.borderLeft);
	borderRight=eSkin::getActive()->queryValue("eWindow.borderRight", deco.borderRight);
	borderBottom=eSkin::getActive()->queryValue("eWindow.borderBottom", deco.borderBottom);
	borderTop=eSkin::getActive()->queryValue("eWindow.borderTop", deco.borderTop );

	titleOffsetLeft=eSkin::getActive()->queryValue("eWindow.titleOffsetLeft", 0);
	titleOffsetRight=eSkin::getActive()->queryValue("eWindow.titleOffsetRight", 0);
	titleOffsetTop=eSkin::getActive()->queryValue("eWindow.titleOffsetTop", 0);
	titleHeight=eSkin::getActive()->queryValue("eWindow.titleHeight", titleFontSize+10);
	titleFontSize=eSkin::getActive()->queryValue("eWindow.titleFontSize", 20);

	font = eSkin::getActive()->queryFont("eWindow.Childs");
}

eWindow::~eWindow()
{
}

eRect eWindow::getTitleBarRect()
{
	eRect rc;
	rc.setLeft( deco.borderLeft > titleOffsetLeft ? deco.borderLeft : titleOffsetLeft );
	rc.setTop( titleOffsetTop );
	rc.setRight( width() - ( deco.borderRight > titleOffsetRight ? deco.borderRight : titleOffsetRight ) );
	rc.setBottom( rc.top() + (titleHeight?titleHeight:deco.borderTop) );  // deco.borderTop sucks...
	return rc;
}

void eWindow::redrawWidget(gPainter *target, const eRect &where)
{
	bool drawBorder = eSkin::getActive()->queryValue("eWindow.drawBorder", 1) != 0;
	if ( deco )  // then draw Deco
	{
		deco.drawDecoration(target, ePoint(width(), height()));
		drawBorder=false;
	}
	drawTitlebar(target);
	if ( drawBorder )
	{
		gColor border = eSkin::getActive()->queryColor("eWindow.border");
		target->setForegroundColor(border);
		// target->setBackgroundColor(border);
		target->line( ePoint(0,0), ePoint(0, height()-1) );
		target->line( ePoint(0,0), ePoint(width()-1, 0) );
		target->line( ePoint(width()-1,0), ePoint(width()-1, height()-1) );
		target->line( ePoint(0,height()-1), ePoint(width()-1, height()-1) );
	}
}

void eWindow::eraseBackground(gPainter *target, const eRect &clip)
{
	target->clip(getClientRect());
	target->clear();
	target->clippop();
}

void eWindow::drawTitlebar(gPainter *target)
{
	eRect rc = getTitleBarRect();
	int margin = eSkin::getActive()->queryValue("eWindow.titleMargin", 0);
	target->clip( rc );
	if ( titleHeight )
	{
		target->setForegroundColor(titleBarColor);
		target->fill( rc );
	}
	if (margin) 
	{
		rc.setLeft(rc.left() + margin);
		rc.setWidth(rc.width() - 2*margin);
	}
	else
		rc.setWidth(rc.width()-10); // to maintain old behaviour
	eTextPara *p = new eTextPara( rc );
	p->setFont( eSkin::getActive()->queryFont("eWindow.TitleBar") );
	p->renderString( text );
	target->setBackgroundColor(titleBarColor);
	target->setForegroundColor(fontColor);
	target->renderPara( *p );
	p->destroy();
	target->clippop();
}

void eWindow::recalcClientRect()
{
	clientrect=eRect( borderLeft, borderTop, width() - (borderLeft+borderRight), height() - ( borderTop+borderBottom) );
}

int eWindow::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
		case eWidgetEvent::willShow:
			if (focus)
				focusChanged( focus );
			break;
		case eWidgetEvent::changedText:
			redraw(getTitleBarRect());
			return 1;
		case eWidgetEvent::evtAction:
			if (globCancel && (event.action == &i_cursorActions->cancel) && in_loop)	// hack
				close(-1);
			else
				break;
			return 1;
		default:
			break;
	}
	return eWidget::eventHandler(event);
}

void eWindow::willShow()
{
#ifndef DISABLE_LCD
	if (LCDTitle)
		LCDTitle->setText(text);
#endif
	eWidget::willShow();
}

void eWindow::willHide()
{
	eWidget::willHide();
}

static eWidget *create_eWindow(eWidget *parent)
{
	return new eWindow();
}

class eWindowSkinInit
{
public:
	eWindowSkinInit()
	{
		eSkin::addWidgetCreator("eWindow", create_eWindow);
	}
	~eWindowSkinInit()
	{
		eSkin::removeWidgetCreator("eWindow", create_eWindow);
	}
};

eAutoInitP0<eWindowSkinInit> init_eWindowSkinInit(eAutoInitNumbers::guiobject, "eWindow");
