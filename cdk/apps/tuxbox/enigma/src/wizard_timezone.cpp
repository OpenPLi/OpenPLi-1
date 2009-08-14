#include <wizard_timezone.h>
#include <setup_timezone.h>
#include <lib/system/init.h>
#include <lib/system/econfig.h>
#include <lib/system/init_num.h>

int eWizardTimezone::run()
{
	eZapTimeZoneSetup settimezone(false);
	char *timezone=0;
	if ( eConfig::getInstance()->getKey("/elitedvb/timezone", timezone) )
	{
		eDebug("Timezone is not set.. run Timezone Setup");
		settimezone.show();
		settimezone.exec();
		settimezone.hide();
	}
	else
	{
		free(timezone);
		eDebug("Dont run Timezone Setup.. timezone is already selected");
		settimezone.setTimeZone();
	}
	return 0;
}

class eWizardTimezoneInit
{
public:
	eWizardTimezoneInit()
	{
		eWizardTimezone::run();
	}
};

eAutoInitP0<eWizardTimezoneInit> init_eWizardTimeZoneInit(eAutoInitNumbers::wizard+2, "wizard: timezone");
