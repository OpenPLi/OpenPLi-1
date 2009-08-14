#include <lib/gui/epicon.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/dvb/decoder.h>
#include <lib/gdi/epng.h>
#include <lib/gdi/gfbdc.h>

const char *ePicon::piconPaths[] = { CONFIGDIR "/enigma/picon/", TUXBOXDATADIR "/enigma/picon/", "/media/cf/picon/", "/media/usb/picon/", "" };

ePicon::ePicon(eWidget *parent)
: eWidget(parent)
{
	picon = NULL;
	CONNECT(eServiceInterface::getInstance()->serviceEvent, ePicon::handleServiceEvent);
}

ePicon::~ePicon()
{
}

void ePicon::redrawWidget(gPainter *paint, const eRect &area)
{
	if (picon)
	{
		paint->blit(*picon, ePoint(0, 0));
	}
	else if (pixmap)
	{
		paint->blit(*pixmap, ePoint(0, 0));
	}
}

gPixmap *ePicon::loadPicon(eString &refname, const eSize &size)
{
	gPixmap *picon = NULL;
	eString name;

	for (int i = 0; piconPaths[i][0]; i++)
	{
		if (access(piconPaths[i], X_OK) < 0) continue;
		name = piconPaths[i];
		name += refname;
		name += ".png";
		if (access(name.c_str(), R_OK) < 0) continue;
		picon = loadPNG(name.c_str());
		if (picon)
		{
			picon->resize(size);
			gPixmapDC mydc(picon);
			gPainter p(mydc);
			p.mergePalette(gFBDC::getInstance()->getPixmap());
			break;
		}
	}
	return picon;
}

gPixmap *ePicon::loadPicon(eServiceReference &ref, const eSize &size)
{
	gPixmap *picon = NULL;
	eString name;
	eString refname = ref.toEnigma2CompatibleString();
	if (refname.empty()) return NULL;

	refname = refname.substr(0, refname.size() - 1);
	refname.strReplace(":", eString("_"));
	refname.upper();

	eDebug("try to load picon %s", refname.c_str());

	picon = loadPicon(refname, size);
	if (picon) return picon;

	eString fallbackrefname = ref.toString();
	if (fallbackrefname.empty() || fallbackrefname == refname) return NULL;

	fallbackrefname = fallbackrefname.substr(0, fallbackrefname.size() - 1);
	fallbackrefname.strReplace(":", eString("_"));
	fallbackrefname.upper();

	eDebug("try to fallback to picon %s", fallbackrefname.c_str());

	return loadPicon(fallbackrefname, size);
}

void ePicon::handleServiceEvent(const eServiceEvent &event)
{
	if (Decoder::locked == 2)
	{
		/* timer zap in background */
		return;
	}

	switch (event.type)
	{
	case eServiceEvent::evtStart:
		{
			eServiceReference &ref = eServiceInterface::getInstance()->service;
			if (picon != pixmap)
			{
				delete picon;
			}
			picon = loadPicon(ref, getSize());
			invalidate();
		}
		break;
	case eServiceEvent::evtStop:
		if (picon != pixmap)
		{
			delete picon;
		}
		picon = NULL;
		invalidate();
		break;
	default:
		break;
	}
}

static eWidget *create_ePicon(eWidget *parent)
{
	return new ePicon(parent);
}

class ePiconSkinInit
{
public:
	ePiconSkinInit()
	{
		eSkin::addWidgetCreator("ePicon", create_ePicon);
	}
	~ePiconSkinInit()
	{
		eSkin::removeWidgetCreator("ePicon", create_ePicon);
	}
};

eAutoInitP0<ePiconSkinInit> init_ePiconSkinInit(eAutoInitNumbers::guiobject, "ePicon");
