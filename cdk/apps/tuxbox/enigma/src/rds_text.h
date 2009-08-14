#ifndef __SRC_RDS_TEXT_H_
#define __SRC_RDS_TEXT_H_

#include <lib/base/ebase.h>
#include <lib/base/estring.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ewidget.h>

class RassInteractivemode;

class RDSTextDecoder : public Object
{
	RassInteractivemode *m_interactive;
	int wasVisible;
	int bytesread, ptr, p1, p2, bsflag, qdar_pos, rass_imode_active;
	char fname[50];
	unsigned char buf[263], message[66], lastmessage[66], datamessage[256], rtp_buf[5], leninfo, text_len, text_len2, m_ptr, t_ptr, state, is_sync, paket_size, sync_try;
	unsigned char rtp_item[64][64], rtplus_osd[64]; //rtp
	unsigned char qdar[60*1024]; //60 kB for holding Rass qdar archive
	unsigned short crc16, crc;
	long part, parts, partcnt; 
	eSocketNotifier *sn;
	eLabel *rds_text,*rtp_text,rass_logo;
	void process_data(int);
	void process_qdar(unsigned char*);
	void globalFocusHasChanged(const eWidget* newFocus);
	void init_RDSTextDecoder();
public:
	RDSTextDecoder();
	~RDSTextDecoder();
	Signal1<void, eString> textReady;
	int qdarmvi_show,interactive_avail;
	void clear_service();
	void rass_interactive();
};

class RassInteractivemode : public eWidget
{
	eLabel *rass_page[10],*rass_page_sub[10];
	gPixmap *rass_pm1,*rass_pm2,*rass_pm3,*rass_pm4;
	int active_slide,active_slide_sub;
	void sel_entry(int val);
	int check_file(char* fname);
	int check_avail(int page);
	int eventHandler( const eWidgetEvent &e );
	void init_RassInteractivemode();
public:
	RassInteractivemode();
	~RassInteractivemode();
	void update_avail(int);
};

#endif
