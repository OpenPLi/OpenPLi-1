#include <setup_timezone.h>

#include <config.h>
#include <lib/gui/slider.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/elabel.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/eskin.h>
#include <lib/gui/actions.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <enigma_main.h>

#ifndef ZONEINFODIR
#define ZONEINFODIR DATADIR "/zoneinfo"
#endif

eZapTimeZoneSetup::eZapTimeZoneSetup(bool showHint)
	:ePLiWindow(_("Time zone setup"), 530),
	showHint(showHint),
	parser(NULL)
{
	init_eZapTimeZoneSetup();
}
void eZapTimeZoneSetup::init_eZapTimeZoneSetup()
{
	ltimeZone=new eLabel(this);
	ltimeZone->move(ePoint(10, yPos()));
	ltimeZone->resize(eSize(500, widgetHeight()));
	ltimeZone->setText(_("Time zone:"));

	nextYPos(35);
	timeZone=new eComboBox(this, 8, ltimeZone);
	timeZone->move(ePoint(10, yPos()));
	timeZone->resize(eSize(510, widgetHeight()));
	timeZone->setHelpText(_("Select your time zone"));
	timeZone->loadDeco();
	
	errLoadTimeZone = loadTimeZonesXML();
	if (errLoadTimeZone == 0)
		errLoadTimeZone = loadTimeZones();

	buildWindow();
	CONNECT(bOK->selected, eZapTimeZoneSetup::okPressed);

	/* help text for timezone setup */
	setHelpText(_("\tTimezone Configuration\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n" \
			" >>> [1] Time Settings >>> [1] Timezone Configuration\n" \
			". . . . . . . . . .\n\nHere you can select your timezone, if changed a reboot is needed for the system to use the right time\n" \
			". . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nTime Zone:\n" \
			"Select Timezone and use [OK] to make a selection\n\n[GREEN]\tSave Settings and Close Window\n\n" \
			"[EXIT]\tClose window without saving changes"));
}

struct delString
{
	delString()
	{
	}

	bool operator()(eListBoxEntryText &e)
	{
		delete (eString*)(e.getKey());
		return false;
	}
};

eZapTimeZoneSetup::~eZapTimeZoneSetup()
{
	timeZone->forEachEntry(delString());
	delete parser;
}

void eZapTimeZoneSetup::okPressed()
{
	if (!errLoadTimeZone)
	{
		// save current selected time zone
		if ( eConfig::getInstance()->setKey("/elitedvb/timezone", ((eString*) timeZone->getCurrent()->getKey())->c_str()))
		{
			eConfig::getInstance()->delKey("/elitedvb/timezone");
			eDebug("Write timezone with error %i", eConfig::getInstance()->setKey("/elitedvb/timezone", ((eString*) timeZone->getCurrent()->getKey())->c_str()));
		}
		eConfig::getInstance()->flush();
		setTimeZone();
		if (showHint) // when run from setup menus
		{
			eMessageBox msg(_("You have to restart enigma to apply the new time zone\nRestart now?"), _("Timezone changed"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btYes );
			msg.show();
			if ( msg.exec() == eMessageBox::btYes )
				eApp->quit(2);
			msg.hide();
		}
		else // when run from wizard
		{
			eApp->quit(2);
		}
	}
	close(0);
}

void eZapTimeZoneSetup::setTimeZone()
{
	char *ctimeZone= getTimeZoneAttr("zone");
	eString zfile = eString().sprintf("/var" ZONEINFODIR "/%s", ctimeZone);
	if (access(zfile.c_str(), R_OK) == -1)
		zfile = eString().sprintf(ZONEINFODIR "/%s", ctimeZone);
	if (!ctimeZone || (system(eString().sprintf("cp %s /var/etc/localtime", zfile.c_str()).c_str() ) >> 8))
	{
		eDebug("[eZapTimeZoneSetup]: couldn't set %s as /var/etc/localtime: %m", zfile.c_str());
	}
	if (ctimeZone) free(ctimeZone);
}

int eZapTimeZoneSetup::loadTimeZonesXML()
{
	if (!parser) parser = new XMLTreeParser("ISO-8859-1");
	int done=0;
	const char *filename="/var/etc/timezone.xml";
	
	if (access(filename, R_OK) == -1)
		filename="/etc/timezone.xml";

	FILE *in=fopen(filename, "rt");
	if (!in)
	{
		eWarning("[eZapTimeZoneSetup]: unable to open %s", filename);
		return -1;
	}
	do
	{
		char buf[2048];
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		done=len<sizeof(buf);
		if (!parser->Parse(buf, len, done))
		{
			eDebug("[eZapTimeZoneSetup]: parse error: %s at line %d",
				parser->ErrorString(parser->GetErrorCode()),
				parser->GetCurrentLineNumber());
			fclose(in);
			return -1;
		}
	} while (!done);
	fclose(in);

	return 0;
}

int eZapTimeZoneSetup::loadTimeZones()
{
	XMLTreeNode *root = NULL;
	if (parser) root = parser->RootNode();
	if (!root)
		return -1;

	char *ctimeZone;
	if ( eConfig::getInstance()->getKey("/elitedvb/timezone", ctimeZone) )
		ctimeZone = NULL;

	eListBoxEntryText *cur = 0;
	for (XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
		if (!strcmp(node->GetType(), "zone"))
		{
			const char *name=node->GetAttributeValue("name");
			if (!name)
			{
				eWarning("[eZapTimeZoneSetup]: error in a file timezone.xml, no name timezone");
				return -1;
			}
			eListBoxEntryText *tz=new eListBoxEntryText( *timeZone, name, (void*) new eString(name) );
			if ( ctimeZone && !strcmp(ctimeZone, name) )
			{
				cur=tz;
			}
		} else
			eWarning("[eZapTimeZoneSetup]: error in a file timezone.xml, unknown timezone");	

	if ( timeZone->setCurrent(cur) == eComboBox::E_INVALID_ENTRY )
		timeZone->setCurrent(27);  // GMT+1

	if (ctimeZone) free(ctimeZone);

	return 0;
}

char *eZapTimeZoneSetup::getTimeZoneAttr(char * attribute)
{
	XMLTreeNode *root = NULL;
	if (parser) root = parser->RootNode();
	if (!root)
		return NULL;

	char *ctimeZone = NULL;
	if (eConfig::getInstance()->getKey("/elitedvb/timezone", ctimeZone))
		return NULL;

	for (XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
		if (!strcmp(node->GetType(), "zone"))
		{
			const char *name=node->GetAttributeValue("name"),
					*attr=node->GetAttributeValue(attribute);
//					*dst=node->GetAttributeValue("dst");
			if (!strcmp(ctimeZone, name))
			{
				free(ctimeZone);
				if (!attr)
				{
					eDebug("[eZapTimeZoneSetup]: error in timezone.xml, no %s for %s", attribute, ctimeZone);
					return NULL;
				}
				return strdup(attr);
			}
		}
		else
			eWarning("[eZapTimeZoneSetup]: error in timezone.xml, not a 'zone'  node");

	free(ctimeZone);
	
	return NULL;
}

