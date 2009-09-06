#ifndef __subtitle_h
#define __subtitle_h

#include <lib/base/ebase.h>

typedef unsigned char __u8;

struct subtitle_clut_entry
{
	__u8 Y, Cr, Cb, T;
};

struct subtitle_clut
{
	int clut_id;
	int size;
	struct subtitle_clut_entry entries[16];
	int CLUT_version_number;
	
	struct subtitle_clut *next;
};

struct subtitle_page_region
{
	int region_id;
	int region_horizontal_address;
	int region_vertical_address;
	struct subtitle_page_region *next;
};

struct subtitle_region_object
{
	int object_id;
	int object_type;
	int object_provider_flag;
	
	int object_horizontal_position;
	int object_vertical_position;
	
		// not supported right now...
	int foreground_pixel_value;
	int background_pixel_value;

	struct subtitle_region_object *next;
};

struct subtitle_region
{
	int region_id;
	int region_version_number;
	int region_height, region_width;
	__u8 *region_buffer;
	
	int clut_id;
	
	struct subtitle_region_object *region_objects;
	
	struct subtitle_region *next;
};

struct subtitle_page
{
	int page_id;
	time_t page_time_out;
	int page_version_number;
	int pcs_size;
	struct subtitle_page_region *page_regions;
	
	struct subtitle_region *regions;

	struct subtitle_clut *cluts;

	struct subtitle_page *next;
};

struct subtitle_ctx
{
	struct subtitle_page *pages;

	int current_clut_id, current_clut_page_id;

	__u8 *screen_buffer;
	int screen_width, screen_height;
	int screen_enabled;
	int bpp, stride;
	eTimer *timeout_timer;

	int bbox_left, bbox_top, bbox_right, bbox_bottom;
	void (*set_palette)(struct subtitle_clut *pal);
	int isdrawn;
};

struct bitstream
{
	__u8 *data;
	int size;
	int avail;
	int consumed;
};

void bitstream_init(struct bitstream *bit, void *buffer, int size);
int bitstream_get(struct bitstream *bit);
void subtitle_process_line(struct subtitle_ctx *sub, struct subtitle_page *page, int object_id, int line, __u8 *data, int len);
int subtitle_process_pixel_data(struct subtitle_ctx *sub, struct subtitle_page *page, int object_id, int *linenr, int *linep, __u8 *data);
int subtitle_process_segment(struct subtitle_ctx *sub, __u8 *segment);
void subtitle_process_pes(struct subtitle_ctx *sub, void *buffer, int len);
void subtitle_clear_screen(struct subtitle_ctx *sub);
void subtitle_reset(struct subtitle_ctx *sub);
void subtitle_redraw_all(struct subtitle_ctx *sub);
void subtitle_redraw(struct subtitle_ctx *sub, int pageid);
void subtitle_screen_enable(struct subtitle_ctx *sub, int enable);

#endif
