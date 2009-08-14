/******************************************************************************
 *                      <<< TuxTxt - Teletext Plugin >>>                      *
 *                                                                            *
 *             (c) Thomas "LazyT" Loewe 2002-2003 (LazyT@gmx.net)             *
 *                                                                            *
 *    continued 2004-2005 by Roland Meier <RolandMeier@Siemens.com>           *
 *                       and DBLuelle <dbluelle@blau-weissoedingen.de>        *
 *	russian and arabic support by Leonid Protasov <Lprot@mail.ru>         *
 *                                                                            *
 ******************************************************************************/



#include "tuxtxt.h"


int getIndexOfPageInHotlist()
{
	int i;
	for (i = 0; i <= maxhotlist; i++)
	{
		if (tuxtxt_cache.page == hotlist[i])
			return i;
	}
	return -1;
}

void gethotlist()
{
	FILE *hl;
	char line[100];

	hotlistchanged = 0;
	maxhotlist = -1;
	sprintf(line, CONFIGDIR "/tuxtxt/hotlist%d.conf", tuxtxt_cache.vtxtpid);
#if TUXTXT_DEBUG
	printf("TuxTxt <gethotlist %s", line);
#endif
	if ((hl = fopen(line, "rb")) != 0)
	{
		do {
			if (!fgets(line, sizeof(line), hl))
				break;

			if (1 == sscanf(line, "%x", &hotlist[maxhotlist+1]))
			{
				if (hotlist[maxhotlist+1] >= 0x100 && hotlist[maxhotlist+1] <= 0x899)
				{
#if TUXTXT_DEBUG
					printf(" %03x", hotlist[maxhotlist+1]);
#endif
					maxhotlist++;
					continue;
				}
			}
#if TUXTXT_DEBUG
			else
				printf(" ?%s?", line);
#endif
		} while (maxhotlist < (sizeof(hotlist)/sizeof(hotlist[0])-1));
		fclose(hl);
	}
#if TUXTXT_DEBUG
	printf(">\n");
#endif
	if (maxhotlist < 0) /* hotlist incorrect or not found */
	{
		hotlist[0] = 0x100; /* create one */
		hotlist[1] = 0x303;
		maxhotlist = 1;
	}
}

void savehotlist()
{
	FILE *hl;
	char line[100];
	int i;

	hotlistchanged = 0;
	sprintf(line, CONFIGDIR "/tuxtxt/hotlist%d.conf", tuxtxt_cache.vtxtpid);
#if TUXTXT_DEBUG
	printf("TuxTxt <savehotlist %s", line);
#endif
	if (maxhotlist != 1 || hotlist[0] != 0x100 || hotlist[1] != 0x303)
	{
		if ((hl = fopen(line, "wb")) != 0)
		{
			for (i = 0; i <= maxhotlist; i++)
			{
				fprintf(hl, "%03x\n", hotlist[i]);
#if TUXTXT_DEBUG
				printf(" %03x", hotlist[i]);
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

/* hexdump of page contents to stdout for debugging */
void dump_page()
{
	int r, c;
	char *p;
	unsigned char pagedata[23*40];

	if (!tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage])
		return;
	tuxtxt_decompress_page(tuxtxt_cache.page,tuxtxt_cache.subpage,pagedata);
	for (r=1; r < 24; r++)
	{
		p = pagedata+40*(r-1);
		for (c=0; c < 40; c++)
			printf(" %02x", *p++);
		printf("\n");
		p = renderinfo.page_char + 40*r;
		for (c=0; c < 40; c++)
			printf("  %c", *p++);
		printf("\n");
	}
}



/******************************************************************************
 * plugin_exec                                                                *
 ******************************************************************************/

void plugin_exec(PluginParam *par)
{
	char cvs_revision[] = "$Revision: 1.109 $";

#if !TUXTXT_CFG_STANDALONE
	int initialized = tuxtxt_init();
	if ( initialized )
		tuxtxt_cache.page = 0x100;
#endif

	/* show versioninfo */
	sscanf(cvs_revision, "%*s %s", versioninfo);
	printf("TuxTxt %s\n", versioninfo);

	tuxtxt_SetRenderingDefaults(&renderinfo);
	/* get params */
	tuxtxt_cache.vtxtpid = renderinfo.fb = lcd = rc = renderinfo.sx = renderinfo.ex = renderinfo.sy = renderinfo.ey = -1;

	for (; par; par = par->next)
	{
		if (!strcmp(par->id, P_ID_VTXTPID))
			tuxtxt_cache.vtxtpid = atoi(par->val);
		else if (!strcmp(par->id, P_ID_FBUFFER))
			renderinfo.fb = atoi(par->val);
		else if (!strcmp(par->id, P_ID_LCD))
			lcd = atoi(par->val);
		else if (!strcmp(par->id, P_ID_RCINPUT))
			rc = atoi(par->val);
		else if (!strcmp(par->id, P_ID_OFF_X))
			renderinfo.sx = atoi(par->val);
		else if (!strcmp(par->id, P_ID_END_X))
			renderinfo.ex = atoi(par->val);
		else if (!strcmp(par->id, P_ID_OFF_Y))
			renderinfo.sy = atoi(par->val);
		else if (!strcmp(par->id, P_ID_END_Y))
			renderinfo.ey = atoi(par->val);
	}

	if (tuxtxt_cache.vtxtpid == -1 || renderinfo.fb == -1 || rc == -1 || renderinfo.sx == -1 || renderinfo.ex == -1 || renderinfo.sy == -1 || renderinfo.ey == -1)
	{
		printf("TuxTxt <Invalid Param(s)>\n");
		return;
	}

	/* initialisations */
	if (Init() == 0){
#if !TUXTXT_CFG_STANDALONE
		if ( initialized ){
			tuxtxt_close();
		}
#endif
		return;
	}
	
	/* main loop */
	do {
		if (GetRCCode() == 1)
		{
			if (renderinfo.transpmode == 2) /* TV mode */
			{
				switch (RCCode)
				{
//#if TUXTXT_DEBUG /* FIXME */
				case RC_OK:
					if (renderinfo.showhex)
					{
						dump_page(); /* hexdump of page contents to stdout for debugging */
					}
					continue; /* otherwise ignore key */
//#endif /* TUXTXT_DEBUG */
				case RC_UP:
				case RC_DOWN:
				case RC_0:
				case RC_1:
				case RC_2:
				case RC_3:
				case RC_4:
				case RC_5:
				case RC_6:
				case RC_7:
				case RC_8:
				case RC_9:
				case RC_GREEN:
				case RC_YELLOW:
				case RC_BLUE:
				case RC_PLUS:
				case RC_MINUS:
				case RC_DBOX:
				case RC_STANDBY:
					renderinfo.transpmode = 1; /* switch to normal mode */
					SwitchTranspMode();
					break;		/* and evaluate key */

				case RC_MUTE:		/* regular toggle to transparent */
					break;

				case RC_HELP: /* switch to scart input and back */
				{
					int i, n;
#ifdef HAVE_DBOX_HARDWARE
					int vendor = tuxbox_get_vendor() - 1;
#else
					int vendor = 3; /* values unknown, rely on requested values */
#endif

					if (vendor < 3) /* scart-parameters only known for 3 dboxes, FIXME: order must be like in info.h */
					{
						for (i = 0; i < 6; i++) /* FIXME: FBLK seems to cause troubles */
						{
							if (!restoreaudio || !(i & 1)) /* not for audio if scart-audio active */
							{
								if ((ioctl(renderinfo.avs, avstable_ioctl_get[i], &n)) < 0) /* get current values for restoration */
									perror("TuxTxt <ioctl(avs)>");
								else
									avstable_dvb[vendor][i] = n;
							}

							n = avstable_scart[vendor][i];
							if ((ioctl(renderinfo.avs, avstable_ioctl[i], &n)) < 0)
								perror("TuxTxt <ioctl(avs)>");
						}

						while (GetRCCode() != 1) /* wait for any key */
							UpdateLCD();

						if (RCCode == RC_HELP)
							restoreaudio = 1;
						else
							restoreaudio = 0;

						for (i = 0; i < 6; i += (restoreaudio ? 2 : 1)) /* exit with ?: just restore video, leave audio */
						{
							n = avstable_dvb[vendor][i];
							if ((ioctl(renderinfo.avs, avstable_ioctl[i], &n)) < 0)
								perror("TuxTxt <ioctl(avs)>");
						}
					}
					continue; /* otherwise ignore exit key */
				}
				default:
					continue; /* ignore all other keys */
				}
			}

			switch (RCCode)
			{
			case RC_UP:
				GetNextPageOne(!swapupdown);
				break;
			case RC_DOWN:
				GetNextPageOne(swapupdown);
				break;
			case RC_RIGHT:	
				if (renderinfo.boxed)
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
				break;
			case RC_LEFT:
				if (renderinfo.boxed)
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
				break;
			case RC_OK:
				if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] == 0xFF)
					continue;
				PageCatching();
				break;

			case RC_0:
			case RC_1:
			case RC_2:
			case RC_3:
			case RC_4:
			case RC_5:
			case RC_6:
			case RC_7:
			case RC_8:
			case RC_9:
				PageInput(RCCode - RC_0);
				break;
			case RC_RED:	 ColorKey(renderinfo.prev_100);		break;
			case RC_GREEN:	 ColorKey(renderinfo.prev_10);		break;
			case RC_YELLOW:	 ColorKey(renderinfo.next_10);		break;
			case RC_BLUE:	 ColorKey(renderinfo.next_100);		break;
			case RC_PLUS:	 SwitchZoomMode();		break;
			case RC_MINUS:	 tuxtxt_SwitchScreenMode(&renderinfo,-1);renderinfo.prevscreenmode = renderinfo.screenmode; break;
			case RC_MUTE:	 SwitchTranspMode();	break;
			case RC_HELP:	 SwitchHintMode();		break;
			case RC_DBOX:	 ConfigMenu(0);			break;
			}
		}

		/* update page or timestring and lcd */
		UpdateLCD();
		tuxtxt_RenderPage(&renderinfo);
	} while ((RCCode != RC_HOME) && (RCCode != RC_STANDBY));

	/* exit */
	CleanUp();

#if !TUXTXT_CFG_STANDALONE
	if ( initialized )
		tuxtxt_close();
#endif

 	printf("Tuxtxt: plugin ended\n");

}


/******************************************************************************
 * Init                                                                       *
 ******************************************************************************/

int Init()
{
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

	/* init lcd */
	UpdateLCD();

	/* config defaults */
	menulanguage = 0;	/* german */
	tuxtxt_cache.national_subset = 0;/* default */
	swapupdown      = 0;
	dumpl25         = 0;

	/* load config */
	if ((conf = fopen(TUXTXTCONF, "rt")) == 0)
	{
		perror("TuxTxt <fopen tuxtxt.conf>");
	}
	else
	{
		while(1)
		{
			char line[100];
			int ival;

			if (!fgets(line, sizeof(line), conf))
				break;

			if (1 == sscanf(line, "ScreenMode16x9Normal %i", &ival))
				renderinfo.screen_mode1 = ival & 1;
			else if (1 == sscanf(line, "ScreenMode16x9Divided %i", &ival))
				renderinfo.screen_mode2 = ival & 1;
			else if (1 == sscanf(line, "Brightness %i", &ival))
				renderinfo.color_mode = ival;
			else if (1 == sscanf(line, "AutoNational %i", &ival))
				renderinfo.auto_national = ival & 1;
			else if (1 == sscanf(line, "NationalSubset %i", &ival))
			{
				if (ival >= 0 && ival <= MAX_NATIONAL_SUBSET)
					tuxtxt_cache.national_subset = ival;
			}
			else if (1 == sscanf(line, "MenuLanguage %i", &ival))
			{
				if (ival >= 0 && ival <= MAXMENULANGUAGE)
					menulanguage = ival;
			}
			else if (1 == sscanf(line, "SwapUpDown %i", &ival))
				swapupdown = ival & 1;
			else if (1 == sscanf(line, "ShowHexPages %i", &ival))
				renderinfo.showhex = ival & 1;
			else if (1 == sscanf(line, "Transparency %i", &ival))
				renderinfo.trans_mode = ival;
			else if (1 == sscanf(line, "TTFWidthFactor16 %i", &ival))
				renderinfo.TTFWidthFactor16 = ival;
			else if (1 == sscanf(line, "TTFHeightFactor16 %i", &ival))
				renderinfo.TTFHeightFactor16 = ival;
			else if (1 == sscanf(line, "TTFShiftX %i", &ival))
				renderinfo.TTFShiftX = ival;
			else if (1 == sscanf(line, "TTFShiftY %i", &ival))
				renderinfo.TTFShiftY = ival;
			else if (1 == sscanf(line, "Screenmode %i", &ival))
				renderinfo.screenmode = ival;
			else if (1 == sscanf(line, "ShowFLOF %i", &ival))
				renderinfo.showflof = ival & 1;
			else if (1 == sscanf(line, "Show39 %i", &ival))
				renderinfo.show39 = ival & 1;
			else if (1 == sscanf(line, "ShowLevel2p5 %i", &ival))
				renderinfo.showl25 = ival & 1;
			else if (1 == sscanf(line, "DumpLevel2p5 %i", &ival))
				dumpl25 = ival & 1;
			else if (1 == sscanf(line, "UseTTF %i", &ival))
				renderinfo.usettf = ival & 1;
		}
		fclose(conf);
	}
	if (!tuxtxt_InitRendering(&renderinfo,1))
		return 0;
	saveconfig = 0;
	savedscreenmode = renderinfo.screenmode;
	tuxtxt_cache.national_subset_secondary = NAT_DEFAULT;


	/*  if no vtxtpid for current service, search PIDs */
	if (tuxtxt_cache.vtxtpid == 0)
	{
		/* get all vtxt-pids */
		getpidsdone = -1;						 /* don't kill thread */
		if (GetTeletextPIDs() == 0)
		{
			tuxtxt_EndRendering(&renderinfo);
			return 0;
		}

		if (renderinfo.auto_national)
			tuxtxt_cache.national_subset = pid_table[0].national_subset;
		if (pids_found > 1)
			ConfigMenu(1);
		else
		{
			tuxtxt_cache.vtxtpid = pid_table[0].vtxt_pid;
			current_service = 0;
			RenderMessage(ShowServiceName);
		}
	}
	else
	{
		SDT_ready = 0;
		getpidsdone = 0;
//		tuxtxt_cache.pageupdate = 1; /* force display of message page not found (but not twice) */

	}
#if TUXTXT_CFG_STANDALONE
	tuxtxt_init_demuxer();
	tuxtxt_start_thread();
#else
	tuxtxt_start(tuxtxt_cache.vtxtpid);
#endif


	/* setup rc */
	fcntl(rc, F_SETFL, O_NONBLOCK);
	ioctl(rc, RC_IOCTL_BCODES, 1);




	gethotlist();
	tuxtxt_SwitchScreenMode(&renderinfo,renderinfo.screenmode);
	renderinfo.prevscreenmode = renderinfo.screenmode;
	
	printf("TuxTxt: init ok\n");

	/* init successfull */
	return 1;
}

/******************************************************************************
 * Cleanup                                                                    *
 ******************************************************************************/

void CleanUp()
{
	int i, n, curscreenmode = renderinfo.screenmode;

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

	if (restoreaudio)
	{
#ifdef HAVE_DBOX_HARDWARE
		int vendor = tuxbox_get_vendor() - 1;
#else
		int vendor = 3; /* values unknown, rely on requested values */
#endif
		if (vendor < 3) /* scart-parameters only known for 3 dboxes, FIXME: order must be like in info.h */
		{
			for (i = 1; i < 6; i += 2) /* restore dvb audio */
			{
				n = avstable_dvb[vendor][i];
				if ((ioctl(renderinfo.avs, avstable_ioctl[i], &n)) < 0)
					perror("TuxTxt <ioctl(avs)>");
			}
		}
	}


	if (hotlistchanged)
		savehotlist();

	/* save config */
	if (saveconfig || curscreenmode != savedscreenmode)
	{
		if ((conf = fopen(TUXTXTCONF, "wt")) == 0)
		{
			perror("TuxTxt <fopen tuxtxt.conf>");
		}
		else
		{
			printf("TuxTxt <saving config>\n");
			fprintf(conf, "ScreenMode16x9Normal %d\n", renderinfo.screen_mode1);
			fprintf(conf, "ScreenMode16x9Divided %d\n", renderinfo.screen_mode2);
			fprintf(conf, "Brightness %d\n", renderinfo.color_mode);
			fprintf(conf, "MenuLanguage %d\n", menulanguage);
			fprintf(conf, "AutoNational %d\n", renderinfo.auto_national);
			fprintf(conf, "NationalSubset %d\n", tuxtxt_cache.national_subset);
			fprintf(conf, "SwapUpDown %d\n", swapupdown);
			fprintf(conf, "ShowHexPages %d\n", renderinfo.showhex);
			fprintf(conf, "Transparency 0x%X\n", renderinfo.trans_mode);
			fprintf(conf, "TTFWidthFactor16 %d\n", renderinfo.TTFWidthFactor16);
			fprintf(conf, "TTFHeightFactor16 %d\n", renderinfo.TTFHeightFactor16);
			fprintf(conf, "TTFShiftX %d\n", renderinfo.TTFShiftX);
			fprintf(conf, "TTFShiftY %d\n", renderinfo.TTFShiftY);
			fprintf(conf, "Screenmode %d\n", curscreenmode);
			fprintf(conf, "ShowFLOF %d\n", renderinfo.showflof);
			fprintf(conf, "Show39 %d\n", renderinfo.show39);
			fprintf(conf, "ShowLevel2p5 %d\n", renderinfo.showl25);
			fprintf(conf, "DumpLevel2p5 %d\n", dumpl25);
			fprintf(conf, "UseTTF %d\n", renderinfo.usettf);
			fclose(conf);
		}
	}
	tuxtxt_EndRendering(&renderinfo);
}
/******************************************************************************
 * GetTeletextPIDs                                                           *
 ******************************************************************************/
int GetTeletextPIDs()
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
		close(dmx);
		return 0;
	}

	/* show infobar */
	RenderMessage(ShowInfoBar);

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
		close(dmx);
		return 0;
	}

	if (read(dmx, PAT, sizeof(PAT)) == -1)
	{
		perror("TuxTxt <read PAT>");
		ioctl(dmx, DMX_STOP);
		close(dmx);
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
		close(dmx);
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

		RenderMessage(NoServicesFound);
		sleep(3);
		ioctl(dmx, DMX_STOP);
		close(dmx);
		return 0;
	}

	/* read SDT to get servicenames */
	SDT_ready = 0;

	dmx_flt.pid              = 0x0011;
	dmx_flt.flags            = DMX_CHECK_CRC | DMX_IMMEDIATE_START;
	dmx_flt.filter.filter[0] = 0x42;
	dmx_flt.filter.mask[0]   = 0xFF;
	dmx_flt.timeout          = 5000;

	if (ioctl(dmx, DMX_SET_FILTER, &dmx_flt) == -1)
	{
		perror("TuxTxt <DMX_SET_FILTER SDT>");

		RenderMessage(ShowServiceName);
		ioctl(dmx, DMX_STOP);
		close(dmx);

		return 1;
	}

	first_sdt_sec = -1;
	while (1)
	{
		if (read(dmx, SDT, 3) == -1)
		{
			perror("TuxTxt <read SDT>");

			ioctl(dmx, DMX_STOP);
			close(dmx);
			RenderMessage(ShowServiceName);
			return 1;
		}

		if (read(dmx, SDT+3, ((SDT[1] & 0x0f) << 8) | SDT[2]) == -1)
		{
			perror("TuxTxt <read SDT>");

			ioctl(dmx, DMX_STOP);
			close(dmx);
			RenderMessage(ShowServiceName);
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
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'Ä')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x5B;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'ä')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x7B;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'Ö')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x5C;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'ö')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x7C;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'Ü')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x5D;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'ü')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x7D;
						if (SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] == (unsigned char)'ß')
							SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] = 0x7E;
						if (byte + diff >= 24 || SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] >= 0x80 && SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte] <= 0x9F)
							diff--;
						else
							pid_table[pid_test].service_name[byte + diff] = SDT[sdt_scan+10 + SDT[sdt_scan + 8] + byte];
					}

					pid_table[pid_test].service_name_len += diff;
				}
			}
		}
	}
	ioctl(dmx, DMX_STOP);
	SDT_ready = 1;

	/* show current servicename */
	current_service = 0;

	if (tuxtxt_cache.vtxtpid != 0)
	{
		while (pid_table[current_service].vtxt_pid != tuxtxt_cache.vtxtpid && current_service < pids_found)
			current_service++;

		if (renderinfo.auto_national && current_service < pids_found)
			tuxtxt_cache.national_subset = pid_table[current_service].national_subset;
		RenderMessage(ShowServiceName);
	}

	getpidsdone = 1;

	RenderCharLCD(pids_found/10,  7, 44);
	RenderCharLCD(pids_found%10, 19, 44);
	ioctl(dmx, DMX_STOP);
	close(dmx);

	return 1;
}

/******************************************************************************
 * GetNationalSubset                                                          *
 ******************************************************************************/

int GetNationalSubset(char *cc)
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

/******************************************************************************
 * ConfigMenu                                                                 *
 ******************************************************************************/
#if TUXTXT_DEBUG
void charpage()
{
	PosY = StartY;
	PosX = StartX;
	char cachefill[100];
	int fullsize =0,hexcount = 0, col, p,sp;
	int escpage = 0;
	tstCachedPage* pg;
	ClearFB(black);

	int zipsize = 0;
	for (p = 0; p < 0x900; p++)
	{
		for (sp = 0; sp < 0x80; sp++)
		{
			pg = tuxtxt_cache.astCachetable[p][sp];
			if (pg)
			{

				fullsize+=23*40;
				zipsize += tuxtxt_get_zipsize(p,sp);
			}
		}
	}


	memset(cachefill,' ',40);
	sprintf(cachefill,"f:%d z:%d h:%d c:%d %03x",fullsize, zipsize, hexcount, tuxtxt_cache.cached_pages, escpage);

	for (col = 0; col < 40; col++)
	{
		tuxtxt_RenderCharFB(&renderinfo,cachefill[col], &tuxtxt_atrtable[ATR_WB]);
	}
	tstPageAttr atr;
	memcpy(&atr,&tuxtxt_atrtable[ATR_WB],sizeof(tstPageAttr));
	int row;
	atr.charset = C_G0P;
	PosY = StartY+fontheight;
	for (row = 0; row < 16; row++)
	{
		PosY+= fontheight;
		SetPosX(1);
		for (col=0; col < 6; col++)
		{
			tuxtxt_RenderCharFB(&renderinfo,col*16+row+0x20, &atr);
		}
	}
	atr.setX26 = 1;
	PosY = StartY+fontheight;
	for (row = 0; row < 16; row++)
	{
		PosY+= fontheight;
		SetPosX(10);
		for (col=0; col < 6; col++)
		{
			tuxtxt_RenderCharFB(&renderinfo,col*16+row+0x20, &atr);
		}
	}
	PosY = StartY+fontheight;
	atr.charset = C_G2;
	atr.setX26 = 0;
	for (row = 0; row < 16; row++)
	{
		PosY+= fontheight;
		SetPosX(20);
		for (col=0; col < 6; col++)
		{
			tuxtxt_RenderCharFB(&renderinfo,col*16+row+0x20, &atr);
		}
	}
	atr.charset = C_G3;
	PosY = StartY+fontheight;
	for (row = 0; row < 16; row++)
	{
		PosY+= fontheight;
		SetPosX(30);
		for (col=0; col < 6; col++)
		{
			tuxtxt_RenderCharFB(&renderinfo,col*16+row+0x20, &atr);
		}
	}
	do
	{
		GetRCCode();
	}
	while (RCCode != RC_OK && RCCode != RC_HOME);
}
#endif
void Menu_HighlightLine(char *menu, int line, int high)
{
	int active_national_subset=tuxtxt_cache.national_subset;
	char hilitline[] = "0111111111111111111111111111102";
	int itext = Menu_Width*line; /* index start menuline */
	int byte;
	int national_subset_bak = tuxtxt_cache.national_subset;

	renderinfo.PosX = Menu_StartX;
	renderinfo.PosY = Menu_StartY + line*renderinfo.fontheight;
	if (line == MenuLine[M_NAT])
		tuxtxt_cache.national_subset = national_subset_bak;
	else
		tuxtxt_cache.national_subset = menusubset[menulanguage];
	if (line == MenuLine[M_PID] && getpidsdone) { // channel names should be rendered in NAT_DEFAULT
		active_national_subset=tuxtxt_cache.national_subset;
		tuxtxt_cache.national_subset=NAT_DEFAULT;
	}
	for (byte = 0; byte < Menu_Width; byte++)
		tuxtxt_RenderCharFB(&renderinfo,menu[itext + byte],
						 high ?
						 &tuxtxt_atrtable[hilitline[byte] - '0' + ATR_MENUHIL0] :
						 &tuxtxt_atrtable[menuatr[itext + byte] - '0' + ATR_MENU0]);
	if (line == MenuLine[M_PID] && getpidsdone) // restore national_subset
		tuxtxt_cache.national_subset=active_national_subset;
	tuxtxt_cache.national_subset = national_subset_bak;
}

void Menu_UpdateHotlist(char *menu, int hotindex, int menuitem)
{
	int i, j, k;
	tstPageAttr *attr;

	renderinfo.PosX = Menu_StartX + 6*renderinfo.fontwidth;
	renderinfo.PosY = Menu_StartY + (MenuLine[M_HOT]+1)*renderinfo.fontheight;
	j = Menu_Width*(MenuLine[M_HOT]+1) + 6; /* start index in menu */

	for (i = 0; i <= maxhotlist+1; i++)
	{
		if (i == maxhotlist+1) /* clear last+1 entry in case it was deleted */
		{
			attr = &tuxtxt_atrtable[ATR_MENU5];
			memset(&menu[j], ' ', 3);
		}
		else
		{
			if (i == hotindex)
				attr = &tuxtxt_atrtable[ATR_MENU1];
			else
			attr = &tuxtxt_atrtable[ATR_MENU5];
			tuxtxt_hex2str(&menu[j+2], hotlist[i]);
		}

		for (k = 0; k < 3; k++)
			tuxtxt_RenderCharFB(&renderinfo,menu[j+k], attr);

		if (i == 4)
		{
			renderinfo.PosX = Menu_StartX + 6*renderinfo.fontwidth;
			renderinfo.PosY += renderinfo.fontheight;
			j += 2*Menu_Width - 4*4;
		}
		else
		{
			j += 4; /* one space distance */
			renderinfo.PosX += renderinfo.fontwidth;
		}
	}

	tuxtxt_hex2str(&menu[Menu_Width*MenuLine[M_HOT] + hotlistpagecolumn[menulanguage]], (hotindex >= 0) ? hotlist[hotindex] : tuxtxt_cache.page);
	memcpy(&menu[Menu_Width*MenuLine[M_HOT] + hotlisttextcolumn[menulanguage]], &hotlisttext[menulanguage][(hotindex >= 0) ? 5 : 0], 5);
	renderinfo.PosX = Menu_StartX + 20*renderinfo.fontwidth;
	renderinfo.PosY = Menu_StartY + MenuLine[M_HOT]*renderinfo.fontheight;

	Menu_HighlightLine(menu, MenuLine[M_HOT], (menuitem == M_HOT) ? 1 : 0);
}

void Menu_Init(char *menu, int current_pid, int menuitem, int hotindex)
{
	int byte, line, name_len, active_national_subset;
	int national_subset_bak = tuxtxt_cache.national_subset;

	memcpy(menu, configmenu[menulanguage], Menu_Height*Menu_Width);

	if (getpidsdone)
	{
		memset(&menu[MenuLine[M_PID]*Menu_Width+3], 0x20,24);
		if (SDT_ready)
		{
			name_len = pid_table[current_pid].service_name_len < 24 ? pid_table[current_pid].service_name_len : 24;	// Maximum of 24 chars will fit
			memcpy(&menu[MenuLine[M_PID]*Menu_Width+3+(24-name_len)/2], &pid_table[current_pid].service_name, name_len);
		}
		else
			tuxtxt_hex2str(&menu[MenuLine[M_PID]*Menu_Width + 13 + 3], tuxtxt_cache.vtxtpid);
	}
	if (!getpidsdone || current_pid == 0 || pids_found == 1)
		menu[MenuLine[M_PID]*Menu_Width +  1] = ' ';

	if (!getpidsdone || current_pid == pids_found - 1 || pids_found == 1)
		menu[MenuLine[M_PID]*Menu_Width + 28] = ' ';


	/* set 16:9 modi, colors & national subset */
	memcpy(&menu[Menu_Width*MenuLine[M_SC1] + Menu_Width - 5], &configonoff[menulanguage][renderinfo.screen_mode1  ? 3 : 0], 3);
	memcpy(&menu[Menu_Width*MenuLine[M_SC2] + Menu_Width - 5], &configonoff[menulanguage][renderinfo.screen_mode2  ? 3 : 0], 3);

	menu[MenuLine[M_COL]*Menu_Width +  1] = (renderinfo.color_mode == 1  ? ' ' : 'í');
	menu[MenuLine[M_COL]*Menu_Width + 28] = (renderinfo.color_mode == 24 ? ' ' : 'î');
	memset(&menu[Menu_Width*MenuLine[M_COL] + 3             ], 0x7f,renderinfo.color_mode);
	memset(&menu[Menu_Width*MenuLine[M_COL] + 3+renderinfo.color_mode  ], 0x20,24-renderinfo.color_mode);
//	memcpy(&menu[Menu_Width*MenuLine[M_COL] + Menu_Width - 5], &configonoff[menulanguage][color_mode    ? 3 : 0], 3);
	menu[MenuLine[M_TRA]*Menu_Width +  1] = (renderinfo.trans_mode == 1  ? ' ' : 'í');
	menu[MenuLine[M_TRA]*Menu_Width + 28] = (renderinfo.trans_mode == 24 ? ' ' : 'î');
	memset(&menu[Menu_Width*MenuLine[M_TRA] + 3             ], 0x7f,renderinfo.trans_mode);
	memset(&menu[Menu_Width*MenuLine[M_TRA] + 3+renderinfo.trans_mode  ], 0x20,24-renderinfo.trans_mode);

	memcpy(&menu[Menu_Width*MenuLine[M_AUN] + Menu_Width - 5], &configonoff[menulanguage][renderinfo.auto_national ? 3 : 0], 3);
	memcpy(&menu[Menu_Width*MenuLine[M_NAT] + 2], &countrystring[tuxtxt_cache.national_subset*COUNTRYSTRING_WIDTH], COUNTRYSTRING_WIDTH);
	if (renderinfo.auto_national)
		menu[MenuLine[M_NAT]*Menu_Width +  1] = ' ';
	if (renderinfo.auto_national)
		menu[MenuLine[M_NAT]*Menu_Width + 28] = ' ';
	if (renderinfo.showhex)
		menu[MenuLine[M_PID]*Menu_Width + 27] = '?';
	/* render menu */
	renderinfo.PosY = Menu_StartY;
	for (line = 0; line < Menu_Height; line++)
	{
		renderinfo.PosX = Menu_StartX;
		if (line == MenuLine[M_NAT])
			tuxtxt_cache.national_subset = national_subset_bak;
		else if (line == MenuLine[M_TRA] || line == MenuLine[M_COL])
			tuxtxt_cache.national_subset = 0;
		else
			tuxtxt_cache.national_subset = menusubset[menulanguage];

		if ((line == MenuLine[M_PID]) && getpidsdone) {
			active_national_subset=tuxtxt_cache.national_subset;
			tuxtxt_cache.national_subset = NAT_DEFAULT;
		}
			
		if (line == Menu_Height-2) { // version info should be rendered in NAT_DEFAULT always
			memcpy(&menu[line*Menu_Width + 20], versioninfo, 5);
			tuxtxt_cache.national_subset=NAT_DEFAULT;
		}

		for (byte = 0; byte < Menu_Width; byte++)
			tuxtxt_RenderCharFB(&renderinfo,menu[line*Menu_Width + byte], &tuxtxt_atrtable[menuatr[line*Menu_Width + byte] - '0' + ATR_MENU0]);

		if ((line == MenuLine[M_PID]) && getpidsdone) { //restore charset
			tuxtxt_cache.national_subset=active_national_subset;
		}
		if (line == Menu_Height-2) {
			tuxtxt_cache.national_subset=active_national_subset;
		}

		renderinfo.PosY += renderinfo.fontheight;
	}
	tuxtxt_cache.national_subset = national_subset_bak;
	Menu_HighlightLine(menu, MenuLine[menuitem], 1);
	Menu_UpdateHotlist(menu, hotindex, menuitem);
}

void ConfigMenu(int Init)
{
	int val, menuitem = M_Start;
	int current_pid = 0;
	int hotindex;
	int oldscreenmode;
	int i, name_len;
	int national_subset_bak = tuxtxt_cache.national_subset;
	char menu[Menu_Height*Menu_Width];

	if (renderinfo.auto_national && tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage] &&
		tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.nationalvalid)
		tuxtxt_cache.national_subset = countryconversiontable[tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.national];

	if (getpidsdone)
	{
		/* set current vtxt */
		if (tuxtxt_cache.vtxtpid == 0)
			tuxtxt_cache.vtxtpid = pid_table[0].vtxt_pid;
		else
			while(pid_table[current_pid].vtxt_pid != tuxtxt_cache.vtxtpid && current_pid < pids_found)
				current_pid++;
	}

	/* reset to normal mode */
	if (renderinfo.zoommode)
		renderinfo.zoommode = 0;

	if (renderinfo.transpmode)
	{
		renderinfo.transpmode = 0;
		tuxtxt_ClearBB(&renderinfo,tuxtxt_color_black);
	}

	oldscreenmode = renderinfo.screenmode;
	if (renderinfo.screenmode)
		tuxtxt_SwitchScreenMode(&renderinfo,0); /* turn off divided screen */

	hotindex = getIndexOfPageInHotlist();

	/* clear framebuffer */
	tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);
	renderinfo.clearbbcolor = tuxtxt_color_black;
	Menu_Init(menu, current_pid, menuitem, hotindex);

	/* set blocking mode */
	val = fcntl(rc, F_GETFL);
	fcntl(rc, F_SETFL, val &~ O_NONBLOCK);

	/* loop */
	do {
		if (GetRCCode() == 1)
		{

			if (
#if (RC_1 > 0)
				RCCode >= RC_1 && /* generates a warning... */
#endif
				RCCode <= RC_1+M_MaxDirect) /* direct access */
			{
				Menu_HighlightLine(menu, MenuLine[menuitem], 0);
				menuitem = RCCode-RC_1;
				Menu_HighlightLine(menu, MenuLine[menuitem], 1);

				if (menuitem != M_PID) /* just select */
					RCCode = RC_OK;
			}

			switch (RCCode)
			{
			case RC_UP:
				Menu_HighlightLine(menu, MenuLine[menuitem], 0);
				if (--menuitem < 0)
					menuitem = M_Number-1;
				if (renderinfo.auto_national && (menuitem == M_NAT))
					menuitem--;
				Menu_HighlightLine(menu, MenuLine[menuitem], 1);
				break;

			case RC_DOWN:
				Menu_HighlightLine(menu, MenuLine[menuitem], 0);
				if (++menuitem > M_Number-1)
					menuitem = 0;
				if (renderinfo.auto_national && (menuitem == M_NAT))
					menuitem++;
				Menu_HighlightLine(menu, MenuLine[menuitem], 1);
				break;

			case RC_LEFT:
				switch (menuitem)
				{
				case M_COL:
					saveconfig = 1;
					renderinfo.color_mode--;
					if (renderinfo.color_mode < 1) renderinfo.color_mode = 1;
					menu[MenuLine[M_COL]*Menu_Width +  1] = (renderinfo.color_mode == 1  ? ' ' : 'í');
					menu[MenuLine[M_COL]*Menu_Width + 28] = (renderinfo.color_mode == 24 ? ' ' : 'î');
					memset(&menu[Menu_Width*MenuLine[M_COL] + 3             ], 0x7f,renderinfo.color_mode);
					memset(&menu[Menu_Width*MenuLine[M_COL] + 3+renderinfo.color_mode  ], 0x20,24-renderinfo.color_mode);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
					tuxtxt_setcolors(&renderinfo,(unsigned short *)tuxtxt_defaultcolors, 0, tuxtxt_color_SIZECOLTABLE);
					break;
				case M_TRA:
					saveconfig = 1;
					renderinfo.trans_mode--;
					if (renderinfo.trans_mode < 1) renderinfo.trans_mode = 1;
					menu[MenuLine[M_TRA]*Menu_Width +  1] = (renderinfo.trans_mode == 1  ? ' ' : 'í');
					menu[MenuLine[M_TRA]*Menu_Width + 28] = (renderinfo.trans_mode == 24 ? ' ' : 'î');
					memset(&menu[Menu_Width*MenuLine[M_TRA] + 3             ], 0x7f,renderinfo.trans_mode);
					memset(&menu[Menu_Width*MenuLine[M_TRA] + 3+renderinfo.trans_mode  ], 0x20,24-renderinfo.trans_mode);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
					tuxtxt_setcolors(&renderinfo,(unsigned short *)tuxtxt_defaultcolors, 0, tuxtxt_color_SIZECOLTABLE);
					break;
				case M_PID:
				{
					if (!getpidsdone)
					{
						GetTeletextPIDs();
						tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);
						/* set current vtxt */
						if (tuxtxt_cache.vtxtpid == 0)
							tuxtxt_cache.vtxtpid = pid_table[0].vtxt_pid;
						else
							while(pid_table[current_pid].vtxt_pid != tuxtxt_cache.vtxtpid && current_pid < pids_found)
								current_pid++;
						Menu_Init(menu, current_pid, menuitem, hotindex);
					}
					if (current_pid > 0)
					{
						current_pid--;

						memset(&menu[MenuLine[M_PID]*Menu_Width + 3], ' ', 24);

						if (SDT_ready)
						{
							name_len = pid_table[current_pid].service_name_len < 24 ? pid_table[current_pid].service_name_len : 24;	// Maximum of 24 chars will fit
							memcpy(&menu[MenuLine[M_PID]*Menu_Width+3+
								(24-name_len)/2],
								&pid_table[current_pid].service_name, name_len);
						}
						else
							tuxtxt_hex2str(&menu[MenuLine[M_PID]*Menu_Width + 13 + 3], tuxtxt_cache.vtxtpid);

						if (pids_found > 1)
						{
							if (current_pid == 0)
							{
								menu[MenuLine[M_PID]*Menu_Width +  1] = ' ';
								menu[MenuLine[M_PID]*Menu_Width + 28] = 'î';
							}
							else
							{
								menu[MenuLine[M_PID]*Menu_Width +  1] = 'í';
								menu[MenuLine[M_PID]*Menu_Width + 28] = 'î';
							}
						}

						Menu_HighlightLine(menu, MenuLine[menuitem], 1);

						if (renderinfo.auto_national)
						{
							tuxtxt_cache.national_subset = pid_table[current_pid].national_subset;

							memcpy(&menu[Menu_Width*MenuLine[M_NAT] + 2], &countrystring[tuxtxt_cache.national_subset*COUNTRYSTRING_WIDTH], COUNTRYSTRING_WIDTH);
							Menu_HighlightLine(menu, MenuLine[M_NAT], 0);
						}
					}
					break;
				}

				case M_NAT:
					saveconfig = 1;
					if (tuxtxt_cache.national_subset >= 0)
					{
						tuxtxt_cache.national_subset--;

						if (tuxtxt_cache.national_subset < 0)
							tuxtxt_cache.national_subset = MAX_NATIONAL_SUBSET;
						menu[MenuLine[M_NAT]*Menu_Width +  1] = 'í';
						menu[MenuLine[M_NAT]*Menu_Width + 28] = 'î';
						Menu_Init(menu, current_pid, menuitem, hotindex);
					}
					break;

				case M_HOT: /* move towards top of hotlist */
					if (hotindex <= 0) /* if not found, start at end */
						hotindex = maxhotlist;
					else
						hotindex--;
					Menu_UpdateHotlist(menu, hotindex, menuitem);
					break;

				case M_LNG:
					saveconfig = 1;
					if (--menulanguage < 0)
						menulanguage = MAXMENULANGUAGE;
					Menu_Init(menu, current_pid, menuitem, hotindex);
					break;
				} /* switch menuitem */
				break; /* RC_LEFT */

			case RC_RIGHT:
				switch (menuitem)
				{
				case M_COL:
					saveconfig = 1;
					renderinfo.color_mode++;
					if (renderinfo.color_mode > 24) renderinfo.color_mode = 24;
					menu[MenuLine[M_COL]*Menu_Width +  1] = (renderinfo.color_mode == 1  ? ' ' : 'í');
					menu[MenuLine[M_COL]*Menu_Width + 28] = (renderinfo.color_mode == 24 ? ' ' : 'î');
					memset(&menu[Menu_Width*MenuLine[M_COL] + 3             ], 0x7f,renderinfo.color_mode);
					memset(&menu[Menu_Width*MenuLine[M_COL] + 3+renderinfo.color_mode  ], 0x20,24-renderinfo.color_mode);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
					tuxtxt_setcolors(&renderinfo,(unsigned short *)tuxtxt_defaultcolors, 0, tuxtxt_color_SIZECOLTABLE);
					break;
				case M_TRA:
					saveconfig = 1;
					renderinfo.trans_mode++;
					if (renderinfo.trans_mode > 24) renderinfo.trans_mode = 24;
					menu[MenuLine[M_TRA]*Menu_Width +  1] = (renderinfo.trans_mode == 1  ? ' ' : 'í');
					menu[MenuLine[M_TRA]*Menu_Width + 28] = (renderinfo.trans_mode == 24 ? ' ' : 'î');
					memset(&menu[Menu_Width*MenuLine[M_TRA] + 3             ], 0x7f,renderinfo.trans_mode);
					memset(&menu[Menu_Width*MenuLine[M_TRA] + 3+renderinfo.trans_mode  ], 0x20,24-renderinfo.trans_mode);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
					tuxtxt_setcolors(&renderinfo,(unsigned short *)tuxtxt_defaultcolors, 0, tuxtxt_color_SIZECOLTABLE);
					break;
				case M_PID:
					if (!getpidsdone)
					{
						GetTeletextPIDs();
						tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);
						/* set current vtxt */
						if (tuxtxt_cache.vtxtpid == 0)
							tuxtxt_cache.vtxtpid = pid_table[0].vtxt_pid;
						else
							while(pid_table[current_pid].vtxt_pid != tuxtxt_cache.vtxtpid && current_pid < pids_found)
								current_pid++;
						Menu_Init(menu, current_pid, menuitem, hotindex);
					}
					if (current_pid < pids_found - 1)
					{
						current_pid++;

						memset(&menu[MenuLine[M_PID]*Menu_Width + 3], ' ', 24);

						if (SDT_ready)
						{
							name_len = pid_table[current_pid].service_name_len < 24 ? pid_table[current_pid].service_name_len : 24;	// Maximum of 24 chars will fit
							memcpy(&menu[MenuLine[M_PID]*Menu_Width + 3 +
								(24-name_len)/2],
								&pid_table[current_pid].service_name, name_len);
						}
						else
							tuxtxt_hex2str(&menu[MenuLine[M_PID]*Menu_Width + 13 + 3], pid_table[current_pid].vtxt_pid);

						if (pids_found > 1)
						{
							if (current_pid == pids_found - 1)
							{
								menu[MenuLine[M_PID]*Menu_Width +  1] = 'í';
								menu[MenuLine[M_PID]*Menu_Width + 28] = ' ';
							}
							else
							{
								menu[MenuLine[M_PID]*Menu_Width +  1] = 'í';
								menu[MenuLine[M_PID]*Menu_Width + 28] = 'î';
							}
						}

						Menu_HighlightLine(menu, MenuLine[menuitem], 1);

						if (renderinfo.auto_national)
						{
							if (getpidsdone)
								tuxtxt_cache.national_subset = pid_table[current_pid].national_subset;
							memcpy(&menu[Menu_Width*MenuLine[M_NAT] + 2], &countrystring[tuxtxt_cache.national_subset*COUNTRYSTRING_WIDTH], COUNTRYSTRING_WIDTH);
							Menu_HighlightLine(menu, MenuLine[M_NAT], 0);
						}
					}
					break;

				case M_NAT:
					saveconfig = 1;
					if (tuxtxt_cache.national_subset <= MAX_NATIONAL_SUBSET)
					{
						tuxtxt_cache.national_subset++;

						if (tuxtxt_cache.national_subset > MAX_NATIONAL_SUBSET)
							tuxtxt_cache.national_subset = NAT_DEFAULT;
						menu[MenuLine[M_NAT]*Menu_Width +  1] = 'í';
						menu[MenuLine[M_NAT]*Menu_Width + 28] = 'î';
						Menu_Init(menu, current_pid, menuitem, hotindex);
					}
					break;

				case M_HOT: /* select hotindex */
					if ((unsigned int)hotindex >= maxhotlist) /* if not found, start at 0 */
						hotindex = 0;
					else
						hotindex++;
					Menu_UpdateHotlist(menu, hotindex, menuitem);
					break;

				case M_LNG:
					saveconfig = 1;
					if (++menulanguage > MAXMENULANGUAGE)
						menulanguage = 0;
					Menu_Init(menu, current_pid, menuitem, hotindex);
					break;
				}
				break; /* RC_RIGHT */

			case RC_PLUS:
				switch (menuitem)
				{
				case M_HOT: /* move towards end of hotlist */
				{
					if (hotindex<0) /* not found: add page at end */
					{
						if (maxhotlist < (sizeof(hotlist)/sizeof(hotlist[0])-1)) /* only if still room left */
						{
							hotindex = ++maxhotlist;
							hotlist[hotindex] = tuxtxt_cache.page;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
					else /* found */
					{
						if (hotindex < maxhotlist) /* not already at end */
						{
							int temp = hotlist[hotindex];
							hotlist[hotindex] = hotlist[hotindex+1];
							hotlist[hotindex+1] = temp;
							hotindex++;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
				}
				break;
				}
				break; /* RC_PLUS */

			case RC_MINUS:
				switch (menuitem)
				{
				case M_HOT: /* move towards top of hotlist */
				{
					if (hotindex<0) /* not found: add page at top */
					{
						if (maxhotlist < (sizeof(hotlist)/sizeof(hotlist[0])-1)) /* only if still room left */
						{
							for (hotindex = maxhotlist; hotindex >= 0; hotindex--) /* move rest of list */
							{
								hotlist[hotindex+1] = hotlist[hotindex];
							}
							maxhotlist++;
							hotindex = 0;
							hotlist[hotindex] = tuxtxt_cache.page;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
					else /* found */
					{
						if (hotindex > 0) /* not already at front */
						{
							int temp = hotlist[hotindex];
							hotlist[hotindex] = hotlist[hotindex-1];
							hotlist[hotindex-1] = temp;
							hotindex--;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
				}
				break;
				}
				break; /* RC_MINUS */

			case RC_HELP:
				switch (menuitem)
				{
				case M_HOT: /* current page is added to / removed from hotlist */
				{
					if (hotindex<0) /* not found: add page */
					{
						if (maxhotlist < (sizeof(hotlist)/sizeof(hotlist[0])-1)) /* only if still room left */
						{
							hotlist[++maxhotlist] = tuxtxt_cache.page;
							hotindex = maxhotlist;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
					else /* found: remove */
					{
						if (maxhotlist > 0) /* don't empty completely */
						{
							int i;

							for (i=hotindex; i<maxhotlist; i++) /* move rest of list */
							{
								hotlist[i] = hotlist[i+1];
							}
							maxhotlist--;
							if (hotindex > maxhotlist)
								hotindex = maxhotlist;
							hotlistchanged = 1;
							Menu_UpdateHotlist(menu, hotindex, menuitem);
						}
					}
				}
				break;
				case M_PID:
					renderinfo.showhex ^= 1;
					menu[MenuLine[M_PID]*Menu_Width + 27] = (renderinfo.showhex ? '?' : ' ');
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
				break;
#if TUXTXT_DEBUG
				case M_LNG:
					charpage();
					ClearFB(transp);
					Menu_Init(menu, current_pid, menuitem, hotindex);
				break;
#endif
				}
				break; /* RC_MUTE */

			case RC_OK:
				switch (menuitem)
				{
				case M_PID:
					if (!getpidsdone)
					{
						GetTeletextPIDs();
						tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);
						/* set current vtxt */
						if (tuxtxt_cache.vtxtpid == 0)
							tuxtxt_cache.vtxtpid = pid_table[0].vtxt_pid;
						else
							while(pid_table[current_pid].vtxt_pid != tuxtxt_cache.vtxtpid && current_pid < pids_found)
								current_pid++;
						Menu_Init(menu, current_pid, menuitem, hotindex);
					}
					else if (pids_found > 1)
					{
							if (hotlistchanged)
								savehotlist();

						if (Init || tuxtxt_cache.vtxtpid != pid_table[current_pid].vtxt_pid)
							{
#if TUXTXT_CFG_STANDALONE
								tuxtxt_stop_thread();
								tuxtxt_clear_cache();
#else
								tuxtxt_stop();
							if (Init)
								tuxtxt_cache.vtxtpid = 0; // force clear cache
#endif
								/* reset data */


								//page_atrb[32] = transp<<4 | transp;
								renderinfo.inputcounter = 2;


								tuxtxt_cache.page     = 0x100;
								lastpage = 0x100;
								renderinfo.prev_100 = 0x100;
								renderinfo.prev_10  = 0x100;
								renderinfo.next_100 = 0x100;
								renderinfo.next_10  = 0x100;
								tuxtxt_cache.subpage  = 0;

								tuxtxt_cache.pageupdate = 0;
								tuxtxt_cache.zap_subpage_manual = 0;
								renderinfo.hintmode = 0;
								memset(renderinfo.page_char,' ',40 * 25);

								for (i = 0; i < 40*25; i++)
								{
									renderinfo.page_atrb[i].fg = tuxtxt_color_transp;
									renderinfo.page_atrb[i].bg = tuxtxt_color_transp;
								}
								tuxtxt_ClearFB(&renderinfo,tuxtxt_color_transp);


								/* start demuxer with new vtxtpid */
								if (renderinfo.auto_national)
									tuxtxt_cache.national_subset = pid_table[current_pid].national_subset;

#if TUXTXT_CFG_STANDALONE
								tuxtxt_cache.vtxtpid = pid_table[current_pid].vtxt_pid;
								tuxtxt_start_thread();
#else
								tuxtxt_start(pid_table[current_pid].vtxt_pid);
#endif
							}
//							tuxtxt_cache.pageupdate = 1;

							tuxtxt_ClearBB(&renderinfo,tuxtxt_color_black);
							gethotlist();

						/* show new teletext */
						current_service = current_pid;
//						RenderMessage(ShowServiceName);

						fcntl(rc, F_SETFL, O_NONBLOCK);
						RCCode = -1;
						if (oldscreenmode)
							tuxtxt_SwitchScreenMode(&renderinfo,oldscreenmode); /* restore divided screen */
						return;
					}
					break;

				case M_SC1:
					saveconfig = 1;
					renderinfo.screen_mode1++;
					renderinfo.screen_mode1 &= 1;

					memcpy(&menu[Menu_Width*MenuLine[M_SC1] + Menu_Width - 5], &configonoff[menulanguage][renderinfo.screen_mode1  ? 3 : 0], 3);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);

					ioctl(renderinfo.avs, AVSIOSSCARTPIN8, &fncmodes[renderinfo.screen_mode1]);
					ioctl(renderinfo.saa, SAAIOSWSS, &saamodes[renderinfo.screen_mode1]);

					break;

				case M_SC2:
					saveconfig = 1;
					renderinfo.screen_mode2++;
					renderinfo.screen_mode2 &= 1;

					memcpy(&menu[Menu_Width*MenuLine[M_SC2] + Menu_Width - 5], &configonoff[menulanguage][renderinfo.screen_mode2  ? 3 : 0], 3);
					Menu_HighlightLine(menu, MenuLine[menuitem], 1);
					break;


				case M_AUN:
					saveconfig = 1;
					renderinfo.auto_national++;
					renderinfo.auto_national &= 1;
					if (renderinfo.auto_national)
					{
					 	if (getpidsdone)
							tuxtxt_cache.national_subset = pid_table[current_pid].national_subset;
						else
						{
							if (tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage] &&
								tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.nationalvalid)
								tuxtxt_cache.national_subset = countryconversiontable[tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.national];
							else
								tuxtxt_cache.national_subset = national_subset_bak;
						}

					}
					Menu_Init(menu, current_pid, menuitem, hotindex);
					break;
				case M_HOT: /* show selected page */
				{
					if (hotindex >= 0) /* not found: ignore */
					{
						lastpage = tuxtxt_cache.page;
						tuxtxt_cache.page = hotlist[hotindex];
						tuxtxt_cache.subpage = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
						renderinfo.inputcounter = 2;
						tuxtxt_cache.pageupdate = 1;
						RCCode = RC_HOME;		 /* leave menu */
					}
				}
				break;
				} /* RC_OK */
				break;
			}
		}
		UpdateLCD(); /* update number of cached pages */
	} while ((RCCode != RC_HOME) && (RCCode != RC_DBOX) && (RCCode != RC_MUTE));

	/* reset to nonblocking mode */
	fcntl(rc, F_SETFL, O_NONBLOCK);
	tuxtxt_cache.pageupdate = 1;
	RCCode = -1;
	if (oldscreenmode)
		tuxtxt_SwitchScreenMode(&renderinfo,oldscreenmode); /* restore divided screen */
}

/******************************************************************************
 * PageInput                                                                  *
 ******************************************************************************/

void PageInput(int Number)
{
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

		if (temp_page<0 || temp_page==maxhotlist) /* from any (other) page go to first page in hotlist */
			temp_page = (maxhotlist >= 0) ? hotlist[0] : 0x100;
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
#if TUXTXT_DEBUG
			printf("TuxTxt <DirectInput: %.3X-%.2X>\n", tuxtxt_cache.page, tuxtxt_cache.subpage);
#endif
		}
		else
		{
			tuxtxt_cache.subpage = 0;
//			RenderMessage(PageNotFound);
#if TUXTXT_DEBUG
			printf("TuxTxt <DirectInput: %.3X not found>\n", tuxtxt_cache.page);
#endif
		}
	}
}

/******************************************************************************
 * GetNextPageOne                                                             *
 ******************************************************************************/

void GetNextPageOne(int up)
{
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
#if TUXTXT_DEBUG
		printf("TuxTxt <NextPageOne: %.3X-%.2X>\n", tuxtxt_cache.page, tuxtxt_cache.subpage);
#endif
	}
}

/******************************************************************************
 * GetNextSubPage                                                             *
 ******************************************************************************/
void GetNextSubPage(int offset)
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
#if TUXTXT_DEBUG
			printf("TuxTxt <NextSubPage: %.3X-%.2X>\n", tuxtxt_cache.page, tuxtxt_cache.subpage);
#endif
			return;
		}
	}

#if TUXTXT_DEBUG
	printf("TuxTxt <NextSubPage: no other SubPage>\n");
#endif
}
/******************************************************************************
 * ColorKey                                                                   *
 ******************************************************************************/

void ColorKey(int target)
{
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
#if TUXTXT_DEBUG
	printf("TuxTxt <ColorKey: %.3X>\n", tuxtxt_cache.page);
#endif
}

/******************************************************************************
 * PageCatching                                                               *
 ******************************************************************************/

void PageCatching()
{
	int active_national_subset=tuxtxt_cache.national_subset;
	int val, byte;
	int oldzoommode = renderinfo.zoommode;

	renderinfo.pagecatching = 1;

	/* abort pageinput */
	renderinfo.inputcounter = 2;

	/* show info line */
	renderinfo.zoommode = 0;
	renderinfo.PosX = renderinfo.StartX;
	renderinfo.PosY = renderinfo.StartY + 24*renderinfo.fontheight;
	for (byte = 0; byte < 40-renderinfo.nofirst; byte++) {
		tuxtxt_cache.national_subset=menusubset[menulanguage]; //render page catching line in correct language
		tuxtxt_RenderCharFB(&renderinfo,catchmenutext[menulanguage][byte], &tuxtxt_atrtable[catchmenutext[menulanguage][byte+40] - '0' + ATR_CATCHMENU0]);
		tuxtxt_cache.national_subset=active_national_subset;
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
		renderinfo.pagecatching = 0;
		tuxtxt_cache.pageupdate = 1;
		return;
	}

	/* set blocking mode */
	val = fcntl(rc, F_GETFL);
	fcntl(rc, F_SETFL, val &~ O_NONBLOCK);

	/* loop */
	do {
		GetRCCode();

		switch (RCCode)
		{
		case RC_LEFT:
			CatchNextPage(0, -1);
			break;
		case RC_RIGHT:
			CatchNextPage(0, 1);
			break;
		case RC_UP:
			CatchNextPage(-1, -1);
			break;
		case RC_DOWN:
			CatchNextPage(1, 1);
			break;
		case RC_0:
		case RC_1:
		case RC_2:
		case RC_3:
		case RC_4:
		case RC_5:
		case RC_6:
		case RC_7:
		case RC_8:
		case RC_9:
		case RC_RED:
		case RC_GREEN:
		case RC_YELLOW:
		case RC_BLUE:
		case RC_PLUS:
		case RC_MINUS:
		case RC_DBOX:
		case RC_HOME:
		case RC_HELP:
		case RC_MUTE:
			fcntl(rc, F_SETFL, O_NONBLOCK);
			tuxtxt_cache.pageupdate = 1;
			renderinfo.pagecatching = 0;
			RCCode = -1;
			return;
		}
		UpdateLCD();
	} while (RCCode != RC_OK);

	/* set new page */
	if (renderinfo.zoommode == 2)
		renderinfo.zoommode = 1;

	lastpage     = tuxtxt_cache.page;
	tuxtxt_cache.page         = catched_page;
	renderinfo.hintmode = 0;
	tuxtxt_cache.pageupdate = 1;
	renderinfo.pagecatching = 0;

	int subp = tuxtxt_cache.subpagetable[tuxtxt_cache.page];
	if (subp != 0xFF)
		tuxtxt_cache.subpage = subp;
	else
		tuxtxt_cache.subpage = 0;

	/* reset to nonblocking mode */
	fcntl(rc, F_SETFL, O_NONBLOCK);
}

/******************************************************************************
 * CatchNextPage                                                              *
 ******************************************************************************/

void CatchNextPage(int firstlineinc, int inc)
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
#if TUXTXT_DEBUG
				printf("TuxTxt <PageCatching: %.3X\n", catched_page);
#endif
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
#if TUXTXT_DEBUG
				printf("TuxTxt <PageCatching: no PageNumber>\n");
#endif
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
#if TUXTXT_DEBUG
				printf("TuxTxt <PageCatching: no PageNumber>\n");
#endif
				return;
			}
		}
	}
}


/******************************************************************************
 * RenderCatchedPage                                                          *
 ******************************************************************************/

void RenderCatchedPage()
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

/******************************************************************************
 * SwitchZoomMode                                                             *
 ******************************************************************************/

void SwitchZoomMode()
{
	if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] != 0xFF)
	{
		/* toggle mode */
		renderinfo.zoommode++;

		if (renderinfo.zoommode == 3)
			renderinfo.zoommode = 0;

#if TUXTXT_DEBUG
		printf("TuxTxt <SwitchZoomMode: %d>\n", renderinfo.zoommode);
#endif
		/* update page */
		tuxtxt_cache.pageupdate = 1; /* FIXME */
	}
}


/******************************************************************************
 * SwitchTranspMode                                                           *
 ******************************************************************************/

void SwitchTranspMode()
{
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

#if TUXTXT_DEBUG
	printf("TuxTxt <SwitchTranspMode: %d>\n", renderinfo.transpmode);
#endif

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

/******************************************************************************
 * SwitchHintMode                                                             *
 ******************************************************************************/

void SwitchHintMode()
{
	/* toggle mode */
	renderinfo.hintmode ^= 1;
#if TUXTXT_DEBUG
	printf("TuxTxt <SwitchHintMode: %d>\n", renderinfo.hintmode);
#endif

	if (!renderinfo.hintmode)	/* toggle evaluation of level 2.5 information by explicitly switching off hintmode */
	{
		renderinfo.showl25 ^= 1;
		saveconfig = 1;
#if TUXTXT_DEBUG
		printf("TuxTxt <ShowLevel2p5: %d>\n", renderinfo.showl25);
#endif
	}
	/* update page */
	tuxtxt_cache.pageupdate = 1;
}




/******************************************************************************
 * RenderCharLCD                                                             *
 ******************************************************************************/

void RenderCharLCD(int Digit, int XPos, int YPos)
{
	int x, y;

	/* render digit to lcd backbuffer */
	for (y = 0; y < 15; y++)
	{
		for (x = 0; x < 10; x++)
		{
			if (lcd_digits[Digit*15*10 + x + y*10])
				lcd_backbuffer[XPos + x + ((YPos+y)/8)*120] |= 1 << ((YPos+y)%8);
			else
				lcd_backbuffer[XPos + x + ((YPos+y)/8)*120] &= ~(1 << ((YPos+y)%8));
		}
	}
}

#if 0
void RenderCharLCDsmall(int Char, int XPos, int YPos)
{
	int old_width = fontwidth;
	int old_height = fontheight;
	setfontwidth(fontwidth_small_lcd);
	typettf.font.pix_height = fontheight = fontwidth_small_lcd;
	RenderChar(Char, 0, 0, -(YPos<<8 | XPos));
	setfontwidth(old_width);
	typettf.font.pix_height = fontheight = old_height;
}
#endif

/******************************************************************************
 * RenderMessage                                                              *
 ******************************************************************************/

void RenderMessage(int Message)
{
	int byte;
	int fbcolor, timecolor, menuatr;
	int pagecolumn;
	const char *msg;


/*                     00000000001111111111222222222233333333334 */
/*                     01234567890123456789012345678901234567890 */
	char message_1[] = "àááááááá www.tuxtxt.net x.xxx ááááááâè";
	char message_2[] = "ã                                   äé";
/* 	char message_3[] = "ã   suche nach Teletext-Anbietern   äé"; */
	char message_4[] = "ã                                   äé";
	char message_5[] = "åæææææææææææææææææææææææææææææææææææçé";
	char message_6[] = "ëììììììììììììììììììììììììììììììììììììê";

/* 	char message_7[] = "ã kein Teletext auf dem Transponder äé"; */
/* 	char message_8[] = "ã  warte auf Empfang von Seite 100  äé"; */
/* 	char message_9[] = "ã     Seite 100 existiert nicht!    äé"; */

	memcpy(&message_1[24], versioninfo, 5);
	/* reset zoom */
	renderinfo.zoommode = 0;

	/* set colors */
#ifdef HAVE_DBOX_HARDWARE
	if (renderinfo.screenmode)
	{
		fbcolor   = tuxtxt_color_black;
		timecolor = tuxtxt_color_black<<4 | tuxtxt_color_black;
		menuatr = ATR_MSGDRM0;
	}
	else
#endif
	{
		fbcolor   = tuxtxt_color_transp;
		timecolor = tuxtxt_color_transp<<4 | tuxtxt_color_transp;
		menuatr = ATR_MSG0;
	}

	/* clear framebuffer */
	tuxtxt_ClearFB(&renderinfo,fbcolor);

	/* hide header */
	renderinfo.page_atrb[32].fg = tuxtxt_color_transp;
	renderinfo.page_atrb[32].bg = tuxtxt_color_transp;


	/* set pagenumber */
	if (Message == ShowServiceName)
	{
		pagecolumn = message8pagecolumn[menulanguage];
		msg = message_8[menulanguage];
		memcpy(&message_4, msg, sizeof(message_4));
		tuxtxt_hex2str(message_4+pagecolumn, tuxtxt_cache.page);

		if (SDT_ready)
			memcpy(&message_2[2 + (35 - pid_table[current_service].service_name_len)/2],
					 &pid_table[current_service].service_name, pid_table[current_service].service_name_len);
		else if (Message == ShowServiceName)
			tuxtxt_hex2str(&message_2[17+3], tuxtxt_cache.vtxtpid);

		msg = &message_3_blank[0];
	}
	else if (Message == NoServicesFound)
		msg = &message_7[menulanguage][0];
	else
		msg = &message_3[menulanguage][0];

	/* render infobar */
	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*16;

	int active_national_subset=tuxtxt_cache.national_subset;	// version string should be rendered in NAT_DEFAULT
	tuxtxt_cache.national_subset=NAT_DEFAULT;

	for (byte = 0; byte < 37; byte++)
		tuxtxt_RenderCharFB(&renderinfo,message_1[byte], &tuxtxt_atrtable[menuatr + ((byte >= 9 && byte <= 28) ? 1 : 0)]);
	tuxtxt_RenderCharFB(&renderinfo,message_1[37], &tuxtxt_atrtable[menuatr + 2]);

	tuxtxt_cache.national_subset=menusubset[menulanguage]; //render message in menulanguage

	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*17;
	tuxtxt_RenderCharFB(&renderinfo,message_2[0], &tuxtxt_atrtable[menuatr + 0]);
	for (byte = 1; byte < 36; byte++)
		tuxtxt_RenderCharFB(&renderinfo,message_2[byte], &tuxtxt_atrtable[menuatr + 3]);
	tuxtxt_RenderCharFB(&renderinfo,message_2[36], &tuxtxt_atrtable[menuatr + 0]);
	tuxtxt_RenderCharFB(&renderinfo,message_2[37], &tuxtxt_atrtable[menuatr + 2]);

	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*18;
	tuxtxt_RenderCharFB(&renderinfo,msg[0], &tuxtxt_atrtable[menuatr + 0]);
	for (byte = 1; byte < 36; byte++)
		tuxtxt_RenderCharFB(&renderinfo,msg[byte], &tuxtxt_atrtable[menuatr + 3]);
	tuxtxt_RenderCharFB(&renderinfo,msg[36], &tuxtxt_atrtable[menuatr + 0]);
	tuxtxt_RenderCharFB(&renderinfo,msg[37], &tuxtxt_atrtable[menuatr + 2]);

	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*19;
	tuxtxt_RenderCharFB(&renderinfo,message_4[0], &tuxtxt_atrtable[menuatr + 0]);
	for (byte = 1; byte < 36; byte++)
		tuxtxt_RenderCharFB(&renderinfo,message_4[byte], &tuxtxt_atrtable[menuatr + 3]);
	tuxtxt_RenderCharFB(&renderinfo,message_4[36], &tuxtxt_atrtable[menuatr + 0]);
	tuxtxt_RenderCharFB(&renderinfo,message_4[37], &tuxtxt_atrtable[menuatr + 2]);

	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*20;
	for (byte = 0; byte < 37; byte++)
		tuxtxt_RenderCharFB(&renderinfo,message_5[byte], &tuxtxt_atrtable[menuatr + 0]);
	tuxtxt_RenderCharFB(&renderinfo,message_5[37], &tuxtxt_atrtable[menuatr + 2]);

	renderinfo.PosX = renderinfo.StartX + renderinfo.fontwidth+5;
	renderinfo.PosY = renderinfo.StartY + renderinfo.fontheight*21;
	for (byte = 0; byte < 38; byte++)
		tuxtxt_RenderCharFB(&renderinfo,message_6[byte], &tuxtxt_atrtable[menuatr + 2]);

	tuxtxt_cache.national_subset=active_national_subset; // restore charset
}



/******************************************************************************
 * UpdateLCD                                                                  *
 ******************************************************************************/

void UpdateLCD()
{
	static int init_lcd = 1, old_cached_pages = -1, old_page = -1, old_subpage = -1, old_subpage_max = -1, old_hintmode = -1;
	int  x, y, subpage_max = 0, update_lcd = 0;

	if (lcd == -1) return; // for Dreamboxes without LCD-Display (5xxx)
	/* init or update lcd */
	if (init_lcd)
	{
		init_lcd = 0;

		for (y = 0; y < 64; y++)
		{
			int lcdbase = (y/8)*120;
			int lcdmask = 1 << (y%8);

			for (x = 0; x < 120; )
			{
				int rommask;
				int rombyte = lcd_layout[x/8 + y*120/8];

				for (rommask = 0x80; rommask; rommask >>= 1)
				{
					if (rombyte & rommask)
						lcd_backbuffer[x + lcdbase] |= lcdmask;
					else
						lcd_backbuffer[x + lcdbase] &= ~lcdmask;
					x++;
				}
			}
		}

		write(lcd, &lcd_backbuffer, sizeof(lcd_backbuffer));

		for (y = 16; y < 56; y += 8)	/* clear rectangle in backbuffer */
			for (x = 1; x < 118; x++)
				lcd_backbuffer[x + (y/8)*120] = 0;

		for (x = 3; x <= 116; x++)
			lcd_backbuffer[x + (39/8)*120] |= 1 << (39%8);

		for (y = 42; y <= 60; y++)
			lcd_backbuffer[35 + (y/8)*120] |= 1 << (y%8);

		for (y = 42; y <= 60; y++)
			lcd_backbuffer[60 + (y/8)*120] |= 1 << (y%8);

		RenderCharLCD(10, 43, 20);
		RenderCharLCD(11, 79, 20);

		return;
	}
	else
	{
		int p;

		if (renderinfo.inputcounter == 2)
			p = tuxtxt_cache.page;
		else
			p = temp_page + (0xDD >> 4*(1-renderinfo.inputcounter)); /* partial pageinput (filled with spaces) */

		/* page */
		if (old_page != p)
		{
			RenderCharLCD(p>>8,  7, 20);
			RenderCharLCD((p&0x0F0)>>4, 19, 20);
			RenderCharLCD(p&0x00F, 31, 20);

			old_page = p;
			update_lcd = 1;
		}

		/* current subpage */
		if (old_subpage != tuxtxt_cache.subpage)
		{
			if (!tuxtxt_cache.subpage)
			{
				RenderCharLCD(0, 55, 20);
				RenderCharLCD(1, 67, 20);
			}
			else
			{
				if (tuxtxt_cache.subpage >= 0xFF)
					tuxtxt_cache.subpage = 1;
				else if (tuxtxt_cache.subpage > 99)
					tuxtxt_cache.subpage = 0;

				RenderCharLCD(tuxtxt_cache.subpage>>4, 55, 20);
				RenderCharLCD(tuxtxt_cache.subpage&0x0F, 67, 20);
			}

			old_subpage = tuxtxt_cache.subpage;
			update_lcd = 1;
		}

		/* max subpage */
		for (x = 0; x <= 0x79; x++)
		{
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.page][x])
				subpage_max = x;
		}

		if (old_subpage_max != subpage_max)
		{
			if (!subpage_max)
			{
				RenderCharLCD(0,  91, 20);
				RenderCharLCD(1, 103, 20);
			}
			else
			{
				RenderCharLCD(subpage_max>>4,  91, 20);
				RenderCharLCD(subpage_max&0x0F, 103, 20);
			}

			old_subpage_max = subpage_max;
			update_lcd = 1;
		}

		/* cachestatus */
		if (old_cached_pages != tuxtxt_cache.cached_pages)
		{
			#if 0
			int s;
			int p = tuxtxt_cache.cached_pages;
			for (s=107; s >= 107-4*fontwidth_small_lcd; s -= fontwidth_small_lcd)
			{
				int c = p % 10;
				if (p)
					RenderCharLCDsmall('0'+c, s, 44);
				else
					RenderCharLCDsmall(' ', s, 44);
				p /= 10;
			}
			#else
			RenderCharLCD(tuxtxt_cache.cached_pages/1000, 67, 44);
			RenderCharLCD(tuxtxt_cache.cached_pages%1000/100, 79, 44);
			RenderCharLCD(tuxtxt_cache.cached_pages%100/10, 91, 44);
			RenderCharLCD(tuxtxt_cache.cached_pages%10, 103, 44);
			#endif

			old_cached_pages = tuxtxt_cache.cached_pages;
			update_lcd = 1;
		}

		/* mode */
		if (old_hintmode != renderinfo.hintmode)
		{
			if (renderinfo.hintmode)
				RenderCharLCD(12, 43, 44);
			else
				RenderCharLCD(13, 43, 44);

			old_hintmode = renderinfo.hintmode;
			update_lcd = 1;
		}
	}

	if (update_lcd)
		write(lcd, &lcd_backbuffer, sizeof(lcd_backbuffer));
}


/******************************************************************************
 * GetRCCode                                                                  *
 ******************************************************************************/

int GetRCCode()
{
#if HAVE_DVB_API_VERSION < 3
	static unsigned short LastKey = -1;
#else
	struct input_event ev;
	static __u16 rc_last_key = KEY_RESERVED;
#endif
	/* get code */
#if HAVE_DVB_API_VERSION < 3
	if (read(rc, &RCCode, 2) == 2)
	{
		if (RCCode != LastKey)
		{
			LastKey = RCCode;

			if ((RCCode & 0xFF00) == 0x5C00)
			{
				switch (RCCode)
#else
	if (read(rc, &ev, sizeof(ev)) == sizeof(ev))
	{
		if (ev.value)
		{
			if (ev.code != rc_last_key)
			{
				rc_last_key = ev.code;
				switch (ev.code)
#endif
				{
				case KEY_UP:		RCCode = RC_UP;		break;
				case KEY_DOWN:		RCCode = RC_DOWN;	break;
				case KEY_LEFT:		RCCode = RC_LEFT;	break;
				case KEY_RIGHT:		RCCode = RC_RIGHT;	break;
				case KEY_OK:		RCCode = RC_OK;		break;
				case KEY_0:		RCCode = RC_0;		break;
				case KEY_1:		RCCode = RC_1;		break;
				case KEY_2:		RCCode = RC_2;		break;
				case KEY_3:		RCCode = RC_3;		break;
				case KEY_4:		RCCode = RC_4;		break;
				case KEY_5:		RCCode = RC_5;		break;
				case KEY_6:		RCCode = RC_6;		break;
				case KEY_7:		RCCode = RC_7;		break;
				case KEY_8:		RCCode = RC_8;		break;
				case KEY_9:		RCCode = RC_9;		break;
				case KEY_RED:		RCCode = RC_RED;	break;
				case KEY_GREEN:		RCCode = RC_GREEN;	break;
				case KEY_YELLOW:	RCCode = RC_YELLOW;	break;
				case KEY_BLUE:		RCCode = RC_BLUE;	break;
				case KEY_VOLUMEUP:	RCCode = RC_PLUS;	break;
				case KEY_VOLUMEDOWN:	RCCode = RC_MINUS;	break;
				case KEY_MUTE:		RCCode = RC_MUTE;	break;
				case KEY_HELP:		RCCode = RC_HELP;	break;
				case KEY_SETUP:		RCCode = RC_DBOX;	break;
				case KEY_HOME:		RCCode = RC_HOME;	break;
				case KEY_POWER:		RCCode = RC_STANDBY;	break;
				}
				return 1;
			}
#if HAVE_DVB_API_VERSION < 3
			else
				RCCode &= 0x003F;
#endif
		}
#if HAVE_DVB_API_VERSION < 3
		else
			RCCode = -1;

		return 1;
#else
		else
		{
			RCCode = -1;
			rc_last_key = KEY_RESERVED;
		}
#endif
	}

	RCCode = -1;
	usleep(1000000/100);

	return 0;
}
/* Local Variables: */
/* indent-tabs-mode:t */
/* tab-width:3 */
/* c-basic-offset:3 */
/* comment-column:0 */
/* fill-column:120 */
/* End: */
