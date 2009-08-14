#ifndef __enigma_tuxtxt_h_
#define __enigma_tuxtxt_h_


#include <tuxtxt/tuxtxt_def.h>
#include <lib/gui/ewidget.h>


/* variables and functions from libtuxtxt */
extern "C" tuxtxt_cache_struct tuxtxt_cache;
extern "C" tstPageAttr tuxtxt_atrtable[];
extern "C" int tuxtxt_init();
extern "C" void tuxtxt_close();
extern "C" int  tuxtxt_start(int tpid);  // Start caching
extern "C" int  tuxtxt_stop(); // Stop caching
extern "C" void tuxtxt_next_dec(int *i); /* skip to next decimal */
extern "C" void tuxtxt_prev_dec(int *i); /* counting down */
extern "C" int tuxtxt_is_dec(int i);
extern "C" int tuxtxt_next_hex(int i);
extern "C" void tuxtxt_hex2str(char *s, unsigned int n);
extern "C" void tuxtxt_decode_btt();
extern "C" void tuxtxt_decode_adip(); /* additional information table */
extern "C" void tuxtxt_compress_page(int p, int sp, unsigned char* buffer);
extern "C" void tuxtxt_decompress_page(int p, int sp, unsigned char* buffer);
extern "C" void tuxtxt_hex2str(char *s, unsigned int n);
extern "C" tstPageinfo* tuxtxt_DecodePage(int showl25, unsigned char* page_char, tstPageAttr *page_atrb, int hintmode, int showflof);
extern "C" void tuxtxt_FillRect(unsigned char *lfb, int xres, int x, int y, int w, int h, int color);
extern "C" int tuxtxt_RenderChar(unsigned char *lfb, int xres,int Char, int *pPosX, int PosY, tstPageAttr *Attribute, int zoom, int curfontwidth, int curfontwidth2, int fontheight, int transpmode, unsigned char* axdrcs, int ascender);
extern "C" void tuxtxt_RenderDRCS(int xres,unsigned char *s,unsigned char *d,unsigned char *ax, unsigned char fgcolor, unsigned char bgcolor);
extern "C" void tuxtxt_RenderPage(tstRenderInfo* renderinfo);
extern "C" void tuxtxt_SetPosX(tstRenderInfo* renderinfo, int column);
extern "C" void tuxtxt_RenderCharFB(tstRenderInfo* renderinfo, int Char, tstPageAttr *Attribute);
extern "C" void tuxtxt_ClearBB(tstRenderInfo* renderinfo,int color);
extern "C" void tuxtxt_ClearFB(tstRenderInfo* renderinfo,int color);
extern "C" void tuxtxt_setcolors(tstRenderInfo* renderinfo,unsigned short *pcolormap, int offset, int number);
extern "C" int tuxtxt_SetRenderingDefaults(tstRenderInfo* renderinfo);
extern "C" int tuxtxt_InitRendering(tstRenderInfo* renderinfo,int setTVFormat);
extern "C" void tuxtxt_EndRendering(tstRenderInfo* renderinfo);
extern "C" void tuxtxt_CopyBB2FB(tstRenderInfo* renderinfo);
extern "C" void tuxtxt_SwitchScreenMode(tstRenderInfo* renderinfo,int newscreenmode);

class eTuxtxtWidget : public eWidget
{
private:
	eTimer timer;
	eLock lock;
	int initialized;
	int lastpage;
	int savedscreenmode;
	int catch_row, catch_col, catched_page, pagecatching;
	int pids_found, current_service, getpidsdone;
	int pc_old_row, pc_old_col;     /* for page catching */
	int temp_page;	/* for page input */
	int rendering_initialized;
	tstRenderInfo renderinfo;
	struct timeval tv_delay;
	std::vector<int> hotlist;

	/* functions */
	void ConfigMenu();
	void CleanUp();
	void PageInput(int Number);
	void ColorKey(int);
	void PageCatching();
	void CancelPageCatching();
	void CatchNextPage(int, int);
	void GetNextPageOne(int up);
	void GetNextSubPage(int offset);
	void SwitchZoomMode();
	void SwitchTranspMode();
	void SwitchHintMode();
	void RenderCatchedPage();
	int getIndexOfPageInHotlist();
	void gethotlist();
	void Init();
	int eventHandler(const eWidgetEvent &event);
	void RenderPage();
	void init_eTuxtxtWidget();	
public:
	eTuxtxtWidget();
	~eTuxtxtWidget();
};
#endif
