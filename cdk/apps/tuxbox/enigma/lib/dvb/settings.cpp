#include <lib/dvb/settings.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/frontend.h>

typedef std::list<eServiceReferenceDVB>::iterator ServiceReferenceDVBIterator;

eDVBSettings::eDVBSettings(eDVB &dvb): dvb(dvb)
{
	transponderlist=new eTransponderList;
	loadServices();
	loadBouquets();
	CONNECT( transponderlist->service_found, eDVBSettings::service_found );
	CONNECT( transponderlist->service_removed, eDVBSettings::service_removed );
}

void eDVBSettings::removeDVBBouquets()
{
	for (std::map<int, eBouquet*>::iterator i(bouquet_id_map.begin()); i != bouquet_id_map.end();)
	{
		if ( i->first >= 0)
		{
//			eDebug("removing bouquet '%s'", i->bouquet_name.c_str());
			bouquet_name_map.erase(i->second->bouquet_name);  // remove from name map
			delete i->second; // release heap memory
			bouquet_id_map.erase(i++); // remove from id map
			bouquetsChanged=1;
		}
		else
		{
//			eDebug("leaving bouquet '%s'", i->bouquet_name.c_str());
			++i;
		}
	}
}

void eDVBSettings::removeDVBBouquet(int bouquet_id)
{
	std::map<int, eBouquet*>::iterator i(bouquet_id_map.find(bouquet_id));
	if ( i != bouquet_id_map.end() )
	{
		bouquet_name_map.erase(i->second->bouquet_name);  // remove from name map
		delete i->second; // release heap memory
		bouquet_id_map.erase(i); // remove from id map
		bouquetsChanged=1;
	}
}

void eDVBSettings::renameDVBBouquet(int bouquet_id, eString& new_name)
{
	std::map<int, eBouquet*>::iterator i(bouquet_id_map.find(bouquet_id));
	if ( i != bouquet_id_map.end() )
	{
		bouquet_name_map.erase(i->second->bouquet_name);  // remove from name map
		bouquet_name_map[new_name] = i->second;
		i->second->bouquet_name = new_name;
		bouquetsChanged=1;
	}
}

void eDVBSettings::addDVBBouquet(eDVBNamespace origin, const BAT *bat )
{
	eDebug("wir haben da eine bat, id %x our id is %08x", bat->bouquet_id, -(bat->bouquet_id | 0xF000000));
	eString bouquet_name="Weiteres Bouquet";
	for (ePtrList<Descriptor>::const_iterator i(bat->bouquet_descriptors); i != bat->bouquet_descriptors.end(); ++i)
	{
		if (i->Tag()==DESCR_BOUQUET_NAME)
		{
			bouquet_name=((BouquetNameDescriptor*)*i)->name;
			bouquet_name+=" (BAT)";
		}
	}
	eBouquet *bouquet=createBouquet(-(bat->bouquet_id|0xF000000), bouquet_name);
	bouquet->list.clear();
	for (ePtrList<BATEntry>::const_iterator be(bat->entries); be != bat->entries.end(); ++be)
		for (ePtrList<Descriptor>::const_iterator i(be->transport_descriptors); i != be->transport_descriptors.end(); ++i)
			if (i->Tag()==DESCR_SERVICE_LIST)
			{
				const ServiceListDescriptor *s=(ServiceListDescriptor*)*i;
				for (ePtrList<ServiceListDescriptorEntry>::const_iterator a(s->entries); a != s->entries.end(); ++a)
				{
					bouquet->add(
						eServiceReferenceDVB(
							origin,
							eTransportStreamID(be->transport_stream_id), 
							eOriginalNetworkID(be->original_network_id), 
							eServiceID(a->service_id), a->service_type));
				}
			}
}

eBouquet *eDVBSettings::getBouquet(int bouquet_id)
{
	std::map<int, eBouquet*>::iterator i(bouquet_id_map.find(bouquet_id));
	if ( i != bouquet_id_map.end() )
		return i->second;
	return 0;
}

static eString beautifyBouquetName(eString bouquet_name)
{
/*	if ( (bouquet_name.find("ARD") != eString::npos)
		  || (bouquet_name.find("ZDF") != eString::npos)
			|| (bouquet_name.find("RTL") != eString::npos)
			|| (bouquet_name.find("n-tv") != eString::npos)
			|| (bouquet_name.find("ProSieben") != eString::npos)
			|| (bouquet_name.find("VIVA") != eString::npos) )
		bouquet_name="German Free";		
	else*/ 
	if ( bouquet_name.upper().find("POLSAT") != eString::npos
		|| ( bouquet_name.length() == 5 
			&& (bouquet_name.find("D11") != eString::npos
				|| bouquet_name.find("D10") != eString::npos ) ) )
		bouquet_name="POLSAT";
	else if (bouquet_name.find("HRT") != eString::npos)
		bouquet_name="HRT Zagreb";
	else if (bouquet_name.find("TVP") != eString::npos)
		bouquet_name="TVP";
	else if (bouquet_name.find("RVTS") != eString::npos)
		bouquet_name="RVTS";
	else if (bouquet_name=="AB SAT")
		bouquet_name="ABSAT";
	else if (bouquet_name=="Astra-Net")
		bouquet_name="ASTRA";
	else if (bouquet_name=="CSAT")
		bouquet_name="CANALSATELLITE";
	else if (bouquet_name.find("SES")!=eString::npos)
		bouquet_name="SES Multimedia";
	else if (!bouquet_name)
		bouquet_name="no name";
	return bouquet_name;
}

eBouquet *eDVBSettings::getBouquet(eString& bouquet_name)
{
	std::map<eString, eBouquet*>::iterator i(bouquet_name_map.find(bouquet_name));
	if ( i != bouquet_name_map.end() )
		return i->second;
	return 0;
}

eBouquet* eDVBSettings::createBouquet(int bouquet_id, eString bouquet_name)
{
	eBouquet *n=getBouquet(bouquet_id);
	if (!n)
	{
		n = new eBouquet(bouquet_id, bouquet_name);
		bouquet_name_map[bouquet_name] = n;
		bouquet_id_map[bouquet_id] = n;
		bouquetsChanged=1;
	}
	return n;
}

eBouquet *eDVBSettings::createBouquet(eString bouquet_name)
{
	eBouquet *n=getBouquet(bouquet_name);
	if (!n)
	{
		int bouquet_id=getUnusedBouquetID(0);
		n = new eBouquet(bouquet_id, bouquet_name);
		bouquet_name_map[bouquet_name] = n;
		bouquet_id_map[bouquet_id] = n;
		bouquetsChanged=1;
	}
	return n;
}

int eDVBSettings::getUnusedBouquetID(int range)
{
	if (range)
		range=-1;
	else
		range=1;

	int bouquet_id=0;

	while(true)  // Evtl hier nochmal nachschauen....
	{
		if (!getBouquet(bouquet_id))
			return bouquet_id;

		bouquet_id+=range;
	}
}

void eDVBSettings::revalidateBouquets()
{
	/*emit*/ dvb.bouquetListChanged();
}

struct sortinChannel
{
	eDVBSettings &edvb;
	sortinChannel(eDVBSettings &edvb): edvb(edvb)
	{
	}
	void operator()(eServiceDVB &service)
	{
		eBouquet *b = edvb.createBouquet(beautifyBouquetName(service.service_provider) );
		b->add(eServiceReferenceDVB(service.dvb_namespace, service.transport_stream_id, service.original_network_id, service.service_id, service.service_type));
	}
};

void eDVBSettings::service_found( const eServiceReferenceDVB &ref, bool newAdded )
{
	if ( !eDVB::getInstance()->getScanAPI() && newAdded )
	{
		eServiceDVB *service = transponderlist->searchService(ref);
		if ( service )
		{
			eBouquet *b = createBouquet(beautifyBouquetName(service->service_provider));
			if ( b->bouquet_id >=0 && !b->list.size() || std::find(b->list.begin(),b->list.end(),ref) == b->list.end() )
			{
				b->add(ref);
				/* emit */ eDVB::getInstance()->serviceListChanged();
			}
		}
	}
}

void eDVBSettings::service_removed( const eServiceReferenceDVB &ref)
{
	eServiceDVB *service = transponderlist->searchService(ref);
	if ( !eDVB::getInstance()->getScanAPI() && service )
	{
		eBouquet *b = createBouquet(beautifyBouquetName(service->service_provider));
		if (b->bouquet_id >=0 )
		{
			std::list<eServiceReferenceDVB>::iterator it =
				std::find(b->list.begin(), b->list.end(), ref);
			if ( it != b->list.end() )
			{
				b->list.erase(it);
				if ( !b->list.size()) // delete last service in bouquet
					removeDVBBouquet(b->bouquet_id);
				/* emit */ eDVB::getInstance()->serviceListChanged();
			}
		}
	}
}

void eDVBSettings::sortInChannels()
{
	eDebug("sorting in channels");
	removeDVBBouquets();
	getTransponders()->forEachService(sortinChannel(*this));
	revalidateBouquets();
}

struct saveService
{
	FILE *f;
	saveService(FILE *out): f(out)
	{
	 	fprintf(f, "services\n");
	}
	void operator()(eServiceDVB& s)
	{
		fprintf(f, "%04x:%08x:%04x:%04x:%d:%d\n", s.service_id.get(), s.dvb_namespace.get(), s.transport_stream_id.get(), s.original_network_id.get(), s.service_type, s.service_number);
		fprintf(f, "%s\n", s.service_name.c_str());
		if (s.dxflags)
			fprintf(f, "f:%x,", s.dxflags);
		for (int i=0; i<eServiceDVB::cacheMax; ++i)
		{
			short val = s.get((eServiceDVB::cacheID)i);
			if (val != -1)
				fprintf(f, "c:%02d%04x,", i, val);
		}
		eString prov;
		prov=s.service_provider;
		for (eString::iterator i=prov.begin(); i != prov.end(); ++i)
			if (*i == ',')
				*i='_';
		
		fprintf(f, "p:%s\n", prov.c_str());
	}
	~saveService()
	{
		fprintf(f, "end\n");
	}
};
struct saveSubService
{
	FILE *f;
	saveSubService(FILE *out): f(out)
	{
	 	fprintf(f, "subservices\n");
	}
	void operator()(eServiceDVB& s)
	{
		bool bChanged = false;
		for (int i=0; i<eServiceDVB::cacheMax; ++i)
		{
			if (s.get((eServiceDVB::cacheID)i) != -1)
			{
				bChanged = true;
				break;
			}
		}
		if (!bChanged) return;
		fprintf(f, "%04x:%08x:%04x:%04x\n", s.service_id.get(), s.dvb_namespace.get(), s.transport_stream_id.get(), s.original_network_id.get());
		fprintf(f, "%s\n", s.service_name.c_str());
		if (s.dxflags)
			fprintf(f, "f:%x,", s.dxflags);
		for (int i=0; i<eServiceDVB::cacheMax; ++i)
		{
			short val = s.get((eServiceDVB::cacheID)i);
			if (val != -1)
				fprintf(f, "c:%02d%04x,", i, val);
		}
		eString prov;
		prov=s.service_provider;
		for (eString::iterator i=prov.begin(); i != prov.end(); ++i)
			if (*i == ',')
				*i='_';
		
		fprintf(f, "p:%s\n", prov.c_str());
	}
	~saveSubService()
	{
		fprintf(f, "end\n");
	}
};

struct saveTransponder
{
	FILE *f;
	saveTransponder(FILE *out): f(out)
	{
		fprintf(f, "transponders\n");
	}
	void operator()(eTransponder &t)
	{
		if (t.state&eTransponder::stateOK)
		{
			fprintf(f, "%08x:%04x:%04x\n", t.dvb_namespace.get(), t.transport_stream_id.get(), t.original_network_id.get());
			if (t.cable.valid)
				fprintf(f, "\tc %d:%d:%d:%d:%d", t.cable.frequency, t.cable.symbol_rate, t.cable.inversion, t.cable.modulation, t.cable.fec_inner);
			if (t.satellite.valid)
				fprintf(f, "\ts %d:%d:%d:%d:%d:%d", t.satellite.frequency, t.satellite.symbol_rate, t.satellite.polarisation, t.satellite.fec, t.satellite.orbital_position, t.satellite.inversion);
			if (t.terrestrial.valid)
				fprintf(f, "\tt %d:%d:%d:%d:%d:%d:%d:%d:%d", 
					t.terrestrial.centre_frequency, 
					t.terrestrial.code_rate_hp, 
					t.terrestrial.code_rate_lp,
					t.terrestrial.bandwidth,
					t.terrestrial.constellation,
					t.terrestrial.guard_interval,
					t.terrestrial.hierarchy_information,
					t.terrestrial.transmission_mode,
					t.terrestrial.inversion);
			fprintf(f, ":%d\n/\n", t.state & eTransponder::stateOnlyFree );
		}
	}
	~saveTransponder()
	{
		fprintf(f, "end\n");
	}
};

void eDVBSettings::saveServices()
{
	FILE *f=fopen(CONFIGDIR "/enigma/services", "wt");
	if (!f)
		eFatal("couldn't open servicefile - create " CONFIGDIR "/enigma!");
	fprintf(f, "eDVB services /2/\n");

	getTransponders()->forEachTransponder(saveTransponder(f));
	getTransponders()->forEachService(saveService(f));
	getTransponders()->forEachSubService(saveSubService(f));
	fprintf(f, "Have a lot of fun!\n");
	fclose(f);
}

void eDVBSettings::loadServices()
{
	FILE *f=fopen(CONFIGDIR "/enigma/services", "rt");
	if (!f)
		return;
	char line[256];
	if ((!fgets(line, 256, f)) || strncmp(line, "eDVB services", 13))
	{
		eDebug("not a servicefile");
		return;
	}
	eDebug("reading services");
	if ((!fgets(line, 256, f)) || strcmp(line, "transponders\n"))
	{
		eDebug("services invalid, no transponders");
		return;
	}
	if (transponderlist)
		transponderlist->clearAllTransponders();

	while (!feof(f))
	{
		if (!fgets(line, 256, f))
			break;
		if (!strcmp(line, "end\n"))
			break;
		int dvb_namespace=-1, transport_stream_id=-1, original_network_id=-1;
		sscanf(line, "%x:%x:%x", &dvb_namespace, &transport_stream_id, &original_network_id);
		if (original_network_id == -1)
			continue;
		eTransponder &t=transponderlist->createTransponder(eDVBNamespace(dvb_namespace), eTransportStreamID(transport_stream_id), eOriginalNetworkID(original_network_id));
		t.state=eTransponder::stateOK;
		while (!feof(f))
		{
			fgets(line, 256, f);
			if (!strcmp(line, "/\n"))
				break;
			int onlyFree=0;
			if (line[1]=='s')
			{
				int frequency, symbol_rate, polarisation, fec, sat, inversion=INVERSION_OFF;
				sscanf(line+2, "%d:%d:%d:%d:%d:%d:%d", &frequency, &symbol_rate, &polarisation, &fec, &sat, &inversion, &onlyFree);
				t.setSatellite(frequency, symbol_rate, polarisation, fec, sat, inversion);
			}
			if (line[1]=='c')
			{
				int frequency, symbol_rate, inversion=INVERSION_OFF, modulation=3, fec_inner=0;
				int ret = sscanf(line+2, "%d:%d:%d:%d:%d:%d", &frequency, &symbol_rate, &inversion, &modulation, &fec_inner, &onlyFree);
				if ( ret < 6 )
				{
					onlyFree = fec_inner;
					fec_inner=0;
				}
				t.setCable(frequency, symbol_rate, inversion, modulation, fec_inner);
			}
			if (line[1]=='t')
			{
				int centre_frequency, code_rate_hp, code_rate_lp, bandwidth, constellation, guard_interval, hierarchy_information, transmission_mode, inversion=INVERSION_OFF;
				sscanf(line+2, "%d:%d:%d:%d:%d:%d:%d:%d:%d:%d", &centre_frequency, &code_rate_hp, &code_rate_lp, &bandwidth, &constellation, &guard_interval, &hierarchy_information, &transmission_mode, &inversion, &onlyFree );
				t.setTerrestrial(centre_frequency, bandwidth, constellation, hierarchy_information, code_rate_hp, code_rate_lp, guard_interval, transmission_mode, inversion);
			}
			if ( onlyFree )
				t.state |= eTransponder::stateOnlyFree;
		}
	}

	if ((!fgets(line, 256, f)) || strcmp(line, "services\n"))
	{
		eDebug("services invalid, no services");
		return;
	}

	if (transponderlist)
		transponderlist->clearAllServices();

	int count=0;

	while (!feof(f))
	{
		if (!fgets(line, 256, f))
			break;
		if (!strcmp(line, "end\n"))
			break;

		int service_id=-1, dvb_namespace, transport_stream_id=-1, original_network_id=-1, service_type=-1, service_number=-1;
		sscanf(line, "%x:%x:%x:%x:%d:%d", &service_id, &dvb_namespace, &transport_stream_id, &original_network_id, &service_type, &service_number);
		if (service_number == -1)
			continue;
		eServiceDVB &s=transponderlist->createService(
					eServiceReferenceDVB(
						eDVBNamespace(dvb_namespace),
						eTransportStreamID(transport_stream_id),
						eOriginalNetworkID(original_network_id),
						eServiceID(service_id),
						service_type), service_number);
		count++;
		s.service_type=service_type;
		fgets(line, 256, f);
		if (strlen(line))
			line[strlen(line)-1]=0;
		s.service_name=line;
		fgets(line, 256, f);
		if (strlen(line))
			line[strlen(line)-1]=0;

		eString str=line;

		if (str[1]!=':')	// old ... (only service_provider)
		{
			s.service_provider=line;
		} else
			while ((!str.empty()) && str[1]==':') // new: p:, f:, c:%02d...
			{
				unsigned int c=str.find(',');
				char p=str[0];
				eString v;
				if (c == eString::npos)
				{
					v=str.mid(2);
					str="";
				} else
				{
					v=str.mid(2, c-2);
					str=str.mid(c+1);
				}
//				eDebug("%c ... %s", p, v.c_str());
				if (p == 'p')
					s.service_provider=v;
				else if (p == 'f')
				{
					int dummyval= 0;
					sscanf(v.c_str(), "%x", &dummyval);
					s.dxflags = dummyval;
				} else if (p == 'c')
				{
					int cid, val;
					sscanf(v.c_str(), "%02d%04x", &cid, &val);
					if (cid < eServiceDVB::cacheMax)
						s.set((eServiceDVB::cacheID)cid,val);
				}
			}
	}

	eDebug("loaded %d services", count);
	
	if ((!fgets(line, 256, f)) || strcmp(line, "subservices\n"))
	{
		eDebug("subservices invalid, no subservices");
		return;
	}

	if (transponderlist)
		transponderlist->clearAllSubServices();

	count=0;

	while (!feof(f))
	{
		if (!fgets(line, 256, f))
			break;
		if (!strcmp(line, "end\n"))
			break;

		int service_id=-1, dvb_namespace, transport_stream_id=-1, original_network_id=-1;
		sscanf(line, "%x:%x:%x:%x", &service_id, &dvb_namespace, &transport_stream_id, &original_network_id);
		eServiceDVB &s=transponderlist->createSubService(
					eServiceReferenceDVB(
						eDVBNamespace(dvb_namespace),
						eTransportStreamID(transport_stream_id),
						eOriginalNetworkID(original_network_id),
						eServiceID(service_id),7
						));
		count++;
		s.service_type=7;
		fgets(line, 256, f);
		if (strlen(line))
			line[strlen(line)-1]=0;
		s.service_name=line;
		fgets(line, 256, f);
		if (strlen(line))
			line[strlen(line)-1]=0;

		eString str=line;

		if (str[1]!=':')	// old ... (only service_provider)
		{
			s.service_provider=line;
		} else
			while ((!str.empty()) && str[1]==':') // new: p:, f:, c:%02d...
			{
				unsigned int c=str.find(',');
				char p=str[0];
				eString v;
				if (c == eString::npos)
				{
					v=str.mid(2);
					str="";
				} else
				{
					v=str.mid(2, c-2);
					str=str.mid(c+1);
				}
//				eDebug("%c ... %s", p, v.c_str());
				if (p == 'p')
					s.service_provider=v;
				else if (p == 'f')
				{
					sscanf(v.c_str(), "%x", &s.dxflags);
				} else if (p == 'c')
				{
					int cid, val;
					sscanf(v.c_str(), "%02d%04x", &cid, &val);
					if (cid < eServiceDVB::cacheMax)
						s.set((eServiceDVB::cacheID)cid,val);
				}
			}
	}

	eDebug("loaded %d subservices", count);

	fclose(f);
}

void eDVBSettings::saveBouquets()
{
	if ( !bouquetsChanged )
		return;

	eDebug("saving bouquets...");
	bouquetsChanged=0;
	
	FILE *f=fopen(CONFIGDIR "/enigma/bouquets", "wt");
	if (!f)
		eFatal("couldn't open bouquetfile - create " CONFIGDIR "/enigma!");
	fprintf(f, "eDVB bouquets /2/\n");
	fprintf(f, "bouquets\n");
	for (std::map<int,eBouquet*>::iterator i(bouquet_id_map.begin()); i != bouquet_id_map.end(); ++i)
	{
		eBouquet *b = i->second;
		fprintf(f, "%0d\n", b->bouquet_id);
		fprintf(f, "%s\n", b->bouquet_name.c_str());
		for (ServiceReferenceDVBIterator s = b->list.begin(); s != b->list.end(); s++)
			fprintf(f, "%04x:%08x:%04x:%04x:%d\n", s->getServiceID().get(), s->getDVBNamespace().get(), s->getTransportStreamID().get(), s->getOriginalNetworkID().get(), s->getServiceType());
		fprintf(f, "/\n");
	}
	fprintf(f, "end\n");
	fprintf(f, "Have a lot of fun!\n");
	fclose(f);
	eDebug("done");
}

void eDVBSettings::loadBouquets()
{
	FILE *f=fopen(CONFIGDIR "/enigma/bouquets", "rt");
	if (!f)
		return;
	bouquetsChanged=0;
	char line[256];
	if ((!fgets(line, 256, f)) || strncmp(line, "eDVB bouquets", 13))
	{
		eDebug("not a bouquetfile");
		return;
	}
	eDebug("reading bouquets");
	if ((!fgets(line, 256, f)) || strcmp(line, "bouquets\n"))
	{
		eDebug("settings invalid, no transponders");
		return;
	}

	bouquet_id_map.clear();
	for (std::map<eString, eBouquet*>::iterator it(bouquet_name_map.begin()); it != bouquet_name_map.end(); ++it)
		delete it->second;
	bouquet_name_map.clear();

	while (!feof(f))
	{
		if (!fgets(line, 256, f))
			break;
		if (!strcmp(line, "end\n"))
			break;
		int bouquet_id=-1;
		sscanf(line, "%d", &bouquet_id);
		if (!fgets(line, 256, f))
			break;
		line[strlen(line)-1]=0;
		eBouquet *bouquet=createBouquet(bouquet_id, line);
		while (!feof(f))
		{
			fgets(line, 256, f);
			if (!strcmp(line, "/\n"))
				break;
			int service_id=-1, dvb_namespace=-1, transport_stream_id=-1, original_network_id=-1, service_type=-1;
			sscanf(line, "%x:%x:%x:%x:%d", &service_id, &dvb_namespace, &transport_stream_id, &original_network_id, &service_type);
			if (service_type == -1)
				continue;
			bouquet->add(
				eServiceReferenceDVB(
					eDVBNamespace(dvb_namespace),
					eTransportStreamID(transport_stream_id), 
					eOriginalNetworkID(original_network_id), 
					eServiceID(service_id), 
					service_type));
		}
	}

	eDebug("loaded %d bouquets", getBouquets()->size());
	
	fclose(f);
	
	revalidateBouquets();
	eDebug("ok");
}

void eDVBSettings::clearList()
{
	if (transponderlist)
	{
		transponderlist->clearAllTransponders();
		transponderlist->clearAllServices();
		removeDVBBouquets(); // user Bouquets do not delete...
	}

	/*emit*/ dvb.bouquetListChanged();
}

void eDVBSettings::removeOrbitalPosition(int orbital_position)
{
	if (transponderlist)
	{
		transponderlist->removeOrbitalPosition(orbital_position);
		removeDVBBouquets();
		sortInChannels();
	}

	/*emit*/ dvb.bouquetListChanged();
}

int eDVBSettings::importSatcoDX(eString line)
{
		// hier versuchen wir das komische "SatcoDX"-format zu parsen, was wahrlich noch aus analog-tv zeiten stammt...
	if (line.left(7) != "SATCODX")
	{
		eDebug("SatcoDX: header invalid.");
		return -1;
	}
		// stammen MUSS.
	if (line.mid(7, 1) != "1")
	{
		eDebug("SatcoDX: unsupported page.");
		return -2;
	}
	if (line.mid(8, 2) != "03")
	{
		eDebug("SatcoDX: must be version 03");
		return -3;
	}
		// denn damals kam man wohl mit 20 zeichen fuer den kanalnamen aus, und somit festen strukturen. dvb2k laesst gruessen.
	if (line.size() != 128)
	{
		eDebug("SatcoDX: invalid line length.");
		return -4;
	}
		// also definieren wir den satelliten-NAMEN. bloss nicht zu kompliziert. darf natuerlich nur begrenzt lang sein, klar.
	eString satname=line.mid(10, 18);
		// und den TYP. Ah! Doch etsi? service_type? nee quatsch. T fuer TV, R fuer Radio, D fuer Data (ist mpeg nicht auch data? *wunder*) und _ fuer "package transponder". HAE? naja gut.
	eString type=line.mid(28, 1);
		// oh und das broadcasting system. so wie im etsi? klar. TRAEUM WEITER. hier gibts es:
		// (langweilig) 422_, ADR_, BMAC_, D2MAC (wie das in 4 zeichen passt ist mir WIRKLICH unklar.), DIC1, DIC2, ISDB, und jetzt kommts:
		// MPG1 ... ok, MPEG-1 halt.
		// MP15 MPEG-1.5 (HAEEE???? was issen das? mpeg 2.5 mag es ja noch geben (wer auch immer SOLCHE low bitrate services ueber dvb macht.. pah)
		// MPG2 MPEG-2
		// MPG4 MPEG-4
		// MUSE, NTSC, PAL_, SECM (ich sag ja, analog.)
	eString system=line.mid(29, 4);
		// oh die frequenz. in ascii.
	eString frequency=line.mid(33, 9);
		// ooh! laut DVB? ne quatsch, man brauch mal wieder ne lookup table. 0 ist vertikal, 1 horiz, 2 ist linksdrehend und 3 ist im joghurt. (oder so)
	eString polarization=line.mid(42, 1);
		// der anfang vom ende. der kanalname. WELCH EIN WUNDER dass hier nicht noch die kanalNUMMER steht.
	eString channelname=line.mid(43, 8);
		// orbital position. JUHUU. endlich mal was brauchbares.
	eString orbital_position=line.mid(51, 4);
		// coverage mit dem man nix anfangen kann.
	eString coverage=line.mid(55, 8);
		// audio frequency ;))
	eString audio_freq=line.mid(63, 6);
		// die symbolrate.
	eString symbolrate=line.mid(69, 5);
		// FEC, gaenzlich unkonform aber naja. 0 fuer KEINE FEC .. (??), 1, 2, 3, 5, 7 fuer 1/2, 2/3, 3/4, 5/6, 7/8)
	eString fec=line.mid(74, 1);
		// die pids. in hex. .... .... Satcodx? nein, natuerlich IN ASCII.
	eString vpid=line.mid(75, 4);
	eString apid=line.mid(79, 4);
	eString pcrpid=line.mid(83, 4);
		// die sid
	eString sid=line.mid(87, 5);
		// die nid. (JA, die jungs kennen sich aus.)
	eString onid=line.mid(92, 5);
		// die tsid (JA, die jungs kennen sich WIRKLICH aus.)
	eString tsid=line.mid(97, 5);
		// die primary language. ok, wenn die danach filtern wollen, sollen sie es tun. meinetwegen.
	eString lang=line.mid(102, 3);
		// der mighty underscore
	if (line[105] != '_')
	{
		eDebug("SatcoDX: underscore missing.");
		return -5;
	}
		// blakram
	eString cc=line.mid(106, 2);
	eString lang2=line.mid(108, 2);
	eString crypt=line.mid(111, 4);
		// und der rest vom ende.
	channelname+=line.mid(115, 12);
	if (line[127] != '\r')
	{
		eDebug("SatcoDX: missing CR.");
		return -6;
	}
	
	if (type[0] == '_')
		return 0;
	if (system.left(3) != "MPG")
		return 0;
	
	int service_type=3;
	if (type == "T")
		service_type=1;
	else if (type == "R")
		service_type=2;
	
	eDVBNamespace dvb_namespace;

	// create transponder.
	
	int my_orbital_position=atoi(orbital_position.c_str());
	if (my_orbital_position >= 1800) // convert 0..3599 to -1800..1799
		my_orbital_position-=3600;
		
	dvb_namespace=eDVBNamespace(orbital_position << 16);

	eServiceDVB &dvbservice=transponderlist->createService(
		eServiceReferenceDVB(
			dvb_namespace,
			eTransportStreamID(atoi(tsid.c_str())), eOriginalNetworkID(atoi(onid.c_str())), eServiceID(atoi(sid.c_str())),
			service_type)
		);
	dvbservice.service_type=service_type;
	dvbservice.service_name=channelname;
	dvbservice.service_provider=satname;		// HA! jetzt hab ich's den lamern aber gegeben!
	dvbservice.dxflags=eServiceDVB::dxNoDVB; // ists ja auch nicht.
	if (vpid[0] != '_')
		dvbservice.set(eServiceDVB::cVPID, atoi(vpid.c_str()));
	if (apid[0] != '_')
		dvbservice.set(eServiceDVB::cAPID, atoi(apid.c_str()));
	if (pcrpid[0] != '_')
	dvbservice.set(eServiceDVB::cPCRPID, atoi(pcrpid.c_str()));
	
	eTransponder &t=transponderlist->createTransponder(dvb_namespace, eTransportStreamID(atoi(tsid.c_str())), eOriginalNetworkID(atoi(onid.c_str())));
	t.state=eTransponder::stateOK;

	int myfec;
	switch (atoi(fec.c_str()))
	{
	case 0:
		myfec=FEC_AUTO;
		break;
	case 1:
		myfec=FEC_1_2;
		break;
	case 2:
		myfec=FEC_2_3;
		break;
	case 3:
		myfec=FEC_3_4;
		break;
	case 5:
		myfec=FEC_5_6;
		break;
	case 7:
		myfec=FEC_7_8;
		break;
	default:
		myfec=FEC_AUTO;
		break;
	}

	t.setSatellite(atoi(frequency.c_str()), atoi(symbolrate.c_str())*1000, atoi(polarization.c_str())^1, myfec, my_orbital_position, 0);

	return 0;
}

eDVBSettings::~eDVBSettings()
{
	saveServices();
	saveBouquets();
	for (std::map<int, eBouquet*>::iterator it(bouquet_id_map.begin()); it != bouquet_id_map.end(); ++it )
		delete it->second;
	if (transponderlist)
		delete transponderlist;
}
