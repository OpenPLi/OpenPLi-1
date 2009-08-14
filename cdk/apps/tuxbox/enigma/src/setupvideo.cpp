#include <setupvideo.h>
#include <setup_osd.h>

#include <lib/base/i18n.h>

#include <lib/driver/eavswitch.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/eaudio.h>
#include <lib/driver/rc.h>
#include <lib/driver/streamwd.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eskin.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/testpicture.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

#include <enigma_lcd.h>
                                               
#define MENUNAME N_("Audio and video")

class eZapVideoSetupFactory : public eCallableMenuFactory
{
public:
	eZapVideoSetupFactory() : eCallableMenuFactory("eZapVideoSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapVideoSetup;
	}
};

eZapVideoSetupFactory eZapVideoSetup_factory;

eZapVideoSetup::eZapVideoSetup(int wizardmode)
	:eWindow(0), Wizard(wizardmode)
{
	init_eZapVideoSetup();
}

void eZapVideoSetup::init_eZapVideoSetup()
{
/*	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "setup.video"))
		qFatal("skin load of \"setup.video\" failed");*/

/*	cresize( eSize(height(), width()) );
	cmove( ePoint(0,0) );*/

	if (eConfig::getInstance()->getKey("/elitedvb/video/colorformat", v_colorformat))
		v_colorformat = 1;

	if (eConfig::getInstance()->getKey("/elitedvb/video/pin8", v_pin8))
		v_pin8 = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/video/disableWSS", v_disableWSS ))
		v_disableWSS = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", v_tvsystem ))
		v_tvsystem = 1;
		
	if (!v_tvsystem)
		v_tvsystem = 1;

	if (eConfig::getInstance()->getKey("/elitedvb/video/vcr_switching", v_VCRSwitching ))
		v_VCRSwitching=1;

	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	setText(_(MENUNAME));
	cresize(eSize(390, Wizard ? 350 : 310));
	valign();

	eLabel *l=new eLabel(this);
	l->setText(_("Color Format:"));
	l->move(ePoint(20, 10));
	l->resize(eSize(150, fd+4));

	colorformat=new eListBox<eListBoxEntryText>(this, l);
	colorformat->loadDeco();
	colorformat->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	colorformat->move(ePoint(180, 10));
	colorformat->resize(eSize(120, 35));
	eListBoxEntryText* entrys[4];
	entrys[0]=new eListBoxEntryText(colorformat, _("CVBS"), (void*)1);
	entrys[1]=new eListBoxEntryText(colorformat, _("RGB"), (void*)2);
	entrys[2]=new eListBoxEntryText(colorformat, _("SVideo"), (void*)3);
	entrys[3]=new eListBoxEntryText(colorformat, _("YPbPr"), (void*)4);

/*	http://forum.tuxbox.org/forum/viewtopic.php?t=34005
	if( eSystemInfo::getInstance()->getHwType() > eSystemInfo::dbox2Philips  )
		entrys[3]=new eListBoxEntryText(colorformat, _("YPbPr"), (void*)4);*/

	colorformat->setCurrent(entrys[v_colorformat-1]);
	colorformat->setHelpText(_("choose color format ( left, right )"));
	CONNECT( colorformat->selchanged, eZapVideoSetup::CFormatChanged );

	l=new eLabel(this);
	l->setText(_("Aspect Ratio:"));
	l->move(ePoint(20, 45));
	l->resize(eSize(150, fd+4));
	
	pin8=new eListBox<eListBoxEntryText>(this, l);
	pin8->loadDeco();	
	pin8->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	
	pin8->move(ePoint(180, 45));
	pin8->resize(eSize(170, 35));
	pin8->setHelpText(_("choose aspect ratio ( left, right )"));
	entrys[0]=new eListBoxEntryText(pin8, _("4:3 letterbox"), (void*)0);
	entrys[1]=new eListBoxEntryText(pin8, _("4:3 panscan"), (void*)1);
	entrys[2]=new eListBoxEntryText(pin8, _("16:9"), (void*)2);
	/* dbox, dm700, dm7020 can do black bars left and right of 4:3 video */
	if ( eSystemInfo::getInstance()->getHwType() <= eSystemInfo::DM7020 )
		entrys[3]=new eListBoxEntryText(pin8, _("always 16:9"), (void*)3);
	pin8->setCurrent(entrys[v_pin8]);
	CONNECT( pin8->selchanged, eZapVideoSetup::VPin8Changed );

	l=new eLabel(this);
	l->setText(_("TV System:"));
	l->move(ePoint(20, 80));
	l->resize(eSize(150, fd+4));
	
	tvsystem=new eListBox<eListBoxEntryText>(this, l);
	tvsystem->loadDeco();	
	tvsystem->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	
	// our bitmask is:
	
	// have pal     1
	// have ntsc    2
	// have pal60   4  (aka. PAL-M bis wir PAL60 supporten)
	
	// allowed bitmasks:
	
	//  1    pal only, no ntsc
	//  2    ntsc only, no pal
	//  3    multinorm
	//  5    pal, pal60
	
	tvsystem->move(ePoint(180, 80));
	tvsystem->resize(eSize(170, 35));
	tvsystem->setHelpText(Wizard ? _("choose TV system (left, right).\nThis will reset tuxtxt position settings") : 
					_("choose TV system (left, right)."));
	entrys[0]=new eListBoxEntryText(tvsystem, "PAL", (void*)1);
	entrys[1]=new eListBoxEntryText(tvsystem, "PAL + PAL60", (void*)5);
	entrys[2]=new eListBoxEntryText(tvsystem, "Multinorm", (void*)3);
	entrys[3]=new eListBoxEntryText(tvsystem, "NTSC", (void*)2);
	
	int i = 0;
	switch (v_tvsystem)
	{
		case 1: i = 0; break;
		case 5: i = 1; break;
		case 3: i = 2; break;
		case 2: i = 3; break;
	}
	tvsystem->setCurrent(entrys[i]);
	CONNECT( tvsystem->selchanged, eZapVideoSetup::TVSystemChanged );

	c_disableWSS = new eCheckbox(this, v_disableWSS, 1);
	c_disableWSS->move(ePoint(20,115));
	c_disableWSS->resize(eSize(350,30));
	c_disableWSS->setText(_("Disable WSS on 4:3"));
	c_disableWSS->setHelpText(_("don't send WSS signal when A-ratio is 4:3"));
	CONNECT( c_disableWSS->checked, eZapVideoSetup::DisableWSSChanged );

	int sac3default = 0;
	sac3default=eAudio::getInstance()->getAC3default();

	ac3default=new eCheckbox(this, sac3default, 1);
	ac3default->setText(_("AC3 default output"));
	ac3default->move(ePoint(20, 145));
	ac3default->resize(eSize(350, 30));
	ac3default->setHelpText(_("enable/disable ac3 default output (ok)"));
	CONNECT( ac3default->checked, eZapVideoSetup::ac3defaultChanged );

	if ( eSystemInfo::getInstance()->hasScartSwitch() )
	{
		VCRSwitching=new eCheckbox(this, v_VCRSwitching, 1);
		VCRSwitching->setText(_("Auto VCR switching"));
		VCRSwitching->move(ePoint(20, 175));
		VCRSwitching->resize(eSize(350, 30));
		VCRSwitching->setHelpText(_("auto switch to VCR connector"));
		CONNECT( VCRSwitching->checked, eZapVideoSetup::VCRChanged );
	}

	ok=new eButton(this);
	ok->setText(_("Save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(20, 210));
	ok->resize(eSize(165, 35));
	ok->setHelpText(_("save changes and return"));
	ok->loadDeco();

	CONNECT(ok->selected, eZapVideoSetup::okPressed);		

	testpicture=new eButton(this);
	testpicture->setText(_("Test Screen"));
	testpicture->setShortcut("blue");
	testpicture->setShortcutPixmap("blue");
	testpicture->move(ePoint(205, 210));
	testpicture->resize(eSize(165, 35));
	testpicture->setHelpText(_("show testpicture"));
	testpicture->loadDeco();

	CONNECT(testpicture->selected, eZapVideoSetup::showTestpicture);		

	if (Wizard)
	{
		off_top = 35;
		off_left = 45;
		off_right = 650;
		off_bottom = ((v_tvsystem == 2) ? 450 : 540);
		eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/bottom", off_bottom);
		eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/left", off_left);
		eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/right", off_right);
		eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/top", off_top);
		tuxtxtpos = new eButton(this);
		tuxtxtpos->setText(_("TuxTxt/MultiEPG position"));
		tuxtxtpos->setShortcut("yellow");
		tuxtxtpos->setShortcutPixmap("yellow");
		tuxtxtpos->move(ePoint(20, 255));
		tuxtxtpos->resize(eSize(350, 35));
		tuxtxtpos->setHelpText(_("show TuxTxt/MultiEPG position window"));
		tuxtxtpos->loadDeco();

		CONNECT(tuxtxtpos->selected, eZapVideoSetup::TuxtxtPosition);		
	}

	status = new eStatusBar(this);	
	status->move( ePoint(0, clientrect.height()-50) );
	status->resize( eSize( clientrect.width(), 50) );
	status->loadDeco();

	/* help text for AV setup screen */
	setHelpText(_("\tA/V Settings\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n>>> [2] A/V Settings\n. . . . . . . . . .\n\n" \
			"Set the aspect ratio, color format, and TV System.\n. . . . . . . . . .\nUsage:\n\n" \
			"[UP]/[DOWN]\tSelect Inputfield or Button\n\nColorformat:\n[LEFT]/[RIGHT]\tSelect CVBS, RGB, SVideo, or YPbPr\n\n" \
			"Aspect Ratio:\n[LEFT]/[RIGHT]\tSelect 4:3 (Panscan/Letterbox), or 16:9\n\n[GREEN]\tSave Settings and Close Window\n\n" \
			"[EXIT]\tClose window without saving changes"));
}

eZapVideoSetup::~eZapVideoSetup()
{
	if (status)
		delete status;
}

void eZapVideoSetup::showTestpicture()
{
	hide();
	
	int mode = 1;
	while ((mode > 0) && (mode < 9))
		mode = eTestPicture::display(mode-1);
	
	show();
}

void eZapVideoSetup::TuxtxtPosition()
{
	hide();
	eCallableMenuFactory::showMenu("PluginOffsetScreen", LCDTitle, LCDElement);
	show();
}

void eZapVideoSetup::okPressed()
{
	eConfig::getInstance()->setKey("/elitedvb/video/vcr_switching", v_VCRSwitching );
	eConfig::getInstance()->setKey("/elitedvb/video/colorformat", v_colorformat );
	eConfig::getInstance()->setKey("/elitedvb/video/pin8", v_pin8 );
	eConfig::getInstance()->setKey("/elitedvb/video/disableWSS", v_disableWSS );
	eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", v_tvsystem);

	eAudio::getInstance()->saveSettings();
	eConfig::getInstance()->flush();
	eSize newsize(720, (v_tvsystem == 2) ? 480 : 576);
	eWidget::resizeRoot(newsize);
	close(1);
}

int eZapVideoSetup::eventHandler( const eWidgetEvent &e)
{
	switch(e.type)
	{
		case eWidgetEvent::execDone:
			eAVSwitch::getInstance()->reloadSettings();
			eStreamWatchdog::getInstance()->reloadSettings();
			eAudio::getInstance()->reloadSettings();
			break;
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

void eZapVideoSetup::TVSystemChanged( eListBoxEntryText *e )
{
	if (e) {
		v_tvsystem = (unsigned int) e->getKey();
		if (Wizard)
		{
			// We just set the default position on any tvsystem change in Wizard mode. 
			// this will break when the Wizard is ended with lame/exit and the last set
			// tvsystem is not the one we had when we entered the wizard.
			// Therefor we do not anything in the okPressed function!
			off_top = 35;
			off_left = 45;
			off_right = 650;
			off_bottom = ((v_tvsystem == 2) ? 450 : 540);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/bottom", off_bottom);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/left", off_left);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/right", off_right);
			eConfig::getInstance()->setKey("/enigma/plugins/needoffsets/top", off_top);
		}
		ColTVChanged();
	}
}

void eZapVideoSetup::CFormatChanged( eListBoxEntryText * e )
{
	if ( e )
	{
		v_colorformat = (unsigned int) e->getKey();
		ColTVChanged();
	}
}
		
void eZapVideoSetup::ColTVChanged()
{
	unsigned int oldtv = 0, oldcol = 1;

	eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", oldtv );
	eConfig::getInstance()->getKey("/elitedvb/video/colorformat", oldcol );

	eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", v_tvsystem);
	eConfig::getInstance()->setKey("/elitedvb/video/colorformat", v_colorformat);
	eAVSwitch::getInstance()->reloadSettings();
	eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", oldtv );
	eConfig::getInstance()->setKey("/elitedvb/video/colorformat", oldcol );
}

void eZapVideoSetup::VPin8Changed( eListBoxEntryText * e)
{
	unsigned int old = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/pin8", old);

	if ( e )
	{
		v_pin8 = (unsigned int) e->getKey();
		eConfig::getInstance()->setKey("/elitedvb/video/pin8", v_pin8 );
		eStreamWatchdog::getInstance()->reloadSettings();
		eConfig::getInstance()->setKey("/elitedvb/video/pin8", old );
	}
}

void eZapVideoSetup::DisableWSSChanged( int i )
{
	unsigned int old = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/disableWSS", old );

	v_disableWSS = (unsigned int) i;
	eConfig::getInstance()->setKey("/elitedvb/video/disableWSS", v_disableWSS );
	eStreamWatchdog::getInstance()->reloadSettings();
	eConfig::getInstance()->setKey("/elitedvb/video/disableWSS", old );
}

void eZapVideoSetup::VCRChanged( int i )
{
	unsigned int old = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/vcr_switching", old );

	v_VCRSwitching = (unsigned int) i;
	eConfig::getInstance()->setKey("/elitedvb/video/vcr_switching", v_VCRSwitching );
	eStreamWatchdog::getInstance()->reloadSettings();
	eConfig::getInstance()->setKey("/elitedvb/video/vcr_switching", old );
}

void eZapVideoSetup::ac3defaultChanged( int i )
{
	eAudio::getInstance()->setAC3default( i );
}

void eZapVideoSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#if USE_ORIGINAL_TV_SYSTEM_WIZARD

class eWizardTVSystem: public eWindow
{
	eButton *ok;
	eListBox<eListBoxEntryText> *tvsystem;
	unsigned int v_tvsystem;
	void TVSystemChanged( eListBoxEntryText * );
	void okPressed();
	int eventHandler( const eWidgetEvent &e );
public:
	eWizardTVSystem();
	static int run();
};

void eWizardTVSystem::TVSystemChanged( eListBoxEntryText *e )
{
	unsigned int old = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", old );
	if (e)
	{
		v_tvsystem = (unsigned int) e->getKey();
		eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", v_tvsystem);
		eAVSwitch::getInstance()->reloadSettings();
		eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", old );
	}
}

int eWizardTVSystem::eventHandler( const eWidgetEvent &e)
{
	switch(e.type)
	{
		case eWidgetEvent::execDone:
			eAVSwitch::getInstance()->reloadSettings();
			eStreamWatchdog::getInstance()->reloadSettings();
			break;
		case eWidgetEvent::wantClose:
			unsigned int bla;
			if ( eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", bla ) )
			    break;
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

eWizardTVSystem::eWizardTVSystem(): eWindow(0)
{
	v_tvsystem=1;
	eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", v_tvsystem );

	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	setText(_("TV System Wizard"));
	move(ePoint(160, 120));
	cresize(eSize(390, 170));

	eLabel *l=new eLabel(this);
	l->setText(_("TV System:"));
	l->move(ePoint(20, 10));
	l->resize(eSize(150, fd+4));

	tvsystem=new eListBox<eListBoxEntryText>(this, l);
	tvsystem->loadDeco();
	tvsystem->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);

	// our bitmask is:

	// have pal     1
	// have ntsc    2
	// have pal60   4  (aka. PAL-M bis wir PAL60 supporten)

	// allowed bitmasks:

	//  1    pal only, no ntsc
	//  2    ntsc only, no pal
	//  3    multinorm
	//  5    pal, pal60

	eListBoxEntryText *entrys[4];
	tvsystem->move(ePoint(180, 10));
	tvsystem->resize(eSize(170, 35));
	tvsystem->setHelpText(_("choose TV system ( left, right )"));
	entrys[0]=new eListBoxEntryText(tvsystem, "PAL", (void*)1);
	entrys[1]=new eListBoxEntryText(tvsystem, "PAL + PAL60", (void*)5);
	entrys[2]=new eListBoxEntryText(tvsystem, "Multinorm", (void*)3);
	entrys[3]=new eListBoxEntryText(tvsystem, "NTSC", (void*)2);

	int i = 0;
	switch (v_tvsystem)
	{
	case 1: i = 0; break;
	case 5: i = 1; break;
	case 3: i = 2; break;
	case 2: i = 3; break;
	}
	tvsystem->setCurrent(entrys[i]);
	CONNECT( tvsystem->selchanged, eWizardTVSystem::TVSystemChanged );

	ok=new eButton(this);
	ok->setText(_("save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(20, 65));
	ok->resize(eSize(220, 40));
	ok->setHelpText(_("save changes and return"));
	ok->loadDeco();
	CONNECT(ok->selected, eWizardTVSystem::okPressed);

	eStatusBar *status = new eStatusBar(this);
	status->move( ePoint(0, clientrect.height()-50) );
	status->resize( eSize( clientrect.width(), 50) );
	status->loadDeco();
}

void eWizardTVSystem::okPressed()
{
	eConfig::getInstance()->setKey("/elitedvb/video/tvsystem", v_tvsystem);
	eAudio::getInstance()->saveSettings();
	eConfig::getInstance()->flush();
	eSize newsize(720, (v_tvsystem == 2) ? 480 : 576);
	eWidget::resizeRoot(newsize);
	close(1);
}
#endif


class eWizardTVSystemInit
{
public:
	eWizardTVSystemInit()
	{
		if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
		{
			// only run wizard when tvsystem not yet setup'ed
			unsigned int tvsystem=0;
			if ( eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", tvsystem) )
			{
#if USE_ORIGINAL_TV_SYSTEM_WIZARD
				eWizardTVSystem w;
#else
				eZapVideoSetup w(1);
				w.setText(_("TV System Wizard"));
#endif
#ifndef DISABLE_LCD
				eZapLCD* pLCD = eZapLCD::getInstance();
				pLCD->lcdMain->hide();
				pLCD->lcdMenu->show();
    				w.setLCD( pLCD->lcdMenu->Title, pLCD->lcdMenu->Element );
#endif
				w.show();
				w.exec();
				w.hide();
#ifndef DISABLE_LCD
				pLCD->lcdMenu->hide();
				pLCD->lcdMain->show();
#endif
			}
			else
				eDebug("tvsystem already selected.. do not start tvsystem wizard");
		}
	}
};

eAutoInitP0<eWizardTVSystemInit> init_eWizardTVSystemInit(eAutoInitNumbers::wizard-1, "wizard: tv system");
