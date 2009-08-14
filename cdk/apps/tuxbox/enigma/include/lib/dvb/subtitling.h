#ifndef __subtitling_h
#define __subtitling_h

#include <linux/fb.h>
#include <lib/gui/ewidget.h>
#include <lib/base/ebase.h>
#include <lib/dvb/subtitle.h>
#include <queue>

class eSubtitleWidget: public eWidget
{
	int pid;
	std::set<int> pageids;
	void gotData(int);
	eSocketNotifier *sn;
	int fd;
	int isvisible;
#ifndef TUXTXT_CFG_STANDALONE
	int ttxpage;
	int ttx_running;
	int rememberttxpage;
	int rememberttxsubpage;
	int rendering_initialized;
#endif	
	subtitle_ctx *subtitle; // the subtitle context
	
	struct pes_packet_s
	{
		unsigned long long pts;
		unsigned char *pkt;
		int len;
	};
	
	std::queue<pes_packet_s> queue;
	
	eTimer timer, timeout;
	void processPESPacket(unsigned char *pkt, int len);
	void processNext();
	void displaying_timeout();

	unsigned char pesbuffer[65536];
	int pos;
	int peslen;
	static eSubtitleWidget *instance;
	int eventHandler(const eWidgetEvent &event);
	void globalFocusHasChanged(const eWidget* newFocus);
	void init_eSubtitleWidget();
public:
	void start(int pid, const std::set<int> &pageids);
#ifndef TUXTXT_CFG_STANDALONE
	void startttx(int page);
	void stopttx();
#endif
	void stop();
	int getCurPid();
	eSubtitleWidget();
	~eSubtitleWidget();
	static eSubtitleWidget *getInstance() { return instance; }
};

#endif
