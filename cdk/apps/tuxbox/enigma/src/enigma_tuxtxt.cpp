/*
   Embedded version of the tuxtxt plugin by LazyT
*/


#include "enigma_tuxtxt.h"
#include <enigma.h>
#include <lib/dvb/decoder.h>
#include <lib/gui/actions.h>
#include <lib/gui/emessage.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/slider.h>
#include <lib/gui/numberactions.h>
#include <lib/gui/ePLiWindow.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <setup_window.h>
#include <lib/gdi/gfbdc.h>
#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define TUXTXT_TIMER_TICK 100
#define TUXTXTTTF FONTDIR "/tuxtxt.ttf"
#define TUXTXTTTFVAR "/var/tuxtxt/tuxtxt.ttf"

struct tuxtxtActions
{
	eActionMap map;
	eAction switchZoommode, switchScreenmode, switchTranspmode, switchHintmode;
	tuxtxtActions():
		map("Tuxtxt", _("enigma tuxtxt")),
		switchZoommode(map, "switchZoommode", _("toggle double height"), eAction::prioDialog),
		switchScreenmode(map, "switchScreenmode", _("toggle splitscreen mode"), eAction::prioDialog),
		switchTranspmode(map, "switchTranspmode", _("toggle transparency"), eAction::prioDialog),
		switchHintmode(map, "switchHintmode", _("toggle hidden information"), eAction::prioDialog)
	{
	}
};

eAutoInitP0<tuxtxtActions> i_tuxtxtActions(eAutoInitNumbers::actions, "tuxtxt actions");

class eTuxtxtSetup: public ePLiWindow
{
	tstRenderInfo* renderinfo;
	eComboBox* favourites;
	eButton* removepage;
	eNumber* pagenumber;
	eButton* addpage;
	eComboBox* language;
	eCheckbox* normal169;
	eCheckbox* split169;
	eSlider* sBrightness;
	eSlider* sTransparency;
	eCheckbox* truetype;
	eComboBox* service;
	eStatusBar*  sbar;
	std::vector<int>* hotlist;


	void removepagePressed();
	void addpagePressed();
	void ServiceSelected();
	int GetTeletextPIDs();
	void FillServiceCombo(eListBox<eListBoxEntryText>* lb);
	int GetNationalSubset(char *cc);
	void savehotlist();
	eListBoxEntryText* addhotlistpage(int page);
	void init_eTuxtxtSetup();
	int eventHandler(const eWidgetEvent &event);
	struct _pid_table
	{
		int  vtxt_pid;
		int  service_id;
		int  service_name_len;
		char service_name[24];
		int  national_subset;
	}pid_table[128];
	int pids_found;
	int hotlistchanged;
public:
	eTuxtxtSetup(tstRenderInfo* renderinfo,std::vector<int>* hotlist);
	~eTuxtxtSetup();
};
eTuxtxtSetup::eTuxtxtSetup(tstRenderInfo* renderinfo,std::vector<int>* hotlist)
	:ePLiWindow(_("TuxTxt setup"), 530),renderinfo(renderinfo),hotlist(hotlist),pids_found(0),hotlistchanged(0)
{
	init_eTuxtxtSetup();
}
eTuxtxtSetup::~eTuxtxtSetup()
{
}

void eTuxtxtSetup::init_eTuxtxtSetup()
{
	memset(pid_table,0,128*sizeof(struct _pid_table));
	favourites=new eComboBox(this,5);favourites->setName("favourites");
	for (std::vector<int>::iterator x(hotlist->begin()); x != hotlist->end(); ++x)
	{
		addhotlistpage(*x);
	}

	removepage=new eButton(this); removepage->setName("removepage");

	pagenumber=new eNumber(this,1,1, 899, 3, 0, 0); pagenumber->setName("pagenumber");
	pagenumber->setNumber((tuxtxt_cache.page & 0xf) +(tuxtxt_cache.page>>4 & 0xf)*10 + (tuxtxt_cache.page>>8 & 0x0f)*100);

	addpage=new eButton(this); addpage->setName("addpage");

	service=new eComboBox(this,5); service->setName("service");
	service->setCurrent(new eListBoxEntryText( *service,_("search"), (void*) -1 ));

	normal169=new eCheckbox(this);normal169->setName("normal169");
	normal169->setCheck((unsigned char)renderinfo->screen_mode1);

	split169=new eCheckbox(this);split169->setName("split169");
	split169->setCheck((unsigned char)renderinfo->screen_mode2);

	sBrightness = new eSlider( this, 0, 1, 24 );sBrightness->setName("sBrightness");

	sTransparency = new eSlider( this, 0, 1, 24 );sTransparency->setName("sTransparency");


	language=new eComboBox(this,5); language->setName("language");
	new eListBoxEntryText( *language,_("automatic"), (void*) 0 );
	new eListBoxEntryText( *language,_("Czech/Slovak"), (void*) 1 );
	new eListBoxEntryText( *language,_("English"), (void*) 2 );
	new eListBoxEntryText( *language,_("Estonian"), (void*) 3 );
	new eListBoxEntryText( *language,_("French"), (void*) 4 );
	new eListBoxEntryText( *language,_("German"), (void*) 5 );
	new eListBoxEntryText( *language,_("Italian"), (void*) 6 );
	new eListBoxEntryText( *language,_("Latvian/Lithuanian"), (void*) 7 );
	new eListBoxEntryText( *language,_("Polish"), (void*) 8 );
	new eListBoxEntryText( *language,_("Portuguese/Spanish"), (void*) 9 );
	new eListBoxEntryText( *language,_("Romanian"), (void*)10 );
	new eListBoxEntryText( *language,_("Serbian/Croatian/Slovenian"), (void*)11 );
	new eListBoxEntryText( *language,_("Swedish/Finnish/Hungarian"), (void*)12 );
	new eListBoxEntryText( *language,_("Turkish"), (void*)13 );
	new eListBoxEntryText( *language,_("Serbian/Croatian"), (void*)14 );
	new eListBoxEntryText( *language,_("Russian/Bulgarian"), (void*)15 );
	new eListBoxEntryText( *language,_("Ukranian"), (void*)16 );
	new eListBoxEntryText( *language,_("Greek"), (void*)17 );
	new eListBoxEntryText( *language,_("Hebrew"), (void*)18 );
	new eListBoxEntryText( *language,_("Arabic"), (void*)19 );

	truetype=new eCheckbox(this);truetype->setName("truetype");
	truetype->setCheck((unsigned char)renderinfo->usettf);
	// if ttf font does not exist hide the checkbox
	struct stat64 s;
	if (stat64(TUXTXTTTF, &s)<0 &&
			stat64(TUXTXTTTFVAR, &s)<0)
	{
		truetype->hide();
	}

	sbar = new eStatusBar(this); sbar->setName("statusbar");

	if (eSkin::getActive()->build(this, "SetupTuxtxt"))
		eFatal("skin load of \"SetupTuxtxt\" failed");

	language->setCurrent((void*)(renderinfo->auto_national ? 0 : tuxtxt_cache.national_subset));
	favourites->setCurrent(0);
	sBrightness->setValue( renderinfo->color_mode);
	sTransparency->setValue( renderinfo->trans_mode);

	CONNECT(removepage->selected, eTuxtxtSetup::removepagePressed);
	CONNECT(addpage->selected, eTuxtxtSetup::addpagePressed);
	CONNECT(service->selected, eTuxtxtSetup::ServiceSelected);
}


void eTuxtxtSetup::savehotlist()
{
	FILE *hl;
	char line[100];
	int i;

	hotlistchanged = 0;
	sprintf(line, CONFIGDIR "/tuxtxt/hotlist%d.conf", tuxtxt_cache.vtxtpid);
#if TUXTXT_DEBUG
	printf("TuxTxt <savehotlist %s", line);
#endif
	int maxhotlist = hotlist->size();
	if (maxhotlist != 2 || (*hotlist)[0] != 0x100 || (*hotlist)[1] != 0x303)
	{
		if ((hl = fopen(line, "wb")) != 0)
		{
			for (i = 0; i < maxhotlist; i++)
			{
				fprintf(hl, "%03x\n", (*hotlist)[i]);
#if TUXTXT_DEBUG
				printf(" %03x", (*hotlist)[i]);
#endif
			}
			fclose(hl);
		}
	}
	else
	{
		unlink(line); /* remove current hotlist file */
#if TUXTXT_DEBUG
		printf(" (default - just deleted)");
#endif
	}
#if TUXTXT_DEBUG
	printf(">\n");
#endif
}

void eTuxtxtSetup::removepagePressed()
{
	favourites->removeEntry(favourites->getCurrent());
	favourites->setCurrent(0);
	hotlistchanged = 1;
}
void eTuxtxtSetup::addpagePressed()
{
	int page = pagenumber->getNumber();
	page = (((page/100) &0xf)<<8)+((((page%100)/10) &0xf)<<4) + ((page%10) &0xf);
	favourites->setCurrent(addhotlistpage(page));
	hotlist->push_back(page);
	hotlistchanged = 1;
}
eListBoxEntryText* eTuxtxtSetup::addhotlistpage(int page)
{
	eString pg = eString().sprintf("%d%d%d",(page>>8) & 0xf,(page>>4) & 0xf,page & 0xf);
	return new eListBoxEntryText( *favourites,pg, (void*) page );
}

int eTuxtxtSetup::eventHandler(const eWidgetEvent &event)
{
	if  (event.type == eWidgetEvent::wantClose)
	{
		renderinfo->color_mode =sBrightness->getValue()+1;
		eConfig::getInstance()->setKey("/ezap/teletext/Brightness",  renderinfo->color_mode);
		renderinfo->trans_mode =sTransparency->getValue()+1;
		eConfig::getInstance()->setKey("/ezap/teletext/Transparency",  renderinfo->trans_mode);
	
		renderinfo->screen_mode1 = normal169->isChecked();
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Normal", renderinfo->screen_mode1 );
		renderinfo->screen_mode2 = split169->isChecked();
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Divided", renderinfo->screen_mode2 );
		renderinfo->usettf = truetype->isChecked();
		eConfig::getInstance()->setKey("/ezap/teletext/UseTTF", renderinfo->usettf );
	
		tuxtxt_cache.national_subset =(int)language->getCurrent()->getKey();
		renderinfo->auto_national = (tuxtxt_cache.national_subset ? 0 : 1);
		eConfig::getInstance()->setKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset);
		eConfig::getInstance()->setKey("/ezap/teletext/AutoNational", renderinfo->auto_national);
		if (hotlistchanged)
			savehotlist();
	}
	return ePLiWindow::eventHandler(event);
}
void eTuxtxtSetup::FillServiceCombo(eListBox<eListBoxEntryText>* lb)
{
	lb->beginAtomic();
	service->removeEntry((void*)-1);
	for (int i = 0; i < pids_found; i++)
	{
		eString s = pid_table[i].service_name;
		new eListBoxEntryText( lb,convertLatin1UTF8(s), (void*) i );
	}
	if (pids_found ==  0)
		new eListBoxEntryText( lb,_("no services on this transponder"), (void*) -2 );
	
	lb->endAtomic();
	service->setCurrent(0);
}
void eTuxtxtSetup::ServiceSelected()
{
	int nr = (int)service->getCurrent()->getKey();
	if (nr == -2)
		return;// no services on transponder
	if (nr == -1)
	{
		GetTeletextPIDs();
		FillServiceCombo(*service);
	}
	else
	{
		if (tuxtxt_cache.vtxtpid != pid_table[nr].vtxt_pid)
		{
#if TUXTXT_CFG_STANDALONE
			tuxtxt_stop_thread();
			tuxtxt_clear_cache();
#else
			tuxtxt_stop();
#endif
			/* reset data */
			renderinfo->inputcounter = 2;


			tuxtxt_cache.page     = 0x100;
			renderinfo->prev_100 = 0x100;
			renderinfo->prev_10  = 0x100;
			renderinfo->next_100 = 0x100;
			renderinfo->next_10  = 0x100;
			tuxtxt_cache.subpage  = 0;

			tuxtxt_cache.pageupdate = 0;
			tuxtxt_cache.zap_subpage_manual = 0;
			renderinfo->hintmode = 0;
			memset(renderinfo->page_char,' ',40 * 25);

			if (renderinfo->auto_national)
				tuxtxt_cache.national_subset = pid_table[nr].national_subset;

#if TUXTXT_CFG_STANDALONE
			tuxtxt_cache.vtxtpid = pid_table[current_pid].vtxt_pid;
			tuxtxt_start_thread();
#else
			tuxtxt_start(pid_table[nr].vtxt_pid);
#endif
		}
		close(1);
	}
}
int eTuxtxtSetup::GetTeletextPIDs()
{
	struct dmx_sct_filter_params dmx_flt;
	int pat_scan, pmt_scan, sdt_scan, desc_scan, pid_test, byte, diff, first_sdt_sec;

	unsigned char PAT[1024];
	unsigned char SDT[1024];
	unsigned char PMT[1024];
	int dmx;


	/* open demuxer */
	if ((dmx = open(DMX, O_RDWR)) == -1)
	{
		perror("TuxTxt <open DMX>");
		return 0;
	}
	if (ioctl(dmx, DMX_SET_BUFFER_SIZE, 64*1024) < 0)
	{
		perror("Tuxtxt <DMX_SET_BUFFERSIZE>");
		::close(dmx);
		return 0;
	}

	eMessageBox box(_("Searching for Teletext services on this Transponder"), _("Search"), eMessageBox::iconInfo );
	box.show();

	/* read PAT to get all PMT's */
#if HAVE_DVB_API_VERSION < 3
	memset(dmx_flt.filter.filter, 0, DMX_FILTER_SIZE);
	memset(dmx_flt.filter.mask, 0, DMX_FILTER_SIZE);
#else
	memset(&dmx_flt.filter, 0x00, sizeof(struct dmx_filter));
#endif

	dmx_flt.pid              = 0x0000;
	dmx_flt.flags            = DMX_ONESHOT | DMX_CHECK_CRC | DMX_IMMEDIATE_START;
	dmx_flt.filter.filter[0] = 0x00;
	dmx_flt.filter.mask[0]   = 0xFF;
	dmx_flt.timeout          = 5000;


	if (ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
	{
		perror("TuxTxt <DMX_SET_FILTER PAT>");
		ioctl(dmx, DMX_STOP);
		::close(dmx);
		return 0;
	}

	if (read(dmx, PAT, sizeof(PAT)) == -1)
	{
		perror("TuxTxt <read PAT>");
		ioctl(dmx, DMX_STOP);
		::close(dmx);
		return 0;
	}

	/* scan each PMT for vtxt-pid */
	pids_found = 0;

	for (pat_scan = 0x0A; pat_scan < 0x0A + (((PAT[0x01]<<8 | PAT[0x02]) & 0x0FFF) - 9); pat_scan += 4)
	{
#if TUXTXT_DEBUG
		printf("PAT liefert:%04x, %04x \n",((PAT[pat_scan - 2]<<8) | (PAT[pat_scan - 1])),(PAT[pat_scan]<<8 | PAT[pat_scan+1]) & 0x1FFF);
#endif
		if (((PAT[pat_scan - 2]<<8) | (PAT[pat_scan - 1])) == 0)
			continue;
// workaround for Dreambox PMT "Connection timed out"-problem (not very nice, but it works...)
#ifdef HAVE_DREAMBOX_HARDWARE
		ioctl(dmx, DMX_STOP);
		::close(dmx);
		if ((dmx = open(DMX, O_RDWR)) == -1)
		{
			perror("TuxTxt <open DMX>");
			return 0;
		}
#endif
		dmx_flt.pid               = (PAT[pat_scan]<<8 | PAT[pat_scan+1]) & 0x1FFF;
		dmx_flt.flags             = DMX_ONESHOT | DMX_CHECK_CRC | DMX_IMMEDIATE_START;
		dmx_flt.filter.filter[0]  = 0x02;
		dmx_flt.filter.mask[0]    = 0xFF;
		dmx_flt.timeout           = 5000;

		if (ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
		{
			perror("TuxTxt <DMX_SET_FILTER PMT>");
			continue;
		}

		if (read(dmx, PMT, sizeof(PMT)) == -1)
		{
			perror("TuxTxt <read PMT>");
			continue;
		}
		for (pmt_scan = 0x0C + ((PMT[0x0A]<<8 | PMT[0x0B]) & 0x0FFF);
			  pmt_scan < (((PMT[0x01]<<8 | PMT[0x02]) & 0x0FFF) - 7);
			  pmt_scan += 5 + PMT[pmt_scan + 4])
		{
			if (PMT[pmt_scan] == 6)
			{
				for (desc_scan = pmt_scan + 5;
					  desc_scan < pmt_scan + ((PMT[pmt_scan + 3]<<8 | PMT[pmt_scan + 4]) & 0x0FFF) + 5;
					  desc_scan += 2 + PMT[desc_scan + 1])
				{
					if (PMT[desc_scan] == 0x56)
					{
						char country_code[4];

						for (pid_test = 0; pid_test < pids_found; pid_test++)
							if (pid_table[pid_test].vtxt_pid == ((PMT[pmt_scan + 1]<<8 | PMT[pmt_scan + 2]) & 0x1FFF))
								goto skip_pid;

						pid_table[pids_found].vtxt_pid     = (PMT[pmt_scan + 1]<<8 | PMT[pmt_scan + 2]) & 0x1FFF;
						pid_table[pids_found].service_id = PMT[0x03]<<8 | PMT[0x04];
						if (PMT[desc_scan + 1] == 5)
						{
							country_code[0] = PMT[desc_scan + 2] | 0x20;
							country_code[1] = PMT[desc_scan + 3] | 0x20;
							country_code[2] = PMT[desc_scan + 4] | 0x20;
							country_code[3] = 0;
							pid_table[pids_found].national_subset = GetNationalSubset(country_code);
						}
						else
						{
							country_code[0] = 0;
							pid_table[pids_found].national_subset = NAT_DEFAULT; /* use default charset */
						}

#if TUXTXT_DEBUG
						printf("TuxTxt <Service %04x Country code \"%3s\" national subset %2d%s>\n",
								 pid_table[pids_found].service_id,
								 country_code,
								 pid_table[pids_found].national_subset,
								 (pid_table[pids_found].vtxt_pid == tuxtxt_cache.vtxtpid) ? " * " : ""
								 );
#endif

						pids_found++;
skip_pid:
					;
					}
				}
			}
		}
	}

	/* check for teletext */
	if (pids_found == 0)
	{
		printf("TuxTxt <no Teletext on TS found>\n");

		ioctl(dmx, DMX_STOP);
		::close(dmx);
		return 0;
	}

	/* read SDT to get servicenames */
	int SDT_ready = 0;

	dmx_flt.pid              = 0x0011;
	dmx_flt.flags            = DMX_CHECK_CRC | DMX_IMMEDIATE_START;
	dmx_flt.filter.filter[0] = 0x42;
	dmx_flt.filter.mask[0]   = 0xFF;
	dmx_flt.timeout          = 5000;

	if (ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
	{
		perror("TuxTxt <DMX_SET_FILTER SDT>");

		ioctl(dmx, DMX_STOP);
		::close(dmx);

		return 1;
	}

	first_sdt_sec = -1;
	while (1)
	{
		if (read(dmx, SDT, 3) == -1)
		{
			perror("TuxTxt <read SDT>");

			ioctl(dmx, DMX_STOP);
			::close(dmx);
			return 1;
		}

		if (read(dmx, SDT+3, ((SDT[1] & 0x0f) << 8) | SDT[2]) == -1)
		{
			perror("TuxTxt <read SDT>");

			ioctl(dmx, DMX_STOP);
			::close(dmx);
			return 1;
		}

		if (first_sdt_sec == SDT[6])
			break;

		if (first_sdt_sec == -1)
			first_sdt_sec = SDT[6];

		/* scan SDT to get servicenames */
		for (sdt_scan = 0x0B; sdt_scan < ((SDT[1]<<8 | SDT[2]) & 0x0FFF) - 7; sdt_scan += 5 + ((SDT[sdt_scan + 3]<<8 | SDT[sdt_scan + 4]) & 0x0FFF))
		{
			for (pid_test = 0; pid_test < pids_found; pid_test++)
			{
				if ((SDT[sdt_scan]<<8 | SDT[sdt_scan + 1]) == pid_table[pid_test].service_id && SDT[sdt_scan + 5] == 0x48)
				{
					diff = 0;
					pid_table[pid_test].service_name_len = SDT[sdt_scan+9 + SDT[sdt_scan+8]];

					for (byte = 0; byte < pid_table[pid_test].service_name_len; byte++)
					{
						pid_table[pid_test].service_name[byte + diff] = SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte];
					}

					pid_table[pid_test].service_name_len += diff;
				}
			}
		}
	}
	ioctl(dmx, DMX_STOP);
	SDT_ready = 1;



	ioctl(dmx, DMX_STOP);
	::close(dmx);
	return 1;
}

int eTuxtxtSetup::GetNationalSubset(char *cc)
{
	if (memcmp(cc, "cze", 3) == 0 || memcmp(cc, "ces", 3) == 0 ||
	    memcmp(cc, "slo", 3) == 0 || memcmp(cc, "slk", 3) == 0)
		return 0;
	if (memcmp(cc, "eng", 3) == 0)
		return 1;
	if (memcmp(cc, "est", 3) == 0)
		return 2;
	if (memcmp(cc, "fre", 3) == 0 || memcmp(cc, "fra", 3) == 0)
		return 3;
	if (memcmp(cc, "ger", 3) == 0 || memcmp(cc, "deu", 3) == 0)
		return 4;
	if (memcmp(cc, "ita", 3) == 0)
		return 5;
	if (memcmp(cc, "lav", 3) == 0 || memcmp(cc, "lit", 3) == 0)
		return 6;
	if (memcmp(cc, "pol", 3) == 0)
		return 7;
	if (memcmp(cc, "spa", 3) == 0 || memcmp(cc, "por", 3) == 0)
		return 8;
	if (memcmp(cc, "rum", 3) == 0 || memcmp(cc, "ron", 3) == 0)
		return 9;
	if (memcmp(cc, "scc", 3) == 0 || memcmp(cc, "srp", 3) == 0 ||
	    memcmp(cc, "scr", 3) == 0 || memcmp(cc, "hrv", 3) == 0 ||
	    memcmp(cc, "slv", 3) == 0)
		return 10;
	if (memcmp(cc, "swe", 3) == 0 ||
	    memcmp(cc, "dan", 3) == 0 ||
	    memcmp(cc, "nor", 3) == 0 ||
	    memcmp(cc, "fin", 3) == 0 ||
	    memcmp(cc, "hun", 3) == 0)
		return 11;
	if (memcmp(cc, "tur", 3) == 0)
		return 12;
	if (memcmp(cc, "rus", 3) == 0 || memcmp(cc, "bul", 3) == 0)
	    return NAT_RB;
	if (memcmp(cc, "ser", 3) == 0 || memcmp(cc, "cro", 3) == 0)
	    return NAT_SC;
	if (memcmp(cc, "ukr", 3) == 0)
		return NAT_UA;
	if (memcmp(cc, "gre", 3) == 0)
		return NAT_GR;
	if (memcmp(cc, "heb", 3) == 0)
		return NAT_HB;
	if (memcmp(cc, "ara", 3) == 0)
		return NAT_AR;		
	return NAT_DEFAULT;	/* use default charset */
}

eTuxtxtWidget::eTuxtxtWidget()
	: eWidget(0, 1),timer(eApp), initialized(0),pagecatching(0),rendering_initialized(0)
{
	init_eTuxtxtWidget();
}
void eTuxtxtWidget::init_eTuxtxtWidget()
{
	addActionMap(&i_tuxtxtActions->map);
	addActionMap(&i_shortcutActions->map);
	addActionMap(&i_cursorActions->map);

	addActionToHelpList(&i_tuxtxtActions->switchZoommode);
	addActionToHelpList(&i_tuxtxtActions->switchScreenmode);
	addActionToHelpList(&i_tuxtxtActions->switchTranspmode);
	addActionToHelpList(&i_tuxtxtActions->switchHintmode);
	addActionToHelpList(i_cursorActions->ok.setDescription(_("switch to direct page selecting mode")));
	addActionToHelpList(i_shortcutActions->number0.setDescription(_("show previously selected page")));
	addActionToHelpList(i_shortcutActions->number9.setDescription(_("go to next favourite page")));

	tuxtxt_cache.vtxtpid = Decoder::current.tpid;
	gethotlist();
}
eTuxtxtWidget::~eTuxtxtWidget()
{
}
int eTuxtxtWidget::getIndexOfPageInHotlist()
{
	int i;
	for (i = 0; i <= (int)hotlist.size(); i++)
	{
		if (tuxtxt_cache.page == hotlist[i])
			return i;
	}
	return -1;
}
void eTuxtxtWidget::gethotlist()
{
	FILE *hl;
	char line[100];
	int pg;

	hotlist.clear();
	sprintf(line, CONFIGDIR "/tuxtxt/hotlist%d.conf", tuxtxt_cache.vtxtpid);
#if TUXTXT_DEBUG
	printf("TuxTxt <gethotlist %s", line);
#endif
	if ((hl = fopen(line, "rb")) != 0)
	{
		do {
			if (!fgets(line, sizeof(line), hl))
				break;

			if (1 == sscanf(line, "%x", &pg))
			{
				if (pg >= 0x100 && pg <= 0x899)
				{
#if TUXTXT_DEBUG
					printf(" %03x", pg);
#endif
					hotlist.push_back(pg);
					continue;
				}
			}
#if TUXTXT_DEBUG
			else
				printf(" ?%s?", line);
#endif
		} while (hotlist.size() < 100);
		fclose(hl);
	}
#if TUXTXT_DEBUG
	printf(">\n");
#endif
	if (hotlist.size() == 0) /* hotlist incorrect or not found */
	{
		hotlist.push_back(0x100); /* create one */
		hotlist.push_back(0x303);
	}
}

int eTuxtxtWidget::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::willShow:
		lock.lock();
		Init();
		lock.unlock();
		break;
	case eWidgetEvent::execBegin:
		if (!rendering_initialized)
			in_loop=0;
		return eWidget::eventHandler(event);
	case eWidgetEvent::willHide:
		lock.lock();
		CleanUp();
		lock.unlock();
		break;
	case eWidgetEvent::evtAction:
		lock.lock();
		if (event.action == &i_cursorActions->up)
		{
			if (pagecatching)
				CatchNextPage(-1, -1);
			else
				GetNextPageOne(1);
		}
		else if (event.action == &i_cursorActions->down)
		{
			if (pagecatching)
				CatchNextPage(1, 1);
			else
				GetNextPageOne(0);
		}
		else if (event.action == &i_cursorActions->right)
		{
			if (pagecatching)
				CatchNextPage(0, 1);
			else if (renderinfo.boxed)
			{
				renderinfo.subtitledelay++;				    
				// display subtitledelay
				renderinfo.PosY = renderinfo.StartY;
				char ns[10];
				tuxtxt_SetPosX(&renderinfo,1);
				sprintf(ns,"+%d    ",renderinfo.subtitledelay);
				tuxtxt_RenderCharFB(&renderinfo,ns[0],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[1],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[2],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[4],&tuxtxt_atrtable[ATR_WB]);
			}
			else
				GetNextSubPage(1);	
		}
		else if (event.action == &i_cursorActions->left)
		{
			if (pagecatching)
				CatchNextPage(0, -1);
			else if (renderinfo.boxed)
			{
				renderinfo.subtitledelay--;
				if (renderinfo.subtitledelay < 0) renderinfo.subtitledelay = 0;
				// display subtitledelay
				renderinfo.PosY = renderinfo.StartY;
				char ns[10];
				tuxtxt_SetPosX(&renderinfo,1);
				sprintf(ns,"+%d    ",renderinfo.subtitledelay);
				tuxtxt_RenderCharFB(&renderinfo,ns[0],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[1],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[2],&tuxtxt_atrtable[ATR_WB]);
				tuxtxt_RenderCharFB(&renderinfo,ns[4],&tuxtxt_atrtable[ATR_WB]);
			}
			else
				GetNextSubPage(-1);	
		}
		else if (event.action == &i_cursorActions->help)
		{
			int vtxtpid = tuxtxt_cache.vtxtpid;
			int page = tuxtxt_cache.page;
			CancelPageCatching();
			lock.unlock();
			hide();
			eWidget::eventHandler(event);
			tuxtxt_cache.vtxtpid = vtxtpid;
			show();
			tuxtxt_cache.page = page;
			return 1;
		}
		else if (event.action == &i_cursorActions->ok)
		{
			if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] != 0xFF)
				PageCatching();
		}
		else if (event.action == &i_shortcutActions->number0)
			PageInput(0);
		else if (event.action == &i_shortcutActions->number1)
			PageInput(1);
		else if (event.action == &i_shortcutActions->number2)
			PageInput(2);
		else if (event.action == &i_shortcutActions->number3)
			PageInput(3);
		else if (event.action == &i_shortcutActions->number4)
			PageInput(4);
		else if (event.action == &i_shortcutActions->number5)
			PageInput(5);
		else if (event.action == &i_shortcutActions->number6)
			PageInput(6);
		else if (event.action == &i_shortcutActions->number7)
			PageInput(7);
		else if (event.action == &i_shortcutActions->number8)
			PageInput(8);
		else if (event.action == &i_shortcutActions->number9)
			PageInput(9);
		else if (event.action == &i_shortcutActions->red)
			ColorKey(renderinfo.prev_100);
		else if (event.action == &i_shortcutActions->green)
			ColorKey(renderinfo.prev_10);
		else if (event.action == &i_shortcutActions->yellow)
			ColorKey(renderinfo.next_10);
		else if (event.action == &i_shortcutActions->blue)
			ColorKey(renderinfo.next_100);
		else if (event.action == &i_tuxtxtActions->switchZoommode)
			SwitchZoomMode();
		else if (event.action == &i_tuxtxtActions->switchScreenmode)
		{
			CancelPageCatching();
			if (renderinfo.transpmode == 2) /* TV mode */
			{
				renderinfo.transpmode = 1; /* switch to normal mode */
				SwitchTranspMode();
			}
			tuxtxt_SwitchScreenMode(&renderinfo,-1);
			renderinfo.prevscreenmode = renderinfo.screenmode;
		}
		else if (event.action == &i_tuxtxtActions->switchTranspmode)
			SwitchTranspMode();
		else if (event.action == &i_tuxtxtActions->switchHintmode)
			SwitchHintMode();
		else if (event.action == &i_shortcutActions->menu)
			ConfigMenu();
		else if (event.action == &i_shortcutActions->escape)
		{
			timer.stop();
			lock.unlock();
			close(0);
			return 1;
		}
		else
		{
			lock.unlock();
			return eWidget::eventHandler(event);
		}
		if (!pagecatching)
			tuxtxt_RenderPage(&renderinfo);
		lock.unlock();
		return 1;
	default:
		return eWidget::eventHandler(event);
	}
	return 0;
}

void eTuxtxtWidget::RenderPage()
{
	lock.lock();
	tuxtxt_RenderPage(&renderinfo);
	lock.unlock();
}

void eTuxtxtWidget::Init()
{

#if !TUXTXT_CFG_STANDALONE
	int vtxtpid = tuxtxt_cache.vtxtpid;
	initialized = tuxtxt_init();
	if ( initialized )
		tuxtxt_cache.page = 0x100;
	tuxtxt_cache.vtxtpid = vtxtpid;
#endif
	tuxtxt_SetRenderingDefaults(&renderinfo);

	int left=20, top=20, right=699, bottom=555;
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", left);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", top);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/right", right);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/bottom", bottom);
	renderinfo.sx = left;
	renderinfo.sy = top;
	renderinfo.ex = right;
	renderinfo.ey = bottom;
	renderinfo.fb =fbClass::getInstance()->lock();


	unsigned char magazine;

	/* init data */


 	//page_atrb[32] = transp<<4 | transp;

	for (magazine = 1; magazine < 9; magazine++)
	{
		tuxtxt_cache.current_page  [magazine] = -1;
		tuxtxt_cache.current_subpage [magazine] = -1;
	}
#if TUXTXT_CFG_STANDALONE
/* init data */
	memset(&tuxtxt_cache.astCachetable, 0, sizeof(tuxtxt_cache.astCachetable));
	memset(&tuxtxt_cache.subpagetable, 0xFF, sizeof(tuxtxt_cache.subpagetable));
	memset(&tuxtxt_cache.astP29, 0, sizeof(tuxtxt_cache.astP29));

	memset(&tuxtxt_cache.basictop, 0, sizeof(tuxtxt_cache.basictop));
	memset(&tuxtxt_cache.adip, 0, sizeof(tuxtxt_cache.adip));
	memset(&tuxtxt_cache.flofpages, 0 , sizeof(tuxtxt_cache.flofpages));
	memset(renderinfo.subtitlecache,0,sizeof(renderinfo.subtitlecache));
	tuxtxt_cache.maxadippg  = -1;
	tuxtxt_cache.bttok      = 0;
	maxhotlist = -1;

	//page_atrb[32] = transp<<4 | transp;
	tuxtxt_cache.cached_pages  = 0;

	tuxtxt_cache.page_receiving = -1;
	tuxtxt_cache.page       = 0x100;
#endif
	lastpage   = tuxtxt_cache.page;
	tuxtxt_cache.subpage    = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
	if (tuxtxt_cache.subpage == 0xff)
	tuxtxt_cache.subpage    = 0;
	
	tuxtxt_cache.pageupdate = 0;

	tuxtxt_cache.zap_subpage_manual = 0;

	tuxtxt_cache.national_subset = 0;/* default */

	/* load config */
	if (eConfig::getInstance()->getKey("/ezap/teletext/ScreenMode16x9Normal", renderinfo.screen_mode1 ))
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Normal", renderinfo.screen_mode1);
	if (eConfig::getInstance()->getKey("/ezap/teletext/ScreenMode16x9Divided", renderinfo.screen_mode2 ))
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Divided", renderinfo.screen_mode2);
	if (eConfig::getInstance()->getKey("/ezap/teletext/Brightness", renderinfo.color_mode ))
		eConfig::getInstance()->setKey("/ezap/teletext/Brightness", renderinfo.color_mode);
	if (eConfig::getInstance()->getKey("/ezap/teletext/Transparency", renderinfo.trans_mode ))
		eConfig::getInstance()->setKey("/ezap/teletext/Transparency", renderinfo.trans_mode);
	if (eConfig::getInstance()->getKey("/ezap/teletext/AutoNational", renderinfo.auto_national ))
		eConfig::getInstance()->setKey("/ezap/teletext/AutoNational", renderinfo.auto_national);
	if (eConfig::getInstance()->getKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset ))
		eConfig::getInstance()->setKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset);
	if (eConfig::getInstance()->getKey("/ezap/teletext/Screenmode", renderinfo.screenmode ))
		eConfig::getInstance()->setKey("/ezap/teletext/Screenmode", renderinfo.screenmode);
	if (eConfig::getInstance()->getKey("/ezap/teletext/ShowLevel2p5", renderinfo.showl25 ))
		eConfig::getInstance()->setKey("/ezap/teletext/ShowLevel2p5", renderinfo.showl25);
	if (eConfig::getInstance()->getKey("/ezap/teletext/UseTTF", renderinfo.usettf ))
		eConfig::getInstance()->setKey("/ezap/teletext/UseTTF", renderinfo.usettf);
	if (renderinfo.usettf)
	{
		struct stat64 s;
		
		if (stat64(TUXTXTTTF, &s)<0 &&
		    stat64(TUXTXTTTFVAR, &s)<0)
		{
			renderinfo.usettf = 0;
			eConfig::getInstance()->setKey("/ezap/teletext/UseTTF", renderinfo.usettf);
		}
	}

	if (!tuxtxt_InitRendering(&renderinfo,1))
	{
				
#if !TUXTXT_CFG_STANDALONE
		if ( initialized ){
			tuxtxt_close();
		}
#endif
		return;
	}
	savedscreenmode = renderinfo.screenmode;
	tuxtxt_cache.national_subset_secondary = NAT_DEFAULT;


#if TUXTXT_CFG_STANDALONE
	tuxtxt_init_demuxer();
	tuxtxt_start_thread();
#else
	tuxtxt_start(tuxtxt_cache.vtxtpid);
#endif


	tuxtxt_SwitchScreenMode(&renderinfo,renderinfo.screenmode);
	renderinfo.prevscreenmode = renderinfo.screenmode;
	CONNECT(timer.timeout, eTuxtxtWidget::RenderPage);
	timer.start(TUXTXT_TIMER_TICK, false);
	rendering_initialized=1;
}

void eTuxtxtWidget::CleanUp()
{
	timer.stop();
	if (rendering_initialized)
	{
		if (renderinfo.transpmode == 2) /* TV mode */
		{
			renderinfo.transpmode = 1; /* switch to normal mode */
			SwitchTranspMode();
		}
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Normal", renderinfo.screen_mode1 );
		eConfig::getInstance()->setKey("/ezap/teletext/ScreenMode16x9Divided", renderinfo.screen_mode2 );
		eConfig::getInstance()->setKey("/ezap/teletext/Brightness", renderinfo.color_mode );
		eConfig::getInstance()->setKey("/ezap/teletext/Transparency", renderinfo.trans_mode );
		eConfig::getInstance()->setKey("/ezap/teletext/AutoNational", renderinfo.auto_national );
		eConfig::getInstance()->setKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset );
		eConfig::getInstance()->setKey("/ezap/teletext/Screenmode", renderinfo.screenmode );
		eConfig::getInstance()->setKey("/ezap/teletext/ShowLevel2p5", renderinfo.showl25 );
		eConfig::getInstance()->setKey("/ezap/teletext/UseTTF", renderinfo.usettf);
		/* hide and close pig */
		if (renderinfo.screenmode)
			tuxtxt_SwitchScreenMode(&renderinfo,0); /* turn off divided screen */
#if TUXTXT_CFG_STANDALONE
		tuxtxt_stop_thread();
		tuxtxt_clear_cache();
		if (tuxtxt_cache.dmx != -1)
		close(tuxtxt_cache.dmx);
		tuxtxt_cache.dmx = -1;
#else
		tuxtxt_stop();
#endif


		tuxtxt_EndRendering(&renderinfo);
#if !TUXTXT_CFG_STANDALONE
		if ( initialized )
			tuxtxt_close();
#endif
	}
	fbClass::getInstance()->unlock();
	eSkin::getActive()->setPalette(gFBDC::getInstance());
}


void eTuxtxtWidget::ConfigMenu()
{
	CancelPageCatching();
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}
	int oldscreenmode;
	oldscreenmode = renderinfo.screenmode;
	if (renderinfo.screenmode)
		tuxtxt_SwitchScreenMode(&renderinfo,0); /* turn off divided screen */
	int tpid = tuxtxt_cache.vtxtpid;
	lock.unlock();
	hide();
	eTuxtxtSetup wnd(&renderinfo,&hotlist);
	wnd.show();
	int res = wnd.exec();
	wnd.hide();
	if (res >= 0)
		gethotlist();
	else
		tuxtxt_cache.vtxtpid = tpid;

	show();
	if (oldscreenmode)
		tuxtxt_SwitchScreenMode(&renderinfo,oldscreenmode); /* restore divided screen */
}

void eTuxtxtWidget::PageInput(int Number)
{
	CancelPageCatching();
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}
	int zoom = 0;

	/* clear temp_page */
	if (renderinfo.inputcounter == 2)
		temp_page = 0;

	/* check for 0 & 9 on first position */
	if (Number == 0 && renderinfo.inputcounter == 2)
	{
		/* set page */
		temp_page = lastpage; /* 0 toggles to last page as in program switching */
		renderinfo.inputcounter = -1;
	}
	else if (Number == 9 && renderinfo.inputcounter == 2)
	{
		/* set page */
		temp_page = getIndexOfPageInHotlist(); /* 9 toggles through hotlist */

		if (temp_page<0 || temp_page==(int)hotlist.size()-1) /* from any (other) page go to first page in hotlist */
			temp_page = ((int)hotlist.size() > 0) ? hotlist[0] : 0x100;
		else
			temp_page = hotlist[temp_page+1];
		renderinfo.inputcounter = -1;
	}

	/* show pageinput */
	if (renderinfo.zoommode == 2)
	{
		renderinfo.zoommode = 1;
		tuxtxt_CopyBB2FB(&renderinfo);
	}

	if (renderinfo.zoommode == 1)
		zoom = 1<<10;

	renderinfo.PosY = renderinfo.StartY;

	switch (renderinfo.inputcounter)
	{
	case 2:
		tuxtxt_SetPosX(&renderinfo,1);
		tuxtxt_RenderCharFB(&renderinfo,Number | '0', &tuxtxt_atrtable[ATR_WB]);
		tuxtxt_RenderCharFB(&renderinfo,'-', &tuxtxt_atrtable[ATR_WB]);
		tuxtxt_RenderCharFB(&renderinfo,'-', &tuxtxt_atrtable[ATR_WB]);
		break;

	case 1:
		tuxtxt_SetPosX(&renderinfo,2);
		tuxtxt_RenderCharFB(&renderinfo,Number | '0', &tuxtxt_atrtable[ATR_WB]);
		break;

	case 0:
		tuxtxt_SetPosX(&renderinfo,3);
		tuxtxt_RenderCharFB(&renderinfo,Number | '0', &tuxtxt_atrtable[ATR_WB]);
		break;
	}

	/* generate pagenumber */
	temp_page |= Number << renderinfo.inputcounter*4;

	renderinfo.inputcounter--;

	if (renderinfo.inputcounter < 0)
	{
		/* disable subpage zapping */
		tuxtxt_cache.zap_subpage_manual = 0;

		/* reset input */
		renderinfo.inputcounter = 2;

		/* set new page */
		lastpage = tuxtxt_cache.page;

		tuxtxt_cache.page = temp_page;
		renderinfo.hintmode = 0;

		/* check cache */
		int subp = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
		if (subp != 0xFF)
		{
			tuxtxt_cache.subpage = subp;
			tuxtxt_cache.pageupdate = 1;
		}
		else
		{
			tuxtxt_cache.subpage = 0;
		}
	}
}

void eTuxtxtWidget::GetNextPageOne(int up)
{
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}

	/* disable subpage zapping */
	tuxtxt_cache.zap_subpage_manual = 0;

	/* abort pageinput */
	renderinfo.inputcounter = 2;

	/* find next cached page */
	lastpage = tuxtxt_cache.page;

	int subp;
	do {
		if (up)
			tuxtxt_next_dec(&tuxtxt_cache.page);
		else
			tuxtxt_prev_dec(&tuxtxt_cache.page);
		subp = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
	} while (subp == 0xFF && tuxtxt_cache.page != lastpage);

	/* update page */
	if (tuxtxt_cache.page != lastpage)
	{
		if (renderinfo.zoommode == 2)
			renderinfo.zoommode = 1;

		tuxtxt_cache.subpage = subp;
		renderinfo.hintmode = 0;
		tuxtxt_cache.pageupdate = 1;
	}
}

void eTuxtxtWidget::GetNextSubPage(int offset)
{
	int loop;

	/* abort pageinput */
	renderinfo.inputcounter = 2;

	for (loop = tuxtxt_cache.subpage + offset; loop != tuxtxt_cache.subpage; loop += offset)
	{
		if (loop < 0)
			loop = 0x79;
		else if (loop > 0x79)
			loop = 0;
		if (loop == tuxtxt_cache.subpage)
			break;

		if (tuxtxt_cache.astCachetable[tuxtxt_cache.page][loop])
		{
			/* enable manual subpage zapping */
			tuxtxt_cache.zap_subpage_manual = 1;

			/* update page */
			if (renderinfo.zoommode == 2) /* if zoomed to lower half */
				renderinfo.zoommode = 1; /* activate upper half */

			tuxtxt_cache.subpage = loop;
			renderinfo.hintmode = 0;
			tuxtxt_cache.pageupdate = 1;
			return;
		}
	}

}

void eTuxtxtWidget::ColorKey(int target)
{
	CancelPageCatching();
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}
	if (!target)
		return;
	if (renderinfo.zoommode == 2)
		renderinfo.zoommode = 1;
	lastpage     = tuxtxt_cache.page;
	tuxtxt_cache.page         = target;
	tuxtxt_cache.subpage      = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
	renderinfo.inputcounter = 2;
	renderinfo.hintmode     = 0;
	tuxtxt_cache.pageupdate   = 1;
}

/******************************************************************************
 * PageCatching                                                               *
 ******************************************************************************/

void eTuxtxtWidget::PageCatching()
{
	if (!pagecatching)
	{
		timer.stop();
		if (renderinfo.transpmode == 2) /* TV mode */
		{
			renderinfo.transpmode = 1; /* switch to normal mode */
			SwitchTranspMode();
		}
		int byte;
		int oldzoommode = renderinfo.zoommode;
	
		pagecatching = 1;
	
		/* abort pageinput */
		renderinfo.inputcounter = 2;
	
		/* show info line */
		renderinfo.zoommode = 0;
		renderinfo.PosX = renderinfo.StartX;
		renderinfo.PosY = renderinfo.StartY + 24*renderinfo.fontheight;
		const char catchmenutext[] = { "        н п р о       ст                " };
		for (byte = 0; byte < 40-renderinfo.nofirst; byte++) {
			tuxtxt_RenderCharFB(&renderinfo,catchmenutext[byte], &tuxtxt_atrtable[ATR_CATCHMENU1]);
		}
		renderinfo.zoommode = oldzoommode;
	
		/* check for pagenumber(s) */
		catch_row    = 1;
		catch_col    = 0;
		catched_page = 0;
		pc_old_row = pc_old_col = 0; /* no inverted page number to restore yet */
		CatchNextPage(0, 1);
	
		if (!catched_page)
		{
			pagecatching = 0;
			tuxtxt_cache.pageupdate = 1;
			return;
		}
	}
	else
	{
		/* set new page */
		if (renderinfo.zoommode == 2)
			renderinfo.zoommode = 1;
	
		lastpage     = tuxtxt_cache.page;
		tuxtxt_cache.page         = catched_page;
		renderinfo.hintmode = 0;
		tuxtxt_cache.pageupdate = 1;
		pagecatching = 0;
	
		int subp = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
		if (subp != 0xFF)
			tuxtxt_cache.subpage = subp;
		else
			tuxtxt_cache.subpage = 0;
		timer.start(TUXTXT_TIMER_TICK, false);
	}
}

/******************************************************************************
 * CatchNextPage                                                              *
 ******************************************************************************/

void eTuxtxtWidget::CatchNextPage(int firstlineinc, int inc)
{
	int tmp_page, allowwrap = 1; /* allow first wrap around */

	/* catch next page */
	for(;;)
	{
		unsigned char *p = &(renderinfo.page_char[catch_row*40 + catch_col]);
		tstPageAttr a = renderinfo.page_atrb[catch_row*40 + catch_col];

		if (!(a.charset == C_G1C || a.charset == C_G1S) && /* no mosaic */
			 (a.fg != a.bg) && /* not hidden */
			 (*p >= '1' && *p <= '8' && /* valid page number */
			  *(p+1) >= '0' && *(p+1) <= '9' &&
			  *(p+2) >= '0' && *(p+2) <= '9') &&
			 (catch_row == 0 || (*(p-1) < '0' || *(p-1) > '9')) && /* non-numeric char before and behind */
			 (catch_row == 37 || (*(p+3) < '0' || *(p+3) > '9')))
		{
			tmp_page = ((*p - '0')<<8) | ((*(p+1) - '0')<<4) | (*(p+2) - '0');

#if 0
			if (tmp_page != catched_page)	/* confusing to skip identical page numbers - I want to reach what I aim to */
#endif
			{
				catched_page = tmp_page;
				RenderCatchedPage();
				catch_col += inc;	/* FIXME: limit */
				return;
			}
		}

		if (firstlineinc > 0)
		{
			catch_row++;
			catch_col = 0;
			firstlineinc = 0;
		}
		else if (firstlineinc < 0)
		{
			catch_row--;
			catch_col = 37;
			firstlineinc = 0;
		}
		else
			catch_col += inc;

		if (catch_col > 37)
		{
			catch_row++;
			catch_col = 0;
		}
		else if (catch_col < 0)
		{
			catch_row--;
			catch_col = 37;
		}

		if (catch_row > 23)
		{
			if (allowwrap)
			{
				allowwrap = 0;
				catch_row = 1;
				catch_col = 0;
			}
			else
			{
				return;
			}
		}
		else if (catch_row < 1)
		{
			if (allowwrap)
			{
				allowwrap = 0;
				catch_row = 23;
				catch_col =37;
			}
			else
			{
				return;
			}
		}
	}
}

void eTuxtxtWidget::RenderCatchedPage()
{
	int zoom = 0;

	/* handle zoom */
	if (renderinfo.zoommode)
		zoom = 1<<10;

	if (pc_old_row || pc_old_col) /* not at first call */
	{
		/* restore pagenumber */
		tuxtxt_SetPosX(&renderinfo,pc_old_col);

		if (renderinfo.zoommode == 2)
			renderinfo.PosY = renderinfo.StartY + (pc_old_row-12)*renderinfo.fontheight*((zoom>>10)+1);
		else
			renderinfo.PosY = renderinfo.StartY + pc_old_row*renderinfo.fontheight*((zoom>>10)+1);

		tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[pc_old_row*40 + pc_old_col    ], &renderinfo.page_atrb[pc_old_row*40 + pc_old_col    ]);
		tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[pc_old_row*40 + pc_old_col + 1], &renderinfo.page_atrb[pc_old_row*40 + pc_old_col + 1]);
		tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[pc_old_row*40 + pc_old_col + 2], &renderinfo.page_atrb[pc_old_row*40 + pc_old_col + 2]);
	}

	pc_old_row = catch_row;
	pc_old_col = catch_col;

	/* mark pagenumber */
	if (renderinfo.zoommode == 1 && catch_row > 11)
	{
		renderinfo.zoommode = 2;
		tuxtxt_CopyBB2FB(&renderinfo);
	}
	else if (renderinfo.zoommode == 2 && catch_row < 12)
	{
		renderinfo.zoommode = 1;
		tuxtxt_CopyBB2FB(&renderinfo);
	}
	tuxtxt_SetPosX(&renderinfo,catch_col);


	if (renderinfo.zoommode == 2)
		renderinfo.PosY = renderinfo.StartY + (catch_row-12)*renderinfo.fontheight*((zoom>>10)+1);
	else
		renderinfo.PosY = renderinfo.StartY + catch_row*renderinfo.fontheight*((zoom>>10)+1);

	tstPageAttr a0 = renderinfo.page_atrb[catch_row*40 + catch_col    ];
	tstPageAttr a1 = renderinfo.page_atrb[catch_row*40 + catch_col + 1];
	tstPageAttr a2 = renderinfo.page_atrb[catch_row*40 + catch_col + 2];
	int t;

	/* exchange colors */
	t = a0.fg; a0.fg = a0.bg; a0.bg = t;
	t = a1.fg; a1.fg = a1.bg; a1.bg = t;
	t = a2.fg; a2.fg = a2.bg; a2.bg = t;

	tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[catch_row*40 + catch_col    ], &a0);
	tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[catch_row*40 + catch_col + 1], &a1);
	tuxtxt_RenderCharFB(&renderinfo,renderinfo.page_char[catch_row*40 + catch_col + 2], &a2);
}

void eTuxtxtWidget::CancelPageCatching()
{
	if (pagecatching)
	{
		pagecatching = 0;
		renderinfo.hintmode = 0;
		tuxtxt_cache.pageupdate = 1;
		timer.start(TUXTXT_TIMER_TICK, false);
	}
}
void eTuxtxtWidget::SwitchZoomMode()
{
	CancelPageCatching();
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}
	if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] != 0xFF)
	{
		/* toggle mode */
		renderinfo.zoommode++;

		if (renderinfo.zoommode == 3)
			renderinfo.zoommode = 0;

		/* update page */
		tuxtxt_cache.pageupdate = 1; /* FIXME */
	}
}


void eTuxtxtWidget::SwitchTranspMode()
{
	CancelPageCatching();
	if (renderinfo.screenmode)
	{
		renderinfo.prevscreenmode = renderinfo.screenmode;
		tuxtxt_SwitchScreenMode(&renderinfo,0); /* turn off divided screen */
	}
	/* toggle mode */
	if (!renderinfo.transpmode)
		renderinfo.transpmode = 2;
	else
		renderinfo.transpmode--; /* backward to immediately switch to TV-screen */


	/* set mode */
	if (!renderinfo.transpmode) /* normal text-only */
	{
		tuxtxt_ClearBB(&renderinfo,tuxtxt_cache.FullScrColor);
		tuxtxt_cache.pageupdate = 1;
	}
	else if (renderinfo.transpmode == 1) /* semi-transparent BG with FG text */
	{
		/* restore videoformat */
		ioctl(renderinfo.avs, AVSIOSSCARTPIN8, &renderinfo.fnc_old);
		ioctl(renderinfo.saa, SAAIOSWSS, &renderinfo.saa_old);

		tuxtxt_ClearBB(&renderinfo,tuxtxt_color_transp);
		tuxtxt_cache.pageupdate = 1;
	}
	else /* TV mode */
	{
		/* restore videoformat */
		ioctl(renderinfo.avs, AVSIOSSCARTPIN8, &renderinfo.fnc_old);
		ioctl(renderinfo.saa, SAAIOSWSS, &renderinfo.saa_old);

		tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);
		renderinfo.clearbbcolor = tuxtxt_cache.FullScrColor;
	}
}

void eTuxtxtWidget::SwitchHintMode()
{
	CancelPageCatching();
	if (renderinfo.transpmode == 2) /* TV mode */
	{
		renderinfo.transpmode = 1; /* switch to normal mode */
		SwitchTranspMode();
	}
	/* toggle mode */
	renderinfo.hintmode ^= 1;

	if (!renderinfo.hintmode)	/* toggle evaluation of level 2.5 information by explicitly switching off hintmode */
	{
		renderinfo.showl25 ^= 1;
	}
	/* update page */
	tuxtxt_cache.pageupdate = 1;
}




