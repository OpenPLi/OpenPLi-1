#include <showbnversion.h>
#include <lib/driver/rc.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/si.h>
#include <lib/gui/elabel.h>
#include <lib/dvb/dvbservice.h>
#include <lib/gui/guiactions.h>
#include <lib/gdi/font.h>

/*
	was hier fehlt: parsen der BAT (batid: 5001) auf 0001:0085. daher wird 0001:0085:0F03 assumed.
	ebenfalls: ContentDescriptor, stream_identifier fuer population-zuordnung.
	
	extrahieren und tag anzeigen sowie decompilieren/simulieren um "Starte ..." zu ermitteln.
*/

class BNDirectory: public eSection
{
	int sectionRead(__u8 *d);
	void sectionFinish(int err);
	eLabel *result;
	eString text;
public:
	BNDirectory(int pid, eString text, eLabel *res);
};

int BNDirectory::sectionRead(__u8 *d)
{
	d+=0x117;
	eString versions=text;
	int first=1;
	while (1)
	{
		int l=*d++;
		if (l>0x10)	// ugly
			break;
		eString dst="", file="", dst2="", version="";
		while (l--)
			dst+=*d++;
		l=*d++;
		while (l--)
			file+=*d++;
		l=*d++;
		while (l--)
			dst2+=*d++;
		d+=12;
		l=*d++;
		while (l--)
			version+=*d++;
		d+=17;
		eDebug("%s/%s/%s/%s", dst.c_str(), file.c_str(), dst2.c_str(), version.c_str());
		if (versions.find(version)==eString::npos)
		{
			if (!first)
				versions+=", ";
			first=0;
			versions+=version;
		}
	}
	result->setText(versions);
	return 1;
}

void BNDirectory::sectionFinish(int err)
{
}

BNDirectory::BNDirectory(int pid, eString text, eLabel *res): eSection(pid, 0x80, -1, -1, 0), result(res), text(text)
{
}

void ShowBNVersion::willShow()
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
	{
		text->setText(_("Service system unavailable"));
		return;
	}

			// try astra 19.2 and cable.
	if (sapi->switchService(eServiceReferenceDVB(eDVBNamespace(192<<16), eTransportStreamID(0x0001), eOriginalNetworkID(0x0085), eServiceID(0x0F03), -1)) &&
			sapi->switchService(eServiceReferenceDVB(eDVBNamespace(0), eTransportStreamID(0x0001), eOriginalNetworkID(0x0085), eServiceID(0x0F03), -1)))
		text->setText(_("Transponder not found. Do a channelsearch first."));
	else
		text->setText(_("Tuning in transponder..."));
}

int ShowBNVersion::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if ((event.action == &i_cursorActions->ok) || (event.action == &i_cursorActions->cancel))
			close(0);
		else
			break;
		return 1;
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

void ShowBNVersion::willHide()
{
}

void ShowBNVersion::eventOccured(const eDVBEvent &event)
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
		return;

	if (event.type==eDVBServiceEvent::eventServiceSwitched)
	{
		if ((sapi->service.getServiceID()==eServiceID(0x0F03)) &&
				(sapi->service.getOriginalNetworkID()==eOriginalNetworkID(0x0085)))
		{
			text->setText(_("Tuned in transponder.\nReading version information..."));
			PMT *pmt=eDVB::getInstance()->getPMT();
			if (pmt)
			{
				int pid[2];
				int n=0;
				for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end() && n<2; ++i)
					pid[n++]=i->elementary_PID;
				
				if (n>=1)
				{
					bnd[1]=new BNDirectory(pid[0], "pop=1: ", res1);
					bnd[1]->start();
				}
				
				if (n>=2)
				{
					bnd[0]=new BNDirectory(pid[1], "pop=2: ", res2);
					bnd[0]->start();
				}
				pmt->unlock();
			}
		}
	}
	if (event.type==eDVBServiceEvent::eventServiceFailed)
		text->setText(_("Tune failed. Please do a channelsearch first."));
}

ShowBNVersion::ShowBNVersion(): eWindow(1)
{
	init_ShowBNVersion();
}
void ShowBNVersion::init_ShowBNVersion()
{
	setText("Show current BN version");
	cresize(eSize(400, 300));
	valign();
	
	text=new eLabel(this);
	text->move(ePoint(10, 40));
	text->resize(eSize(380, 60));
	text->setFlags(RS_WRAP);
	
	res1=new eLabel(this);
	res1->move(ePoint(10, 100));
	res1->resize(eSize(380, 30));
	
	res2=new eLabel(this);
	res2->move(ePoint(10, 130));
	res2->resize(eSize(380, 30));
	
	CONNECT(eDVB::getInstance()->eventOccured, ShowBNVersion::eventOccured);
	bnd[0]=0;
	bnd[1]=0;

	/* help text for BN version screen */
	setHelpText(_("\tShow current BN version\n\n-> [MENU] -> ->(4)Information ->Show BN Version\n. . . . . . . . . .\n\n" \
								"Show current BN Version\n. . . . . . . . . .\n\nUsage:\n\nCurrent BN-Version\n\n[GREEN]/[OK]/[EXIT]\tClose window"));
}

ShowBNVersion::~ShowBNVersion()
{
	delete bnd[0];
	delete bnd[1];
}
