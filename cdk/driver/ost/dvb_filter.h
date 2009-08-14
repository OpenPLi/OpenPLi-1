#ifndef _DVB_FILTER_H_
#define _DVB_FILTER_H_

#include <linux/slab.h>
#include <linux/vmalloc.h>

#ifdef __DVB_PACK__
#include <ost/demux.h>
#else
#include <linux/ost/demux.h>
#endif

typedef int (pes2ts_cb_t) (void *, unsigned char *);

typedef struct pes2ts_s {
	unsigned char buf[188];
        unsigned char cc;
        pes2ts_cb_t *cb;
	void *priv;
} pes2ts_t;

void pes2ts_init(pes2ts_t *p2ts, unsigned short pid, 
		 pes2ts_cb_t *cb, void *priv);
int pes2ts(pes2ts_t *p2ts, unsigned char *pes, int len);


#define PROG_STREAM_MAP  0xBC
#define PRIVATE_STREAM1  0xBD
#define PADDING_STREAM   0xBE
#define PRIVATE_STREAM2  0xBF
#define AUDIO_STREAM_S   0xC0
#define AUDIO_STREAM_E   0xDF
#define VIDEO_STREAM_S   0xE0
#define VIDEO_STREAM_E   0xEF
#define ECM_STREAM       0xF0
#define EMM_STREAM       0xF1
#define DSM_CC_STREAM    0xF2
#define ISO13522_STREAM  0xF3
#define PROG_STREAM_DIR  0xFF

//flags2
#define PTS_DTS_FLAGS    0xC0
#define ESCR_FLAG        0x20
#define ES_RATE_FLAG     0x10
#define DSM_TRICK_FLAG   0x08
#define ADD_CPY_FLAG     0x04
#define PES_CRC_FLAG     0x02
#define PES_EXT_FLAG     0x01

//pts_dts flags 
#define PTS_ONLY         0x80
#define PTS_DTS          0xC0

#define TS_SIZE        188
#define TRANS_ERROR    0x80
#define PAY_START      0x40
#define TRANS_PRIO     0x20
#define PID_MASK_HI    0x1F
//flags
#define TRANS_SCRMBL1  0x80
#define TRANS_SCRMBL2  0x40
#define ADAPT_FIELD    0x20
#define PAYLOAD        0x10
#define COUNT_MASK     0x0F

// adaptation flags
#define DISCON_IND     0x80
#define RAND_ACC_IND   0x40
#define ES_PRI_IND     0x20
#define PCR_FLAG       0x10
#define OPCR_FLAG      0x08
#define SPLICE_FLAG    0x04
#define TRANS_PRIV     0x02
#define ADAP_EXT_FLAG  0x01

// adaptation extension flags
#define LTW_FLAG       0x80
#define PIECE_RATE     0x40
#define SEAM_SPLICE    0x20


#define MAX_PLENGTH 0xFFFF
#define MMAX_PLENGTH (256*MAX_PLENGTH)

#ifndef IPACKS
#define IPACKS 2048
#endif

typedef struct ipack_s {
	int size;
	int found;
	u8 *buf;
	u8 cid;
	uint32_t plength;
	u8 plen[2];
	u8 flag1;
	u8 flag2;
	u8 hlength;
	u8 pts[5];
	u16 *pid;
	int mpeg;
	u8 check;
	int which;
	int done;
	void *data;
	void (*func)(u8 *buf,  int size, void *priv);
	int count;
	int repack_subids;
} ipack;

typedef struct video_i{
	u32 horizontal_size;
	u32 vertical_size       ;
	u32 aspect_ratio        ;
	double framerate        ;
	u32 video_format;
	u32 bit_rate    ;
	u32 comp_bit_rate       ;
	u32 vbv_buffer_size;
	u32 CSPF                ;
	u32 off;
} VideoInfo;            

typedef struct audio_i{
	int layer               ;
	u32 bit_rate    ;
	u32 frequency   ;
	u32 mode                ;
	u32 mode_extension ;
	u32 emphasis    ;
	u32 framesize;
	u32 off;
} AudioInfo;

void reset_ipack(ipack *p);
int instant_repack(u8 *buf, int count, ipack *p);
void init_ipack(ipack *p, int size,
		void (*func)(u8 *buf,  int size, void *priv));
void free_ipack(ipack * p);
void setup_ts2pes(ipack *pa, ipack *pv, u16 *pida, u16 *pidv, 
		  void (*pes_write)(u8 *buf, int count, void *data),
		  void *priv);
void ts_to_pes(ipack *p, u8 *buf); 
void send_ipack(ipack *p);
void send_ipack_rest(ipack *p);
int get_ainfo(uint8_t *mbuf, int count, AudioInfo *ai, int pr);
int get_ac3info(uint8_t *mbuf, int count, AudioInfo *ai, int pr);
int get_vinfo(uint8_t *mbuf, int count, VideoInfo *vi, int pr);
uint8_t *skip_pes_header(uint8_t **bufp);
#endif
