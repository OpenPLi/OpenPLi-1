#ifndef __GTX_DMX_H
#define __GTX_DMX_H

#define GTXDMX_MAJOR    62

#define NUM_QUEUES      32
#define VIDEO_QUEUE     0
#define AUDIO_QUEUE     1
#define TELETEXT_QUEUE  2
#define USER_QUEUE_START 3
#define LAST_USER_QUEUE 30
#define MESSAGE_QUEUE   31
#define HIGHSPEED_QUEUE 32

#define NUM_PID_FILTER  32

                        // nokia api stuff
#define GTX_OUTPUT_TS           0
#define GTX_OUTPUT_TSPAYLOAD    2
#define GTX_OUTPUT_PESPAYLOAD   3
#define GTX_OUTPUT_8BYTE        4
#define GTX_OUTPUT_16BYTE       5

#define GTX_FILTER_PID          0
#define GTX_FILTER_SECTION      1

typedef struct gtx_demux_secfilter_s
{
  dmx_section_filter_t filter;
  int index;
  int state;

  struct gtx_demux_secfilter_s *next;
  struct gtx_demux_feed_s *feed;
} gtx_demux_secfilter_t;

typedef struct gtx_demux_filter_s
{
  int index;
  int state;

  int output;
  int wait_pusi, invalid, pid;
  int queue, fork, cw_offset, cc, start_up, pec;
  // type section
  int no_of_filters;

  struct gtx_demux_feed_s *feed;
} gtx_demux_filter_t;

typedef struct gtx_demux_feed_s
{
  union
  {
    dmx_ts_feed_t ts;
    dmx_section_feed_t sec;
    dmx_pes_feed_t pes;
  } feed;
  union
  {
    dmx_ts_cb ts;
    dmx_section_cb sec;
    dmx_pes_cb pes;
  } cb;

  struct gtx_demux_s *demux;
  int type;
  int state;

#define DMX_TYPE_TS  0
#define DMX_TYPE_SEC 1
#define DMX_TYPE_PES 2
#define DMX_TYPE_HW_SEC 3
#define DMX_TYPE_MESSAGE 4

#define DMX_STATE_FREE      0
#define DMX_STATE_ALLOCATED 1
#define DMX_STATE_SET       2
#define DMX_STATE_READY     3
#define DMX_STATE_GO        4

  dmx_ts_pes_t pes_type;
  int output;
  int pid;
  gtx_demux_filter_t *filter;
  gtx_demux_secfilter_t *secfilter;

  // for sections
  __u8 *sec_buffer;
  int sec_recv;
  int sec_len;
  int check_crc;
  u32 sec_crc;
  char sec_ccn;

  int index;
  u8 irq_is_active;

} gtx_demux_feed_t;

typedef struct gtx_demux_s
{
  dmx_demux_t dmx;
   // keine ahnung was hier noch kommt :) evtl. die wait_queues oder so.
  int users;
  gtx_demux_feed_t feed[32];
  gtx_demux_filter_t filter[32];
  gtx_demux_secfilter_t secfilter[32];
  int filter_definition_table_entry_user[32];
  unsigned char hw_sec_filt_enabled;

  struct list_head frontend_list;
} gtx_demux_t;

#pragma pack(1)

typedef struct {

	u32 synch_byte: 8;
	u32 transport_error_indicator: 1;
	u32 payload_unit_start_indicator: 1;
	u32 transport_priority: 1;
	u32 PID: 13;
	u32 transport_scrambling_control: 2;
	u32 adaptation_field: 1;
	u32 payload: 1;
	u32 continuity_counter: 4;

} sDVBTsHeader;

#pragma pack()

#endif
