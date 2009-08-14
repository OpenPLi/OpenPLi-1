#ifndef DISABLE_CI

#include <enigma_ci.h>
#include <enigma.h>
#include <lib/base/i18n.h>
#include <lib/driver/rc.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/dvbci.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

#define MENUNAME N_("CI module")

class enigmaCIFactory : public eCallableMenuFactory
{
public:
	enigmaCIFactory() : eCallableMenuFactory("enigmaCI", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new enigmaCI;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasCI();
	}
};

enigmaCIFactory enigmaCI_factory;

enigmaCI::enigmaCI()
	:ci_messages(eApp,1), ci2_messages(eApp,1)
{
	init_enigmaCI();
}

void enigmaCI::init_enigmaCI()
{
	CONNECT(ci_messages.recv_msg, enigmaCI::updateCIinfo );
	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	DVBCI=eDVB::getInstance()->DVBCI;

	setText(_(MENUNAME));
	if( eSystemInfo::getInstance()->hasCI() > 1 )
	{
		cresize(eSize(350, 330));
		valign();
		DVBCI2=eDVB::getInstance()->DVBCI2;
		CONNECT(ci2_messages.recv_msg, enigmaCI::updateCI2info );
	}
	else
	{
		cresize(eSize(350, 250));
		valign();
	}

	reset=new eButton(this);
	reset->setText(_("Reset"));
	reset->move(ePoint(10, 13));
	reset->resize(eSize(330, fd+10));
	reset->setHelpText(_("reset the Common Interface module"));
	reset->loadDeco();

	CONNECT(reset->selected, enigmaCI::resetPressed);

	init=new eButton(this);
	init->setText(_("Init"));
	init->move(ePoint(10, 53));
	init->resize(eSize(330, fd+10));
	init->setHelpText(_("send the ca-pmt to CI"));
	init->loadDeco();

	CONNECT(init->selected, enigmaCI::initPressed);		

	app=new eButton(this);
	app->setText(_("waiting for module"));
	app->move(ePoint(10, 93));
	app->resize(eSize(330, fd+10));
	app->setHelpText(_("enter Common Interface menu (mmi)"));
	app->loadDeco();

	CONNECT(app->selected, enigmaCI::appPressed);		

	if( eSystemInfo::getInstance()->hasCI() > 1 )
	{
		reset2=new eButton(this);
		reset2->setText(_("Reset"));
		reset2->move(ePoint(10, 143));
		reset2->resize(eSize(330, fd+10));
		reset2->setHelpText(_("reset the Common Interface module"));
		reset2->loadDeco();

		CONNECT(reset2->selected, enigmaCI::reset2Pressed);		

		init2=new eButton(this);
		init2->setText(_("Init"));
		init2->move(ePoint(10, 183));
		init2->resize(eSize(330, fd+10));
		init2->setHelpText(_("send the ca-pmt to CI"));
		init2->loadDeco();

		CONNECT(init2->selected, enigmaCI::init2Pressed);		

		app2=new eButton(this);
		app2->setText(_("waiting for module"));
		app2->move(ePoint(10, 223));
		app2->resize(eSize(330, fd+10));
		app2->setHelpText(_("enter Common Interface menu (mmi)"));
		app2->loadDeco();

		CONNECT(app2->selected, enigmaCI::app2Pressed);		
	}

	status = new eStatusBar(this);	
	status->move( ePoint(0, clientrect.height()-50) );
	status->resize( eSize( clientrect.width(), 50) );
	status->loadDeco();

	CONNECT(DVBCI->ci_progress, enigmaCI::gotCIinfoText);
	DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::getAppName));

	if( eSystemInfo::getInstance()->hasCI() > 1 )
	{
		CONNECT(DVBCI2->ci_progress, enigmaCI::gotCI2infoText);
		DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::getAppName));
	}
}

enigmaCI::~enigmaCI()
{
	if (status)
		delete status;
}

void enigmaCI::handleTwoServicesChecked(int val)
{
	eConfig::getInstance()->setKey("/ezap/ci/handleTwoServices", val);
}

void enigmaCI::gotCIinfoText(const char *text)
{
	// called from CI thread !!
	if (text)
		ci_messages.send(text);
}

void enigmaCI::gotCI2infoText(const char *text)
{
	// called from CI2 thread !!
	if (text)
		ci2_messages.send(text);
}

void enigmaCI::updateCIinfo(const char * const &buffer)
{
	eDebug("new info %s",buffer);
	app->setText(buffer);
}

void enigmaCI::updateCI2info(const char * const &buffer)
{
	eDebug("new info %s",buffer);
	app2->setText(buffer);
}

void enigmaCI::resetPressed()
{
	app->setText(_("resetting....please wait"));
	DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::reset));
}

void enigmaCI::reset2Pressed()
{
	app2->setText(_("resetting....please wait"));
	DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::reset));
}

void enigmaCI::initPressed()
{
	DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::init));
}

void enigmaCI::init2Pressed()
{
	DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::init));
}

void enigmaCI::appPressed()
{
	hide();
	DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_begin));
	enigmaCIMMI::getInstance(DVBCI)->exec();
	DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_end));
	show();
}

void enigmaCI::app2Pressed()
{
	hide();
	DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_begin));
	enigmaCIMMI::getInstance(DVBCI2)->exec();
	DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_end));
	show();
}

void enigmaCI::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

// -----------  CI MMI ----------------
std::map<eDVBCI*,enigmaCIMMI*> enigmaCIMMI::exist;

enigmaCIMMI* enigmaCIMMI::getInstance( eDVBCI* ci )
{
	std::map<eDVBCI*, enigmaCIMMI*>::iterator it = exist.find(ci);
	if ( it == exist.end() )
		exist[ci]=new enigmaCIMMI(ci);
	return exist[ci];
}

enigmaCIMMI::enigmaCIMMI( eDVBCI *ci )
	:ci(ci)
{
	setText(_("Common Interface Module - mmi"));
	lText->setText(_("waiting for CI answer..."));
	int newHeight = size.height() - getClientSize().height() + lText->getExtend().height() + 10 + 20;
	resize( eSize( size.width(), newHeight ) );
}

void enigmaCIMMI::sendAnswer( AnswerType ans, int param, unsigned char *data )
{
	switch(ans)
	{
		case ENQAnswer:
			ci->messages.send( eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_enqansw, param, data));
			break;
		case LISTAnswer:
		case MENUAnswer:
			ci->messages.send( eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::mmi_menuansw,param));
			break;
	}
}

void enigmaCIMMI::beginExec()
{
	conn = CONNECT(ci->ci_mmi_progress, enigmaMMI::gotMMIData );
}

#endif // DISABLE_CI
