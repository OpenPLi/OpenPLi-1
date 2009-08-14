#ifndef DISABLE_NETWORK

#include <errno.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/system/xmlrpc.h>
#include <epgwindow.h>

static eServiceReferenceDVB getServiceByID(const char *id)
{
	eTransponderList *tl=eDVB::getInstance()->settings->getTransponders();
	if (!tl)
		return eServiceReferenceDVB();
	int dvb_namespace, original_network_id, transport_stream_id, service_id, service_type;
	if (sscanf(id, "S:%x:%x:%x:%x:%x", &dvb_namespace, &original_network_id, &transport_stream_id, &service_id, &service_type)!=4)
		if (sscanf(id, "E:%x:%x:%x:%x:%x", &dvb_namespace, &original_network_id, &transport_stream_id, &service_id, &service_type)!=4)
			return eServiceReferenceDVB();

	return eServiceReferenceDVB(eDVBNamespace(dvb_namespace), eTransportStreamID(transport_stream_id), eOriginalNetworkID(original_network_id), eServiceID(service_id), service_type);
}

static int testrpc(std::vector<eXMLRPCVariant> &params, ePtrList<eXMLRPCVariant> &result)
{
	if (xmlrpc_checkArgs("ii", params, result))
		return 1;
	
	std::map<eString, eXMLRPCVariant*> *s=new std::map<eString, eXMLRPCVariant*>;
	s->INSERT("sum",new eXMLRPCVariant(new __s32(*params[0].getI4()+*params[1].getI4())));
	s->INSERT("difference",new eXMLRPCVariant(new __s32(*params[0].getI4()-*params[1].getI4())));
	
	result.push_back(new eXMLRPCVariant(s));

	return 0;
}

			// SID2

static int getList(std::vector<eXMLRPCVariant> &params, ePtrList<eXMLRPCVariant> &result)
{
	if (xmlrpc_checkArgs("s", params, result))
		return 1;
	
	eString &param=*params[0].getString();
	
	eDebug("getList(%s);", param.c_str());
	
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
	{
		xmlrpc_fault(result, 3, "currently not available");
		return 0;
	}
	
	if (!param.length())		// root
	{
		ePtrList<eXMLRPCVariant> l;
		l.setAutoDelete(true);

		std::map<int,eBouquet*> *pBouquets=eDVB::getInstance()->settings->getBouquets();
		if (pBouquets)
		{
			for (std::map<int,eBouquet*>::iterator i(pBouquets->begin()); i != pBouquets->end(); ++i)
			{
				eBouquet *b=i->second;
				std::map<eString, eXMLRPCVariant*> *s=new std::map<eString, eXMLRPCVariant*>;
				static eString s0("caption");
				static eString s1("type");
				static eString s2("handle");
				static eString s3("zappable");
				
				s->INSERT(s0, new eXMLRPCVariant(new eString(b->bouquet_name.c_str())));
				static eString g("Group");
				s->INSERT(s1, new eXMLRPCVariant(new eString(g)));
				static eString bs("B:");
				eString handle=bs;
				handle+=eString().setNum(b->bouquet_id, 16);
				s->INSERT(s2, new eXMLRPCVariant(new eString(handle)));
				s->INSERT(s3, new eXMLRPCVariant(new bool(0)));
				l.push_back(new eXMLRPCVariant(s));
			}
		}

		result.push_back( new eXMLRPCVariant( l.getVector() ) );

	} else if (param[0]=='B')
	{
		eBouquet *b=NULL;
		int bouquet_id;
		if (sscanf(param.c_str(), "B:%x", &bouquet_id)==1)
			b=eDVB::getInstance()->settings->getBouquet(bouquet_id);
		if (!b)
			xmlrpc_fault(result, 3, "invalid handle");
		else
		{
			ePtrList<eXMLRPCVariant> l;
			l.setAutoDelete(true);
			for (std::list<eServiceReferenceDVB>::iterator s = b->list.begin(); s != b->list.end(); s++)
			{
				eService *service=eDVB::getInstance()->settings->getTransponders()->searchService(*s);
				if (!service)
					continue;
				std::map<eString, eXMLRPCVariant*> *sm=new std::map<eString, eXMLRPCVariant*>;
				static eString s0("caption");
				static eString s1("type");
				static eString s2("handle");
				static eString s3("zappable");

				sm->INSERT(s0, new eXMLRPCVariant(new eString(service->service_name.c_str())));
				static eString g("Service");
				sm->INSERT(s1, new eXMLRPCVariant(new eString(g)));
				static eString bs("S:");
				eString handle=bs;
				handle+=eString().setNum(s->getDVBNamespace().get(), 16);
				handle+=':';
				handle+=eString().setNum(s->getOriginalNetworkID().get(), 16);
				handle+=':';
				handle+=eString().setNum(s->getTransportStreamID().get(), 16);
				handle+=':';
				handle+=eString().setNum(s->getServiceID().get(), 16);
				handle+=':';
				handle+=eString().setNum(s->getServiceType(), 16);
				sm->INSERT(s2, new eXMLRPCVariant(new eString(handle)));
				sm->INSERT(s3, new eXMLRPCVariant(new bool(1)));
				l.push_back(new eXMLRPCVariant(sm));
			}
			result.push_back(new eXMLRPCVariant( l.getVector() ));
		}
	} else if (param[0]=='S')
	{
		eServiceReferenceDVB service=getServiceByID(param.c_str());
		if (sapi->service != service)
		{
			xmlrpc_fault(result, 4, "service currently not tuned in");
			return 0;
		}
		if (sapi->service_state==ENOENT)
		{
			xmlrpc_fault(result, 4, "service couldn't be tuned in");
			return 0;
		}
		EIT *eit=eDVB::getInstance()->getEIT();
		if (!eit)
		{
			xmlrpc_fault(result, 1, "no EIT yet");
			return 0;
		}

		ePtrList<eXMLRPCVariant> l;
		l.setAutoDelete(true);

		for (ePtrList<EITEvent>::iterator i(eit->events); i != eit->events.end(); ++i)
		{
			EITEvent *event=*i;

			eString event_name;
			LocalEventData led;
			led.getLocalData(event, &event_name);
			if (!event_name)
				continue;

			std::map<eString, eXMLRPCVariant*> *s=new std::map<eString, eXMLRPCVariant*>;
			static eString s0("caption");
			static eString s1("type");
			static eString s2("handle");
			static eString s3("zappable");

			s->INSERT(s0, new eXMLRPCVariant(new eString(event_name)));
			static eString g("Event");
			s->INSERT(s1, new eXMLRPCVariant(new eString(g)));
			static eString bs("E:");
			eString handle=bs;
			handle+=eString().setNum(service.getDVBNamespace().get(), 16);
			handle+=':';
			handle+=eString().setNum(service.getOriginalNetworkID().get(), 16);
			handle+=':';
			handle+=eString().setNum(service.getTransportStreamID().get(), 16);
			handle+=':';
			handle+=eString().setNum(service.getServiceID().get(), 16);
			handle+=':';
			handle+=eString().setNum(service.getServiceType(), 16);
			handle+=':';
			handle+=eString().setNum(event->event_id, 16);

			s->INSERT(s2, new eXMLRPCVariant(new eString(handle)));
			s->INSERT(s3, new eXMLRPCVariant(new bool(1)));
			l.push_back(new eXMLRPCVariant(s));
		}
		eit->unlock();
		result.push_back(new eXMLRPCVariant( l.getVector() ));
	} else
		xmlrpc_fault(result, 3, "couldn't get of this");
	return 0;
}

static int zapTo(std::vector<eXMLRPCVariant> &params, ePtrList<eXMLRPCVariant> &result)
{
	if (xmlrpc_checkArgs("s", params, result))
		return 1;

	eString &param=*params[0].getString();

	eDebug("zapTo(%s);", param.c_str());

	if (param[0] != 'S')
	{
		xmlrpc_fault(result, 3, "nene nur service bitte");
		return 0;
	}
	
	eServiceReferenceDVB s=getServiceByID(param.c_str());

	if ( eServiceInterface::getInstance()->getService()->getID() != s.type )
	{
		xmlrpc_fault(result, 3, "currently not available");
		return 0;
	}
	
	eServiceInterface::getInstance()->play(s);

	return 0;
}

static int getInfo(std::vector<eXMLRPCVariant> &params, ePtrList<eXMLRPCVariant> &result)
{
	if (xmlrpc_checkArgs("s", params, result))
		return 1;

	eString &param=*params[0].getString();

	eDebug("getInfo(%s);", param.c_str());
	
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
	{
		xmlrpc_fault(result, 3, "currently not available");
		return 0;
	}
	
	if (!param.length())    // currently running
	{
			// mal gucken
  } else if (param[0]=='S')
  {
		std::map<eString, eXMLRPCVariant*> *s=new std::map<eString, eXMLRPCVariant*>;

		eServiceReferenceDVB service=getServiceByID(param.c_str());
		
		if (sapi->service != service)
		{
			xmlrpc_fault(result, 4, "service currently not tuned in");
			return 0;
		}
		if (sapi->service_state==ENOENT)
		{
			xmlrpc_fault(result, 4, "service couldn't be tuned in");
			return 0;
		}
		
		static eString s1("type");
		static eString g("Service");
		s->INSERT(s1, new eXMLRPCVariant(new eString(g)));

		static eString s0("caption");

		eService *ser=eDVB::getInstance()->settings->getTransponders()->searchService(service);
		s->INSERT(s0, new eXMLRPCVariant(new eString(ser?ser->service_name.c_str():"")));
		
		static eString s2("parentHandle");
		static eString g2("NA");
		s->INSERT(s2, new eXMLRPCVariant(new eString(g2)));
		
		PMT *pmt=eDVB::getInstance()->getPMT();
		if (!pmt)
		{
			xmlrpc_fault(result, 1, "no PMT yet");
			return 0;
		}

		PMTEntry *v=0;
		for (ePtrList<PMTEntry>::iterator i(pmt->streams); (!v) && i != pmt->streams.end(); ++i)
		{
			PMTEntry *pe=*i;
			switch (pe->stream_type)
			{
			case 1:	// ISO/IEC 11172 Video
			case 2: // ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
				v=pe;
				break;
			}
		}
		
		if (v)
		{
			static eString s2("videoPid");
			s->INSERT(s2, new eXMLRPCVariant(new int(v->elementary_PID)));
		}

		ePtrList<eXMLRPCVariant> l;
		l.setAutoDelete(true);
		for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end(); ++i)
		{
			PMTEntry *pe=*i;
			int isaudio=0, isAC3=0;
			
			switch (pe->stream_type)
			{
			case 3:	// ISO/IEC 11172 Audio
			case 4: // ISO/IEC 13818-3 Audio
				isaudio=1;
				break;
			case 6:
			{
				for (ePtrList<Descriptor>::iterator i(pe->ES_info); i != pe->ES_info.end(); ++i)
				{
					Descriptor *d=*i;
					if (d->Tag()==DESCR_AC3)
					{
						isaudio=1;
						isAC3=1;
					}
				}
			}
			}
			if (isaudio)
			{
				std::map<eString, eXMLRPCVariant*> *a=new std::map<eString, eXMLRPCVariant*>;
				static eString s1("audioPid"), s2("handle"), s3("type"), s4("language"), s5("mpeg"), s6("ac3");
				
				a->INSERT(s1, new eXMLRPCVariant(new int(pe->elementary_PID)));
				a->INSERT(s2, new eXMLRPCVariant(new eString("A")));	// nyi
				a->INSERT(s3, new eXMLRPCVariant(new eString(isAC3?s6:s5)));

				l.push_back( new eXMLRPCVariant( a ) );
			}
		}
		static eString as("audioPids");
		s->INSERT(as, new eXMLRPCVariant( l.getVector() ) );

		pmt->unlock();
		
		result.push_back(new eXMLRPCVariant(s));

		eString res="";
		result.first()->toXML(res);
		eDebug("%s", res.c_str());
	} else
		xmlrpc_fault(result, 3, "nene nur service bitte");
	return 0;
}

void ezapInitializeXMLRPC()
{
	xmlrpc_addMethod("test.test", testrpc);
	xmlrpc_addMethod("getList", getList);
	xmlrpc_addMethod("zapTo", zapTo);
	xmlrpc_addMethod("getInfo", getInfo);
}

#endif //DISABLE_NETWORK
