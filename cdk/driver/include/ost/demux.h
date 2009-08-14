/* * demux.h * * Copyright (c) 2000 Nokia Research Center
 * Tampere, FINLAND
 *
 * Project:
 * Universal Broadcast Access
 *
 * Contains:
 * Type definitions of a Linux kernel-level API for filtering MPEG-2 TS
 * packets and MPEG-2 sections. Support for PES packet filtering will be
 * added later.
 *
 * History:
 * 12.01.2000/JPL File created - Initial version.
 * 18.02.2000/JPL Minor corrections.
 * 21.02.2000/JPL DMX_NAME_SIZE and dmx_in_use() removed, typos fixed,
 * some names changed.
 * 23.02.2000/JPL Added a parameter indicating the callback source in
 * the callback functions.
 * 10.03.2000/JPL Added the macros DMX_DIR_ENTRY() and DMX_FE_ENTRY().
 * 15.03.2000/JPL Added the capabilities field to dmx_demux_t.
 * 22.03.2000/JPL Corrected the callback parameter in the
 * allocate_x_feed() functions.
 * 03.04.2000/JPL Added support for optional resource conflict resolution 
 * and scarce resource handling. 
 * 05.04.2000/JPL Changed the dmx_resolve_conflict() to use resource 
 * type as a parameter. 
 * 12.04.2000/JPL Added a second buffer parameter for dmx_x_callback() 
 * functions to better handle buffer wrapping. 
 * 26.04.2000/JPL Added functions for section-level descrambling. 
 * 03.09.2000/JPL Removed support for conflict resolution and scarce 
 * resource handling. Otherwise only minor changes to 
 * data structures and function prototypes. 
 * 
 *
 * Author:
 * Juha-Pekka Luoma (JPL)
 * Nokia Research Center
 *
 * Notes:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
/* $Id: demux.h,v 1.5 2002/08/16 08:42:43 obi Exp $ */ 

#ifndef __DEMUX_H 
#define __DEMUX_H 

#ifndef __KERNEL__ 
#define __KERNEL__ 
#endif 

#include <linux/types.h> /* __u8, __u16, ... */ 
#include <linux/list.h> /* list_entry(), struct list_head */ 
#include <linux/time.h> /* struct timespec */ 
#include <linux/errno.h> /* Function return values */ 

/*--------------------------------------------------------------------------*/ 
/* Common definitions */ 
/*--------------------------------------------------------------------------*/ 

/*
 * DMX_MAX_FILTER_SIZE: Maximum length (in bytes) of a section/PES filter.
 */ 

#ifndef DMX_MAX_FILTER_SIZE 
#define DMX_MAX_FILTER_SIZE 18
#endif 
/*
 * dmx_success_t: Success codes for the Demux Callback API. 
 */ 

typedef enum { 
  DMX_OK = 0, /* Received Ok */ 
  DMX_LENGTH_ERROR, /* Incorrect length */ 
  DMX_OVERRUN_ERROR, /* Receiver ring buffer overrun */ 
  DMX_CRC_ERROR, /* Incorrect CRC */ 
  DMX_FRAME_ERROR, /* Frame alignment error */ 
  DMX_FIFO_ERROR, /* Receiver FIFO overrun */ 
  DMX_MISSED_ERROR /* Receiver missed packet */ 
} dmx_success_t; 

/*--------------------------------------------------------------------------*/ 
/* TS packet reception */ 
/*--------------------------------------------------------------------------*/

/* TS filter type for set_type() */

#define TS_PACKET       1   /* send TS packets (188 bytes) to callback (default) */ 
#define	TS_PAYLOAD_ONLY 2   /* in case TS_PACKET is set, only send the TS
			       payload (<=184 bytes per packet) to callback */
#define TS_DECODER      4   /* send stream to built-in decoder (if present) */

/* PES type for filters which write to built-in decoder */
/* these should be kept identical to the types in dmx.h */

typedef enum
{
        DMX_TS_PES_AUDIO,   /* also send packets to audio decoder (if it exists) */
	DMX_TS_PES_VIDEO,   /* ... */
	DMX_TS_PES_TELETEXT,
	DMX_TS_PES_SUBTITLE,
	DMX_TS_PES_PCR,
	DMX_TS_PES_OTHER,
} dmx_ts_pes_t;


struct dmx_ts_feed_s { 
        int is_filtering; /* Set to non-zero when filtering in progress */
        struct dmx_demux_s* parent; /* Back-pointer */
        void* priv; /* Pointer to private data of the API client */ 
        int (*set) (struct dmx_ts_feed_s* feed, 
		    __u16 pid,
		    size_t callback_length, 
		    size_t circular_buffer_size, 
		    int descramble, 
		    struct timespec timeout); 
        int (*start_filtering) (struct dmx_ts_feed_s* feed); 
        int (*stop_filtering) (struct dmx_ts_feed_s* feed); 
        int (*set_type) (struct dmx_ts_feed_s* feed, 
			 int type, 
			 dmx_ts_pes_t pes_type);
};

typedef struct dmx_ts_feed_s dmx_ts_feed_t; 

/*--------------------------------------------------------------------------*/ 
/* PES packet reception (not supported yet) */
/*--------------------------------------------------------------------------*/ 

typedef struct dmx_pes_filter_s {
        struct dmx_pes_s* parent; /* Back-pointer */ 
        void* priv; /* Pointer to private data of the API client */ 
} dmx_pes_filter_t; 

typedef struct dmx_pes_feed_s {
        int is_filtering; /* Set to non-zero when filtering in progress */
        struct dmx_demux_s* parent; /* Back-pointer */
        void* priv; /* Pointer to private data of the API client */ 
        int (*set) (struct dmx_pes_feed_s* feed, 
		    __u16 pid,
		    size_t circular_buffer_size, 
		    int descramble, 
		    struct timespec timeout); 
        int (*start_filtering) (struct dmx_pes_feed_s* feed); 
        int (*stop_filtering) (struct dmx_pes_feed_s* feed); 
        int (*allocate_filter) (struct dmx_pes_feed_s* feed, 
				dmx_pes_filter_t** filter); 
        int (*release_filter) (struct dmx_pes_feed_s* feed, 
			       dmx_pes_filter_t* filter); 
} dmx_pes_feed_t; 

/*--------------------------------------------------------------------------*/ 
/* Section reception */ 
/*--------------------------------------------------------------------------*/ 

typedef struct { 
        __u8 filter_value [DMX_MAX_FILTER_SIZE]; 
        __u8 filter_mask [DMX_MAX_FILTER_SIZE]; 
        struct dmx_section_feed_s* parent; /* Back-pointer */ 
        void* priv; /* Pointer to private data of the API client */ 
} dmx_section_filter_t;

struct dmx_section_feed_s { 
        int is_filtering; /* Set to non-zero when filtering in progress */ 
        struct dmx_demux_s* parent; /* Back-pointer */
        void* priv; /* Pointer to private data of the API client */ 
        int (*set) (struct dmx_section_feed_s* feed, 
		    __u16 pid, 
		    size_t circular_buffer_size, 
		    int descramble, 
		    int check_crc); 
        int (*allocate_filter) (struct dmx_section_feed_s* feed, 
				dmx_section_filter_t** filter); 
        int (*release_filter) (struct dmx_section_feed_s* feed, 
			       dmx_section_filter_t* filter); 
        int (*start_filtering) (struct dmx_section_feed_s* feed); 
        int (*stop_filtering) (struct dmx_section_feed_s* feed); 
};
typedef struct dmx_section_feed_s dmx_section_feed_t; 

/*--------------------------------------------------------------------------*/ 
/* Callback functions */ 
/*--------------------------------------------------------------------------*/ 

typedef int (*dmx_ts_cb) ( __u8 * buffer1, 
			   size_t buffer1_length,
			   __u8 * buffer2, 
			   size_t buffer2_length,
			   dmx_ts_feed_t* source, 
			   dmx_success_t success); 

typedef int (*dmx_section_cb) (	__u8 * buffer1,
				size_t buffer1_len,
				__u8 * buffer2, 
				size_t buffer2_len,
			       	dmx_section_filter_t * source,
			       	dmx_success_t success);

typedef int (*dmx_pes_cb) ( __u8 * buffer1, 
			    size_t buffer1_len,
			    __u8 * buffer2,
			    size_t buffer2_len,
			    dmx_pes_filter_t* source, 
			    dmx_success_t success); 

/*--------------------------------------------------------------------------*/ 
/* DVB Front-End */
/*--------------------------------------------------------------------------*/ 

typedef enum { 
        DMX_OTHER_FE = 0, 
	DMX_SATELLITE_FE, 
	DMX_CABLE_FE, 
	DMX_TERRESTRIAL_FE, 
	DMX_LVDS_FE, 
	DMX_ASI_FE, /* DVB-ASI interface */
	DMX_MEMORY_FE
} dmx_frontend_source_t; 

typedef struct { 
        /* The following char* fields point to NULL terminated strings */ 
        char* id;                    /* Unique front-end identifier */ 
        char* vendor;                /* Name of the front-end vendor */ 
        char* model;                 /* Name of the front-end model */ 
        struct list_head connectivity_list; /* List of front-ends that can 
					       be connected to a particular 
					       demux */ 
        void* priv;     /* Pointer to private data of the API client */ 
        dmx_frontend_source_t source;
} dmx_frontend_t;

/*--------------------------------------------------------------------------*/ 
/* MPEG-2 TS Demux */ 
/*--------------------------------------------------------------------------*/ 

/* 
 * Flags OR'ed in the capabilites field of struct dmx_demux_s. 
 */ 

#define DMX_TS_FILTERING                        1 
#define DMX_PES_FILTERING                       2 
#define DMX_SECTION_FILTERING                   4 
#define DMX_MEMORY_BASED_FILTERING              8    /* write() available */ 
#define DMX_CRC_CHECKING                        16 
#define DMX_TS_DESCRAMBLING                     32 
#define DMX_SECTION_PAYLOAD_DESCRAMBLING        64 
#define DMX_MAC_ADDRESS_DESCRAMBLING            128 

/* 
 * Demux resource type identifier. 
*/ 

/* 
 * DMX_FE_ENTRY(): Casts elements in the list of registered 
 * front-ends from the generic type struct list_head
 * to the type * dmx_frontend_t
 *. 
*/

#define DMX_FE_ENTRY(list) list_entry(list, dmx_frontend_t, connectivity_list) 

struct dmx_demux_s { 
        /* The following char* fields point to NULL terminated strings */ 
        char* id;                    /* Unique demux identifier */ 
        char* vendor;                /* Name of the demux vendor */ 
        char* model;                 /* Name of the demux model */ 
        __u32 capabilities;          /* Bitfield of capability flags */ 
        dmx_frontend_t* frontend;    /* Front-end connected to the demux */ 
        struct list_head reg_list;   /* List of registered demuxes */
        void* priv;                  /* Pointer to private data of the API client */ 
        int users;                   /* Number of users */
        int (*open) (struct dmx_demux_s* demux); 
        int (*close) (struct dmx_demux_s* demux); 
        int (*write) (struct dmx_demux_s* demux, const char* buf, size_t count); 
        int (*allocate_ts_feed) (struct dmx_demux_s* demux, 
				 dmx_ts_feed_t** feed, 
				 dmx_ts_cb callback,
				 int type,
				 dmx_ts_pes_t pes_type); 
        int (*release_ts_feed) (struct dmx_demux_s* demux, 
				dmx_ts_feed_t* feed); 
        int (*allocate_pes_feed) (struct dmx_demux_s* demux, 
				  dmx_pes_feed_t** feed, 
				  dmx_pes_cb callback); 
        int (*release_pes_feed) (struct dmx_demux_s* demux, 
				 dmx_pes_feed_t* feed); 
        int (*allocate_section_feed) (struct dmx_demux_s* demux, 
				      dmx_section_feed_t** feed, 
				      dmx_section_cb callback); 
        int (*release_section_feed) (struct dmx_demux_s* demux,
				     dmx_section_feed_t* feed); 
        int (*descramble_mac_address) (struct dmx_demux_s* demux, 
				       __u8* buffer1, 
				       size_t buffer1_length, 
				       __u8* buffer2, 
				       size_t buffer2_length,
				       __u16 pid); 
        int (*descramble_section_payload) (struct dmx_demux_s* demux,
					   __u8* buffer1, 
					   size_t buffer1_length,
					   __u8* buffer2, size_t buffer2_length,
					   __u16 pid); 
        int (*add_frontend) (struct dmx_demux_s* demux, 
			     dmx_frontend_t* frontend); 
        int (*remove_frontend) (struct dmx_demux_s* demux,
				dmx_frontend_t* frontend); 
        struct list_head* (*get_frontends) (struct dmx_demux_s* demux); 
        int (*connect_frontend) (struct dmx_demux_s* demux, 
				 dmx_frontend_t* frontend); 
        int (*disconnect_frontend) (struct dmx_demux_s* demux); 

   
        /* added because js cannot keep track of these himself */
        int (*get_pes_pids) (struct dmx_demux_s* demux, __u16 *pids);

	void (*flush_pcr)(void);
	void (*set_pcr_pid) (int pid);
}; 
typedef struct dmx_demux_s dmx_demux_t; 

/*--------------------------------------------------------------------------*/ 
/* Demux directory */ 
/*--------------------------------------------------------------------------*/ 

/* 
 * DMX_DIR_ENTRY(): Casts elements in the list of registered 
 * demuxes from the generic type struct list_head* to the type dmx_demux_t
 *. 
 */ 

#define DMX_DIR_ENTRY(list) list_entry(list, dmx_demux_t, reg_list)

int dmx_register_demux (dmx_demux_t* demux); 
int dmx_unregister_demux (dmx_demux_t* demux); 
struct list_head* dmx_get_demuxes (void); 

#endif /* #ifndef __DEMUX_H */

