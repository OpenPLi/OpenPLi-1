#include <parentallock.h>

#include <enigma.h>
#include <enigma_main.h>
#include <sselect.h>
#include <lib/base/i18n.h>
#include <lib/gui/emessage.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>

#define MENUNAME N_("Parental lock")

class eParentalSetupFactory : public eCallableMenuFactory
{
public:
	eParentalSetupFactory() : eCallableMenuFactory("eParentalSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eParentalSetup;
	}
};

eParentalSetupFactory eParentalSetup_factory;

ParentalLockWindow::ParentalLockWindow( const char* windowText, int curNum )
:eWindow(0)
{
	init_ParentalLockWindow(windowText,curNum);
}
void ParentalLockWindow::init_ParentalLockWindow(const char* windowText, int curNum )
{
	resize( eSize( 380, 150 ) );
	valign();
	setText(windowText);

	lPin = new eLabel(this);
	lPin->move( ePoint( 10, 10 ) );
	lPin->resize( eSize( width()-20, 30 ) );
	lPin->setText(_("please enter pin:"));
	lPin->loadDeco();

	nPin=new eNumber(this, 4, 0, 9, 1, 0, 0, lPin, 1);
	nPin->move( ePoint( 10, 50 ) );
	nPin->resize( eSize( 100, 30 ) );
	nPin->loadDeco();
	nPin->setNumber(curNum);
	nPin->setFlags( eNumber::flagHideInput );
	CONNECT( nPin->selected, ParentalLockWindow::numEntered );
}

void ParentalLockWindow::numEntered(int *i)
{
	close( nPin->getNumber() );
}

eParentalSetup::eParentalSetup():
	ePLiWindow(_(MENUNAME), 400, 75)
{
	init_eParentalSetup();
}
void eParentalSetup::init_eParentalSetup()
{
	loadSettings();

	parentallock=new eCheckbox(this, parentalpin_enabled, 1);
	parentallock->setText(_("Parental lock"));
	parentallock->move(ePoint(10, yPos()));
	parentallock->resize(eSize(200, widgetHeight()));
	parentallock->setHelpText(_("enable/disable parental lock"));
	CONNECT(parentallock->checked, eParentalSetup::plockChecked );

	changeParentalPin = new eButton(this);
	changeParentalPin->setText(_("change PIN"));
	changeParentalPin->move(ePoint(230, yPos()));
	changeParentalPin->resize(eSize(160, widgetHeight()));
	changeParentalPin->setHelpText(_("change Parental PIN (ok)"));
	changeParentalPin->loadDeco();
	CONNECT(changeParentalPin->selected_id, eParentalSetup::changePin );
	if ( !parentalpin_enabled )
	{
		changeParentalPin->hide();
	}

	nextYPos(35);  
	setuplock=new eCheckbox(this, setuppin_enabled, 1);
	setuplock->setText(_("Setup lock"));
	setuplock->move(ePoint(10, yPos()));
	setuplock->resize(eSize(200, widgetHeight()));
	setuplock->setHelpText(_("enable/disable setup lock"));
	CONNECT(setuplock->checked, eParentalSetup::slockChecked );

	changeSetupPin = new eButton(this);
	changeSetupPin->setText(_("change PIN"));
	changeSetupPin->move( ePoint( 230, yPos()));
	changeSetupPin->resize( eSize(160, widgetHeight()));
	changeSetupPin->setHelpText(_("change Setup PIN (ok)"));
	changeSetupPin->loadDeco();
	CONNECT(changeSetupPin->selected_id, eParentalSetup::changePin );
	if ( !setuppin_enabled )
	{
		changeSetupPin->hide();
	}

	nextYPos(35);  
	pin_timeout_label = new eLabel (this);
	pin_timeout_label->setText (_("Pin timeout"));
	pin_timeout_label->move (ePoint (10, yPos()));
	pin_timeout_label->resize (eSize (370, widgetHeight()));

	pin_timeout = new eNumber (this, 1, 0, 999, 4, &pintimeout, 0, pin_timeout_label);
	pin_timeout->move (ePoint (230, yPos()));
	pin_timeout->resize (eSize (50, widgetHeight()));
	pin_timeout->setHelpText (_("Number of minutes the entered pin is valid. After this timeout you need to re-enter the pin when zapping to protected service (0 to disable)"));
	pin_timeout->loadDeco();

	nextYPos(35);  
	maxpin_errors_label = new eLabel (this);
	maxpin_errors_label->setText (_("Max Pin errors"));
	maxpin_errors_label->move (ePoint (10, yPos()));
	maxpin_errors_label->resize (eSize (370, widgetHeight()));

	maxpin_errors = new eNumber (this, 1, 0, 999, 4, &maxpinerrors, 0, maxpin_errors_label);
	maxpin_errors->move (ePoint (230, yPos()));
	maxpin_errors->resize (eSize (50, widgetHeight()));
	maxpin_errors->setHelpText (_("Maximum number of chances to enter correct pin. When pin is entered wrong for x times the pin validation will be blocked temporarily (0 to disable)"));
	maxpin_errors->loadDeco();

	nextYPos(35);  
	pinerror_block_time_label = new eLabel (this);
	pinerror_block_time_label->setText (_("Pin block timeout"));
	pinerror_block_time_label->move (ePoint (10, yPos()));
	pinerror_block_time_label->resize (eSize (370, widgetHeight()));

	pinerror_block_time = new eNumber (this, 1, 1, 999, 4, &pinerrorblocktime, 0, pinerror_block_time_label);
	pinerror_block_time->move (ePoint (230, yPos()));
	pinerror_block_time->resize (eSize (50, widgetHeight()));
	pinerror_block_time->setHelpText (_("Number of minutes pincode check is disabled when pin validation has failed maximum times."));
	pinerror_block_time->loadDeco();

	nextYPos(35);  
	hidelocked=new eCheckbox(this, shidelocked, 1);
	hidelocked->setText(_("Hide locked services"));
	hidelocked->move(ePoint(10, yPos()));
	hidelocked->resize(eSize(380, widgetHeight()));
	hidelocked->setHelpText(_("don't show locked services in any list"));
	hidelocked->loadDeco();
	CONNECT(hidelocked->checked, eParentalSetup::hidelockChecked );
	if ( !parentalpin_enabled )
	{
		hidelocked->hide();
	}

	/* help text for parental setup */
	setHelpText(_("\tParental Lock\n\n>>> [MENU] >>> [6] Setup >>> [5] Parental Lock\n. . . . . . . . . .\n\n" \
								"Here you can enable and setup Parental Lock. After reboot locked channels will not be available unless unlocked with your PIN code\n" \
								". . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nParental lock\tToggle Channel access on/off\n\n" \
								"Setup lock\tToggle setup access on/off\n\nChange PIN\tEnter a new PIN code\n\n[GREEN]\tSave Settings and Close Window\n\n" \
								"[EXIT]\tClose window without saving changes"));
								

	buildWindow();

	CONNECT(bOK->selected, eParentalSetup::okPressed);
}

void eParentalSetup::plockChecked(int i)
{
	parentalpin_enabled = i;
	if ( i && !changeParentalPin->isVisible() )
	{
		// Force the user to setup a new pin
		if (ChangePin(changeParentalPin))
		{
			changeParentalPin->show();
			hidelocked->show();
		}
	}
	else
	{
		if ( pinCheck::getInstance()->checkPin(pinCheck::parental) )
		{
			parentalpin=-1;
			changeParentalPin->hide();
			hidelocked->hide();
		}
		else
		{
			hidelocked->show();
			parentallock->setCheck(1);
		}
	}
}

void eParentalSetup::slockChecked(int i)
{
	setuppin_enabled = i;
	if ( i && !changeSetupPin->isVisible() )
	{
		// Force the user to setup a new pin
		if (ChangePin(changeSetupPin))
		{
			changeSetupPin->show();
		}
		else
		{
			setuplock->setCheck(0);
		}
	}
	else
	{
		if ( pinCheck::getInstance()->checkPin(pinCheck::setup) )
		{
			setuppin=-1;
			changeSetupPin->hide();
		}
		else
			setuplock->setCheck(1);
	}
}

bool eParentalSetup::ChangePin(eButton *p)
{
	const char *text = ( p == changeParentalPin ) ? _("parental") : _("setup");

	int oldpin = (p == changeParentalPin) ? parentalpin : setuppin;
	
	int ret = 0;

	if ( oldpin >= 0 )  // let enter the oldpin.. and validate
	{
		if ( !pinCheck::getInstance()->checkPin((p == changeParentalPin) ? pinCheck::parental : pinCheck::setup, true) )
			return false;
	}

	int newPin=0;
	do
	{
		{
			ParentalLockWindow w(eString().sprintf(_("New %s PIN"),text).c_str(), 0 );
			w.show();
			newPin = w.exec();
			w.hide();
			if ( newPin == -1 ) // cancel pressed
				return false;
		}
		ParentalLockWindow w(eString().sprintf(_("Reenter %s PIN"),text).c_str(), 0 );
		w.show();
		ret = w.exec();
		w.hide();
		if ( ret == -1 ) // cancel pressed
			return false;
		else if ( ret != newPin )
		{
			int ret = eMessageBox::ShowBox(_("The PINs are not equal!\n\nDo you want to retry?"),
					_("PIN validation failed"),
					eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
					eMessageBox::btYes );
			if ( ret == eMessageBox::btNo || ret == -1 )
				return false;
		}
	}
	while ( newPin != ret );

	if ( p == changeParentalPin )
		parentalpin = newPin;
	else
		setuppin = newPin;

	eMessageBox::ShowBox(_("PIN change completed"),
		_("PIN changed"),
		eMessageBox::btOK|eMessageBox::iconInfo,
		eMessageBox::btOK );
	return true;
}

void eParentalSetup::changePin(eButton *p)
{
	ChangePin(p);
}

void eParentalSetup::hidelockChecked(int i)
{
	shidelocked = i;
}

void eParentalSetup::loadSettings()
{
	if (eConfig::getInstance()->getKey("/elitedvb/pins/parentallock", parentalpin))
		parentalpin = -1;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/enableparentallock", parentalpin_enabled))
		parentalpin_enabled = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/maxpinerrors", maxpinerrors))
		maxpinerrors = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/pinerrorblocktime", pinerrorblocktime))
		pinerrorblocktime = 1;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/setuplock", setuppin))
		setuppin = -1;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/enablesetuplock", setuppin_enabled))
		setuppin_enabled = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/hidelocked", shidelocked ))
		shidelocked = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/pintimeout", pintimeout))
		pintimeout = 0;
}

void eParentalSetup::saveSettings()
{
	eConfig::getInstance()->setKey("/elitedvb/pins/setuplock", setuppin);
	eConfig::getInstance()->setKey("/elitedvb/pins/enablesetuplock", setuppin_enabled);
	eConfig::getInstance()->setKey("/elitedvb/pins/parentallock", parentalpin);
	eConfig::getInstance()->setKey("/elitedvb/pins/enableparentallock", parentalpin_enabled);
	eConfig::getInstance()->setKey("/elitedvb/pins/maxpinerrors", maxpin_errors->getNumber());
	eConfig::getInstance()->setKey("/elitedvb/pins/pinerrorblocktime", pinerror_block_time->getNumber());
	eConfig::getInstance()->setKey("/elitedvb/hidelocked", shidelocked);
	eConfig::getInstance()->setKey("/elitedvb/pintimeout", pin_timeout->getNumber());
	eConfig::getInstance()->flush();
	eZap::getInstance()->getServiceSelector()->actualize();
}

void eParentalSetup::okPressed()
{
	saveSettings();
	close(1);
}

void eParentalSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

pinCheck* pinCheck::instance = NULL;

pinCheck::pinCheck() : locked(false), parentalpinErrorCnt(0), setuppinErrorCnt(0)
{
	resetParentalPinOk();
	resetSetupPinOk();
	resetParentalPinSetInError();
	resetSetupPinSetInError();
	time_lock_disabled.tv_sec = 0;
	time_lock_disabled.tv_usec = 0;
}

pinCheck* pinCheck::getInstance()
{
	if (instance == NULL)
	{
		instance = new pinCheck();
	}
	return instance;
}

bool pinCheck::checkPin(pinType pintype, bool allwaysValidatePin )
{
	int pin = -1;
	int pinenabled = 0;
	eString text;

	switch (pintype)
	{
		case parental:
			eConfig::getInstance()->getKey("/elitedvb/pins/enableparentallock", pinenabled);
			// Check if pincheck is enabled
			if (!pinenabled)
				return true;
			if (eConfig::getInstance()->getKey("/elitedvb/pins/parentallock", pin))
				pin = -1;
			text = "parental";
			break;
		case setup:
			eConfig::getInstance()->getKey("/elitedvb/pins/enablesetuplock", pinenabled);
			// Check if pincheck is enabled
			if (!pinenabled)
				return true;
			if (eConfig::getInstance()->getKey("/elitedvb/pins/setuplock", pin))
				pin = -1;
			text = "setup";
			break;
		default:
			// Invalid pintype
			return false;
	}

	struct timeval current_time;
	gettimeofday(&current_time, NULL);

	// Check if a pin error is currently active. If active immediatly return false
	bool error = false;
	int pinerrorblocktime;
	if (eConfig::getInstance()->getKey("/elitedvb/pins/pinerrorblocktime", pinerrorblocktime))
		pinerrorblocktime = 1;
	switch (pintype)
	{
		case parental:
			if (time_parental_set_in_error.tv_sec)
			{
				if (current_time.tv_sec < time_parental_set_in_error.tv_sec + (pinerrorblocktime * 60))
				{
					eDebug("Parental pin error still active!!");
					error = true;
				}
				else
				{
					// Longer then x time ago, reset the pin error
					resetParentalPinSetInError();
				}
			}
			break;
		case setup:
			if (time_setup_set_in_error.tv_sec)
			{
				if (current_time.tv_sec < time_setup_set_in_error.tv_sec + (pinerrorblocktime * 60))
				{
					eDebug("Setup pin error still active!!");
					error = true;
				}
				else
				{
					// Longer then x time ago, reset the pin error
					resetSetupPinSetInError();
				}
			}
			break;
	}
	if (error)
	{
		eMessageBox::ShowBox(_("Pin code validation is still blocked\n"),
				_("PIN validation failed"),
				eMessageBox::btOK|eMessageBox::iconError);
		return false;
	}

	int pintimeout;
	int pinerrors;

	int maxpinerrors;

	if (eConfig::getInstance()->getKey("/elitedvb/pintimeout", pintimeout))
		pintimeout = 0;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/maxpinerrors", maxpinerrors))
		maxpinerrors = 0;

	switch (pintype)
	{
		case parental:
			pinerrors = parentalpinErrorCnt;
			if (!allwaysValidatePin && current_time.tv_sec < last_time_parental_pinok.tv_sec + (pintimeout * 60) || (!pintimeout && last_time_parental_pinok.tv_sec))
			{
				parentalpinErrorCnt = 0;
				return true;
			}
			break;
		case setup:
			pinerrors = setuppinErrorCnt;
			if (!allwaysValidatePin && current_time.tv_sec < last_time_setup_pinok.tv_sec + (pintimeout * 60))
			{
				setuppinErrorCnt = 0;
				return true;
			}
			break;
		default:
			eDebug("Invalid pintype (%d)", pintype);
			return false;
	}

	ParentalLockWindow w(eString().sprintf(_("current %s PIN"),text.c_str()).c_str(), 0 );
	int ret = 0;
	do
	{
		w.show();
		ret = w.exec();
		w.hide();
		if ( ret == -1 )  // cancel pressed
			return false;
		else if ( ret != pin )
		{
			switch (pintype)
			{
				case parental:
					parentalpinErrorCnt++;
					pinerrors = parentalpinErrorCnt;
					break;
				case setup:
					setuppinErrorCnt++;
					pinerrors = setuppinErrorCnt;
					break;
			}
			if (maxpinerrors > 0)
			{
				eDebug("Checking maxpinerrors");
				if (pinerrors >= maxpinerrors)
				{
					// Too many pin errors has been made, no pincode for the next x minutes
					// Set the time of when this pin error occurred
					switch (pintype)
					{
						case parental:
							eDebug("Parental pin error set!");
							gettimeofday(&time_parental_set_in_error, NULL);
							break;
						case setup:
							eDebug("Setup pin error set!");
							gettimeofday(&time_setup_set_in_error, NULL);
							break;
					}
					eMessageBox::ShowBox(_("You've entered too many incorrect pins\nPin code validation will be blocked temporarily!"),
							_("PIN validation failed"),
							eMessageBox::btOK|eMessageBox::iconError);
					return false;
				}
			}
			ret = eMessageBox::ShowBox(_("The entered PIN is incorrect.\nDo you want to retry?"),
					_("PIN validation failed"),
					eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
					eMessageBox::btYes );
			if ( ret == eMessageBox::btNo || ret == -1 )
				return false;
		}
	}
	while ( ret != pin );

	switch (pintype)
	{
		case parental:
			parentalpinErrorCnt = 0;
			gettimeofday(&last_time_parental_pinok, NULL);
			break;
		case setup:
			setuppinErrorCnt = 0;
			gettimeofday(&last_time_setup_pinok, NULL);
			break;
	}

	return true;
}

void pinCheck::resetParentalPinOk()
{
	last_time_parental_pinok.tv_sec = 0;
	last_time_parental_pinok.tv_usec = 0;
}

void pinCheck::resetSetupPinOk()
{
	last_time_setup_pinok.tv_sec = 0;
	last_time_setup_pinok.tv_usec = 0;
}

void pinCheck::resetParentalPinSetInError()
{
	time_parental_set_in_error.tv_sec = 0;
	time_parental_set_in_error.tv_usec = 0;
}

void pinCheck::resetSetupPinSetInError()
{
	time_setup_set_in_error.tv_sec = 0;
	time_setup_set_in_error.tv_usec = 0;
}

int pinCheck::pLockActive()
{
	int pintimeout;
	if (eConfig::getInstance()->getKey("/elitedvb/pintimeout", pintimeout))
		pintimeout = 0;

	// Check if a pintimeout of 0 is configured, when time_lock_disabled.tv_sec != 0 a valid pin was entered before
	if ((!pintimeout && time_lock_disabled.tv_sec))
	{
		return false;
	}

	struct timeval current_time;
	gettimeofday(&current_time, NULL);

	if (!locked && !(current_time.tv_sec < time_lock_disabled.tv_sec + (pintimeout * 60)))
	{
		// timeout occured so we automatically set locked on
		locked = true;
	}

	int parentalpin = -1;
	eConfig::getInstance()->getKey("/elitedvb/pins/parentallock", parentalpin);

	int parentalpin_enabled = 0;
	eConfig::getInstance()->getKey("/elitedvb/pins/enableparentallock", parentalpin_enabled);

	int tmp = parentalpin_enabled && (parentalpin >= 0) && locked;
	if ( tmp )
	{
		int hidelocked=0;
		if (eConfig::getInstance()->getKey("/elitedvb/hidelocked", hidelocked ))
			hidelocked=0;
		if ( hidelocked )
			tmp |= 2;
	}
	return tmp;
}

void pinCheck::switchLock()
{
	// Switch lockstatus
	locked = !locked;
	if (!locked)
	{
		gettimeofday(&time_lock_disabled, NULL);
	}
	else
	{
		time_lock_disabled.tv_sec = 0;
		time_lock_disabled.tv_usec = 0;
	}
}

bool pinCheck::getParentalEnabled()
{
	int parentalpin_enabled = 0;
	eConfig::getInstance()->getKey("/elitedvb/pins/enableparentallock", parentalpin_enabled);
	int parentalpin = -1;
	eConfig::getInstance()->getKey("/elitedvb/pins/parentallock", parentalpin);
	return (bool) (parentalpin_enabled > 0 && parentalpin >= 0);
}
