#ifdef ENABLE_RFMOD

#include <setup_rfmod.h>

#include <lib/base/i18n.h>

#include <lib/driver/rc.h>
#include <lib/driver/rfmod.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>

#define MENUNAME N_("UHF modulator")

class eZapRFmodSetupFactory : public eCallableMenuFactory
{
public:
	eZapRFmodSetupFactory() : eCallableMenuFactory("eZapRFmodSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapRFmodSetup;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasRFMod();
	}
};

eZapRFmodSetupFactory eZapRFmodSetup_factory;

eZapRFmodSetup::eZapRFmodSetup()
	:eWindow(0)
{
	init_eZapRFmodSetup();
}

void eZapRFmodSetup::init_eZapRFmodSetup()
{
//	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	setText(_(MENUNAME));
	cresize(eSize(390, 390));
	valign();

	standby=0;
	eConfig::getInstance()->getKey("/elitedvb/rfmod/standby", standby);
	Standby=new eCheckbox(this);
	Standby->setText(_("UHF Modulator on"));
	Standby->move(ePoint(70, 20));
	Standby->resize(eSize(250, 40));
	Standby->setHelpText(_("enable UHF Modulator"));
	CONNECT(Standby->selected, eZapRFmodSetup::Standby_selected);
	if (!standby)
		Standby->setCheck(!standby);

	TestPatternEnable=new eCheckbox(this);
	TestPatternEnable->setText(_("Test Pattern"));
	TestPatternEnable->move(ePoint(70, 60));
	TestPatternEnable->resize(eSize(250, 40));
	TestPatternEnable->setHelpText(_("enable test pattern"));
	TestPatternEnable->loadDeco();
	CONNECT(TestPatternEnable->selected, eZapRFmodSetup::TestPatternEnable_selected);		

	soundenable=0;
	eConfig::getInstance()->getKey("/elitedvb/rfmod/so",soundenable);
	SoundEnable=new eCheckbox(this);
	SoundEnable->setText(_("Sound enable"));
	SoundEnable->move(ePoint(70, 100));
	SoundEnable->resize(eSize(250, 40));
	SoundEnable->setHelpText(_("enable Sound"));
	SoundEnable->loadDeco();
	if(!soundenable)
		SoundEnable->setCheck(1);
	CONNECT(SoundEnable->selected, eZapRFmodSetup::SoundEnable_selected);		

	sscl=new eLabel(this);
	sscl->setText(_("Sound Subcarrier:"));
	sscl->move(ePoint(50,155));
	sscl->resize(eSize(200,40));

	ssc=5500;
	eConfig::getInstance()->getKey("/elitedvb/rfmod/ssc",ssc);
	SoundSubcarrier=new eListBox<eListBoxEntryText>(this,sscl);
	SoundSubcarrier->loadDeco();
	SoundSubcarrier->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	SoundSubcarrier->move(ePoint(230, 155));
	SoundSubcarrier->resize(eSize(100,34));
	eListBoxEntryText* sscentrys[4];
	sscentrys[0]=new eListBoxEntryText(SoundSubcarrier,_("4.5 MHz"),(void*)4500,eTextPara::dirCenter);
	sscentrys[1]=new eListBoxEntryText(SoundSubcarrier,_("5.5 MHz"),(void*)5500,eTextPara::dirCenter);
	sscentrys[2]=new eListBoxEntryText(SoundSubcarrier,_("6.0 MHz"),(void*)6000,eTextPara::dirCenter);
	sscentrys[3]=new eListBoxEntryText(SoundSubcarrier,_("6.5 MHz"),(void*)6500,eTextPara::dirCenter);

	for(int i=0;i<4;i++)
	{
		if((int)sscentrys[i]->getKey() == ssc)
		{
			SoundSubcarrier->setCurrent(sscentrys[i]);
			break;
		}
	}
	SoundSubcarrier->setHelpText(_("change sound subcarrier frequency"));
	CONNECT(SoundSubcarrier->selchanged, eZapRFmodSetup::SoundSubcarrier_selected);		

	cl=new eLabel(this);
	cl->setText(_("Channel Nr.:"));
	cl->move(ePoint(50,195));
	cl->resize(eSize(200,40));

	chan=21;
	eConfig::getInstance()->getKey("/elitedvb/rfmod/channel",chan);

	Channel=new eListBox<eListBoxEntryText>(this,cl);
	Channel->loadDeco();
	Channel->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	Channel->move(ePoint(230,195));
	Channel->resize(eSize(100,34));

	eListBoxEntryText* clentrys[49];

	for(int i=21;i<70;i++)
	{
		clentrys[i-21]=new eListBoxEntryText(Channel,eString().sprintf("%d",i),(void*)i,eTextPara::dirCenter);
	}

	Channel->setCurrent(clentrys[chan-21]);
	Channel->setHelpText(_("change channel"));
	CONNECT(Channel->selchanged, eZapRFmodSetup::Channel_selected);		

	ftl=new eLabel(this);
	ftl->setText(_("Fine Tune:"));
	ftl->move(ePoint(50,235));
	ftl->resize(eSize(200,40));

	finetune=0;
	eConfig::getInstance()->getKey("/elitedvb/rfmod/finetune",finetune);
	FineTune=new eListBox<eListBoxEntryText>(this,ftl);
	FineTune->loadDeco();
	FineTune->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	FineTune->move(ePoint(230, 235));
	FineTune->resize(eSize(100,34));

	eListBoxEntryText* flentrys[81];

	for(int i=-40;i<41;i++)
	{
		if(i>0)
			flentrys[i+40]=new eListBoxEntryText(FineTune,eString().sprintf("+%d",i),(void*)i,eTextPara::dirCenter);
		else
			flentrys[i+40]=new eListBoxEntryText(FineTune,eString().sprintf("%d",i),(void*)i,eTextPara::dirCenter);
	}

	FineTune->setCurrent(flentrys[finetune+40]);
	FineTune->setHelpText(_("250Khz steps"));
	CONNECT(FineTune->selchanged, eZapRFmodSetup::FineTune_selected);

	ok=new eButton(this);
	ok->setText(_("save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(20, 285));
	ok->resize(eSize(220, 40));
	ok->setHelpText(_("save settings and leave rf setup"));
	ok->loadDeco();
	CONNECT(ok->selected, eWidget::accept);

	status = new eStatusBar(this);	
	status->move( ePoint(0, clientrect.height()-50) );
	status->resize( eSize( clientrect.width(), 50) );
	status->loadDeco();

	Standby_selected();
}

void eZapRFmodSetup::TestPatternEnable_selected()
{
	eRFmod::getInstance()->setTestPattern((int)1);		

	eMessageBox::ShowBox(_("if you can read this your rfmod will not work."),_("Test Pattern"),eMessageBox::iconWarning|eMessageBox::btOK);

	TestPatternEnable->setCheck(0);
	eRFmod::getInstance()->setTestPattern((int)0);		
}

void eZapRFmodSetup::SoundEnable_selected()
{
	int val;
	
	if(SoundEnable->isChecked())
		val=0;	
	else
		val=1;		
		
	eRFmod::getInstance()->setSoundEnable(val);		
}

void eZapRFmodSetup::Standby_selected()
{
	int val;

	if(Standby->isChecked())
	{
		sscl->show();
		cl->show();
		ftl->show();
		TestPatternEnable->show();
		SoundEnable->show();
		SoundSubcarrier->show();
		Channel->show();
		FineTune->show();
		SoundEnable_selected();
		val=0;
	}
	else
	{
		sscl->hide();
		cl->hide();
		ftl->hide();
		TestPatternEnable->hide();
		SoundEnable->hide();
		SoundSubcarrier->hide();
		Channel->hide();
		FineTune->hide();
		val=1;
	}
	eRFmod::getInstance()->setStandby(val);
}

int eZapRFmodSetup::eventHandler( const eWidgetEvent &e)
{
	switch(e.type)
	{
		case eWidgetEvent::wantClose:
			if ( !e.parameter )
				eRFmod::getInstance()->save();  // store values in registry..
			else
			{
				// reset original values..
				eRFmod::getInstance()->setSoundEnable(soundenable);
				eRFmod::getInstance()->setSoundSubCarrier(ssc);
				eRFmod::getInstance()->setChannel(chan);
				eRFmod::getInstance()->setFinetune(finetune);
				eRFmod::getInstance()->setStandby(standby);
			}
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}


void eZapRFmodSetup::SoundSubcarrier_selected(eListBoxEntryText* entry)
{
	eRFmod::getInstance()->setSoundSubCarrier((int)SoundSubcarrier->getCurrent()->getKey());
}

void eZapRFmodSetup::Channel_selected(eListBoxEntryText* entry)
{
	eRFmod::getInstance()->setChannel((int)Channel->getCurrent()->getKey());
}

void eZapRFmodSetup::FineTune_selected(eListBoxEntryText* entry)
{
	eRFmod::getInstance()->setFinetune((int)FineTune->getCurrent()->getKey());
}

void eZapRFmodSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#endif //ENABLE_RFMOD
