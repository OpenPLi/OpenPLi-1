#ifndef DISABLE_LCD
#include <setup_lcd.h>

#include <lib/gui/slider.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>
#include <lib/base/i18n.h>
#include <lib/dvb/dvbwidgets.h>
#include <lib/gdi/lcd.h>
#include <lib/system/info.h>

#define MENUNAME N_("LCD")

class eZapLCDSetupFactory : public eCallableMenuFactory
{
public:
	eZapLCDSetupFactory() : eCallableMenuFactory("eZapLCDSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapLCDSetup;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasLCD();
	}
};

eZapLCDSetupFactory eZapLCDSetup_factory;

void eZapLCDSetup::brightnessChanged( int i )
{
	eDebug("Brightness changed to %i", i);
	lcdbrightness = i;
	update(lcdbrightness, lcdcontrast);
}

void eZapLCDSetup::contrastChanged( int i )
{
	eDebug("contrast changed to %i", i);
	lcdcontrast = i;
	update(lcdbrightness, lcdcontrast);
}

void eZapLCDSetup::standbyChanged( int i )
{
	eDebug("standby changed to %i", i);
	lcdstandby = i;
	update(lcdstandby, lcdcontrast);
}

void eZapLCDSetup::invertedChanged( int i )
{
	eDebug("invertion changed to %s", (i?"inverted":"not inverted") );
	eDBoxLCD::getInstance()->setInverted( i?255:0 );
}

void eZapLCDSetup::update(int brightness, int contrast)
{
	eDBoxLCD::getInstance()->setLCDParameter(brightness, contrast);
}

eZapLCDSetup::eZapLCDSetup()
	:ePLiWindow(_(MENUNAME), 400)
{
	init_eZapLCDSetup();
}

void eZapLCDSetup::init_eZapLCDSetup()
{
	eConfig::getInstance()->getKey("/ezap/lcd/brightness", lcdbrightness);
	eConfig::getInstance()->getKey("/ezap/lcd/contrast", lcdcontrast);
	eConfig::getInstance()->getKey("/ezap/lcd/standby", lcdstandby );
	int tmp;
	eConfig::getInstance()->getKey("/ezap/lcd/inverted", tmp );
	unsigned char lcdinverted = (unsigned char) tmp;

	bbrightness=new eLabel(this);
	bbrightness->setText(_("Brightness:"));
	bbrightness->move(ePoint(10, yPos()));
	bbrightness->resize(eSize(110, widgetHeight()));

	p_brightness=new eSlider(this, bbrightness, 0, LCD_BRIGHTNESS_MAX );
	p_brightness->setName("brightness");
	p_brightness->move(ePoint(150, yPos()));
	p_brightness->resize(eSize(240, widgetHeight()));
	p_brightness->setHelpText(_("set LCD brightness ( left / right )"));
	CONNECT( p_brightness->changed, eZapLCDSetup::brightnessChanged );

	nextYPos(35);
	bcontrast=new eLabel(this);
	bcontrast->setText(_("Contrast:"));
	bcontrast->move(ePoint(10, yPos()));
	bcontrast->resize(eSize(110, widgetHeight()));

	p_contrast=new eSlider(this, bcontrast, 0, LCD_CONTRAST_MAX );
	p_contrast->setName("contrast");
	p_contrast->move(ePoint(150, yPos()));
	p_contrast->resize(eSize(240, widgetHeight()));
	p_contrast->setHelpText(_("set LCD contrast ( left / right )"));
	CONNECT( p_contrast->changed, eZapLCDSetup::contrastChanged );

	nextYPos(35);
	bstandby=new eLabel(this);
	bstandby->setText(_("Standby:"));
	bstandby->move(ePoint(10, yPos()));
	bstandby->resize(eSize(110, widgetHeight()));

	p_standby=new eSlider(this, bstandby, 0, LCD_BRIGHTNESS_MAX );
	p_standby->setName("standby");
	p_standby->move(ePoint(150, yPos()));
	p_standby->resize(eSize(240, widgetHeight()));
	p_standby->setHelpText(_("set LCD brightness for Standby Mode ( left / right )"));
	CONNECT( p_standby->changed, eZapLCDSetup::standbyChanged );

	nextYPos(35);
	inverted=new eCheckbox(this);
	inverted->move(ePoint(10, yPos()));
	inverted->resize(eSize(380, widgetHeight()));
	inverted->setText(_("Inverted"));
	inverted->setCheck(lcdinverted);
	inverted->setHelpText(_("enable/disable inverted LCD (ok)"));
	CONNECT( inverted->checked, eZapLCDSetup::invertedChanged );

	p_brightness->setValue(lcdbrightness);
	p_contrast->setValue(lcdcontrast);
	p_standby->setValue(lcdstandby);

	/* help text for LCD settings */
	setHelpText(_("\tLCD Settings\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n>>> [8] LCD Settings\n. . . . . . . . . .\n\n" \
								"Setup of the LCD screen on your Dreambox (front)\n. . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\n" \
								"[LEFT]/[RIGHT]\nSet  Brightness/Contrast/Standby(brightness)\n\nINVERTED:\tToggle inverted LCD screen\n\n" \
								"[GREEN]\tSave Settings and Close Window\n\n[EXIT]\tClose window without saving changes"));
	
	buildWindow();
	CONNECT(bOK->selected, eZapLCDSetup::okPressed);
}

eZapLCDSetup::~eZapLCDSetup()
{
}

void eZapLCDSetup::okPressed()
{
	eConfig::getInstance()->setKey("/ezap/lcd/brightness", lcdbrightness);
	eConfig::getInstance()->setKey("/ezap/lcd/contrast", lcdcontrast);
	eConfig::getInstance()->setKey("/ezap/lcd/standby", lcdstandby);
	eConfig::getInstance()->setKey("/ezap/lcd/inverted", inverted->isChecked()?255:0 );
	eConfig::getInstance()->flush();
	update(lcdbrightness, lcdcontrast);
	close(1);
}

int eZapLCDSetup::eventHandler( const eWidgetEvent& e)
{
	switch (e.type)
	{
		case eWidgetEvent::execDone:
			eConfig::getInstance()->getKey("/ezap/lcd/brightness", lcdbrightness);
			eConfig::getInstance()->getKey("/ezap/lcd/contrast", lcdcontrast);
			eDBoxLCD::getInstance()->setInverted( inverted->isChecked()?255:0 );
			update(lcdbrightness, lcdcontrast);
			break;
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

void eZapLCDSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#endif //DISABLE_LCD
