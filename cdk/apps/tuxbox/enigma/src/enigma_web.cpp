#include <lib/system/http_dyn.h>
#include <lib/dvb/service.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/servicemp3.h>
#include <epgwindow.h>

extern eString httpUnescape(const eString &string);
extern eString httpEscape(const eString &string);
extern std::map<eString,eString> getRequestOptions(eString opt, char delimiter = '&');
extern eString ref2string(const eServiceReference &r);
extern eServiceReference string2ref(const eString &service);

eString xmlEscape(const eString &string)
{
	eString ret="";
	for (unsigned int i=0; i<string.length(); ++i)
	{
		int c=string[i];
		
		if (c == '&')
			ret+="&amp;";
		else
			ret+=c;
	}
	return ret;
}


static const eString xmlversion="<?xml version=\"1.0\"?>\n";
static inline eString xmlstylesheet(const eString &ss)
{
	return eString("<?xml-stylesheet type=\"text/xsl\" href=\"/stylesheets/") + ss + ".xsl\"?>\n";
}

static eString web_root(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString ret;
	content->local_header["Content-Type"]="text/xml; charset=utf-8";
	
	ret=xmlversion;
	ret+=xmlstylesheet("services");
	ret+="<services>\n";
	
	for (int i=0; i<10; ++i)
		ret+="<service><name>" + eString().sprintf("Service #%d", i) + "</name></service>\n";
	
	ret+="</services>\n";
	return ret;
}

class eServiceToXml: public Object
{
	eString &result;
	eServiceInterface &iface;
public:
	eServiceToXml(eString &result, eServiceInterface &iface): result(result), iface(iface)
	{
	}
	void addEntry(const eServiceReference &e)
	{
		result+="<service>\n";
		result+="<reference>" + ref2string(e) + "</reference>\n";
		eService *service=iface.addRef(e);
		if (service)
		{
			result+="<name>" + xmlEscape(service->service_name) + "</name>\n";
			if (service->dvb)
			{
				result+="<dvb><namespace>";
				result+=eString().setNum(service->dvb->dvb_namespace.get(), 0x10);
				result+="</namespace><tsid>";
				result+=eString().setNum(service->dvb->transport_stream_id.get(), 0x10);
				result+="</tsid><onid>";
				result+=eString().setNum(service->dvb->original_network_id.get(), 0x10);
				result+="</onid><sid>";
				result+=eString().setNum(service->dvb->service_id.get(), 0x10);
				result+="</sid><type>";
				result+=eString().setNum(service->dvb->service_type, 0x10);
				result+="</type><provider>";
				result+=xmlEscape(service->dvb->service_provider);
				result+="</provider><number>";
				result+=eString().setNum(service->dvb->service_number, 10);
				result+="</number></dvb>\n";
			}
#ifndef DISABLE_FILE
			if (service->id3)
			{
				std::map<eString, eString> & tags = service->id3->getID3Tags();
				result+="<id3>";
				for (std::map<eString, eString>::iterator i(tags.begin()); i != tags.end(); ++i)
					result+="<tag id=\"" + i->first + "\"><" + i->second + "<tag/>\n";
				result+="</id3>";
			}
#endif
		}
		iface.removeRef(e);
		result+="</service>\n";
	}
};

static eString xml_services(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	std::map<eString,eString> opts=getRequestOptions(opt);
	eString spath=opts["path"];
	
	eServiceInterface *iface=eServiceInterface::getInstance();

	if (!iface)
		return "n/a\n";

	eString current;

	unsigned int pos;
	if ((pos=spath.rfind(';')) != eString::npos)
	{
		current=spath.mid(pos+1);
		spath=spath.left(pos);
	} else
	{
		current=spath;
		spath="";
	}
	
	eServiceReference current_service=string2ref(current);
	
	if (!opts["path"])
		current_service=eServiceReference(eServiceReference::idStructure,
			eServiceReference::isDirectory, 0);
	
	eDebug("current_service: %s", current_service.path.c_str());

	eString res;
	
	eServiceToXml conv(res, *iface);

	Signal1<void,const eServiceReference&> signal;
	signal.connect(slot(conv, &eServiceToXml::addEntry));

	res=xmlversion;
	res+=xmlstylesheet("services");
	res+="<services>\n";

	iface->enterDirectory(current_service, signal);
	iface->leaveDirectory(current_service);
	
	res+="</services>\n";

	return res;
}

class eHTTPLog: public eHTTPDataSource, public Object
{
	int mask, format;
	int ok, last;
	void recvMessage(int lvl, const eString &str);
	eString toWrite;
	void init_eHTTPLog();
public:
	eHTTPLog(eHTTPConnection *c, int mask, int format);
	~eHTTPLog();
	
	int doWrite(int);
};

eHTTPLog::eHTTPLog(eHTTPConnection *c, int mask, int format):
	eHTTPDataSource(c), mask(mask), format(format), ok(0)
{
	init_eHTTPLog();
}
void eHTTPLog::init_eHTTPLog()
{
	if (format == 0)
		connection->local_header["Content-Type"]="text/plain";
	else if (format == 1)
		connection->local_header["Content-Type"]="text/html";
	connection->code=200;
	connection->code_descr="OK";
	CONNECT(logOutput, eHTTPLog::recvMessage);
	last = -1;
	if (format == 1)
	{
		toWrite="<html><head>" 
		"<link type=\"text/css\" rel=\"stylesheet\" href=\"/stylesheets/log.css\">"
		"<title>Enigma Event Log</title>"
		"</head><body><pre>\n";
	}
}

int eHTTPLog::doWrite(int hm)
{
	// we don't have YET data to send (but there's much to come)
	ok=1;
	if (toWrite.size())
	{
		connection->writeBlock(toWrite.c_str(), toWrite.size());
		toWrite="";
	}
	return 0;
}

void eHTTPLog::recvMessage(int lvl, const eString &msg)
{
	eString res;
	if (lvl & mask)
	{
		if (format == 0) // text/plain
		{
			res=msg;
			res.strReplace("\n", "\r\n");
		} else
		{
			if (last != lvl)
			{
				eString cl="unknown";
				if (lvl == lvlWarning)
					cl="warning";
				else if (lvl == lvlFatal)
					cl="fatal";
				else if (lvl == lvlDebug)
					cl="debug";
				
				if (last != -1)
					res+="</div>";
				res+="<div class=\"" + cl + "\">";
				last=lvl;
			}
			res+=msg;
			// res.strReplace("\n", "<br>\n");  <-- we are <pre>, so no need
		}
		if (ok)
			connection->writeBlock(res.c_str(), res.size());
		else
			toWrite+=res;
	}
}

eHTTPLog::~eHTTPLog()
{
}

eHTTPLogResolver::eHTTPLogResolver()
{
}

eHTTPDataSource *eHTTPLogResolver::getDataSource(eString request, eString path, eHTTPConnection *conn)
{
	if ((path=="/log/debug") && (request=="GET"))
		return new eHTTPLog(conn, -1, 0);
	if ((path=="/log/warn") && (request=="GET"))
		return new eHTTPLog(conn, 3, 0);
	if ((path=="/log/crit") && (request=="GET"))
		return new eHTTPLog(conn, 1, 0);

	if ((path=="/log/debug.html") && (request=="GET"))
		return new eHTTPLog(conn, -1, 1);
	if ((path=="/log/warn.html") && (request=="GET"))
		return new eHTTPLog(conn, 3, 1);
	if ((path=="/log/crit.html") && (request=="GET"))
		return new eHTTPLog(conn, 1, 1);
	return 0;
}

extern eString filter_string(eString string);

class ERCServiceHandle: public Object
{
	eString &result, search;
	eServiceInterface &iface;
public:
	ERCServiceHandle(eString &result, eServiceInterface &iface, eString search): result(result), search(search.upper()), iface(iface)
	{
	}
	void addEntry(const eServiceReference &e)
	{
		eService *service=iface.addRef(e);
		if (service)
		{
			if (filter_string(service->service_name).upper().find(search) == eString::npos)
				return;
			result += ref2string(e) + "\n";
			result += service->service_name + "\n";
			iface.removeRef(e);
		}
	}
};

static eString erc_services(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	std::map<eString,eString> opts=getRequestOptions(opt);

	eServiceInterface *iface=eServiceInterface::getInstance();
	eServiceReference all_services=eServiceReference(eServiceReference::idDVB,
		eServiceReference::flagDirectory|eServiceReference::shouldSort,
		-2, -1, 0xFFFFFFFF);

	if (!iface)
		return "n/a\n";
	
	if (opts.find("name") == opts.end())
		return "-specify name=\n";
	
	// search "all dvb services":

	eString res = "+\n";
	
	ERCServiceHandle conv(res, *iface, opts["name"]);
	
	Signal1<void,const eServiceReference&> signal;
	signal.connect(slot(conv, &ERCServiceHandle::addEntry));
	
	iface->enterDirectory(all_services, signal);
	iface->leaveDirectory(all_services);
	
	return res;
}

static void processEvent(eString &res, EITEvent *ev, const eString &search, int wantext)
{
	eString title, description;

	LocalEventData led;
	led.getLocalData(ev, &title, wantext ? &description : NULL);

	if (title.find(search) != eString::npos)
	{
		res += "I: ";
		res += eString().setNum(ev->event_id, 0x10);
		res += "\nB: ";
		res += eString().setNum(ev->start_time);
		res += "\nD: ";
		res += eString().setNum(ev->duration);
		res += "\nN: " + title + "\n";
		if (description)
		{
			res += "T: ";
			description.strReplace("\n", eString("\\n"));
			res += description;
			res +=" \n";
		}
	}
}

static eString erc_epg(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	std::map<eString,eString> opts=getRequestOptions(opt);
	time_t begin = 0;
	int duration = 0;
	eString search;
	eString res;
	int wantext;
	int event_id = -1;
	
	if (opts.find("service") == opts.end())
		return "-specify service";
	if ((opts.find("text") == opts.end()) && (opts.find("begin") == opts.end()) && (opts.find("event_id") == opts.end()))
		return "-specify text and/or begin or event_id";
	eString text = opts["text"];
	if (opts.find("begin") != opts.end())
		begin = atoi(opts["begin"].c_str());
	if (opts.find("duration") != opts.end())
		duration = atoi(opts["duration"].c_str());
	search = opts["text"];
	
	if (opts.find("extended") != opts.end())
		wantext = 1;
	else
		wantext = 0;
	
	if (opts.find("event_id") != opts.end())
		sscanf(opts["event_id"].c_str(), "%x", &event_id);

	eEPGCache *epgcache=eEPGCache::getInstance();
	eServiceReference ref(opts["service"]);
	
	if (event_id == -1)
	{
		timeMapPtr evmap = epgcache->getTimeMapPtr((eServiceReferenceDVB&)ref);
		if (!evmap)
		{
			return "-no events for this service";
		}
		eServiceReferenceDVB &rref=(eServiceReferenceDVB&)ref;
		timeMap::const_iterator ibegin = evmap->begin(), iend = evmap->end();
		if (begin != 0)
		{
			ibegin = evmap->lower_bound(begin);
			if ((ibegin != evmap->end()) && (ibegin != evmap->begin()))
				--ibegin;
			else
				ibegin=evmap->begin();
	
			timeMap::const_iterator iend = evmap->upper_bound(begin + duration);
			if (iend != evmap->end())
				++iend;
		}
		int tsidonid =
			(rref.getTransportStreamID().get()<<16)|rref.getOriginalNetworkID().get();
		for (timeMap::const_iterator event(ibegin); event != iend; ++event)
		{
			EITEvent *ev = new EITEvent(*event->second, tsidonid, event->second->type );
			processEvent(res, ev, search, wantext);
			delete ev;
		}
	} 
	else
	{
		EITEvent *ev = epgcache->lookupEvent((eServiceReferenceDVB&)ref, event_id);
		if (!ev)
			return "-service or event_id invalid";
		else
		{
			processEvent(res, ev, search, wantext);
			delete ev;
		}
	}
	
	return res;
}

void ezapInitializeWeb(eHTTPDynPathResolver *dyn_resolver)
{
	dyn_resolver->addDyn("GET", "/dyn2/", web_root);
	dyn_resolver->addDyn("GET", "/dyn2/services", xml_services);

	dyn_resolver->addDyn("GET", "/erc/services", erc_services);
	dyn_resolver->addDyn("GET", "/erc/epg", erc_epg);
}

