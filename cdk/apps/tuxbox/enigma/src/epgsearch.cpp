#include <epgsearch.h>
#include <lib/gdi/font.h> // eTextPara
#include <src/enigma_dyn_utils.h>

using namespace std;
eEPGSearchDATA *eEPGSearchDATA::instance;
SearchEPGDATA SearchResultsEPG;

bool sortByEventStart(const SEARCH_EPG_DATA& a, const SEARCH_EPG_DATA& b)
{
	return a.start_time < b.start_time;
}

static int searchforEvent(EITEvent *ev, const eString &search, eString &titlefound, int intExactMatch, int intCaseSensitive, int genre)
{
	eString title, description;
	eString sFind;
	
	int intFound = 0;

	// Genre Suchkriterium schlägt immer Titelsuche ;)
	if ( genre != 0)
	{
		int Range = 0;
		switch (genre)
		{
			case 32:
				Range = 36;
				break;
			case 48:
				Range = 51;
				break;
			case 64:
				Range = 75;
				break;
			case 80:
				Range = 85;
				break;
			case 96:
				Range = 102;
				break;
			case 112:
				Range = 123;
				break;
			case 128:
				Range = 131;
				break;
			case 144:
				Range = 151;
				break;
			case 160:
				Range = 167;
				break;
			case 176:
				Range = 179;
				break;
			default:
				break;

		}

		for (ePtrList<Descriptor>::iterator d(ev->descriptor); d != ev->descriptor.end(); ++d)
		{
			Descriptor *descriptor=*d;
			if(descriptor->Tag()==DESCR_CONTENT)
			{
				ContentDescriptor *cod=(ContentDescriptor*)descriptor;

				for(ePtrList<descr_content_entry_struct>::iterator ce(cod->contentList.begin()); ce != cod->contentList.end(); ++ce)
				{

					if ( genre < 32 )
					{
						if (genre  == ce->content_nibble_level_1*16+ce->content_nibble_level_2)
							intFound = 1;
					}
					else
					{
						int genreID = ce->content_nibble_level_1*16+ce->content_nibble_level_2;
						if ( (genreID >= genre) && (genreID <= Range))
							intFound = 1;
					}
				}
			}
		}
		if (intFound)
		{
			LocalEventData led;
			led.getLocalData(ev, &title, &description);
			titlefound = title;
		}
	}
	else
	{

			if (search != "")
			{
				LocalEventData led;
				
				led.getLocalData(ev, &title, &description);
				titlefound = title;
				
				if (intExactMatch || intCaseSensitive) 
					sFind = title;
				else
					sFind = title.upper();
				if (!intExactMatch)
				{
					if (sFind.find(search) != eString::npos)
						intFound = 1;
				}
				else
				{
					if (!strcmp(search.c_str(),sFind.c_str()))
						intFound = 1;
				}
			}

	}


	return intFound;
}

static void SearchInChannel(const eServiceReference &e, eString search, int begin, int intExactMatch, int intCaseSensitive, int genre)
{
	int duration = 0;
	if ((search != "") || (genre != 0))
	{
		eEPGCache *epgcache=eEPGCache::getInstance();
		eServiceReferenceDVB &ref = (eServiceReferenceDVB&)e;
		epgcache->Lock();
		//const timeMap *evmap = epgcache->getTimeMap((eServiceReferenceDVB&)ref);		// ims
		timeMapPtr evmap = eEPGCache::getInstance()->getTimeMapPtr((eServiceReferenceDVB&)ref); // ims for PLi
		if (!evmap)
		{
			epgcache->Unlock();
			// nix gefunden :-(
		}
		else
		{
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
			int tsidonid =(rref.getTransportStreamID().get()<<16)|rref.getOriginalNetworkID().get();
			eService* current;
			eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
			if (sapi)
			{
				current = eDVB::getInstance()->settings->getTransponders()->searchService(e);
				if (current)
				{
					for (timeMap::const_iterator event(ibegin); event != iend; ++event)
					{
						EITEvent *ev = new EITEvent(*event->second, tsidonid, event->second->type);
						eString titleFound = "";
						int intFound = 0;
						intFound = searchforEvent( ev, search, titleFound, intExactMatch, intCaseSensitive, genre);
						if (intFound)
						{
							SEARCH_EPG_DATA tempSEARCH_EPG_DATA;
							tempSEARCH_EPG_DATA.ref = e;
							tempSEARCH_EPG_DATA.name = current->service_name;
							tempSEARCH_EPG_DATA.start_time = ev->start_time;
							tempSEARCH_EPG_DATA.duration = ev->duration;
							tempSEARCH_EPG_DATA.title = titleFound;
							SearchResultsEPG.push_back(tempSEARCH_EPG_DATA);
						}
						delete ev;
					}
				}
			}
			epgcache->Unlock();
			if ( SearchResultsEPG.size())
				sort(SearchResultsEPG.begin(), SearchResultsEPG.end(), sortByEventStart);
		}
	}
}


class eSearchAllTVServices: public Object
{
	eServiceInterface &iface;
	eString search;
	time_t begin;
	int intExactMatch;
	int intCaseSensitive;
	int genre;
	
public:
	eSearchAllTVServices(eServiceInterface &iface, eString search, time_t begin, int intExactMatch, int intCaseSensitive, int genre): iface(iface), search(search), begin(begin), intExactMatch(intExactMatch), intCaseSensitive(intCaseSensitive), genre(genre)
	{
	}
	void addEntry(const eServiceReference &e)
	{
		int duration = 0;
		if ((search != "") || (genre != 0))
		{
			eEPGCache *epgcache=eEPGCache::getInstance();
			eServiceReferenceDVB &ref = (eServiceReferenceDVB&)e;
			epgcache->Lock();
		//	const timeMap *evmap = epgcache->getTimeMap((eServiceReferenceDVB&)ref);		//ims
			timeMapPtr evmap = eEPGCache::getInstance()->getTimeMapPtr((eServiceReferenceDVB&)ref);	//ims for PLi
			if (!evmap)
			{
				epgcache->Unlock();
				// nix gefunden :-(
			}
			else
			{
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
				int tsidonid = (rref.getTransportStreamID().get()<<16)|rref.getOriginalNetworkID().get();
				eService *service=iface.addRef(e);
				if (service)
				{
					for (timeMap::const_iterator event(ibegin); event != iend; ++event)
					{
						EITEvent *ev = new EITEvent(*event->second, tsidonid, event->second->type );
						int intFound = 0;
						eString titleFound = "";
						intFound = searchforEvent(ev, search, titleFound, intExactMatch, intCaseSensitive, genre);
						if (intFound)
						{

							SEARCH_EPG_DATA tempSEARCH_EPG_DATA;
							tempSEARCH_EPG_DATA.ref = e;
							tempSEARCH_EPG_DATA.name = service->service_name;
							tempSEARCH_EPG_DATA.start_time = ev->start_time;
							tempSEARCH_EPG_DATA.duration = ev->duration;
							tempSEARCH_EPG_DATA.title = titleFound;
							SearchResultsEPG.push_back(tempSEARCH_EPG_DATA);
							
						}
						delete ev;
					}
				}
				epgcache->Unlock();
				if ( SearchResultsEPG.size())
					sort(SearchResultsEPG.begin(), SearchResultsEPG.end(), sortByEventStart);
			}
			iface.removeRef(e);
		}
	}
};

/*  disabled, is not called more
eEPGSearch::eEPGSearch(eServiceReference ref, EITEvent e):ePLiWindow(_("EPG Search"),470)
{
	sServiceReference = ref2string(ref);
	eString AnzeigeCheckBox = _("unknown");
	eService* current;
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		current = eDVB::getInstance()->settings->getTransponders()->searchService(ref);
		if (current)
			AnzeigeCheckBox = current->service_name;
	}
	eString descr;
	
	for (ePtrList<Descriptor>::const_iterator d(e.descriptor); d != e.descriptor.end(); ++d)
		{
			if ( d->Tag() == DESCR_SHORT_EVENT)
			{
				ShortEventDescriptor *s=(ShortEventDescriptor*)*d;
				descr=s->event_name;
				break;
			}
		}
	Titel = descr;
	sServiceReferenceSearch = AnzeigeCheckBox;
	eEPGSearchBox();
}
*/
eEPGSearch::eEPGSearch(eServiceReference ref, eString CurrentEventName):ePLiWindow(_("EPG Search"),410)
{
	sServiceReference = ref2string(ref);
	eString AnzeigeCheckBox = _("unknown");
	eService* current;
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		current = eDVB::getInstance()->settings->getTransponders()->searchService(ref);
		if (current)
			AnzeigeCheckBox = current->service_name;
	}
	Titel = CurrentEventName;
	sServiceReferenceSearch = AnzeigeCheckBox;

	const int xPos1 = 10;		
	const int entrySize = 410 - 2 * xPos1; 
	const int xPos2 = entrySize / 2 + xPos1;  // second column
	const int maxL  = xPos2 - xPos1 - 5;      // max. lenght of labels

	lb_Caption=new eLabel(this);
	lb_Caption->move(ePoint(xPos1, yPos()));
	lb_Caption->resize(eSize(maxL, widgetHeight())); 
	lb_Caption->setText(_("Title:"));

	InputName=new eTextInputField(this,lb_Caption); 
	InputName->setMaxChars(50); 
	InputName->move(ePoint(xPos1+80, yPos())); 
	InputName->resize(eSize(entrySize-80, widgetHeight()));
	InputName->loadDeco(); 
	InputName->setText(Titel);
	InputName->setHelpText(_("Enter title for epg searching"));

	nextYPos(35);
	lb_Caption4=new eLabel(this);
	lb_Caption4->move(ePoint(xPos1, yPos()));
	lb_Caption4->resize(eSize(xPos1+80, widgetHeight())); 
	lb_Caption4->setText(_("Genre:"));

	cboGenre = new eComboBox(this);
	new eListBoxEntryText(*cboGenre, _("No genre"), (void *) (0));
	// Movies lasse ich in Einzelkategorien, des Rest fasse ich in Gruppen zusammen...
	new eListBoxEntryText(*cboGenre, _("Movie"), (void *) (16));
	new eListBoxEntryText(*cboGenre, _("Thriller"), (void *) (17));
	new eListBoxEntryText(*cboGenre, _("Adventure"), (void *) (18));
	new eListBoxEntryText(*cboGenre, _("SciFi"), (void *) (19));
	new eListBoxEntryText(*cboGenre, _("Comedy"), (void *) (20));
	new eListBoxEntryText(*cboGenre, _("Soap"), (void *) (21));
	new eListBoxEntryText(*cboGenre, _("Romance"), (void *) (22));
	new eListBoxEntryText(*cboGenre, _("Serious"), (void *) (23));
	new eListBoxEntryText(*cboGenre, _("Adult"), (void *) (24));
	new eListBoxEntryText(*cboGenre, _("News"), (void *) (32)); // Range = 32 bis 36
	new eListBoxEntryText(*cboGenre, _("Show"), (void *) (48)); // Range = 48 bis 51
	new eListBoxEntryText(*cboGenre, _("Sports"), (void *) (64)); // Range = 64 bis 75
	new eListBoxEntryText(*cboGenre, _("Children"), (void *) (80)); // Range = 80 bis 85
	new eListBoxEntryText(*cboGenre, _("Music"), (void *) (96)); // Range = 96 bis 102
	new eListBoxEntryText(*cboGenre, _("Culture"), (void *) (112)); // Range = 112 bis 123
	new eListBoxEntryText(*cboGenre, _("Social"), (void *) (128)); // Range = 128 bis 131
	new eListBoxEntryText(*cboGenre, _("Education"), (void *) (144)); // Range = 144 bis 151
	new eListBoxEntryText(*cboGenre, _("Hobbies"), (void *) (160)); // Range = 160 bis 167buildWindow();
	new eListBoxEntryText(*cboGenre, _("Live"), (void *) (176)); // Range = 176 bis 179

	cboGenre->move(ePoint(xPos1+80, yPos()));
	cboGenre->resize(eSize(entrySize - 80, widgetHeight()));
	cboGenre->setCurrent((void*)0);
	cboGenre->setHelpText(_("Select a genre if you want to search for a genre"));
	cboGenre->loadDeco();
	CONNECT(cboGenre->selchanged, eEPGSearch::cboGenreChanged);
	
	nextYPos(40);
	lb_Caption2=new eLabel(this);
	lb_Caption2->move(ePoint(xPos1, yPos()));
	lb_Caption2->resize(eSize(maxL, widgetHeight())); 
	lb_Caption2->setText(_("Search in:"));
	
	intAllServices = 0;
	intExactMatch = 0;
	intCaseSensitive = 0;
	
	nextYPos();
	chkCurrentService = new eCheckbox(this, 0, 1);
	chkCurrentService->setText(sServiceReferenceSearch); // current service
	chkCurrentService->move(ePoint(xPos1, yPos()));
	chkCurrentService->resize(eSize(maxL,widgetHeight()));
	chkCurrentService->setHelpText(_("Select this option if you want to search the title only in selected channel"));
	chkCurrentService->loadDeco();
	CONNECT(chkCurrentService->checked, eEPGSearch::chkCurrentServiceStateChanged);
	
	chkAllServices = new eCheckbox(this, 1, 1);
	chkAllServices->setText(_("All services"));
	chkAllServices->move(ePoint(xPos2, yPos()));
	chkAllServices->resize(eSize(maxL,widgetHeight()));
	chkAllServices->setHelpText(_("Select this option if you want to search the title in all services"));
	chkAllServices->loadDeco();
	CONNECT(chkAllServices->checked, eEPGSearch::chkAllServicesStateChanged);
	
	nextYPos(35);
	lb_Caption3=new eLabel(this);
	lb_Caption3->move(ePoint(xPos1, yPos()));
	lb_Caption3->resize(eSize(maxL, widgetHeight())); 
	lb_Caption3->setText(_("Search options:"));
	
	nextYPos();
	chkExactMatch = new eCheckbox(this, 0, 1);
	chkExactMatch->setText(_("Exact match"));
	chkExactMatch->move(ePoint(xPos1, yPos()));
	chkExactMatch->resize(eSize(maxL,widgetHeight()));
	chkExactMatch->setHelpText(_("Select this option if you want to search the exact title"));
	chkExactMatch->loadDeco();
	
	chkCaseSensitive = new eCheckbox(this, 0, 1);
	chkCaseSensitive->setText(_("Case sensitive"));
	chkCaseSensitive->move(ePoint(xPos2, yPos()));
	chkCaseSensitive->resize(eSize(maxL,widgetHeight()));
	chkCaseSensitive->setHelpText(_("Select this option if you want to make this epg search case sensitive"));
	chkCaseSensitive->loadDeco();

	buildWindow();

	CONNECT(bOK->selected, eEPGSearch::Search);
	bOK->setText(_("Search")); // Renamed OK button to Search
	bOK->setHelpText(_("Start searching")); 
	
	setFocus(InputName);
	canCheck = 1;
}

eEPGSearch::eEPGSearch():ePLiWindow(0)
{
	SearchName = "";
}

void eEPGSearch::cboGenreChanged(eListBoxEntryText *item)
{
	if (item)
	{
		int ID = (int)item->getKey();
		if (ID == 0)
			InputName->setText(Titel);
		else
			InputName->setText("");
	}
}

void eEPGSearch::chkCurrentServiceStateChanged(int state)
{
	if (canCheck)
	{
		if (!chkCurrentService->isChecked())
		{
			canCheck = 0;
			chkAllServices->setCheck(1);
			canCheck = 1;
		}
		else
		{
			canCheck = 0;
			chkAllServices->setCheck(0);
			canCheck = 1;
		}
	}
}
void eEPGSearch::chkAllServicesStateChanged(int state)
{
	if (canCheck)
	{
		if (chkAllServices->isChecked())
		{
			canCheck = 0;
			chkCurrentService->setCheck(0);
			canCheck = 1;
		}
		else
		{
			canCheck = 0;
			chkCurrentService->setCheck(1);
			canCheck = 1;
		}
	}
}
void eEPGSearch::Search()
{
	hide();
	sleep(1); // sonst ist das Fenster manchmal zerstückelt.... :-/
	eString h;
	if (chkAllServices->isChecked())
		h = eString(_("in all services"));
	else
		h = eString(_("in")) + " " + sServiceReferenceSearch;

	eString Anzeige = "";
	int genre = 0;
	genre = (int)cboGenre->getCurrent()->getKey();
	if ( genre == 0)
	{
			Anzeige = eString(_("Searching for")) + ": " + InputName->getText() + "\n" + h + "...";
			SearchName = InputName->getText();
	}
	else
	{
		Anzeige = eString(_("Searching for")) + " " + eString(_("genre:")) + " " + cboGenre->getText() + "\n" + h + "...";
		SearchName = eString(_("Genre:")) + " " + cboGenre->getText();
	}
	eMessageBox msg(Anzeige, _("EPG Search"), eMessageBox::iconInfo);
	msg.show();
	int back = Searching(InputName->getText());
	msg.hide();
	close(back);
	
}
eString eEPGSearch::getSearchName()
{
	return SearchName;
}
int eEPGSearch::Searching(eString SearchName)
{
	eString search;
	time_t begin = 0; //1184515200;
	eString mDescription = SearchName;
	eString current;
	int intFound = 2;
	if (chkExactMatch->isChecked())
	{
		intExactMatch = 1;
		search = mDescription;
	}
	else
		intExactMatch = 0;
	
	if (chkCaseSensitive->isChecked())
	{
		intCaseSensitive = 1;
		search = mDescription;
	}
	else
		intCaseSensitive = 0;
	
	if (!chkExactMatch->isChecked() && !chkCaseSensitive->isChecked())
	{
		search = mDescription.upper();
		intExactMatch = 0;
		intCaseSensitive = 0;
	}
	SearchResultsEPG.clear();

	int genre = 0;
	genre = (int)cboGenre->getCurrent()->getKey();

	if (chkAllServices->isChecked())
	{
		current = "1:15:fffffffe:12:ffffffff:0:0:0:0:0:";
		eServiceInterface *iface=eServiceInterface::getInstance();
		if (iface)
		{		
			if ((search != "") || (genre != 0))
			{
				eServiceReference current_service=string2ref(current);
				eSearchAllTVServices conv( *iface, search, begin, intExactMatch, intCaseSensitive, genre);
				Signal1<void,const eServiceReference&> signal;
				signal.connect(slot(conv, &eSearchAllTVServices::addEntry));
				iface->enterDirectory(current_service, signal);
				iface->leaveDirectory(current_service);
			}
		}
	}
	else
	{
		eServiceReference current_service=string2ref(sServiceReference);
		SearchInChannel(current_service, search, begin,intExactMatch, intCaseSensitive, genre);
	}
	if (SearchResultsEPG.size() )
		intFound = 0;
	return intFound;
}
int eEPGSearch::EPGSearching(eString title, eServiceReference SearchRef, int AllServices, int ExactMatch, int CaseSensitive, int genre)
{
	eString search;
	time_t begin = 0; //1184515200;
	eString current;
	int intFound = 0;
	search = title;

	
	if (!ExactMatch && !CaseSensitive)
		search = title.upper();

	SearchResultsEPG.clear();
	if (AllServices)
	{
		current = "1:15:fffffffe:12:ffffffff:0:0:0:0:0:";
		eServiceInterface *iface=eServiceInterface::getInstance();
		if (iface)
		{		
			if ((search != "") || (genre != 0))
			{
				eServiceReference current_service=string2ref(current);
				eSearchAllTVServices conv( *iface, search, begin, ExactMatch, CaseSensitive, genre);
				Signal1<void,const eServiceReference&> signal;
				signal.connect(slot(conv, &eSearchAllTVServices::addEntry));
				iface->enterDirectory(current_service, signal);
				iface->leaveDirectory(current_service);
			}
		}
	}
	else
		SearchInChannel(SearchRef, search, begin,intExactMatch, intCaseSensitive, genre);
	if (SearchResultsEPG.size() )
		intFound = 1;
	return intFound;
}
eEPGSearchDATA::eEPGSearchDATA()
{
	if (!instance)
		instance=this;
	SearchResultsEPG.clear();
	
}
SearchEPGDATA eEPGSearchDATA::getSearchData()
{
	return SearchResultsEPG;
}
void eEPGSearchDATA::clearList()
{
	SearchResultsEPG.clear();	
}

