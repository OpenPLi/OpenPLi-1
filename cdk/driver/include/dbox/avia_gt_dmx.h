/*
 *   avia_gt_dmx.h - dmx driver for AViA (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Florian Schirmer (jolt@tuxbox.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef AVIA_GT_DMX_H
#define AVIA_GT_DMX_H

#define AVIA_GT_DMX_QUEUE_COUNT			32		// HIGH_SPEED queue isn't really a queue

#define AVIA_GT_DMX_QUEUE_VIDEO			0
#define AVIA_GT_DMX_QUEUE_AUDIO			1
#define AVIA_GT_DMX_QUEUE_TELETEXT		2
#define AVIA_GT_DMX_QUEUE_USER_START	3
#define AVIA_GT_DMX_QUEUE_USER_END		30
#define AVIA_GT_DMX_QUEUE_MESSAGE		31
#define AVIA_GT_DMX_QUEUE_HIGH_SPEED	32

typedef struct {

	u32	(*bytes_avail)(u8 queue_nr);
	u32 (*crc32)(u8 queue_nr, u32 count, u32 seed);
	u32	(*get_data)(u8 queue_nr, void *dest, u32 count, u8 peek);
	u8 (*get_data8)(u8 queue_nr, u8 peek);
	u16 (*get_data16)(u8 queue_nr, u8 peek);
	u32 (*get_data32)(u8 queue_nr, u8 peek);
	u32 (*peek_data)(u8 queue_nr, u32 index, void *dest, u32 count);
	u32	(*put_data)(u8 queue_nr, void *src, u32 count, u8 src_is_user_space);
	u8 nr;

} sAviaGtDmxQueueInfo;

typedef void (AviaGtDmxQueueProc)(u8 queue_nr, void *priv_data);

typedef struct {

	u8 busy;
	AviaGtDmxQueueProc *cb_proc;
	u32 hw_read_pos;
	u32 hw_write_pos;
	sAviaGtDmxQueueInfo info;
	u32 irq_count;
	AviaGtDmxQueueProc *irq_proc;
	u32 mem_addr;
	u8 overflow_count;
	void *priv_data;
	u32 read_pos;
	u32 size;
	u32 write_pos;
	u8 is_qim;

} sAviaGtDmxQueue;

#pragma pack(1)

typedef struct {

    u32 wait_pusi: 1;
    u32 VALID: 1;
    u32 Reserved1: 1;
    u32 PID: 13;

} sPID_Entry;

typedef struct {

	u32 type: 3;
	u32 QID: 5;
	u32 fork: 1;
	u32 CW_offset: 3;
	u32 CC: 4;
	u32 _PSH: 1;
	u32 start_up: 1;
	u32 PEC: 1;
	u32 filt_tab_idx: 5;
	u32 State: 3;
	u32 Reserved1: 1;
	u32 no_of_filter: 4;

} sPID_Parsing_Control_Entry;

typedef struct {

	u8 and_or_flag: 1;
	u8 filter_param_id: 5;
	u8 Reserved: 2;

} sFilter_Definition_Entry;

typedef struct {

	u8 mask_0;
	u8 param_0;
	u8 mask_1;
	u8 param_1;
	u8 mask_2;
	u8 param_2;

} sFilter_Parameter_Entry1;

typedef struct {

	u8 mask_3;
	u8 param_3;
	u8 mask_4;
	u8 param_4;
	u8 mask_5;
	u8 param_5;

} sFilter_Parameter_Entry2;

typedef struct {

	u8 mask_6;
	u8 param_6;
	u8 mask_7;
	u8 param_7;
	u32 Reserved1: 3;
	u32 not_flag: 1;
	u32 Reserved2: 2;
	u32 not_flag_ver_id_byte: 1;
	u32 Reserved3: 1;
	u8 Reserved4;

} sFilter_Parameter_Entry3;

typedef struct {

    u8 Microcode[1024];
    sFilter_Parameter_Entry1 Filter_Parameter_Table1[32];
    u8 Reserved1[16];
    u8 Reserved1c[48];
    sFilter_Parameter_Entry2 Filter_Parameter_Table2[32];
    u8 Reserved2[16];
    u8 Reserved2c[48];
    sFilter_Parameter_Entry3 Filter_Parameter_Table3[32];
    u8 Reserved3[16];
    u8 Reserved3c[48];
    sPID_Entry PID_Search_Table[32];
    sPID_Parsing_Control_Entry PID_Parsing_Control_Table[32];
    sFilter_Definition_Entry Filter_Definition_Table[32];
    u8 Reserved4[30];
    u8 Version_no[2];

} sRISC_MEM_MAP;

typedef struct {

	u8 type;
	u8 expected_cc;
	u8 detected_cc;
	u16 pid;

} sCC_ERROR_MESSAGE;

typedef struct {

	u8 type;
	u8 filter_index;
	u16 pid;

} sSECTION_COMPLETED_MESSAGE;

typedef struct {

	u8 type;
	u16 pid;
	u8 cc;
	u8 length;
	
} sPRIVATE_ADAPTION_MESSAGE;

#pragma pack()

#define DMX_MESSAGE_CC_ERROR			0xFE
#define DMX_MESSAGE_SYNC_LOSS			0xFD
#define DMX_MESSAGE_ADAPTION			0xFC
#define DMX_MESSAGE_SECTION_COMPLETED	0xCE

s32 avia_gt_dmx_alloc_queue_audio(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *cb_data);
s32 avia_gt_dmx_alloc_queue_message(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *cb_data);
s32 avia_gt_dmx_alloc_queue_teletext(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *cb_data);
s32 avia_gt_dmx_alloc_queue_user(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *cb_data);
s32 avia_gt_dmx_alloc_queue_video(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *cb_data);
void avia_gt_dmx_fake_queue_irq(u8 queue_nr);
s32 avia_gt_dmx_free_queue(u8 queue_nr);
void avia_gt_dmx_force_discontinuity(void);
void avia_gt_dmx_set_pcr_pid(u16 pid);
int avia_gt_dmx_set_pid_control_table(u8 entry, u8 type, u8 queue, u8 fork, u8 cw_offset, u8 cc, u8 start_up, u8 pec, u8 filt_tab_idx, u8 _psh);
int avia_gt_dmx_set_pid_table(u8 entry, u8 wait_pusi, u8 valid, u16 pid);
sAviaGtDmxQueue *avia_gt_dmx_get_queue_info(u8 queue_nr);
u16 avia_gt_dmx_get_queue_irq(u8 queue_nr);
u32 avia_gt_dmx_queue_get_bytes_free(u8 queue_nr);
u32 avia_gt_dmx_queue_get_write_pos(u8 queue_nr);
void avia_gt_dmx_queue_irq_disable(u8 queue_nr);
s32 avia_gt_dmx_queue_irq_enable(u8 queue_nr);
s32 avia_gt_dmx_queue_reset(u8 queue_nr);
void avia_gt_dmx_queue_set_write_pos(unsigned char queue_nr, unsigned int write_pointer);
void avia_gt_dmx_set_queue_irq(unsigned char queue_nr, unsigned char qim, s32 write_pos);
void avia_gt_dmx_set_queue(unsigned char queue_nr, unsigned int write_pointer, unsigned char size);
void gtx_set_queue_pointer(int queue, u32 read, u32 write, int size, int halt);
int avia_gt_dmx_start_stop_feed(unsigned entry, unsigned what);
int avia_gt_dmx_set_section_filter(void *v_gtx, unsigned entry, unsigned no_of_filters, void *v_secfilter);
int avia_gt_dmx_set_filter_definition_table(u8 entry, u8 and_or_flag, u8 filter_param_id);
int avia_gt_dmx_set_filter_parameter_table(u8 entry, u8 mask[], u8 param[], u8 not_flag, u8 not_flag_ver_id_byte);

u32 avia_gt_dmx_system_queue_get_read_pos(u8 queue_nr);
void avia_gt_dmx_system_queue_set_pos(u8 queue_nr, u32 read_pos, u32 write_pos);
void avia_gt_dmx_system_queue_set_read_pos(u8 queue_nr, u32 read_pos);
void avia_gt_dmx_system_queue_set_write_pos(u8 queue_nr, u32 write_pos);

u8 avia_gt_dmx_get_hw_sec_filt_avail(void);
void avia_gt_dmx_release_section_filter(void *v_gtx, unsigned entry);
int avia_gt_dmx_init(void);
void avia_gt_dmx_exit(void);

#endif
