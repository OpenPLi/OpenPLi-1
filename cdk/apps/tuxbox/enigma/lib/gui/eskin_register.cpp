#include <lib/gui/eskin_register.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/gfbdc.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>

class eSkinInit
{
	eSkin default_skin;
public:
	eSkinInit()
	{

		init_eSkinInit();
	}
	void eSkinInit::init_eSkinInit()
	{
		if (default_skin.load( CONFIGDIR "/enigma/skins/default.esml"))
			if (default_skin.load( TUXBOXDATADIR "/enigma/skins/default.esml"))
				eFatal("skin load failed (" TUXBOXDATADIR "/enigma/skins/default.esml)");

		eString defaultSkin = TUXBOXDATADIR "/enigma/skins/darkpli_8.esml";
/*
		eString defaultSkin =
			eSystemInfo::getInstance()->getHwType()
				== eSystemInfo::TR_DVB272S
			?
				TUXBOXDATADIR "/enigma/skins/small_red.esml"
			:
				TUXBOXDATADIR "/enigma/skins/stone.esml";
*/
		eString skinfile=defaultSkin;

		char *temp=0;
		if (!eConfig::getInstance()->getKey("/ezap/ui/skin", temp))
		{
			skinfile=temp;
			free(temp);
		}

		if (default_skin.load(skinfile.c_str()))
		{
			eWarning("failed to load user defined skin %s, falling back to %s", skinfile.c_str(), defaultSkin.c_str() );
			if (default_skin.load(defaultSkin.c_str()))
				eFatal("couldn't load fallback skin %s", defaultSkin.c_str() );
		}

		gFBDC::getInstance()->setColorDepth(default_skin.getColorDepth());

		default_skin.parseSkins();

		default_skin.setPalette(gFBDC::getInstance());
		default_skin.makeActive();
	}
};

eAutoInitP0<eSkinInit> init_skin(eAutoInitNumbers::skin, "skin subsystem");
