#include <enigma.h>
#include <enigma_main.h>
#include <enigma_mainmenu.h>
#include <enigma_setup.h>
#include <enigma_plugins.h>
#include <enigma_info.h>
#include <enigma_vcr.h>
#include <enigma_lcd.h>
#include <timer.h>
#include <parentallock.h>

#include <lib/gui/eskin.h>
#include <lib/driver/eavswitch.h>
#include <lib/gui/elabel.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/decoder.h>
#include <lib/base/i18n.h>
#include <lib/gui/guiactions.h>
#include <lib/system/init_num.h>
#include <lib/system/info.h>

struct enigmaMainmenuActions
{
	eActionMap map;
	eAction close, prev, next;
	enigmaMainmenuActions():
		map("mainmenu", _("enigma mainmenu")),
		close(map, "close", _("close the mainmenu"), eAction::prioDialog),
		prev(map, "prev", _("select previous entry"), eAction::prioDialog),
		next(map, "next", _("select next entry"), eAction::prioDialog)
	{
	}
};

eAutoInitP0<enigmaMainmenuActions> i_mainmenuActions(eAutoInitNumbers::actions, "enigma mainmenu actions");


void eMainMenu::setActive(int i)
{
	int count = MENU_ENTRIES;
	if (!eSystemInfo::getInstance()->hasScartSwitch())
		--count;
	for (int a=0; a<7; a++)
	{
		int c=(i+a+count-3)%count;
		if (a != 3)
			label[a]->setPixmap(pixmaps[c][0]);	// unselected
		else
			label[a]->setPixmap(pixmaps[c][1]); // selected
	}

	switch (i)
	{
	case 0:
		description->setText(eString("(1) ") + eString(_("TV mode")));
		break;
	case 1:
		description->setText(eString("(2) ") + eString(_("Radio mode")));
		break;
#ifndef DISABLE_FILE
	case 2:
		description->setText(eString("(3) ") + eString(_("File mode")));
		break;
	case 3:
		description->setText(eString("(4) ") + eString(_("Information")));
		break;
	case 4:
		description->setText(eString("(5) ") + eString(_("Shutdown")));
		break;
	case 5:
		description->setText(eString("(6) ") + eString(_("Setup")));
		break;
	case 6:
		description->setText(eString("(7) ") + eString(_("Games")));
		break;
	case 7:
		description->setText(eString("(8) ") + eString(_("Timer")));
		break;
	case 8:
		description->setText(eString("(9) ") + eString(_("VCR")));
		break;
#else
	case 2:
		description->setText(eString("(3) ") + eString(_("Information")));
		break;
	case 3:
		description->setText(eString("(4) ") + eString(_("Setup")));
		break;
	case 4:
		description->setText(eString("(5) ") + eString(_("Timer")));
		break;
	case 5:
		description->setText(eString("(6) ") + eString(_("VCR")));
		break;
#endif
	}
#ifndef DISABLE_LCD
	if (LCDTitle)
		LCDTitle->setText(_("Mainmenu"));
	if (LCDElement)
		LCDElement->setText( description->getText() );
#endif
}

eMainMenu::eMainMenu()
	: eWidget(0, 1),
	wnd(_("Mainmenu"),
#ifndef DISABLE_FILE	
	eSystemInfo::getInstance()->hasScartSwitch()?11:10,
#else
	eSystemInfo::getInstance()->hasScartSwitch()?8:7,
#endif
	350),
	wndShowTimer(eApp)
{
	init_eMainMenu();
}
void eMainMenu::init_eMainMenu()
{
	backgroundImage = NULL;
	eSkin *skin = eSkin::getActive();
	if (skin)
	{
		gPixmap *pixmap = skin->queryImage("menu.background");
		if (pixmap)
		{
			backgroundImage = new eLabel(NULL);
			if (backgroundImage)
			{
				backgroundImage->move(ePoint(0, 0));
				backgroundImage->resize(eSize(parent->getSize().width(), parent->getSize().height()));
				backgroundImage->setPixmap(pixmap);
				backgroundImage->show();
			}
		}
	}

	simpleMainmenu=0;
	eConfig::getInstance()->getKey("/ezap/osd/simpleMainMenu", simpleMainmenu);

	if ( !simpleMainmenu )
	{
		addActionMap(&i_mainmenuActions->map);
		addActionMap(&i_cursorActions->map);
		addActionMap(&i_shortcutActions->map);
		eLabel *background=new eLabel(this);
		background->setName("background");

		label[0]=new eLabel(this);
		label[0]->setName("l3");
		label[1]=new eLabel(this);
		label[1]->setName("l2");
		label[2]=new eLabel(this);
		label[2]->setName("l1");
		label[3]=new eLabel(this);
		label[3]->setName("m");
		label[4]=new eLabel(this);
		label[4]->setName("r1");
		label[5]=new eLabel(this);
		label[5]->setName("r2");
		label[6]=new eLabel(this);
		label[6]->setName("r3");

		description=new eLabel(this);
		description->setName("description");

		if (eSkin::getActive()->build(this, "eZapMainMenu"))
			eFatal("unable to load main menu");

			char *pixmap_name[]={
		"tv",
		"radio",
#ifndef DISABLE_FILE
		"file",
#endif
		"info",
#ifndef DISABLE_FILE
		"shutdown",
#endif
		"setup",
#ifndef DISABLE_FILE
		"games",
#endif
		"timer",
		"scart"
		};

		int count = MENU_ENTRIES;
		if (!eSystemInfo::getInstance()->hasScartSwitch())
			--count;
		for (int i=0; i<count; i++)
		{
			pixmaps[i][0]=eSkin::getActive()->queryImage(eString("mainmenu.") + eString(pixmap_name[i]) );
			pixmaps[i][1]=eSkin::getActive()->queryImage(eString("mainmenu.") + eString(pixmap_name[i]) + ".sel" );
			if (!pixmaps[i][0])
				eFatal("error, mainmenu bug, mainmenu.%s not defined", pixmap_name[i]);
			if (!pixmaps[i][1])
				eFatal("error, mainmenu bug, mainmenu.%s.sel not defined", pixmap_name[i]);
			pixmaps[i][0]->uncompressdata();
			pixmaps[i][1]->uncompressdata();
		}

		setActive(active=eZapMain::getInstance()->getMode());
	}
	else
	{
		CONNECT(wndShowTimer.timeout, eMainMenu::showWindow);
		wnd.valign();
		int entry=0;
		int count = MENU_ENTRIES;
		for (int i=0; i<count; i++)
		{
			pixmaps[i][0]=NULL;
			pixmaps[i][1]=NULL;
		}
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("TV mode"), eString().sprintf("(%d) %s", ++entry, _("TV mode")) ))->selected, eMainMenu::sel_tv);
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Radio mode"), eString().sprintf("(%d) %s", ++entry, _("Radio mode")) ))->selected, eMainMenu::sel_radio);
#ifndef DISABLE_FILE
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("File mode"), eString().sprintf("(%d) %s", ++entry, _("File mode")) ))->selected, eMainMenu::sel_file);
		if ( eSystemInfo::getInstance()->hasScartSwitch() )
			CONNECT((new eListBoxEntryMenu(wnd.getList(), _("VCR"), eString().sprintf("(%d) %s", ++entry, _("VCR")) ))->selected, eMainMenu::sel_vcr);
		new eListBoxEntryMenuSeparator(wnd.getList(), eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Timer"), eString().sprintf("(%d) %s", ++entry, _("Timer")) ))->selected, eMainMenu::sel_timer);
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Setup"), eString().sprintf("(%d) %s", ++entry, _("Setup")) ))->selected, eMainMenu::sel_setup);
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Games"), eString().sprintf("(%d) %s", ++entry, _("Games")) ))->selected, eMainMenu::sel_plugins);
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Information"), eString().sprintf("(%d) %s", ++entry, _("Information")) ))->selected, eMainMenu::sel_info);
		new eListBoxEntryMenuSeparator(wnd.getList(), eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Shutdown"), eString().sprintf("(%d) %s", ++entry, _("Shutdown")) ))->selected, eMainMenu::sel_quit);
#else
		if ( eSystemInfo::getInstance()->hasScartSwitch() )
			CONNECT((new eListBoxEntryMenu(wnd.getList(), _("VCR"), eString().sprintf("(%d) %s", ++entry, _("VCR")) ))->selected, eMainMenu::sel_vcr);
		new eListBoxEntryMenuSeparator(wnd.getList(), eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Timer"), eString().sprintf("(%d) %s", ++entry, _("Timer")) ))->selected, eMainMenu::sel_timer);
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Setup"), eString().sprintf("(%d) %s", ++entry, _("Setup")) ))->selected, eMainMenu::sel_setup);
		new eListBoxEntryMenuSeparator(wnd.getList(), eSkin::getActive()->queryImage("listbox.separator"), 0, true );
		CONNECT((new eListBoxEntryMenu(wnd.getList(), _("Information"), eString().sprintf("(%d) %s", ++entry, _("Information")) ))->selected, eMainMenu::sel_info);
#endif
	}

	/* help text for main menu */
	setHelpText(_("\tMainMenu\n\n>>> [MENU]\n. . . . . . . . . .\n\n" \
							"The MainMenu gives you access to the main control groups of your DreamBox:\n\n" \
							"  (1)TV mode\t(2)Radio mode\t(3)File mode\n  (4)Information\t(5)Shutdown\t(6)Setup\n  (7)Games\t(8)Timer\t(9)VCR\n. . . . . . . . . .\n\n" \
							"Usage:\n\n[LEFT]/[RIGHT]\tPrevious/Next Menu item\n\n[NUMBERS]\tSelect Menu item directly\n\n[EXIT]/[MENU]\tClose Menu"));
}

eMainMenu::~eMainMenu()
{
	if (backgroundImage)
	{
		backgroundImage->hide();
		delete backgroundImage;
	}
	simpleMainmenu=0;
	eConfig::getInstance()->getKey("/ezap/osd/simpleMainMenu", simpleMainmenu);

	if ( !simpleMainmenu )
	{
		int count = MENU_ENTRIES;
		if (!eSystemInfo::getInstance()->hasScartSwitch())
			--count;
		for (int i=0; i<count; i++)
		{
			if (pixmaps[i][0])
				pixmaps[i][0]->compressdata();
			if (pixmaps[i][1])
				pixmaps[i][1]->compressdata();
		}
	}
}

#ifndef DISABLE_LCD
void eMainMenu::setLCD( eWidget *LCDTitle, eWidget *LCDElement )
{
	eWidget::setLCD(LCDTitle, LCDElement);
	wnd.setLCD(LCDTitle,LCDElement);
}
#endif

void eMainMenu::sel_tv()
{
	eZapMain::getInstance()->setMode(eZapMain::modeTV, 1);
	if (!simpleMainmenu)
		close(0);
	else
		wnd.close(0);
}

void eMainMenu::sel_radio()
{
	eZapMain::getInstance()->setMode(eZapMain::modeRadio, 1);
	if (!simpleMainmenu)
		close(0);
	else
		wnd.close(0);
}

#ifndef DISABLE_FILE
void eMainMenu::sel_file()
{
	eZapMain::getInstance()->setMode(eZapMain::modeFile, 1);
	if (!simpleMainmenu)
		close(0);
	else
		wnd.close(0);
}
#endif

void eMainMenu::sel_vcr()
{
	hide();
	eZapMain::getInstance()->toggleScart(1);
	show();
}

void eMainMenu::sel_info()
{
	eZapInfo info;
#ifndef DISABLE_LCD
	info.setLCD(LCDTitle, LCDElement);
#endif
	hide();
	info.show();
	info.exec();
	info.hide();
	show();
}

void eMainMenu::sel_setup()
{
	hide();
	eCallableMenuFactory::showMenu("eZapSetup", LCDTitle, LCDElement);
	if ( !simpleMainmenu )
		setActive(active);  // --"--
	show();
}

void eMainMenu::sel_plugins()
{
#ifndef DISABLE_LCD
	eZapPlugins plugins(eZapPlugins::GamePlugin, LCDTitle, LCDElement);
#else
	eZapPlugins plugins(eZapPlugins::GamePlugin);
#endif
	hide();
	plugins.exec();
	show();
}

void eMainMenu::sel_timer()
{
	hide();
	eTimerListView setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}

void eMainMenu::sel_quit()
{
	if (!simpleMainmenu)
		close(1);
	else
		wnd.close(1);
}

int eMainMenu::eventHandler(const eWidgetEvent &event)
{
	int num=-1;
	if ( !simpleMainmenu )
	{
		switch (event.type)
		{
			case eWidgetEvent::willShow:
#ifndef DISABLE_LCD
			if (LCDTitle)
				LCDTitle->setText(_("Mainmenu"));
			if (LCDElement)
				LCDElement->setText( description->getText() );
#endif
			break;
		case eWidgetEvent::evtAction:
		{
			int count = MENU_ENTRIES;
			if (!eSystemInfo::getInstance()->hasScartSwitch())
				--count;
			if (event.action == &i_mainmenuActions->close)
				close(0);
			else if (event.action == &i_mainmenuActions->prev)
			{
				active+=count-1;
				active%=count;
				setActive(active);
			}
			else if (event.action == &i_mainmenuActions->next)
			{
				active++;
				active%=count;
				setActive(active);
			}
			else if (event.action == &i_cursorActions->ok)
				selected(active);
			else if (event.action == &i_cursorActions->cancel)
				close(0);
			else if (event.action == &i_shortcutActions->number0)
				num=9;
			else if (event.action == &i_shortcutActions->number1)
				num=0;
			else if (event.action == &i_shortcutActions->number2)
				num=1;
			else if (event.action == &i_shortcutActions->number3)
				num=2;
			else if (event.action == &i_shortcutActions->number4)
				num=3;
			else if (event.action == &i_shortcutActions->number5)
				num=4;
			else if (event.action == &i_shortcutActions->number6)
				num=5;
			else if (event.action == &i_shortcutActions->number7)
				num=6;
			else if (event.action == &i_shortcutActions->number8)
				num=7;
			else if (event.action == &i_shortcutActions->number9)
				num=8;
			else
				break;
			if (num != -1)
			{
				if (num < count)
				{
					setActive(active=num);
					selected(num);
				}
			}
			return 1;
		}
		default:
			break;
		}
	}
	else
	{
		switch (event.type)
		{
			case eWidgetEvent::willShow:
				wnd.show();
				break;
			case eWidgetEvent::willHide:
				wnd.hide();
				break;
			case eWidgetEvent::execBegin:
				wndShowTimer.start(0,true);
				return 1;
			default:
				break;
		}
	}
	return eWidget::eventHandler(event);
}

void eMainMenu::selected(int i)
{
	switch (active)
	{
	case 0:
		sel_tv();
		break;
	case 1:
		sel_radio();
		break;
#ifndef DISABLE_FILE
	case 2:
		sel_file();
		break;
	case 3:
		sel_info();
		break;
	case 4:
		sel_quit();
		break;
	case 5:
		sel_setup();
		break;
	case 6:
		sel_plugins();
		break;
	case 7:
		sel_timer();
		break;
	case 8:
		sel_vcr();
		break;
#else
	case 2:
		sel_info();
		break;
	case 3:
		sel_setup();
		break;
	case 4:
		sel_timer();
		break;
	case 5:
		sel_vcr();
		break;
#endif
	}
}

void eMainMenu::showWindow()
{
	close(wnd.exec());
}

void eMainMenu::eraseBackground(gPainter *, const eRect &where)
{
}
