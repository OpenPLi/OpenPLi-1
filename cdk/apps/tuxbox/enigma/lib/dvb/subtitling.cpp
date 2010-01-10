#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include <lib/gdi/gfbdc.h>
#include <lib/dvb/decoder.h>
#include <lib/gui/eskin.h>

#include <config.h>
#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#include <ost/video.h>
#include <ost/audio.h>
#define VIDEO_DEV "/dev/dvb/card0/video0"
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#define DEMUX_DEV "/dev/dvb/card0/demux0"
#else
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#define VIDEO_DEV "/dev/dvb/adapter0/video0"
#define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#define DEMUX_DEV "/dev/dvb/adapter0/demux0"
#define audioStatus audio_status
#define videoStatus video_status
#define pesType pes_type
#define playState play_state
#define audioStreamSource_t audio_stream_source_t
#define videoStreamSource_t video_stream_source_t
#define streamSource stream_source
#define dmxPesFilterParams dmx_pes_filter_params
#endif

#include <lib/dvb/subtitling.h>

#ifndef TUXTXT_CFG_STANDALONE
#include <tuxtxt/tuxtxt_def.h>
extern "C" tstPageinfo* tuxtxt_DecodePage(int showl25, unsigned char* page_char, tstPageAttr *page_atrb, int hintmode, int showflof);
extern "C" void tuxtxt_RenderPage(tstRenderInfo* renderinfo);
extern "C" int tuxtxt_SetRenderingDefaults(tstRenderInfo* renderinfo);
extern "C" tuxtxt_cache_struct tuxtxt_cache;
extern "C" int tuxtxt_InitRendering(tstRenderInfo* renderinfo,int setTVFormat);
extern "C" void tuxtxt_EndRendering(tstRenderInfo* renderinfo);
tstRenderInfo renderinfo;
#endif

eSubtitleWidget *eSubtitleWidget::instance;

static int extractPTS(unsigned long long &pts, unsigned char *pkt)
{
	pkt += 7;
	int flags = *pkt++;
	
	pkt++; // header length
	
	if (flags & 0x80) /* PTS present? */
	{
			/* damn gcc bug */
		pts  = ((unsigned long long)(((pkt[0] >> 1) & 7))) << 30;
		pts |=   pkt[1] << 22;
		pts |=  (pkt[2]>>1) << 15;
		pts |=   pkt[3] << 7;
		pts |=  (pkt[5]>>1);
		
		return 0;
	} else
		return -1;
}

void eSubtitleWidget::processPESPacket(unsigned char *pkt, int len)
{
	unsigned long long current = 0;
	if (Decoder::getSTC(current))
		eDebug("bloed, going unsyced");
	eDebug("DEMUX STC: %08llx", current);
	
	unsigned long long pts = 0;
	
	int enqueue = !queue.empty();

	if (!extractPTS(pts, pkt))
	{
		eDebug("PES   STC: %08llx", pts);
		signed long long int diff = pts - current;
		eDebug("     diff: %lld(%lldms)", diff, diff/90);
		if (diff > 1800)
			enqueue = 1;
		else if (enqueue) // this should not happen !!
		{
			eDebug("showing instantly, diff small enough... but queue not empy!!!!");
			enqueue = 0;
		}
		else
			eDebug("showing instantly, diff small enough...!");
	}

	if (enqueue)
	{
		int wasempty = queue.empty();
		struct pes_packet_s pes;
		pes.pts = pts;
		pes.pkt = new unsigned char[len];
		memcpy(pes.pkt, pkt, len);
		pes.len = len;
		queue.push(pes);
		eDebug("enqueue");
		if (wasempty)
		{
			eDebug("setting timer to %lld ms!\n", (pes.pts - current) / 90);
			timer.start((pes.pts - current) / 90, 1);
		}
		else
			eDebug("");

		return;
	}
	subtitle_process_pes(subtitle, pesbuffer, peslen);
	if (!subtitle->isdrawn && subtitle->screen_enabled)
		subtitle_redraw_all(subtitle);

}

void eSubtitleWidget::displaying_timeout()
{
	eDebug("displaying timeout reached... hide visible subtitles");
	subtitle_reset(subtitle);
	if ( isVisible() )
		subtitle_clear_screen(subtitle);
}

void eSubtitleWidget::processNext()
{
#ifndef TUXTXT_CFG_STANDALONE
	if (ttx_running) // using teletext subtitles
	{
		renderinfo.pageinfo = tuxtxt_DecodePage(0,renderinfo.page_char,renderinfo.page_atrb,0,0);
		if (renderinfo.pageinfo)
		{
			tuxtxt_RenderPage(&renderinfo);
		}
		return;
	}
#endif
	if (queue.empty())
	{
		eWarning("Subtitle queue is empty, but timer was called!");
		return;
	}

	unsigned long long fpts=0;
	int first = 1;
	while (!queue.empty())
	{
		pes_packet_s pes = queue.front();
		if (pes.pts && !first)
			break;
		if (first)
			fpts = pes.pts;
		first = 0;
		queue.pop();

		subtitle_process_pes(subtitle, pes.pkt, pes.len);

		delete [] pes.pkt;
	}
	if (!subtitle->isdrawn && subtitle->screen_enabled)
		subtitle_redraw_all(subtitle);

	unsigned long long current = 0;
	
	if (Decoder::getSTC(current))
	{
		eWarning("getSTC failed, dropping all Subtitle packets!");
		while (!queue.empty())
		{
			pes_packet_s pkt = queue.front();
			queue.pop();
			delete [] pkt.pkt;
		}
		return;
	}
	
	eDebug("by the way, actual delay was %lld(%lld msek)", current - fpts, (current-fpts)/90 );

	if (!queue.empty()) {
		signed long long int diff = queue.front().pts - current;
		timer.start(diff / 90, 1);
		eDebug("setting timer to %lld ms!\n", diff / 90);
	}
	else
		eDebug("");
}

void eSubtitleWidget::gotData(int what)
{
	while (1)
	{
		unsigned char packet[1024];
		int l;
		l=::read(fd, packet, 1024);
		if (l <= 0)
			break;
		
		unsigned char *p = packet;
		
		while (l)
		{
			if (pos >= 6) // length ok?
			{
				int max = peslen - pos;
				if (max > l)
					max = l;
				memcpy(pesbuffer + pos, p, max);
				pos += max;
				p += max;
				
				l -= max;
				
				if (pos == peslen)
				{
					processPESPacket(pesbuffer, pos);
					pos = 0;
				}
			} else
			{
				if (pos < 4)
					if (*p != "\x00\x00\x01\xbd"[pos])
					{
						pos = 0;
						p++;
						l--;
						continue;
					}
				pesbuffer[pos++] = *p++; l--;
				if (pos == 6)
				{
					peslen = ((pesbuffer[4] << 8) | pesbuffer[5]) + 6;
				}
			}
		}
	}
}

int eSubtitleWidget::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::willShow:
//		eDebug("willShow!!!");
#ifndef TUXTXT_CFG_STANDALONE
		startttx(ttxpage);
#endif
		isvisible = 1;
		subtitle_screen_enable(subtitle, 1);
		break;
#ifndef TUXTXT_CFG_STANDALONE
	case eWidgetEvent::execBegin:
		if (!rendering_initialized)
			in_loop=0;
		return eWidget::eventHandler(event);
#endif
	case eWidgetEvent::willHide:
//		eDebug("willHide!!!");
#ifndef TUXTXT_CFG_STANDALONE
		stopttx();
#endif
			
		isvisible = 0;
		subtitle_screen_enable(subtitle, 0);
		//restore old palette
		eSkin::getActive()->setPalette(gFBDC::getInstance());
		break;
	default:
		return eWidget::eventHandler(event);;
	}
	return 0;
}

#ifndef TUXTXT_CFG_STANDALONE
void eSubtitleWidget::startttx(int page)
{
	if (page == 0|| ttx_running) return;
	if (page < 0) page = ttxpage;
	if (page == 0)	return;
	eDebug("Starting teletext subtitling:%x",page);
	stop();
	if (isvisible)
		subtitle_screen_enable(subtitle, 1);	
	rememberttxpage = tuxtxt_cache.page;
	rememberttxsubpage = tuxtxt_cache.subpage;
	tuxtxt_cache.page = ttxpage = page;
	ttx_running = 1;
	this->pid = -ttxpage;
	tuxtxt_cache.subpage = 0;
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
	if (eConfig::getInstance()->getKey("/ezap/teletext/AutoNational", renderinfo.auto_national ))
		eConfig::getInstance()->setKey("/ezap/teletext/AutoNational", renderinfo.auto_national);
	if (eConfig::getInstance()->getKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset ))
		eConfig::getInstance()->setKey("/ezap/teletext/NationalSubset", tuxtxt_cache.national_subset);
	if (eConfig::getInstance()->getKey("/ezap/teletext/UseTTF", renderinfo.usettf ))
		eConfig::getInstance()->setKey("/ezap/teletext/UseTTF", renderinfo.usettf);

	renderinfo.fb =fbClass::getInstance()->lock();
	if (tuxtxt_InitRendering(&renderinfo,0))
	{
		renderinfo.pageinfo = tuxtxt_DecodePage(0,renderinfo.page_char,renderinfo.page_atrb,0,0);
		if (renderinfo.pageinfo)
		{	
			tuxtxt_cache.pageupdate=1; // force complete redraw of page
			tuxtxt_RenderPage(&renderinfo);
		}
		timer.start(250, false);
		rendering_initialized = 1;
	}
	else
	{
		fbClass::getInstance()->unlock();
		ttx_running = 0;
	}


}
#endif
void eSubtitleWidget::start(int pid, const std::set<int> &ppageids)
{
	pageids = ppageids;
	stop();
	if (isvisible)
		subtitle_screen_enable(subtitle, 1);
	fd = open(DEMUX_DEV, O_RDWR|O_NONBLOCK);
	if (fd == -1)
	{
		eWarning("failed to open " DEMUX_DEV ": %m");
		return;
	}
	
	sn = new eSocketNotifier(eApp, fd, eSocketNotifier::Read);
	CONNECT(sn->activated, eSubtitleWidget::gotData);

	struct dmxPesFilterParams f;
	this->pid = f.pid = pid;
	f.input = DMX_IN_FRONTEND;
	f.output = DMX_OUT_TAP;
	f.pesType = DMX_PES_OTHER;
	f.flags = DMX_IMMEDIATE_START;
	if (::ioctl(fd, DMX_SET_PES_FILTER, &f) == -1)
		eWarning("DMX_SET_PES_FILTER: %m (subtitling)");
	else
		eDebug("started subtitling filter..");
		
	pos = 0;
}

static void subtitle_set_palette(struct subtitle_clut *pal, int subpal)
{
	static gRGB def_palette[64];
	static bool def_palette_initialized;

	gPainter p(*gFBDC::getInstance());
	if ( !pal )// use default pallette
	{
		if ( !def_palette_initialized )  // fill default palette
		{
			for (int i=0; i < 64; ++i)
			{
				if (!i)
					def_palette[i].a = 0xFF;
				else if (i&8)
				{
					if (i & 1)
						def_palette[i].r = 0x80;
					if (i & 2)
						def_palette[i].g = 0x80;
					if (i & 4)
						def_palette[i].b = 0x80;
				}
				else
				{
					if (i & 1)
						def_palette[i].r = 0xFF;
					if (i & 2)
						def_palette[i].g = 0xFF;
					if (i & 4)
						def_palette[i].b = 0xFF;
				}
//				eDebug("%d %02x%02x%02x%02x",
//					i, def_palette[i].r, def_palette[i].g, def_palette[i].b, def_palette[i].a);
			}
			def_palette_initialized=1;
		}
		p.setPalette(def_palette, 192, 64);
	}
	else
	{
	//	eDebug("updating palette!");
		gRGB palette[pal->size];

		for (int i=0; i<pal->size; ++i)
		{
			int y = pal->entries[i].Y, cr = pal->entries[i].Cr, cb = pal->entries[i].Cb;
		
			if (y > 0)
			{
				y -= 16;
				cr -= 128;
				cb -= 128;
#if 1
//				let's try a bit different conversion method
				palette[i].r = MAX(MIN(((298 * y            + 460 * cr) / 256), 255), 0);
				palette[i].g = MAX(MIN(((298 * y -  55 * cb - 137 * cr) / 256), 255), 0);
				palette[i].b = MAX(MIN(((298 * y + 543 * cb           ) / 256), 255), 0);
#else
				palette[i].r = ((1164 * y + 1596 * cr) + 500) / 1000;
				palette[i].g = ((1164 * y - 813 * cr - 392 * cb) + 500) / 1000;
				palette[i].b = ((1164 * y + 2017 * cb) + 500) / 1000;
#endif
				palette[i].a = (pal->entries[i].T) & 0xFF;
			} else
			{
				palette[i].r = 0;
				palette[i].g = 0;
				palette[i].b = 0;
				palette[i].a = 0xFF;
			}
//		eDebug("%d: %d %d %d %d", i, palette[i].r, palette[i].g, palette[i].b, palette[i].a);
		}
		p.setPalette(palette, 240- subpal*16, pal->size);
	}
//	eDebug("palette changed");
}

eSubtitleWidget::eSubtitleWidget()
	:timer(eApp), timeout(eApp)
{
	init_eSubtitleWidget();
}
void eSubtitleWidget::init_eSubtitleWidget()
{
	instance = this;
	fd = -1;
	sn = 0;
#ifndef TUXTXT_CFG_STANDALONE
	ttxpage = 0;
	ttx_running= 0;
	rendering_initialized = 0;
#endif
	subtitle = new subtitle_ctx;
	subtitle->pages = 0;
	subtitle->bbox_left = 0;
	subtitle->bbox_right = 0;
	subtitle->bbox_top = 0;
	subtitle->bbox_bottom = 0;
	subtitle->screen_enabled = 0;
	subtitle->isdrawn=0;
	subtitle->timeout_timer = &timeout;

	gFBDC *fbdc = gFBDC::getInstance();
	gPixmap *pixmap = &fbdc->getPixmap();

	subtitle->screen_width = pixmap->x;
	subtitle->screen_height = pixmap->y;
	subtitle->screen_buffer = (__u8*)pixmap->data;
	subtitle->set_palette = subtitle_set_palette;
	
	CONNECT(timer.timeout, eSubtitleWidget::processNext);
	CONNECT(timeout.timeout, eSubtitleWidget::displaying_timeout);
	CONNECT(eWidget::globalFocusChanged, eSubtitleWidget::globalFocusHasChanged);
}

eSubtitleWidget::~eSubtitleWidget()
{
	stop();
	delete subtitle;
}

int eSubtitleWidget::getCurPid()
{
	return pid;
}

void eSubtitleWidget::stop()
{
	//eDebug("stopping subtitling, queue size:%d",queue.size());
	while (!queue.empty())
	{
		pes_packet_s pkt = queue.front();
		queue.pop();
		delete [] pkt.pkt;
	}
	if ( sn )
	{
		pid=-1;
		delete sn;
		sn = 0;
		subtitle_screen_enable(subtitle, 0);
		subtitle_reset(subtitle);
		if (fd != -1)
		{
			::close(fd);
			fd = -1;
		}
		eSkin::getActive()->setPalette(gFBDC::getInstance());
	}
#ifndef TUXTXT_CFG_STANDALONE
	stopttx();
	ttxpage = 0;
	pid=-1;
#endif
}

#ifndef TUXTXT_CFG_STANDALONE
void eSubtitleWidget::stopttx()
{
	if (ttx_running) 
	{
		eDebug("Stopping teletext subtitling:%x",ttxpage);
		tuxtxt_EndRendering(&renderinfo);
		tuxtxt_cache.page = rememberttxpage;
		tuxtxt_cache.subpage = rememberttxsubpage;
		ttx_running = 0;
		timer.stop();
		fbClass::getInstance()->unlock();
		eSkin::getActive()->setPalette(gFBDC::getInstance());
		
	}
	rendering_initialized = 0;
}
#endif

void eSubtitleWidget::globalFocusHasChanged(const eWidget* newFocus)
{
	if ( !sn && !ttxpage) // not running
		return; 
	if ( newFocus )
		hide();
	else
		show();
}
