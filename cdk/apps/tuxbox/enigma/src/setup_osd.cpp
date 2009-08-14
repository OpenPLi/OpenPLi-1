#include <setup_osd.h>

#include <setupskin.h>
#include <setup_extra_osd.h>
#include <enigma.h>
#include <enigma_main.h>
#include <lib/base/i18n.h>
#include <lib/dvb/edvb.h>
#include <lib/gdi/gfbdc.h>
#include <lib/gdi/font.h>
#include <lib/gui/eskin.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/emessage.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

#define MENUNAMEPOS N_("OSD position")
#define MENUNAMETRANS N_("OSD transparency")

class PluginOffsetScreenFactory : public eCallableMenuFactory
{
	void init_PluginOffsetScreen();
public:
	PluginOffsetScreenFactory() : eCallableMenuFactory("PluginOffsetScreen", MENUNAMEPOS) {}
	eCallableMenu *createMenu()
	{
		return new PluginOffsetScreen;
	}
};

PluginOffsetScreenFactory PluginOffsetScreen_factory;

class eZapOsdSetupFactory : public eCallableMenuFactory
{
public:
	eZapOsdSetupFactory() : eCallableMenuFactory("eZapOsdSetup", MENUNAMETRANS) {}
	eCallableMenu *createMenu()
	{
		return new eZapOsdSetup;
	}
};

eZapOsdSetupFactory eZapOsdSetup_factory;

struct PluginOffsetActions
{
	eActionMap map;
	eAction leftTop, rightBottom, store;
	PluginOffsetActions()
		:map("PluginOffsetActions", _("PluginOffsetActions")),
		leftTop(map,"leftTop", _("enable set the leftTop Point of the rectangle")),
		rightBottom(map,"rightBottom", _("enable set the rightBottom Point of the rectangle")),
		store(map, "store", _("saves the current positions"))
	{
	}
};

eAutoInitP0<PluginOffsetActions> i_PluginOffsetActions(eAutoInitNumbers::actions, "tuxtxt/multiepg offset actions");

int PluginOffsetScreen::eventHandler( const eWidgetEvent &event )
{
	switch ( event.type )
	{
		case eWidgetEvent::execBegin:
			eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", left);
			eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", top);
			eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/right", right);
			eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/bottom", bottom);
			invalidate();
			return 0;
		case eWidgetEvent::execDone:
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/left", left);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/top", top);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/right", right);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/bottom", bottom);
			return 0;
		case eWidgetEvent::willShow:
			invalidate();
			return 0;
		case eWidgetEvent::evtAction:
			if (event.action == &i_PluginOffsetActions->leftTop)
			{
				curPos=posLeftTop;
				return 0;
			}
			else if (event.action == &i_PluginOffsetActions->rightBottom)
			{
				curPos=posRightBottom;
				return 0;
			}
			else if (event.action == &i_cursorActions->cancel)
			{
				close(0);
				return 0;
			}
			else if (event.action == &i_PluginOffsetActions->store)
			{
				close(0);
				return 0;
			}
			else if (event.action == &i_cursorActions->left)
			{
				if ( curPos == posLeftTop )
					left--;
				else if (curPos == posRightBottom )
					right--;
			}
			else if (event.action == &i_cursorActions->right)
			{
				if ( curPos == posLeftTop )
					left++;
				else if (curPos == posRightBottom )
					right++;
			}
			else if (event.action == &i_cursorActions->up)
			{
				if ( curPos == posLeftTop )
					top--;
				else if (curPos == posRightBottom )
					bottom--;
			}
			else if (event.action == &i_cursorActions->down)
			{
				if ( curPos == posLeftTop )
					top++;
				else if (curPos == posRightBottom )
					bottom++;
			}
			else
				break;
			if ( curPos == posLeftTop )
				invalidate( eRect( ePoint(left-1, top-1), eSize(102, 102) ) );
			else if ( curPos == posRightBottom )
				invalidate( eRect( ePoint(right-101, bottom-101), eSize(102, 102) ) );
			return 0;
		default:
			break;
	}
	return eWidget::eventHandler( event );
}

void PluginOffsetScreen::redrawLeftTop( gPainter *target )
{
	target->fill( eRect( ePoint( left, top ), eSize( 100, 3 ) ) );
	target->fill( eRect( ePoint( left, top ), eSize( 3, 100 ) ) );
}

void PluginOffsetScreen::redrawRightBottom( gPainter *target )
{
	target->fill( eRect( ePoint( right-3, bottom-100 ), eSize( 3, 100 ) ) );
	target->fill( eRect( ePoint( right-100, bottom-3 ), eSize( 100, 3 ) ) );
}

void PluginOffsetScreen::redrawWidget(gPainter *target, const eRect &where)
{
	target->setForegroundColor( foreColor );
	if ( where.intersects( eRect(	ePoint( left, top ), eSize( 100, 100 ) ) ) )
		redrawLeftTop( target );
	if ( where.intersects( eRect( ePoint( right-3, bottom-100 ), eSize( 3, 100 ) ) ) )
		redrawRightBottom( target );
	cursize->setText(eString().sprintf("%d:%d  %d:%d  -  %dx%d", left, top, right, bottom, right - left, bottom - top));
}

PluginOffsetScreen::PluginOffsetScreen()
	:eWidget(0, 1), curPos( posLeftTop ),
		left(20), top(20), right( 699 ), bottom( 555 )
{
	init_PluginOffsetScreen();
}
void PluginOffsetScreen::init_PluginOffsetScreen()
{
	backColor = eSkin::getActive()->queryColor("eWindow.titleBar");
	foreColor = eSkin::getActive()->queryColor("eWindow.titleBarFont");
	setBackgroundColor(backColor);
	setForegroundColor(foreColor);
	move(ePoint(0,0));
	resize(eSize(768,576));
	descr = new eLabel( this );
	descr->setFlags( eLabel::flagVCenter|RS_WRAP );
	descr->setForegroundColor( foreColor );
	descr->resize(eSize(568,300));
	descr->move(ePoint(100,100));
	descr->setText(_("Here you can center the TuxTxt/MultiEPG rectangle...\nPress RED to select the left top edge.\nPress GREEN to select the right bottom edge.\nUse the cursor keys to move the selected edges.\nCurrent positions - size:"));
	eSize ext = descr->getExtend();
	ext += eSize(8,4);  // the given Size of the Text is okay... but the renderer sucks...
	eDebug("offsetscreen textsize: %dx%d", ext.width(), ext.height());
	descr->resize( ext );
	descr->move( ePoint((width() - ext.width())/2, (height() - ext.height())/2 ));
	descr->show();

	cursize = new eLabel( this );
	cursize->setFlags( eLabel::flagVCenter);
	cursize->setForegroundColor( foreColor );
	cursize->resize(eSize(300,40));
	cursize->setText(eString().sprintf("%d:%d  %d:%d  -  %dx%d", left, top, right, bottom, right - left, bottom - top));
	cursize->move(ePoint(descr->getPosition().x() + (ext.width() - cursize->getSize().width())/2,
			     descr->getPosition().y() + ext.height()));
	cursize->show();

	addActionMap(&i_PluginOffsetActions->map);
	addActionMap(&i_cursorActions->map);
	addActionToHelpList( &i_PluginOffsetActions->leftTop );
	addActionToHelpList( &i_PluginOffsetActions->rightBottom );

	/* help text for tuxtext dimensions setup */
	setHelpText(_("\tTuxText/MultiEPG position\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n>>> [6] OSD Settings >>> [YELLOW] TuxText position\n" \
				". . . . . . . . . .\n\nHere you can align the horizontal and vertical position where your Dreambox outputs Windows, Messages," \
				" etc. to match your TV. Keep in mind that not all custom made plugins keep this range in mind!" \
				" Make sure both corners remain fully visible to avoid unexpected results!\n. . . . . . . . . .\n\n" \
				"Usage:\n\n[RED]\tSelect upper/left screen corner\n\n[GREEN]\tSelect lower/right screen corner\n\n" \
				"[LEFT]/[RIGHT]\tMove the selected corner around\n[UP]/[DOWN]\n\n[OK]\tSave Settings and Close Window\n\n" \
				"[EXIT]\tClose window without saving changes"));
}

void PluginOffsetScreen::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

eZapOsdSetup::eZapOsdSetup()
	:ePLiWindow(_(MENUNAMETRANS), 460)
{
	init_eZapOsdSetup();
}

void eZapOsdSetup::init_eZapOsdSetup()
{
	alpha = gFBDC::getInstance()->getAlpha();
	eLabel* l = new eLabel(this);
	l->setText(_("Transparency:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(130, widgetHeight()));
	sAlpha = new eSlider( this, l, 0, 512 );
	sAlpha->setIncrement( eSystemInfo::getInstance()->getAlphaIncrement() ); // Percent !

	sAlpha->move( ePoint( 150, yPos()) );
	sAlpha->resize(eSize( 300, widgetHeight()) );
	sAlpha->setHelpText(_("change the transparency correction"));
	sAlpha->setValue( alpha);
	CONNECT( sAlpha->changed, eZapOsdSetup::alphaChanged );

	nextYPos(35);
	brightness = gFBDC::getInstance()->getBrightness();
	l = new eLabel(this);
	l->setText(_("Brightness:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(130, widgetHeight()));
	sBrightness = new eSlider( this, l, 0, 255 );
	sBrightness->setIncrement( 5 ); // Percent !
	sBrightness->move( ePoint( 150, yPos() ) );
	sBrightness->resize(eSize( 300, widgetHeight()) );
	sBrightness->setHelpText(_("change the brightness correction"));
	sBrightness->setValue( brightness);
	CONNECT( sBrightness->changed, eZapOsdSetup::brightnessChanged );

	nextYPos(35);
	gamma = gFBDC::getInstance()->getGamma();
	l = new eLabel(this);
	l->setText(_("Contrast:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(130, widgetHeight()));
	sGamma = new eSlider( this, l, 0, 255 );
	sGamma->setIncrement( 5 ); // Percent !
	sGamma->move( ePoint( 150, yPos()) );
	sGamma->resize(eSize( 300, widgetHeight()) );
	sGamma->setHelpText(_("change the contrast"));
	sGamma->setValue( gamma);
	CONNECT( sGamma->changed, eZapOsdSetup::gammaChanged );

	nextYPos(35);
	int bcktrans = 0xC0;
	eConfig::getInstance()->getKey("/elitedvb/subtitle/backgroundTransparency", bcktrans);
	l = new eLabel(this);
	l->setText(_("DVB Subtitle Black Transparency:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(440, widgetHeight()));
	nextYPos(35);
	sSubtitleBlack = new eSlider( this, l, 0, 255 );
	sSubtitleBlack->setIncrement( 5 ); // Percent !
	sSubtitleBlack->move( ePoint( 150, yPos() ) );
	sSubtitleBlack->resize(eSize( 300, widgetHeight()) );
	sSubtitleBlack->setHelpText(_("change the transparency of the DVB subtitle black background"));
	sSubtitleBlack->setValue( bcktrans);
	CONNECT( sSubtitleBlack->changed, eZapOsdSetup::SubtitleBlackChanged );

	/* help text for OSD settings */
	setHelpText(_("\tOSD Settings\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n>>> [6] OSD Settings\n. . . . . . . . . .\n\n" \
								"Setup of your OSD (On-Screen Display, this is information display on your screen, like channel number, volume bar, Program information, etc.)\n" \
								". . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\n[OK]\tEnter selected Inputfield or button\n\n" \
								"[LEFT]/[RIGHT]\tSet Alpha blend, Brightness/Contrast\n\n[BLUE]\tChange skin Menu\n\n[YELLOW]\tFine tune TuxText position\n\n" \
								"[GREEN]\tSave Settings and Close Window\n\n[EXIT]\tClose window without saving changes"));

	buildWindow();
	CONNECT(bOK->selected, eZapOsdSetup::okPressed);
}

void eZapOsdSetup::alphaChanged( int i )
{
	alpha = i;
	gFBDC::getInstance()->setAlpha(alpha);
	invalidate();
}

void eZapOsdSetup::SubtitleBlackChanged( int i )
{
	int bcktrans = i;
	eConfig::getInstance()->setKey("/elitedvb/subtitle/backgroundTransparency", bcktrans);
}

void eZapOsdSetup::brightnessChanged( int i )
{
	brightness = i;
	gFBDC::getInstance()->setBrightness(brightness);
	invalidate();
}

void eZapOsdSetup::gammaChanged( int i )
{
	gamma = i;
	gFBDC::getInstance()->setGamma(gamma);
	invalidate();
}

void eZapOsdSetup::okPressed()
{
	gFBDC::getInstance()->saveSettings();
	eConfig::getInstance()->flush();
	close(1);
}

int eZapOsdSetup::eventHandler( const eWidgetEvent &e )
{
	switch (e.type)
	{
		case eWidgetEvent::execDone:
		{
			gFBDC::getInstance()->reloadSettings();
			break;
		}
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

void eZapOsdSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
