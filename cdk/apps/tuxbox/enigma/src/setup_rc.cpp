#include <setup_rc.h>

#include <lib/gui/slider.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/elabel.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eskin.h>
#include <lib/gui/actions.h>
#include <lib/system/econfig.h>
#include <lib/base/i18n.h>
#include <lib/dvb/dvbwidgets.h>
#include <lib/driver/rc.h>
#include <enigma.h>
#include <enigma_main.h>

#define MENUNAME N_("Remote control")

class eZapRCSetupFactory : public eCallableMenuFactory
{
public:
	eZapRCSetupFactory() : eCallableMenuFactory("eZapRCSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapRCSetup;
	}
};

eZapRCSetupFactory eZapRCSetup_factory;

void eZapRCSetup::repeatChanged( int i )
{
	eDebug("Repeat rate changed to %i", i);
	rrate = 250-i;
	update();
}

void eZapRCSetup::delayChanged( int i )
{
	eDebug("Repeat delay changed to %i", i);
	rdelay = i;
	update();
}

void eZapRCSetup::update()
{
	eRCInput::getInstance()->config.set(rdelay, rrate);
}

eZapRCSetup::eZapRCSetup()
	:ePLiWindow(_(MENUNAME), 470)
{
	init_eZapRCSetup();
}

void eZapRCSetup::init_eZapRCSetup()
{
	eConfig::getInstance()->getKey("/ezap/rc/repeatRate", rrate);
	eConfig::getInstance()->getKey("/ezap/rc/repeatDelay", rdelay);
	rrate = 250 - rrate;

	lrrate=new eLabel(this);
	lrrate->setText(_("Repeat rate:"));
	lrrate->move(ePoint(10, yPos()));
	lrrate->resize(eSize(170, widgetHeight()));

	srrate=new eSlider(this, lrrate, 0, 250 );
	srrate->setName("rrate");
	srrate->move(ePoint(240, yPos()));
	srrate->resize(eSize(220, widgetHeight()));
	srrate->setHelpText(_("change remote control repeat rate\nleft => less, right => more (... repeats)"));
	CONNECT( srrate->changed, eZapRCSetup::repeatChanged );
	
	nextYPos(35);
	lrdelay=new eLabel(this);
	lrdelay->setText(_("Repeat delay:"));
	lrdelay->move(ePoint(10, yPos()));
	lrdelay->resize(eSize(170, widgetHeight()));

	srdelay=new eSlider(this, lrdelay, 0, 1000 );
	srdelay->setName("rdelay");
	srdelay->move(ePoint(240, yPos()));
	srdelay->resize(eSize(220, widgetHeight()));
	srdelay->setHelpText(_("change remote control repeat delay\nleft => shorter, right => longer (...delay)"));
	CONNECT( srdelay->changed, eZapRCSetup::delayChanged );

	nextYPos(35);
	lNextCharTimeout = new eLabel(this);
	lNextCharTimeout->move(ePoint(10, yPos()));
	lNextCharTimeout->resize(eSize(300, widgetHeight()));
	lNextCharTimeout->setText(_("Next character timeout:"));

	unsigned int t;
	if (eConfig::getInstance()->getKey("/ezap/rc/TextInputField/nextCharTimeout", t) )
		t=0;
	NextCharTimeout = new eNumber(this,1,0,3999,4,0,0,lNextCharTimeout);
	NextCharTimeout->move(ePoint(395, yPos()));
	NextCharTimeout->resize(eSize(65, widgetHeight()));
	NextCharTimeout->loadDeco();
	NextCharTimeout->setHelpText(_("cursor to next char timeout(msek) in textinputfields"));
	NextCharTimeout->setNumber(t);
	CONNECT(NextCharTimeout->selected, eZapRCSetup::nextField);

	nextYPos(35);
	lrcChannel=new eLabel(this);
	lrcChannel->move(ePoint(10, yPos()));
	lrcChannel->resize(eSize(220, widgetHeight()));
	lrcChannel->setText(_("Remote control channel:"));

	rcChannel=new eComboBox(this, 5, lrcChannel);
	rcChannel->move(ePoint(240, yPos()));
	rcChannel->resize(eSize(220, widgetHeight()));
	rcChannel->setHelpText(_("Select the channel to be used for the remote control"));
	rcChannel->loadDeco();

	new eListBoxEntryText(*rcChannel, _("All"), (void *)(0x8f));
	new eListBoxEntryText(*rcChannel, _("1"), (void *)(1));
	new eListBoxEntryText(*rcChannel, _("2"), (void *)(2));
	new eListBoxEntryText(*rcChannel, _("3"), (void *)(4));
	new eListBoxEntryText(*rcChannel, _("4"), (void *)(8));
	new eListBoxEntryText(*rcChannel, _("80"), (void *)(0x80));

	int currentChannel = 0x8f;
	eConfig::getInstance()->getKey("/ezap/rc/channel", currentChannel);
	rcChannel->setCurrent((void *)currentChannel);

	nextYPos(35);
	lrcStyle=new eLabel(this);
	lrcStyle->move(ePoint(10, yPos()));
	lrcStyle->resize(eSize(220, widgetHeight()));
	lrcStyle->setText(_("Remote control style:"));

	rcStyle=new eComboBox(this, 3, lrcStyle);
	rcStyle->move(ePoint(240, yPos()));
	rcStyle->resize(eSize(220, widgetHeight()));
	rcStyle->setHelpText(_("select your favourite RC style (ok)"));
	rcStyle->loadDeco();
	CONNECT( rcStyle->selchanged, eZapRCSetup::styleChanged );

	eListBoxEntryText *current=0;
	const std::set<eString> &activeStyles=eActionMapList::getInstance()->getCurrentStyles();
	for (std::map<eString, eString>::const_iterator it(eActionMapList::getInstance()->getExistingStyles().begin())
		; it != eActionMapList::getInstance()->getExistingStyles().end(); ++it)
	{
		if (activeStyles.find(it->first) != activeStyles.end())
		{
			current = new eListBoxEntryText( *rcStyle, it->second, (void*) &it->first );
			curstyle = it->first;
		}
		else
			new eListBoxEntryText( *rcStyle, it->second, (void*) &it->first );
	}
	if (current)
		rcStyle->setCurrent( current );

	srdelay->setValue(rdelay);
	srrate->setValue(rrate);

	/* help text for RC setup screen */
	setHelpText(_("\tRemote Control\n\n>>> [MENU] >>> [6] Setup >>> [6] Expert Setup\n>>> [3] Remote Control\n. . . . . . . . . .\n\n" \
								"Setup for your Remote Control\n. . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nRepeat Rate/Repeat Delay:\n" \
								"[LEFT]/[RIGHT]\tReduce/Increase Repeat Rate/Delay\n\nRemotecontrol Style:\n[UP]/[DOWN]\tSelect ENIGMA or Neutrino keylayout\n\n" \
								"Next Char Timeout:\n[NUMBERS]\tMilliSeconds before moving to next\n\tCharacter in Text inputfields\n\n" \
								"[GREEN]\tSave Settings and Close Window\n\n[EXIT]\tClose window without saving changes"));

	buildWindow();
	CONNECT(bOK->selected, eZapRCSetup::okPressed);
}

eZapRCSetup::~eZapRCSetup()
{
}

void eZapRCSetup::nextField(int *)
{
	focusNext(eWidget::focusDirNext);
}

void eZapRCSetup::styleChanged( eListBoxEntryText* e)
{
	if (e)
	{
		eActionMapList::getInstance()->deactivateStyle( curstyle );
		eActionMapList::getInstance()->activateStyle( curstyle = *(eString*)e->getKey() );
	}
}

void eZapRCSetup::okPressed()
{
	// save current selected style
	eConfig::getInstance()->setKey("/ezap/rc/style", curstyle.c_str());

	eZap::getInstance()->getServiceSelector()->setKeyDescriptions();
	setStyle();

	rrate = 250 - srrate->getValue();
	eConfig::getInstance()->setKey("/ezap/rc/repeatRate", rrate);
	eConfig::getInstance()->setKey("/ezap/rc/repeatDelay", rdelay);
	unsigned int t = (unsigned int) NextCharTimeout->getNumber();
	eConfig::getInstance()->setKey("/ezap/rc/TextInputField/nextCharTimeout", t );

   
	int channel = (int)rcChannel->getCurrent()->getKey();
	eConfig::getInstance()->setKey("/ezap/rc/channel",channel);
	// Activate current channel now
	char command[32];
	sprintf(command, "echo %x >/proc/stb/ir/rc/mask", channel);
	system(command);

	eConfig::getInstance()->flush();
	close(1);
}

int eZapRCSetup::eventHandler( const eWidgetEvent & e )
{
	switch(e.type)
	{
		case eWidgetEvent::execDone:
			setStyle();
			eConfig::getInstance()->getKey("/ezap/rc/repeatRate", rrate);
			eConfig::getInstance()->getKey("/ezap/rc/repeatDelay", rdelay);
			update();
			break;
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

void eZapRCSetup::setStyle()
{
	eActionMapList::getInstance()->deactivateStyle(curstyle);

	char *style=0;
	if (eConfig::getInstance()->getKey("/ezap/rc/style", style) )
		eActionMapList::getInstance()->activateStyle("default");
	else
	{
		eActionMapList::getInstance()->activateStyle( style );
		free(style);
	}
}

void eZapRCSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
