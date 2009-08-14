#include "RemoteTimer.h"
#include "../../include/plugin.h"
#include <lib/system/init_num.h>

#define version "RemoteTimer - v3.01 "
//#define _E2_  
//#define _debug_

extern "C" int plugin_exec( PluginParam *par );

static void XML(eString result)
{
	eString line, Output;
	ifstream infile(result.c_str());
	if (infile)
	{
		while (getline(infile, line, '\n'))
		{
			Output = Output + line;
		}
		Output.strReplace("&", "&amp;");
		FILE *temp = fopen(result.c_str(), "w");
		if (temp)
			fprintf(temp, "%s\n", Output.c_str());
		fclose(temp);
	}
}

int plugin_exec( PluginParam *par )
{
    
    eSelection Select;
    Select.show();
	if(Select.intFound)
		Select.exec();
	Select.hide();

    if(Select.ok || Select.cfg) 
    {
        int intTimeout = 4;
	    eConfig::getInstance()->setKey("/enigma/plugins/remotetimer/timeout", intTimeout);

        eString url;
#ifdef _E2_ 
        if(E2)
            url = "/web/getallservices?sRef=userbouquet.dbe01.tv";           
        else
#endif
            url = "/xml/services?mode=0&submode=4"; 
	    SendPartnerBoxRequest request(url);
#ifdef _debug_ 
	    eDebug("Plugin exec: %d",request.result);
#endif
	    if (!request.result)
	    {   
	        eServiceList dlg(request.response);
            dlg.show();
	        dlg.exec();
	        dlg.hide();
	    }
	    else
	    {
		    request.dialog(request.error,0);
		    close(0);
	    }
    }
    else
    {
        if(!Select.intFound)
        {
            eMessageBox msg("Missing configuration file(s) in /var/tuxbox/config :"  
                            "\n\n"
                            "TimerEdit.conf"
                            "\n"
                            "or"
                            "\n"
                            "TimerEdit_01.conf  TimerEdit_02.conf ...",
                             version, eMessageBox::iconError|eMessageBox::btOK);
		    msg.show(); msg.exec(); msg.hide();
        }
    }
    return 0;
}

eSelection::eSelection(): ePLiWindow(version,390)
{
	cmove(ePoint(170, 200));
	cresize(eSize(390, 130));

    intFound = cfg = ok = 0;
    
	int save = 0;
	eString filename;

//  addActionMap(&i_shortcutActions->map);
//	addActionMap(&i_cursorActions->map);

    lblKey = new eLabel(this);
	lblKey->setText("Select Dreambox or IP:");
	lblKey->move(ePoint(10, 10));
	lblKey->resize(eSize(370, 30));
	lblKey->loadDeco();

    nextYPos(35);
   
    BoxSelection = new eComboBox(this, 5, lblKey);
    	
	for(int i=1; i<20; i++) // reading multi-conf file (if exist)	
	{
		filename.sprintf("/var/tuxbox/config/RemoteTimer_%.02d.conf", i);
		if (access(filename.c_str(), 0) == 0)
		{
			eString sIP = getAttribute(filename.c_str(), "ip");
			eString sBox = getAttribute(filename.c_str(), "name");
			eString sList = sBox + "\t" + sIP;
			new eListBoxEntryText( *BoxSelection, sList.c_str() , (void*) i, 0, sIP.c_str() );
			if (!intFound)
				save = i;
			intFound++;
		}
	}
	
    BoxSelection->move(ePoint(10, 40)); 
    BoxSelection->resize(eSize(clientrect.width()-20, 30));
    BoxSelection->loadDeco();

    nextYPos(35);
    
	OK=new eButton(this);
    OK->move(ePoint(140, clientrect.height()-40)); 
    OK->resize(eSize(100, 30));
	OK->setShortcut("green");
    OK->setShortcutPixmap("green");
    OK->loadDeco();
    OK->setText("OK");
	CONNECT(OK->selected, eSelection::Selected);


	if (intFound)       // was selected one of RemoteTimer_xx.conf 
		BoxSelection->setCurrent((void*)save);
	else                // doesnt exist multi-conf file => reading from RemoteTimer.conf
	{
	    if (access(ConfFile, 0) == 0)
	    {
		    IP = getAttribute(ConfFile, "ip");
		    Login = getAttribute(ConfFile, "login");
		    Password = getAttribute(ConfFile, "password");
		    Bouquet = getAttribute(ConfFile, "bouquet");
		    eString zoom = getAttribute(ConfFile, "zoom");
		    modeZoom = atoi(zoom.c_str());
		    Action = getAttribute(ConfFile, "action");
		    Box = getAttribute(ConfFile, "name");
#ifdef _E2_ 
		    eString e2 = getAttribute(ConfFile, "e2");
		    E2 = atoi(e2.c_str());
#endif
		    cfg = 1;
		}
	}
	setFocus(BoxSelection);
}

void eSelection::Selected()
{
	eListBoxEntryText *item;
	item = BoxSelection->getCurrent();
	if (item)
	{
		eString filename;
		int selected = (int)item->getKey();
		filename.sprintf("/var/tuxbox/config/RemoteTimer_%.02d.conf", selected);
		if (access(filename.c_str(), 0) == 0)
		{
			IP = getAttribute(filename.c_str(), "ip");
			Login = getAttribute(filename.c_str(), "login");
			Password = getAttribute(filename.c_str(), "password");
			Bouquet = getAttribute(filename.c_str(), "bouquet");
			eString zoom = getAttribute(filename.c_str(), "zoom");
			modeZoom = atoi(zoom.c_str());
			Action = getAttribute(filename.c_str(), "action");
			Box = getAttribute(filename.c_str(), "name");
#ifdef _E2_ 
			eString e2 = getAttribute(filename.c_str(), "e2");
			E2 = atoi(e2.c_str());
#endif
			cfg = 1;
		}
	}
	ok = 1;
	close(0);
}

eServiceList *eServiceList::instance;

eServiceList::eServiceList(eString XMLString): ePLiWindow(version, 610)
{	
	if (modeZoom)
	{
		// Is current Stream 16:9 ?
		// 1=square 2=4:3 3=16:9 4=20:9
		int aspect=0;
		FILE *bitstream=0;
		bitstream=fopen("/proc/bus/bitstream", "rt");
		if (bitstream)
		{
			char buffer[100];	
			while (fgets(buffer, 100, bitstream))
			{
				if (!strncmp(buffer, "A_RATIO: ", 9))
					aspect=atoi(buffer+9);
			}
			fclose(bitstream);
		}
		
		// What format of Monitor ?
		// 0=4:3Letterbox 1=4:3Panscan 2=16:9 3=always16:9
		unsigned int v_pin8 = 0;
		eConfig::getInstance()->getKey("/elitedvb/video/pin8", v_pin8);
		
		if ((v_pin8==2 && aspect==3) || (v_pin8==3))
		{
			x_=40; y_=64;
			w_=640;h_=484;
		}
		else
		{
			x_=40; y_=126;
			w_=640;h_=360;
		}
	}
	else
	{ 
	    // "Original values"...
		x_=60; y_=70;
		w_=610;h_=475;
	}
	
	cmove(ePoint(x_, y_));
	cresize(eSize(w_, h_));
	
	addActionMap(&i_shortcutActions->map);
	
	setText((eString)_("Service list") + eString().sprintf(" - %s  %s / %s" , _("connected:"), Box.c_str(), IP.c_str()));
	
	p_list=new eListBox<eListBoxEntryData>(this);
    p_list->move(ePoint(10, 10));
    p_list->resize(eSize(clientrect.width()-20, clientrect.height()-105));
	CONNECT(p_list->selected, eServiceList::p_listselected);
	CONNECT(p_list->selchanged, eServiceList::p_listselectionChanged);
	
	status = new eStatusBar(this);
    status->move( ePoint(0, clientrect.height()-95) );
	status->resize( eSize( clientrect.width(), 95) );
	status->loadDeco();
	
	// Label
    lb_selected=new eLabel(this);
    lb_selected->move(ePoint(10, clientrect.height()-90));
    lb_selected->resize(eSize(clientrect.width()-10, 25)); 
    
	lb_info=new eLabel(this);
    lb_info->move(ePoint(10, clientrect.height()-65));
    lb_info->resize(eSize(clientrect.width()-10, 25));
    
	// Buttons
	bt_rec=new eButton(this);
    bt_rec->move(ePoint(10, clientrect.height()-40));
    bt_rec->resize(eSize(140, 30));
//    bt_rec->setProperty("align", "center");
//	bt_rec->setProperty("vcenter", "");
//	bt_rec->setProperty("backgroundColor", "std_dred");
    bt_rec->setShortcut("red");
    bt_rec->setShortcutPixmap("red");
    bt_rec->loadDeco();
    bt_rec->setText("REC/STOP");
	CONNECT(bt_rec->selected, eServiceList::REC);
	
	bt_event=new eButton(this);
    bt_event->move(ePoint(160, clientrect.height()-40));
    bt_event->resize(eSize(140, 30));
//    bt_event->setProperty("align", "center");
//	bt_event->setProperty("vcenter", "");
//	bt_event->setProperty("backgroundColor", "std_dgreen");
    bt_event->setShortcut("green");
    bt_event->setShortcutPixmap("green");
    bt_event->loadDeco();
    bt_event->setText("Rec Event");
	CONNECT(bt_event->selected, eServiceList::REC_EVENT);
	
	bt_timerlist=new eButton(this);
    bt_timerlist->move(ePoint(clientrect.width()-300, clientrect.height()-40));
    bt_timerlist->resize(eSize(140, 30));
//    bt_timerlist->setProperty("align", "center");
//	bt_timerlist->setProperty("vcenter", "");
//	bt_timerlist->setProperty("backgroundColor", "std_dyellow");
    bt_timerlist->setShortcut("yellow");
    bt_timerlist->setShortcutPixmap("yellow");
    bt_timerlist->loadDeco();
    bt_timerlist->setText(_("Timer list"));
	CONNECT(bt_timerlist->selected, eServiceList::ShowTimerList);
	
	bt_epg=new eButton(this);
    bt_epg->move(ePoint(clientrect.width()-150, clientrect.height()-40));
    bt_epg->resize(eSize(140, 30));
//    bt_epg->setProperty("align", "center");
//	bt_epg->setProperty("vcenter", "");
//	bt_epg->setProperty("backgroundColor", "std_dblue");
	bt_epg->setShortcut("blue");
    bt_epg->setShortcutPixmap("blue");
    bt_epg->loadDeco();
    bt_epg->setText(_("show EPG"));
	CONNECT(bt_epg->selected, eServiceList::EPG);
	
	
	setHelpText((eString)version + "by ims (based on Dr.Best 2.00) \n\n\n" \
		"Menu - display configuration dialog\n");
	
	timer = new eTimer(eApp);
	CONNECT(timer->timeout, eServiceList::timerHandler);
	
	read_xml(XMLString);
	
    ShowDetails();
	CheckForRecording();
	
//	buildWindow();
}

eServiceList::~eServiceList()
{
	delete timer;
	eDebug("[RemoteTimer] is ending...");
}

void eServiceList::timerHandler()
{
    //eDebug("*** NOW is: %s ***",getActualTime(5).c_str());
    ShowDetails();
}

void eServiceList::ShowTimerList()
{
   hide();
    eString url;
#ifdef _E2_ 
	if (E2)
		url = "/web/timerlist";
	else
#endif
        url = "/xml/timers"; 
	int reload = 1;
	while (reload == 1)
	{
	    SendPartnerBoxRequest request(url);
	    if (!request.result)
	    {
		    eTimerList dlg(request.response);
	        dlg.show();
	        dlg.exec();
	        reload = dlg.Reload;
	        dlg.hide();
	    }
	    else
	        request.dialog(request.error,0);
	}
   show();
}

void eServiceList::CheckForRecording()
{
	// check for Recording!
	eString url ="/xml/boxstatus";
	SendPartnerBoxRequest request(url);
	if (!request.result)
	{   
        Recording = "-1";
//            eDebug("checkRecordings: %s",request.response.c_str());        	
       	XMLTreeParser * parser;
        parser = new XMLTreeParser("ISO-8859-1");
        if ( ! parser->Parse( request.response.c_str(), request.response.length(), 1 ) )
        {
            eMessageBox msg("XML Error!\n Ref-Err-Code: 0001", version, eMessageBox::iconError|eMessageBox::btOK);
	        msg.show(); msg.exec(); msg.hide();
            delete parser;
            parser = NULL;
            return;
        }
        XMLTreeNode * root = parser->RootNode();
        if(!root)
        {
            eMessageBox msg("XML Error!\n Ref-Err-Code: 0002", version, eMessageBox::iconError|eMessageBox::btOK);
	        msg.show(); msg.exec(); msg.hide();
            return;
        }
       	for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
        {
       	    if(!strcmp(node->GetType(), "recording"))
       		    Recording = node->GetData();
       	}
       	delete parser;
    }
	else
	    request.dialog(request.error,0);
}

void eServiceList::Config()
{

}

void eServiceList::p_listselected(eListBoxEntryData *item)
{
        EPG();
}

void eServiceList::p_listselectionChanged(eListBoxEntryData *item)
{
	if (item)
		ShowDetails();
}

void eServiceList::ShowDetails()
{
	eString text = "";
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{   
		if (item->mEPGData.EventDuration != "")
		{   
			if (item->mEPGData.EventDuration != "")
			{
				lb_selected->setText("Event's time: " + eTimerList::getInstance()->Time_from_to(item->mEPGData.start, item->mEPGData.duration)  
				                                      + "  (" + item->mEPGData.EventDuration + " min" + ")");
				lb_info->setText("Actual time:  "+ getActualTime(5));
			}
			else
			{
				lb_info->setText("Actual time:  " + getActualTime(5));
			}
		}
		else
		{   
			lb_selected->setText((eString)_("no epg data available"));
			lb_info->setText("switch local box to channel and receive EPG Data!");
		}
	}
}

void eServiceList::read_xml(eString file)
{
    XMLTreeParser * parser;
#ifdef _E2_ 
    if(E2)
        parser = new XMLTreeParser("UTF-8");
    else
#endif
	    parser = new XMLTreeParser("ISO-8859-1");
	if ( ! parser->Parse( file.c_str(), file.length(), 1 ) )
    {
        eMessageBox msg("XML Error!\n Ref-Err-Code: 0006", version, eMessageBox::iconError|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
        delete parser;
        parser = NULL;
        return;
    }

	XMLTreeNode * root = parser->RootNode();
    if(!root)
    {
        eMessageBox msg("XML Error!\n Ref-Err-Code: 0005", version, eMessageBox::iconError|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
        return;
    }
    
#ifdef _E2_ 
    if(E2)
    {
        int Counter = 0;
	    eString BouquetReference;
	    eString BouquetName;

	    int zCounter = atoi(Bouquet.c_str());
	    for(XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
        {
		    if(!strcmp(node->GetType(), "e2bouquet"))
		    {   eDebug("e2bouquet");
			    if (zCounter == Counter)
			    {
//				    if(!strcmp(node->GetType(), "e2servicelist"))
//				    {eDebug("e2servicelist");
				        for(XMLTreeNode *i = node->GetChild(); i; i = i->GetNext())
				        {
					        if( !strcmp(i->GetType(),  "e2servicereference") )
					        {
					        	BouquetReference = i->GetData();eDebug("BouquetRef=%s",BouquetReference.c_str());
					        }
					        if(!strcmp(node->GetType(), "e2servicelist")); // nevim
					        if(!strcmp(i->GetType(), "e2servicename")) BouquetName = i->GetData();
					        if(!strcmp(i->GetType(), "e2servicelist"))
					        {
					        	for(XMLTreeNode *a = i->GetChild(); a; a = a->GetNext())
					        	{
					        		if(!strcmp(a->GetType(), "e2servicereference")) mEPGData.reference = a->GetData();
					        		if(!strcmp(a->GetType(), "e2servicename")) mEPGData.name = a->GetData();
//					        		if(!strcmp(a->GetType(), "provider")) mEPGData.provider = a->GetData();
//					        		if(!strcmp(a->GetType(), "orbital_position")) mEPGData.orbital_position = a->GetData();
					        	}
					        	eString DisplayList="";
					        	mEPGData.start = "";
					        	mEPGData.duration = "";
					        	mEPGData.Details = "";
					        	mEPGData.Description = "";
					        	mEPGData.EventStart = "";
					        	mEPGData.EventDuration = "";
					        	//mEPGData.Genre = "";
					        	mEPGData.service = 0;
					        	GetEPGData(mEPGData.reference, DisplayList, mEPGData);
					        	eDebug("Displaylist=%s",DisplayList.c_str());
					        	new eListBoxEntryData(p_list, DisplayList, mEPGData);
					        }
				        }   
				    }
//			    }
            }
   	        Counter++;
        }
    }
    else
    {
#endif
    	int Counter = 0;
    	eString BouquetReference;
    	eString BouquetName;

    	int zCounter = atoi(Bouquet.c_str());
    	for(XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
        {
    		if(!strcmp(node->GetType(), "bouquet"))
    		{   
    			if (zCounter == Counter)
    			{
    				for(XMLTreeNode *i = node->GetChild(); i; i = i->GetNext())
    				{
    					if( !strcmp(i->GetType(),  "reference") )
    					{
    						BouquetReference = i->GetData();
    					}
    					if(!strcmp(i->GetType(), "name")) BouquetName = i->GetData();
    					if(!strcmp(i->GetType(), "service"))
    					{
    						for(XMLTreeNode *a = i->GetChild(); a; a = a->GetNext())
    						{
    							if(!strcmp(a->GetType(), "reference")) mEPGData.reference = a->GetData();
    							if(!strcmp(a->GetType(), "name")) mEPGData.name = a->GetData();
    							if(!strcmp(a->GetType(), "provider")) mEPGData.provider = a->GetData();
    							if(!strcmp(a->GetType(), "orbital_position")) mEPGData.orbital_position = a->GetData();
    						}
    						eString DisplayList="";
    						mEPGData.start = "";
    						mEPGData.duration = "";
    						mEPGData.Details = "";
    						mEPGData.Description = "";
    						mEPGData.EventStart = "";
    						mEPGData.EventDuration = "";
    						//mEPGData.Genre = "";
    						mEPGData.service = 0;
    						GetEPGData(mEPGData.reference, DisplayList, mEPGData);
    						new eListBoxEntryData(p_list, DisplayList, mEPGData);
    					}
    				}
    			}
            }
    	    Counter++;
        }
#ifdef _E2_ 
    }
#endif
    delete parser;
//  eDebug("Precten eServiceList::read_xml");
}

void eServiceList::GetEPGData(eString sReference, eString &DisplayList, EPGData &mEPGData)
{ 
	eString /*genre,*/ details, description, event_start, event_duration;

	eServiceReference e = string2ref(sReference);
	eServiceReferenceDVB &ref = (eServiceReferenceDVB&)e;

	eEPGCache::getInstance()->Lock();
	
//	const timeMap* evt = eEPGCache::getInstance()->getTimeMap(ref); //ims
	timeMapPtr evt = eEPGCache::getInstance()->getTimeMapPtr((const eServiceReferenceDVB&)ref);  // ims for PLi
	if (evt)
	{ 
		int tsidonid = (ref.getTransportStreamID().get()<<16)|ref.getOriginalNetworkID().get();
		timeMap::const_iterator It;
		for (It = evt->begin(); (It != evt->end() && !description); ++It)
		{
//			EITEvent event(*It->second,tsidonid); //ims 
			EITEvent event(*It->second, tsidonid, It->second->type); // ims for PLi
			time_t now = time(0) + eDVB::getInstance()->time_difference;
			if ((now >= event.start_time) && (now <= event.start_time + event.duration))
			{
			    LocalEventData led;
				
				led.getLocalData(&event, &description, &details);
//				eDebug("### descr %s det %s",description.c_str(),details.c_str());
				tm t = *localtime(&event.start_time);
				event_start = eString().sprintf("%02d:%02d", t.tm_hour, t.tm_min);
				event_duration = eString().sprintf("%d", (int)event.duration / 60);
				mEPGData.start = eString().sprintf("%d", event.start_time );
				mEPGData.duration = eString().sprintf("%d", event.duration);
				mEPGData.Details = details;
				mEPGData.Description = description;
				mEPGData.EventStart = event_start;
				mEPGData.EventDuration = event_duration;
				//mEPGData.Genre = ""; //genre;
			}
		}
	}
	eEPGCache::getInstance()->Unlock();

	eString tmp ;
	if (ref.descr)
		tmp = filter_string(ref.descr);
	else
	{
		//eService *service = iface.addRef(e);
		eService *service=eDVB::getInstance()->settings->getTransponders()->searchService(e);
		if (service)
		{
			tmp = filter_string(service->service_name);
			mEPGData.service = service;
		}
	}
	if (description)
	{   
	    if(description[0]==0x20) // remove space on start ( some providers including space ) 
        {
           eString pom = description.right(description.length() - 1);
           description = pom;
        }

		tmp = tmp + " \t" +  filter_string(description);
	}
	tmp.strReplace("\"", "'");
	tmp.strReplace("\n", "-");
	DisplayList = tmp;
//	eDebug("DisplayList=%s",DisplayList.c_str());
}

void eServiceList::EPG()
{
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{
		if (item->mEPGData.start != "")
		{
			hide();
			eEPGList EPG(item->mEPGData);
			EPG.show();
			EPG.exec();
			EPG.hide();
			CheckForRecording();
			show();
		}
	}
}

int eServiceList::messages(int msg, int time)
{
    eString txt = "";
    int flag = 1;
    int status = 0;
    switch(msg)
    {
        case 0:
            txt = "Can not start recording, because dreambox is already recording!";
            status = eMessageBox::iconError|eMessageBox::btOK;
        break;
        case 1:
            txt = "Start recording with Timer?";
		    status = eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo;
        break;
        case 2:
            txt = "No EPG data ... Do you want recording without Timer? Time of recording is then not limited";
		    status = eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo;
        break;
        case 3:
            txt = "Do you want to stop recording";
		    status = eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo;
        break;
        case 4:
            txt = "Recording ending now ...";
		    status = eMessageBox::iconInfo;
        break;
        case 5:
            txt = "Recording command is sending...";
    	    status = eMessageBox::iconInfo;
        break;
        case 6:
            txt = "Add selected event to Timer?";
		    status = status = eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo;
        break;
        default:
            flag=0;
    }
    if(flag)
    {
        int res = 0;
        eMessageBox m(txt, version, status);
	    m.show(); 
	    if(!time)
	        res = m.exec(); 
	    else
	        sleep(1); 
	    m.hide();
	    return res;
	}
	else
	    return 0;
}

void eServiceList::StopRec()
{

    eString url = "/cgi-bin/videocontrol?command=stop"; 
	SendPartnerBoxRequest request(url);
#ifdef _debug_
	eDebug("StopRec: %s",request.response.c_str());
#endif
	if (!request.result)
	{   
	    messages(4,1);
	}
	else
	    request.dialog(request.error,0);
}

void eServiceList::REC_EVENT()
{
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	int res = 0;
	
	if (item)
	{
	    if (item->mEPGData.start != "") // is EPG data
		{
		    //hide();
		    res = messages(1,0);
		    if (res == eMessageBox::btYes)  // recording event with timer
		    {
		        if (Recording == "0")
			        SetTimer(item->mEPGData);
			    else
			        res = messages(0,0);
			}
			//show();
		}
		else // service has not EPG info
		{
		    res = messages(2,0);
		    if (res == eMessageBox::btYes)  // recording without timer ?
		    {
    	        if (Recording == "0")
    		        Record(item->mEPGData);
    		    else
    		        res = messages(0,0);
       		}
	    }
	}        
	CheckForRecording();
}

void eServiceList::REC()
{
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	
	if (item)
	{
	    // recording immediatelly without timer
	    if (Recording == "0")
		    Record(item->mEPGData);
		else // stop recording now
		{
		    int res = messages(3,0);
		    if (res == eMessageBox::btYes)
		         StopRec();
		}
		CheckForRecording();
	}
}

int eServiceList::eventHandler(const eWidgetEvent & event)
{
    switch (event.type)
    {
        case eWidgetEvent::evtAction:
            if (event.action == &i_shortcutActions->menu)
            {
                //hide();
                eTimerConfig cfg;
				cfg.show();
				cfg.exec();
				cfg.hide();
                //show();
            }
            else
                break;
            return 1;
            
        default:
            break;
    }
    return eWindow::eventHandler(event);
}

eString eServiceList::getActualTime(int type)
{
    time_t tmp = time(0)+eDVB::getInstance()->time_difference;
	tm now = *localtime( &tmp );
    eString time_now;
    
    timer->stop();
    
    switch(type)
    {
        case 0: // hh:mm
            time_now.sprintf("%02d:%02d",now.tm_hour, now.tm_min);
        break;
/*        case 1: // hh:mm.ss
            time_now.sprintf("%02d:%02d.%02d",now.tm_hour, now.tm_min, now.tm_sec);
        break;
        case 2: // dd.mm. hh:mm 
		    time_now.sprintf("%02d.%02d. %02d:%02d", now.tm_mday, now.tm_mon+1, now.tm_hour, now.tm_min);
        break;
        case 3: // dd.mm. hh:mm.ss 
		    time_now.sprintf("%02d.%02d. %02d:%02d.%02d", now.tm_mday, now.tm_mon+1, now.tm_hour, now.tm_min, now.tm_sec);
        break;
        case 4: // dd.mm.yyyy  hh:mm
		    time_now.sprintf("%02d.%02d.%04d  %02d:%02d", now.tm_mday, now.tm_mon+1, now.tm_year+1900, now.tm_hour, now.tm_min);
        break;
*/        case 5: // dd.mm.yyyy  hh:mm.ss
		    time_now.sprintf("%02d.%02d.%04d  %02d:%02d.%02d", now.tm_mday, now.tm_mon+1, now.tm_year+1900, now.tm_hour, now.tm_min, now.tm_sec);
        break;
        default:
            time_now.sprintf("%02d:%02d",now.tm_hour, now.tm_min);
        break;
    } 
    timer->start(1000, true);       
    return time_now;
}

void eServiceList::Record(EPGData nEPGData)
{
    eString Ref = nEPGData.reference;
    
	eString url = "/cgi-bin/zapTo?path=" + Ref;
	SendPartnerBoxRequest request(url);
#ifdef _debug_
	eDebug("Zapp: %s",request.response.c_str());
#endif
	if (!request.result)
	{   
	    messages(5,1);
        url ="/cgi-bin/videocontrol?command=record";
        SendPartnerBoxRequest request(url);
#ifdef _debug_
        eDebug("Record: %s",request.response.c_str());
#endif
	}
	else
	    request.dialog(request.error,0);
}

void eServiceList::SetTimer(EPGData nEPGData)
{
    int start_offset = 0;
    int end_offset = 0;
    eConfig::getInstance()->getKey("/enigma/plugins/remotetimer/offsetstart", start_offset);
	eConfig::getInstance()->getKey("/enigma/plugins/remotetimer/offsetstop", end_offset);

	nEPGData.start = eString().sprintf("%d", atoi(nEPGData.start.c_str())-start_offset*60);
	nEPGData.duration = eString().sprintf("%d",atoi(nEPGData.duration.c_str())+end_offset*60+start_offset*60);
	
	eString sCommand, sHlp;
#ifdef _E2_ 
	if (E2)
	{
		sCommand = "/web/timerchange";
		sHlp = "?sRef=" + nEPGData.reference + "&begin=" + nEPGData.start + "&end=" + nEPGData.duration 
		+ "&name=" + descr + "&description=Partnerbox-Entry&afterevent=0" 
		+ "&eit=0&disabled=0&justplay=0&repeated=0&channelOld=%(channelOld)&beginOld=%(beginOld)&endOld=%(endOld)&"
		+ "eventID%(eventID)&deleteOldOnSave=0";
	}
	else
	{
#endif
	    sCommand = "/addTimerEvent?";
	    sHlp = "ref=" + nEPGData.reference + "&start=" + nEPGData.start + "&duration=" + nEPGData.duration + "&descr=" + nEPGData.Description + "&channel=" + nEPGData.name + "&action=" + Action;
#ifdef _E2_ 
    }
#endif
	sHlp.strReplace(" ", "%20");
	sHlp.strReplace("+", "%2b");
	//>%21%2A%27%28%29%3b%3a%40%26%3d%2b%24%2c%2f%3f%25%23%5b%5d<");

	eString url = sCommand + sHlp; 
	SendPartnerBoxRequest request(url);
	if (!request.result)
	    request.dialog(request.response,1);
	else
	    request.dialog(request.error,0);
	
}

eEPGList::eEPGList(EPGData nEPGData): ePLiWindow(version,390)
{
	cmove(ePoint(x_, y_));
	cresize(eSize(w_, h_));
    setText("EPG - " + nEPGData.name);
    
    addActionMap(&i_shortcutActions->map);
    
	p_list=new eListBox<eListBoxEntryData>(this);
    p_list->move(ePoint(10, 10));
    p_list->resize(eSize(clientrect.width()-20, clientrect.height()-60));
    CONNECT(p_list->selected, eEPGList::p_listselected);
    
	status = new eStatusBar(this);
    status->move( ePoint(0, clientrect.height()-50) );
	status->resize( eSize( clientrect.width(), 50) );
	status->loadDeco();
	
	bt_add=new eButton(this);
    bt_add->move(ePoint(10 , clientrect.height()-40));
    bt_add->resize(eSize(140, 30));
//    bt_add->setProperty("align", "center");
//	bt_add->setProperty("vcenter", "");
//	bt_add->setProperty("backgroundColor", "std_dgreen");
    bt_add->setShortcut("green");
    bt_add->setShortcutPixmap("green");
    bt_add->loadDeco();
    bt_add->setText(_("Add"));
	CONNECT(bt_add->selected, eEPGList::SetWebTimer);

    bt_timer=new eButton(this);
    bt_timer->move(ePoint(clientrect.width()-300, clientrect.height()-40));
    bt_timer->resize(eSize(140, 30));
//    bt_timer->setProperty("align", "center");
//	bt_timer->setProperty("vcenter", "");
//	bt_timer->setProperty("backgroundColor", "std_dblue");
    bt_timer->setShortcut("yellow");
    bt_timer->setShortcutPixmap("yellow");
    bt_timer->loadDeco();
    bt_timer->setText(_("Timer list"));
    CONNECT(bt_timer->selected, eEPGList::ShowTimerList);
	
    bt_info=new eButton(this);
    bt_info->move(ePoint(clientrect.width()-150, clientrect.height()-40));
    bt_info->resize(eSize(140, 30));
//    bt_info->setProperty("align", "center");
//	bt_info->setProperty("vcenter", "");
//	bt_info->setProperty("backgroundColor", "std_dblue");
    bt_info->setShortcut("blue");
    bt_info->setShortcutPixmap("blue");
    bt_info->loadDeco();
    bt_info->setText("Info");
	CONNECT(bt_info->selected, eEPGList::ShowInfo);
    
    GetEPGData(nEPGData.reference);
    setFocus(p_list);
}

void eEPGList::GetEPGData(eString sReference)
{
	eServiceReference e = string2ref(sReference);
	eString /*genre,*/ details, description, event_start, event_duration;
	
	eEPGCache::getInstance()->Lock();
	eServiceReferenceDVB &ref = (eServiceReferenceDVB&)e;
//	const timeMap* evt = eEPGCache::getInstance()->getTimeMap(ref);//ims
	timeMapPtr evt = eEPGCache::getInstance()->getTimeMapPtr((const eServiceReferenceDVB&)ref); // ims for PLi
	if (evt)
	{   
		eService *service=eDVB::getInstance()->settings->getTransponders()->searchService(e);
		if (service)
		{
			eString tmp = filter_string(service->service_name);
			mEPGData.service = service;
			mEPGData.name = tmp;
		}
		int tsidonid = (ref.getTransportStreamID().get()<<16)|ref.getOriginalNetworkID().get();
		timeMap::const_iterator It;
		for (It = evt->begin(); (It != evt->end()) ; ++It)
		{
//  	    EITEvent event(*It->second,tsidonid);
            EITEvent event(*It->second, tsidonid, It->second->type);
			LocalEventData led;
			led.getLocalData(&event, &description, /*&genre,*/ &details);
				
//			eDebug("### descr %s det %s",description.c_str(),details.c_str());
			tm t = *localtime(&event.start_time);
			event_start = eString().sprintf("%02d:%02d", t.tm_hour, t.tm_min);
			event_duration = eString().sprintf("%d", (int)event.duration / 60);
//			eString Datum = eString().sprintf("%02d.%02d.%02d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
            eString Datum = eString().sprintf("%02d.%02d.", t.tm_mday, t.tm_mon + 1);
			
			mEPGData.reference = sReference;
			mEPGData.start = eString().sprintf("%d", event.start_time );
			mEPGData.duration = eString().sprintf("%d", event.duration);
				
			eString Tag;
			if (t.tm_wday == 0)
				Tag = _("Sun");
			if (t.tm_wday == 1)
				Tag = _("Mon");
			if (t.tm_wday == 2)
				Tag = _("Tue");
			if (t.tm_wday == 3)
				Tag = _("Wed");
			if (t.tm_wday == 4)
				Tag = _("Thu");
			if (t.tm_wday == 5)
				Tag = _("Fri");
			if (t.tm_wday == 6)
				Tag = _("Sat");
					
			mEPGData.Details = details;
			mEPGData.Description = description;
			mEPGData.EventStart = event_start;
			mEPGData.EventDuration = event_duration;
			//mEPGData.Eventdate = Tag + ", " + Datum +  ", " + event_start;
			mEPGData.Eventdate = Tag + "  " + Datum +  "  " + event_start;
			//mEPGData.Genre = ""; //genre;
			mEPGData.event = &event;

//			new eListBoxEntryData(p_list, Tag + ", " + Datum +  "  " + event_start + "\t " + description, mEPGData);
			new eListBoxEntryData(p_list, Tag + " " + Datum +  "   " + event_start + "   " + description, mEPGData);
		}
	}
	eEPGCache::getInstance()->Unlock();
}

void eEPGList::ShowInfo()
{
    eString tmp = "";
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{
		hide();
		eString Output;

		if (item->mEPGData.Details)
			Output += item->mEPGData.Details ;
		if (Output == "")
			Output = _("no description available");
		if (item->mEPGData.EventDuration)
		{
		    tmp = eTimerList::getInstance()->Time_from_to(item->mEPGData.start, item->mEPGData.duration);
			Output += "\n\nDuration: " + item->mEPGData.EventDuration + " min" + "   (" + tmp + ")";
		}
			
		eShowEPG showEPGWindow(item->mEPGData.Description, Output, item->mEPGData.Eventdate, item->mEPGData.name, tmp);
		showEPGWindow.show(); showEPGWindow.exec(); showEPGWindow.hide();
		show();
	}
}

void eEPGList::p_listselected(eListBoxEntryData *item)
{
	if (item)
		ShowInfo();
}

void eEPGList::SetWebTimer()
{
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{
	    int res = eServiceList::getInstance()->messages(6,0);
	    if (res == eMessageBox::btYes)
		        eServiceList::getInstance()->SetTimer(item->mEPGData);
	}
}

void eEPGList::ShowTimerList()
{
   hide();
	eString url;
#ifdef _E2_ 
	if (E2)
		url = "/web/timerlist";
	else
#endif
        url = "/xml/timers"; 
    int reload = 1;
	while (reload == 1)
	{
	    SendPartnerBoxRequest request(url);
        if (!request.result)
	    {
		    eTimerList dlg(request.response);
	        dlg.show();
	        dlg.exec();
	        reload = dlg.Reload;
	        dlg.hide();
	    }
	    else
	        request.dialog(request.error,0);
	}
   show();
}

int eEPGList::eventHandler(const eWidgetEvent & event)
{
    switch (event.type)
    {
        case eWidgetEvent::evtAction:
            if (event.action == &i_shortcutActions->menu)
            {
                //hide();
                eTimerConfig cfg;
				cfg.show();
				cfg.exec();
				cfg.hide();
                //show();
            }
            else
                break;
            return 1;
            
        default:
            break;
    }
    return eWindow::eventHandler(event);
}

eTimerList *eTimerList::instance;

eTimerList::eTimerList(eString XMLString): ePLiWindow(version,610)
{
	cmove(ePoint(x_, y_));
	cresize(eSize(w_, h_));
	setText((eString)_("Timer list") + eString().sprintf(" - %s  %s / %s" ,_("connected:"), Box.c_str(), IP.c_str()));

	p_list=new eListBox<eListBoxEntryData>(this);
    p_list->move(ePoint(10, 10));
    p_list->resize(eSize(clientrect.width()-20, clientrect.height()-105));
	CONNECT(p_list->selchanged, eTimerList::p_listselectionChanged);

	status = new eStatusBar(this);
    status->move( ePoint(0, clientrect.height()-95) );
	status->resize( eSize( clientrect.width(), 95) );
	status->loadDeco();

	// Label Field
    lb_selected=new eLabel(this);
    lb_selected->move(ePoint(10, clientrect.height()-90));
    lb_selected->resize(eSize(clientrect.width()-10, 30));
     
	lb_info=new eLabel(this);
    lb_info->move(ePoint(10, clientrect.height()-65));
    lb_info->resize(eSize(clientrect.width()-10, 30));

	// Buttons
	bt_delete=new eButton(this);
    bt_delete->move(ePoint(10, clientrect.height()-40)); 
    bt_delete->resize(eSize(140, 30));
//  bt_delete->setProperty("align", "center");
//	bt_delete->setProperty("vcenter", "");
//	bt_delete->setProperty("backgroundColor", "std_dred");
    bt_delete->setShortcut("red");
    bt_delete->setShortcutPixmap("red");
    bt_delete->loadDeco();
    bt_delete->setText(_("remove"));
    CONNECT(bt_delete->selected, eTimerList::Delete);

	bt_erase=new eButton(this);
    bt_erase->move(ePoint(clientrect.width()-300, clientrect.height()-40));
    bt_erase->resize(eSize(140, 30));
//  bt_erase->setProperty("align", "center");
//	bt_erase->setProperty("vcenter", "");
//	bt_erase->setProperty("backgroundColor", "std_dyellow");
    bt_erase->setShortcut("yellow");
    bt_erase->setShortcutPixmap("yellow");
    bt_erase->loadDeco();
    bt_erase->setText("Delete all");
    CONNECT(bt_erase->selected, eTimerList::ClearList);

	bt_clean=new eButton(this);
    bt_clean->move(ePoint(clientrect.width()-150, clientrect.height()-40));
    bt_clean->resize(eSize(140, 30));
//  bt_clean->setProperty("align", "center");
//	bt_clean->setProperty("vcenter", "");
//	bt_clean->setProperty("backgroundColor", "std_dblue");
    bt_clean->setShortcut("blue");
    bt_clean->setShortcutPixmap("blue");
    bt_clean->loadDeco();
    bt_clean->setText(_("cleanup"));
	CONNECT(bt_clean->selected, eTimerList::CleanUp);
	
	setFocus(p_list);
	
	read_xml(XMLString);
	ShowDetails();
	
	Reload = 0;
}

void eTimerList::ShowDetails()
{

	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{
	    eString tmp = Time_from_to(item->mTimerData.start, item->mTimerData.duration);
		
		eString ShowName = item->mTimerData.date + "  " + tmp + "    " + _("status:")+ "  " + item->mTimerData.status;
		URLDecode(ShowName);
		lb_selected->setText(ShowName);
		
		eString Text = item->mTimerData.name + ":  " + item->mTimerData.description;
		URLDecode(Text);
		lb_info->setText(Text);
	}
}

void eTimerList::Delete()
{
	eListBoxEntryData *item;
	item = p_list->getCurrent();
	if (item)
	{   // "Really delete this event?"
		eString text = (eString)_("Do you really want to delete this timer event: \n\n") + item->mTimerData.name + ": " + item->mTimerData.description+ "?\n";
		URLDecode(text);
		eMessageBox msg(text, version, eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
		msg.show(); 
		int Answer = msg.exec(); 
		msg.hide();
		if (Answer == eMessageBox::btYes)
		{
		    eString url;
#ifdef _E2_ 
		    if(E2)
		    {
		        url = "/web/timerdelete?sRef=" + item->mTimerData.reference + "&begin=" +item->mTimerData.start + "&end=" +eString().sprintf("%s",atoi(item->mTimerData.start.c_str())+atoi(item->mTimerData.duration.c_str())); /*TODO*/ //dodelat pouziti offsetu 
		    }
		    else
#endif
		    	url = "/deleteTimerEvent?ref=" + item->mTimerData.reference + "&start=" + item->mTimerData.start + "&type=44&force=yes"; 
			
			SendPartnerBoxRequest request(url);
#ifdef _debug_		
			eDebug("Delete: %d",request.response.c_str());
#endif			
			if (!request.result)
			{
				Reload = 1; eWidget::accept();	
			}
			else
			    request.dialog(request.error,0);
        }
	}
}

void eTimerList::CleanUp()
{
    SendPartnerBoxRequest request("/cleanupTimerList");
#ifdef _debug_
    eDebug("Cleanup: %s",request.response.c_str());
#endif    
	if (!request.result)
	{
        Reload = 1; eWidget::accept();
	}
	else
	    request.dialog(request.error,0);
}

void eTimerList::ClearList()
{
	eString text = _("Do you really want to delete all timer events?");
	eMessageBox msg(text, version, eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
	msg.show(); 
	int res = msg.exec(); 
	msg.hide();
	if (res == eMessageBox::btYes)
	{
		eString url = "/clearTimerList"; 
		SendPartnerBoxRequest request(url);
#ifdef _debug_	
		eDebug("ClearList: %s",request.response.c_str());
#endif		
		if (!request.result)
		{
			Reload = 1; eWidget::accept();
		}
		else
			request.dialog(request.error,0);
	}
}

void eTimerList::p_listselectionChanged(eListBoxEntryData *item)
{
	if (item)
		ShowDetails();
}

void eTimerList::read_xml(eString xmlString)
{
//    eDebug("read_xml %s",xmlString.c_str());

    XMLTreeParser * parser;
	parser = new XMLTreeParser("UTF-8");
	if ( ! parser->Parse( xmlString.c_str(), xmlString.length(), 1 ) )
    {
        eMessageBox msg("XML Error!\n Ref-Err-Code: 0006", version, eMessageBox::iconError|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
        delete parser;
        parser = NULL;
        return;
    }

	XMLTreeNode * root = parser->RootNode();
    if(!root)
    {
        eMessageBox msg("XML Error!\n Ref-Err-Code: 0005", version, eMessageBox::iconError|eMessageBox::btOK);
	    msg.show(); msg.exec(); msg.hide();
        return;
    }
#ifdef _E2_     
    if (E2)
	{
		for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
		{
			if(!strcmp(node->GetType(), "e2timer"))
			{
				TimerData mTimerData;
				for(XMLTreeNode * i = node->GetChild(); i; i = i->GetNext())
				{
					if(!strcmp(i->GetType(), "e2servicereference")) mTimerData.reference = i->GetData();
					if(!strcmp(i->GetType(), "e2servicename")) mTimerData.name = i->GetData();
					if(!strcmp(i->GetType(), "e2name")) mTimerData.description = i->GetData();
					if(!strcmp(i->GetType(), "e2disabled")) mTimerData.disabled = i->GetData();
					if(!strcmp(i->GetType(), "e2timebegin")) mTimerData.start = i->GetData();
					if(!strcmp(i->GetType(), "e2timeend")) mTimerData.timeend = i->GetData();
					if(!strcmp(i->GetType(), "e2duration")) mTimerData.duration = i->GetData();
					if(!strcmp(i->GetType(), "e2state")) mTimerData.state = i->GetData();

					if(!strcmp(i->GetType(), "e2repeated"))
					{
					    mTimerData.repeated = i->GetData();
					    eString ShowName = mTimerData.state + "  >" + mTimerData.start /*mTimerData.date.left(mTimerData.date.length()-4)*/ + "   " 
            			                 + Time_from_to(mTimerData.start,mTimerData.duration) 
            			                 + "   " + mTimerData.name + "  " + mTimerData.description;
						
						URLDecode(ShowName);
	    				new eListBoxEntryData(p_list, ShowName , mTimerData);
					}
				}
			}
		}
	}
	else
	{
#endif
	    for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
        {
		    if(!strcmp(node->GetType(), "timer"))
		    {
//			    TimerData mTimerData;
			    for(XMLTreeNode * i = node->GetChild(); i; i = i->GetNext())
			    {
				    if(!strcmp(i->GetType(), "type")) mTimerData.type = i->GetData();
				    if(!strcmp(i->GetType(), "action")) mTimerData.action = i->GetData();
				    if(!strcmp(i->GetType(), "status")) mTimerData.status = i->GetData();
				    if(!strcmp(i->GetType(), "typedata")) mTimerData.typedata = i->GetData();
				    if(!strcmp(i->GetType(), "service"))
				    {
					    for(XMLTreeNode * a = i->GetChild(); a; a = a->GetNext())
					    {
					    	if(!strcmp(a->GetType(), "reference")) mTimerData.reference = a->GetData();
					    	if(!strcmp(a->GetType(), "name")) mTimerData.name = a->GetData();
					    }
				    } 
    				if(!strcmp(i->GetType(), "event"))
	    			{
	    				for(XMLTreeNode * a = i->GetChild(); a; a = a->GetNext())
	    				{
	    					if(!strcmp(a->GetType(), "date")) mTimerData.date = a->GetData();
	    					if(!strcmp(a->GetType(), "time")) mTimerData.time = a->GetData();
	    					if(!strcmp(a->GetType(), "start")) mTimerData.start = a->GetData();
	    					if(!strcmp(a->GetType(), "duration")) mTimerData.duration = a->GetData();
	    					if(!strcmp(a->GetType(), "description")) mTimerData.description = a->GetData();
	    				}
					
                        eString status = mTimerData.status;
                        if(mTimerData.status == "FINISHED" )
                        {
                    
                        }
                        else if (mTimerData.status == "ACTIVE" )
                        {
                            status+="  ";
                        }
                        else if (mTimerData.status == "ERROR" )
                        {
                            status+="   ";
                        }
                    
   	    				eString ShowName = status + "  " + mTimerData.date.left(mTimerData.date.length()-4) + "   " 
            			                 + Time_from_to(mTimerData.start,mTimerData.duration) 
            			                 + "   " + mTimerData.name + "  " + mTimerData.description;

            			URLDecode(ShowName);//eDebug("read_xml %s",ShowName.c_str());
	    				new eListBoxEntryData(p_list, ShowName , mTimerData);
	    			}
	    		}
			}
        }
#ifdef _E2_ 
    }
#endif
    delete parser;
    parser = NULL;
    ShowDetails();
}

eString eTimerList::Time_from_to( eString &start, eString &duration )
{
    time_t tmp1 = (const time_t)atoi(start.c_str()); 
    time_t tmp2 = (const time_t)atoi(duration.c_str());
    tm start_time = *localtime(&tmp1);
    time_t t = tmp1 + tmp2;
    tm stop_time = *localtime(&t);
    eString tmp = getTimeStr(&start_time, 0) + "-" + getTimeStr(&stop_time, 0);
    return tmp;
}

void eTimerList::URLDecode(eString &encodedString)
{
	char *newString=NULL;
	const char *string = encodedString.c_str();
	int count=0;
	char hex[3]={'\0'};
	unsigned long iStr;

	count = 0;
	if((newString = (char *)malloc(sizeof(char) * strlen(string) + 1) ) != NULL)
	{

	/* copy the new string with the values decoded */
		while(string[count]) /* use the null character as a loop terminator */
		{
			if (string[count] == '%')
			{
				hex[0]=string[count+1];
				hex[1]=string[count+2];
				hex[2]='\0';
				iStr = strtoul(hex,NULL,16); /* convert to Hex char */
				newString[count]=(char)iStr;
				count++;
				string = string + 2; /* need to reset the pointer so that we don't write hex out */
			}
			else
			{
				if (string[count] == '+') 
					newString[count] = ' ';
				else
					newString[count] = string[count];
				count++;
			}
		} /* end of while loop */

		newString[count]='\0'; /* when done copying the string,need to terminate w/ null char */
	}
	else
	{
		return;
	}
	encodedString = newString;
	free(newString);
}

eShowEPG::eShowEPG(eString Description, eString ShowEPGData, eString Eventdate, eString ServiceName, eString Temp): eWindow(1)
{
    cmove(ePoint(100, 120));
    cresize(eSize(540, 340));
    setText(Description);
   
    status = new eStatusBar(this);
    status->move( ePoint(0, clientrect.height()-40) );
    status->resize( eSize( clientrect.width(), 40) );
    status->loadDeco();

    scrollbar = new eProgress(this);
    scrollbar->setName("scrollbar");
    scrollbar->setStart(0);
    scrollbar->setPerc(100);
    scrollbar->move(ePoint(width() - 30, 5));
    scrollbar->resize(eSize(20, height() - 100));
    scrollbar->setProperty("direction", "1");

    visible = new eWidget(this);
    visible->setName("visible");
    visible->move(ePoint(10, 5));
    visible->resize(eSize(width() - 40, height() - 100));

    label = new eLabel(visible);
    label->setFlags(RS_WRAP);
    label->setFont(eSkin::getActive()->queryFont("eEPGSelector.Entry.Description"));
    float lineheight = fontRenderClass::getInstance()->getLineHeight(label->getFont());
    int lines = (int) (visible->getSize().height() / lineheight);
    pageHeight = (int) (lines * lineheight);
    visible->resize(eSize(visible->getSize().width(), pageHeight + (int) (lineheight / 6)));
    label->resize(eSize(visible->getSize().width(), pageHeight * 16));

    label->hide();
    label->move(ePoint(0, 0));
    label->setText(ShowEPGData);
   
    lb_date=new eLabel(this);
    lb_date->move(ePoint(10, clientrect.height()-30));
    lb_date->resize(eSize(320, 30)); 
    
    eString ddmm = Eventdate.left(Eventdate.length() - 5);
    lb_date->setText(ddmm + "(" + Temp + ")");
    
    lb_channel=new eLabel(this);
    lb_channel->setAlign(eTextPara::dirRight);
    lb_channel->move(ePoint(clientrect.width()-210, clientrect.height()-30));
    lb_channel->resize(eSize(200, 30));
    lb_channel->setText(ServiceName);
    
    updateScrollbar();
    label->show();
}

int eShowEPG::eventHandler(const eWidgetEvent & event)
{
    switch (event.type)
    {
        case eWidgetEvent::evtAction:
            if (total && event.action == &i_cursorActions->up)
            {
                ePoint curPos = label->getPosition();
                if (curPos.y() < 0)
                {
                    label->move(ePoint(curPos.x(), curPos.y() + pageHeight));
                    updateScrollbar();
                }
            }
            else if (total && event.action == &i_cursorActions->down)
            {
                ePoint curPos = label->getPosition();
                if ((total - pageHeight) >= abs(curPos.y() - pageHeight))
                {
                    label->move(ePoint(curPos.x(), curPos.y() - pageHeight));
                    updateScrollbar();
                }
            }
            else if (event.action == &i_cursorActions->cancel)
                close(0);
            else
                break;
            return 1;
            
        default:
            break;
    }
    return eWindow::eventHandler(event);
}

void eShowEPG::updateScrollbar()
{
    total = pageHeight;
    int pages = 1;
    while (total < label->getExtend().height())
    {
        total += pageHeight;
        pages++;
    }

    int start = -label->getPosition().y() * 100 / total;
    int vis = pageHeight * 100 / total;
    scrollbar->setParams(start, vis);
    scrollbar->show();
    if (pages == 1)
        total = 0;
}


eTimerConfig::eTimerConfig(): ePLiWindow(_("Options"), 400)  // Config window
{
	iTimerOffsetStart = 0;
	eConfig::getInstance()->getKey("/enigma/plugins/remotetimer/offsetstart", iTimerOffsetStart);

	iTimerOffsetStop = 0;
	eConfig::getInstance()->getKey("/enigma/plugins/remotetimer/offsetstop", iTimerOffsetStop); 

    eLabel* timerOffsetStart_label = new eLabel(this);
	timerOffsetStart_label->setText(_("Timer start offset"));
	timerOffsetStart_label->move(ePoint(10, yPos()));
	timerOffsetStart_label->resize(eSize(250, widgetHeight()));

	timerOffsetStart = new eNumber(this, 1, 0, 60, 2, &iTimerOffsetStart, 0, timerOffsetStart_label);
	timerOffsetStart->move(ePoint(340, yPos()));
	timerOffsetStart->resize(eSize(50, widgetHeight()));
	timerOffsetStart->setHelpText(_("Time in minutes a timer event starts earlier (0-60)"));
	timerOffsetStart->loadDeco();

	nextYPos(35);

	eLabel* timerOffsetStop_label = new eLabel(this);
	timerOffsetStop_label->setText(_("Timer stop offset"));
	timerOffsetStop_label->move(ePoint(10, yPos()));
	timerOffsetStop_label->resize(eSize(250, widgetHeight()));

	timerOffsetStop = new eNumber(this, 1, 0, 60, 2, &iTimerOffsetStop, 0, timerOffsetStop_label);
	timerOffsetStop->move(ePoint(340, yPos()));
	timerOffsetStop->resize(eSize(50, widgetHeight()));
	timerOffsetStop->setHelpText(_("Time in minutes a timer event finishes later (0-60)"));
	timerOffsetStop->loadDeco();

    nextYPos(35);
    nextYPos(35);
    
    eString Info = (eString)_("Free Recordspace:") + "  " + _("n/a");
	
    eString url;
#ifdef _E2_ 
    if(E2)
		url = "/web/about";
	else
#endif
	    url ="/data";
	SendPartnerBoxRequest request(url);
	if (!request.result)
	    Info = (eString)_("Free Recordspace:") + "  " + request.getBoxParInfo(request.response,"diskGB",1) 
                                               + "  " + request.getBoxParInfo(request.response,"diskH",1);
	else
	    request.dialog(request.error,0);

    eLabel* lb_hddSize = new eLabel(this);
	lb_hddSize->setText(Info);
	lb_hddSize->move(ePoint(10, yPos()));
	lb_hddSize->resize(eSize(380, widgetHeight()));

	buildWindow();
	CONNECT (bOK->selected, eTimerConfig::okPressed);
}

void eTimerConfig::okPressed()
{
	eConfig::getInstance()->setKey("/enigma/plugins/remotetimer/offsetstart", timerOffsetStart->getNumber());
	eConfig::getInstance()->setKey("/enigma/plugins/remotetimer/offsetstop", timerOffsetStop->getNumber());
	close(0);
}


SendPartnerBoxRequest::SendPartnerBoxRequest(eString url)
{
    result = (int)sendGetRequest("http://"+ IP + url , response, Login, Password);
}
/*
void SendPartnerBoxRequest::PartnerBoxGetRequest(eString url)
{
	result = (int)sendGetRequest("http://"+ IP + url , response, Login, Password);
	eDebug("### calling PartnerBoxGetRequest result = %d",result);
}
*/
static int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
{
	int result = 0;
	if (buffer != NULL)
	{
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

CURLcode SendPartnerBoxRequest::sendGetRequest (const eString& url, eString& response, eString User, eString Password)
{
	CURL *curl;
	CURLcode httpres;
	eString auth;
	auth = User + ":" + Password;
	error = "";
	response="";	
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)&response);
	curl_easy_setopt (curl, CURLOPT_USERPWD, auth.c_str());	
	curl_easy_setopt (curl, CURLOPT_FAILONERROR, true);
	int intTimeout = 10;
	eConfig::getInstance()->getKey("/enigma/partnerbox/timeout", intTimeout);
	curl_easy_setopt (curl, CURLOPT_TIMEOUT, intTimeout);
	httpres = curl_easy_perform (curl);
#ifdef _debug_
    eDebug("*** CURLcode response: %s\n",response.c_str());
#endif    
	if (httpres != CURLE_OK) // Error --> welcher denn?
	{
		CURLcode httpcode; 
		curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &httpcode);
		error="Unknown error";
		if ((int)httpcode == 0)
		{
			switch ((int)httpres)
			{
				case 6: error="The given remote host was not resolved."; break;
				case 7: error="Failed to connect to host."; break;
				case 22: error="Host not found!"; break;
				case 28: error="The specified time-out period was reached according to the conditions."; break;
			}
		}
		else	
		{
			switch ((int)httpcode)
			{
				case 400: error="Bad Request"; break;
				case 401: error="Unauthorized"; break;
				case 403: error="Forbidden"; break;
				case 404: error="Not found"; break;
				case 405: error="Method not allowed"; break;
				case 500: error="Internal server error"; break;		
			}

		}
	}
	curl_easy_cleanup(curl);
	return httpres;
}

void SendPartnerBoxRequest::dialog(eString message, int ok)
{
        eMessageBox msg(message, version, ok?eMessageBox::iconInfo|eMessageBox::btOK:eMessageBox::iconError|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
}

eString SendPartnerBoxRequest::getBoxParInfo(eString buf, const char* param, int removeSpace)
{
    eString par = "";
    eString temp = "var " + (eString)param + " = ";
    
    const char *parameter = temp.c_str();

	unsigned int start = 0;
	for (unsigned int pos = buf.find('\n', 0); pos != std::string::npos; pos = buf.find('\n', start))
	{
		eString entry = ""; entry = buf.substr(start, pos - start); // one line
		if(entry.find(parameter) != eString::npos)
		{
			par = entry.strReplace(parameter,"");
			par = par.left(par.find_last_of(';')).strReplace("\t","").strReplace("\"","");
			if(removeSpace)
    			par = par.strReplace(" ","");
			break;
		}
		start = pos + 1;
	}
#ifdef _debug_	
	eDebug("[getPar] parameter: %s par: %s",parameter,par.c_str());
#endif
	return par;
}

