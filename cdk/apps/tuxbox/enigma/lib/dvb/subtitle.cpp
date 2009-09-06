#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <time.h>

#include <asm/types.h>
#include <lib/dvb/subtitle.h>
#include <lib/base/eerror.h>
#include <lib/gdi/fb.h>
#include <lib/gdi/font.h>
#include <lib/gdi/gfbdc.h>
#include <lib/gui/eskin.h>

void bitstream_init(struct bitstream *bit, void *buffer, int size)
{
	bit->data = (__u8*) buffer;
	bit->size = size;
	bit->avail = 8;
	bit->consumed = 0;
}

int bitstream_get(struct bitstream *bit)
{
	int val;
	bit->avail -= bit->size;
	val = ((*bit->data) >> bit->avail) & ((1<<bit->size) - 1);
	if (!bit->avail)
	{
		bit->data++;
		bit->consumed++;
		bit->avail = 8;
	}
	return val;
}

void subtitle_process_line(struct subtitle_ctx *sub, struct subtitle_page *page, int object_id, int line, __u8 *data, int len)
{
	struct subtitle_region *region = page->regions;
//	eDebug("line for %d:%d", page->page_id, object_id);
	while (region)
	{
		struct subtitle_region_object *object = region->region_objects;
		while (object)
		{
			if (object->object_id == object_id)
			{
				int x = object->object_horizontal_position;
				int y = object->object_vertical_position + line;
				if (x + len > region->region_width)
				{
					//eDebug("[SUB] !!!! XCLIP %d + %d > %d", x, len, region->region_width);
					len = region->region_width - x;
				}
				if (len < 0)
					break;
				if (y >= region->region_height)
				{
					//eDebug("[SUB] !!!! YCLIP %d >= %d", y, region->region_height);
					break;
				}
//				//eDebug("inserting %d bytes (into region %d)", len, region->region_id);
				memcpy(region->region_buffer + region->region_width * y + x, data, len);
			}
			object = object->next;
		}
		region = region->next;
	}
}

void subtitle_process_string(struct subtitle_ctx *sub, struct subtitle_page *page, int object_id, __u8 *data, int len)
{
	struct subtitle_region *region = page->regions;
	gFont font=eSkin::getActive()->queryFont("global.normal");

	while (region)
	{
		struct subtitle_region_object *object = region->region_objects;
		while (object)
		{
			if (object->object_id == object_id)
			{
				eString s = convertDVBUTF8(data,len);

				gPixmap pixmap;
				pixmap.x=region->region_width;
				pixmap.y=region->region_height;
				pixmap.bpp=8;
				pixmap.bypp=1;
				pixmap.stride=region->region_width;
				pixmap.data=region->region_buffer;
				pixmap.clut.colors=gFBDC::getInstance()->getPixmap().clut.colors;
				pixmap.clut.data=gFBDC::getInstance()->getPixmap().clut.data;

				eRect pos( object->object_horizontal_position, object->object_vertical_position, region->region_width - object->object_horizontal_position, region->region_height - object->object_vertical_position);
				gPixmapDC* outputDC = new gPixmapDC(&pixmap);
				gPainter* p = new gPainter(*outputDC);
				p->setBackgroundColor(gColor(object->background_pixel_value| 0xF0));
				p->setForegroundColor(gColor(object->foreground_pixel_value| 0xF0));
				eTextPara *para = new eTextPara( pos );
				para->setFont( font );
				para->renderString( s );
				eRect bbox(para->getBoundBox());
				bbox.moveBy(-2, -2);
				bbox.setWidth(bbox.width()+4);
				bbox.setHeight(bbox.height()+4);

				p->clip(bbox);
				p->clear();
				p->renderPara( *para );
				para->destroy();
				p->clippop();
				p->flush();
				delete p;	
				delete outputDC;
				pixmap.clut.data=0;
//eDebug("process_string:%s,%d,%d,%d,%d",s.c_str(),len,object->background_pixel_value,object->foreground_pixel_value,region->clut_id);
			}
			object = object->next;
		}
		region = region->next;
	}
}

static int map_2_to_4_bit_table[4] = { 0, 8, 7, 15 };

int subtitle_process_pixel_data(struct subtitle_ctx *sub, struct subtitle_page *page, int object_id, int *linenr, int *linep, __u8 *data)
{
	int data_type = *data++;
	static __u8 line[720];
	struct bitstream bit;
	bit.size=0;
	switch (data_type)
	{
	case 0x10: // 2bit pixel data
		bitstream_init(&bit, data, 2);
		while (1)
		{
			int len=0, col=0;
			int code = bitstream_get(&bit);
			if (code)
			{
				col = code;
				len = 1;
			} else
			{
				code = bitstream_get(&bit);
				if (!code)
				{
					code = bitstream_get(&bit);
					if (code == 1)
					{
						col = 0;
						len = 2;
					} else if (code == 2)
					{
						len = bitstream_get(&bit) << 2;
						len |= bitstream_get(&bit);
						len += 12;
						col = bitstream_get(&bit);
					} else if (code == 3)
					{
						len = bitstream_get(&bit) << 6;
						len |= bitstream_get(&bit) << 4;
						len |= bitstream_get(&bit) << 2;
						len |= bitstream_get(&bit);
						len += 29;
						col = bitstream_get(&bit);
					} else
						break;
				} else if (code==1)
				{
					col = 0;
					len = 1;
				} else if (code&2)  
				{
					if (code&1)
						len = 3 + 4 + bitstream_get(&bit);
					else
						len = 3 + bitstream_get(&bit);
					col = bitstream_get(&bit);
				}
			}
			col = map_2_to_4_bit_table[col];
			while (len && ((*linep) < 720))
			{
				line[(*linep)++] = col | 0xF0;
				len--;
			}
		}
		while (bit.avail != 8)
			bitstream_get(&bit);
		return bit.consumed + 1;
	case 0x11: // 4bit pixel data
		bitstream_init(&bit, data, 4);
		while (1)
		{
			int len=0, col=0;
			int code = bitstream_get(&bit);
			if (code)
			{
				col = code;
				len = 1;
			} else
			{
				code = bitstream_get(&bit);
				if (!code)
					break;
				else if (code == 0xC)
				{
					col = 0;
					len = 1;
				} else if (code == 0xD)
				{
					col = 0;
					len = 2;
				} else if (code < 8)
				{
					col = 0;
					len = (code & 7) + 2;
				} else if ((code & 0xC) == 0x8)
				{
					col = bitstream_get(&bit);
					len = (code & 3) + 4;
				} else if (code == 0xE)
				{
					len = bitstream_get(&bit) + 9;
					col = bitstream_get(&bit);
				} else if (code == 0xF)
				{
					len  = bitstream_get(&bit) << 4;
					len |= bitstream_get(&bit);
					len += 25;
					col  = bitstream_get(&bit);
				}
			}
			while (len && ((*linep) < 720))
			{
				line[(*linep)++] = col | 0xF0;
				len--;
			}
		}
		while (bit.avail != 8)
			bitstream_get(&bit);
		return bit.consumed + 1;
	case 0x12: // 8bit pixel data
		bitstream_init(&bit, data, 8);
		while(1)
		{
			int len=0, col=0;
			int code = bitstream_get(&bit);
			if (code)
			{
				col = code;
				len = 1;
			} else
			{
				code = bitstream_get(&bit);
				if ((code & 0x80) == 0x80)
				{
					len = code&0x7F;
					col = bitstream_get(&bit);
				} else if (code&0x7F)
				{
					len = code&0x7F;
					col = 0;
				} else
					break;
			}
			col >>= 4; // 8 -> 4 bit reduction
			while (len && ((*linep) < 720))
			{
				line[(*linep)++] = col | 0xF0;
				len--;
			}
		}
		return bit.consumed + 1;
	case 0x20:  // 2 -> 4bit map table
		bitstream_init(&bit, data, 4);
		for ( int i=0; i < 4; ++i )
			map_2_to_4_bit_table[i] = bitstream_get(&bit);
		return bit.consumed + 1;
	case 0x21:  // ignore 2 -> 8bit map table
		bitstream_init(&bit, data, 8);
		for ( int i=0; i < 4; ++i )
			bitstream_get(&bit);
		return bit.consumed + 1;
	case 0x22:  // ignore 4 -> 8bit map table
		bitstream_init(&bit, data, 8);
		for ( int i=0; i < 16; ++i )
			bitstream_get(&bit);
		return bit.consumed + 1;
	case 0xF0:
		subtitle_process_line(sub, page, object_id, *linenr, line, *linep);
/*		{
			int i;
			for (i=0; i<720; ++i)
				//eDebugNoNewLine("%d ", line[i]);
			//eDebug("");
		} */
		(*linenr)+=2; // interlaced
		*linep = 0;
//		//eDebug("[SUB] EOL");
		return 1;
	default:
		eDebug("subtitle_process_pixel_data: invalid data_type %02x", data_type);
		return -1;
	}
	return 0;
}

int subtitle_process_segment(struct subtitle_ctx *sub, __u8 *segment)
{
	int segment_type, page_id, segment_length, processed_length;
	if (*segment++ !=  0x0F)
	{
		eDebug("out of sync.");
		return -1;
	}
	segment_type = *segment++;
	page_id  = *segment++ << 8;
	page_id |= *segment++;
	segment_length  = *segment++ << 8;
	segment_length |= *segment++;
	if (segment_type == 0xFF)
		return segment_length + 6;
//	//eDebug("have %d bytes of segment data", segment_length);
	
//	//eDebug("page_id %d, segtype %02x", page_id, segment_type);

	sub->isdrawn = 0;
	// recreate default map
	map_2_to_4_bit_table[0] = 0;
	map_2_to_4_bit_table[1] = 8;
	map_2_to_4_bit_table[2] = 7;
	map_2_to_4_bit_table[3] = 15;
	
	struct subtitle_page *page, **ppage;
		
	page = sub->pages; ppage = &sub->pages;
		
	while (page)
	{
		if (page->page_id == page_id)
			break;
		ppage = &page->next;
		page = page->next;
	}

	processed_length = 0;
	
	switch (segment_type)
	{
	case 0x10: // page composition segment
	{
		int page_time_out = *segment++; processed_length++;
		int page_version_number = *segment >> 4;
		int page_state = (*segment >> 2) & 0x3;
		//eDebug("pcs with %d bytes data (%d:%d:%d)", segment_length, page_id, page_version_number, page_state);
		segment++;
		processed_length++;
		
		//eDebug("page time out: %d", page_time_out);
		//eDebug("page_version_number: %d" ,page_version_number);
		//eDebug("page_state: %d", page_state);
		
		if (!page)
		{
			//eDebug("page not found");
			page = new subtitle_page;
			page->page_regions = 0;
			page->regions = 0;
			page->page_id = page_id;
			page->cluts = 0;
			page->next = 0;
			*ppage = page;
		} else
		{
			if (page->pcs_size != segment_length)
				page->page_version_number = -1;
				// if no update, just skip this data.
			if (page->page_version_number == page_version_number)
			{
				eDebug("skip data... ");
				break;
			}
		}

//		eDebug("page updated: old: %d, new: %d", page->page_version_number, page_version_number);
			// when acquisition point or mode change: remove all displayed pages.
		if ((page_state == 1) || (page_state == 2))
		{
			while (page->page_regions)
			{
				struct subtitle_page_region *p = page->page_regions->next;
				delete page->page_regions;
				page->page_regions = p;
			}
			while (page->regions)
			{
				struct subtitle_region *p = page->regions->next;
				while(page->regions->region_objects)
				{
					subtitle_region_object *ob = page->regions->region_objects->next;
					delete page->regions->region_objects;
					page->regions->region_objects = ob;
				}
				delete [] page->regions->region_buffer;
				delete page->regions;
				page->regions = p;
			}
		}

//		eDebug("new page.. (%d)", page_state);

		page->page_time_out = page_time_out;

		page->page_version_number = page_version_number;

		/* 
			only display regions mentioned in this segment
			=> remove all previous region references from this page
			(at least if I understand ETSI EN 300 743 V1.2.1 (2002-10) Chapter 5.1.3 correctly)
		*/
		while (page->page_regions)
		{
			struct subtitle_page_region *p = page->page_regions->next;
			delete page->page_regions;
			page->page_regions = p;
		}

		struct subtitle_page_region **r = &page->page_regions;
		
		//eDebug("%d  / %d data left", processed_length, segment_length);
		
			// go to last entry
		while (*r)
			r = &(*r)->next;

		if (processed_length == segment_length && !page->page_regions)
		{
//			eDebug("no regions in page.. clear screen!!");
			if (sub->screen_enabled)
				subtitle_redraw_all(sub);
		}

		while (processed_length < segment_length)
		{
			struct subtitle_page_region *pr;
			
				// append new entry to list
			pr = new subtitle_page_region;
			pr->next = 0;
			*r = pr;
			r = &pr->next;
			
			pr->region_id = *segment++; processed_length++;
			segment++; processed_length++;

			pr->region_horizontal_address  = *segment++ << 8; 
			pr->region_horizontal_address |= *segment++;
			processed_length += 2;
			
			pr->region_vertical_address  = *segment++ << 8;
			pr->region_vertical_address |= *segment++;
			processed_length += 2;
			
			//eDebug("appended active region");
		}
		
		if (processed_length != segment_length)
			eDebug("%d != %d", processed_length, segment_length);
		break;
	}
	case 0x11: // region composition segment
	{
		int region_id = *segment++; processed_length++;
		int region_version_number = *segment >> 4;
		int region_fill_flag = (*segment >> 3) & 1;
		segment++; processed_length++;
		
			// if we didn't yet received the pcs for this page, drop the region
		if (!page)
		{
			eDebug("ignoring region %x, since page %02x doesn't yet exist.", region_id, page_id);
			break;
		}
		
		struct subtitle_region *region, **pregion;
		
		region = page->regions; pregion = &page->regions;

		while (region)
		{
			fflush(stdout);
			if (region->region_id == region_id)
				break;
			pregion = &region->next;
			region = region->next;
		}
		
		if (!region)
		{
			*pregion = region = new subtitle_region;
			region->next = 0;
		}
		else if (region->region_version_number != region_version_number)
		{
			struct subtitle_region_object *objects = region->region_objects;
			while (objects)
			{
				struct subtitle_region_object *n = objects->next;
				delete objects;
				objects = n;
			}
			delete [] region->region_buffer;
		}
		else 
			break;
		
		//eDebug("region %d:%d update", page_id, region_id);
			
		region->region_id = region_id;
		region->region_version_number = region_version_number;
		
		region->region_width  = *segment++ << 8;
		region->region_width |= *segment++;
		processed_length += 2;
		
		region->region_height  = *segment++ << 8;
		region->region_height |= *segment++;
		processed_length += 2;

		region->region_buffer = new __u8[region->region_height * region->region_width];

		int region_level_of_compatibility, region_depth;
		
		region_level_of_compatibility = (*segment >> 5) & 7;
		region_depth = (*segment++ >> 2) & 7;
		processed_length++;
		
		int CLUT_id;
		CLUT_id = *segment++; processed_length++;
		
		region->clut_id = CLUT_id;
		
		int region_8bit_pixel_code, region_4bit_pixel_code, region_2bit_pixel_code;
		region_8bit_pixel_code = *segment++; processed_length++;
		region_4bit_pixel_code = *segment >> 4;
		region_2bit_pixel_code = (*segment++ >> 2) & 3;	
		processed_length++;
		
		if (!region_fill_flag)
		{
			region_2bit_pixel_code = region_4bit_pixel_code = region_8bit_pixel_code = 0;
			region_fill_flag = 1;
		}
		
		if (region_fill_flag)
		{
			if (region_depth == 1)
				memset(region->region_buffer, region_2bit_pixel_code, region->region_height * region->region_width);
			else if (region_depth == 2)
				memset(region->region_buffer, region_4bit_pixel_code, region->region_height * region->region_width);
			else if (region_depth == 3)
				memset(region->region_buffer, region_8bit_pixel_code, region->region_height * region->region_width);
			else
				eDebug("!!!! invalid depth");
		}
		
		//eDebug("region %02x, version %d, %dx%d", region->region_id, region->region_version_number, region->region_width, region->region_height);
		
		region->region_objects = 0;
		struct subtitle_region_object **pobject = &region->region_objects;
		
		while (processed_length < segment_length)
		{
			
			struct subtitle_region_object *object;
			
			object = new subtitle_region_object;

			*pobject = object;
			object->next = 0;
			pobject = &object->next;
			
			object->object_id  = *segment++ << 8;
			object->object_id |= *segment++; processed_length += 2;
			
			object->object_type = *segment >> 6;
			object->object_provider_flag = (*segment >> 4) & 3;
			object->object_horizontal_position  = (*segment++ & 0xF) << 8;
			object->object_horizontal_position |= *segment++; 
			processed_length += 2;
			
			object->object_vertical_position  = *segment++ << 8;
			object->object_vertical_position |= *segment++ ;
			processed_length += 2;
			
			if ((object->object_type == 1) || (object->object_type == 2))
			{
				object->foreground_pixel_value = *segment++;
				object->background_pixel_value = *segment++;
				processed_length += 2;
			}
		}

		if (processed_length != segment_length)
			eDebug("too less data! (%d < %d)", segment_length, processed_length);
		
		break;
	}
	case 0x12: // CLUT definition segment
	{
		int CLUT_id, CLUT_version_number;
		struct subtitle_clut *clut, **pclut;
		
		if (!page)
			break;

		//eDebug("CLUT: %02x", *segment);
		CLUT_id = *segment++;
		
		CLUT_version_number = *segment++ >> 4;
		processed_length += 2;

		//eDebug("page %d, CLUT %02x, version %d", page->page_id, CLUT_id, CLUT_version_number);

		clut = page->cluts; pclut = &page->cluts;
		
		while (clut)
		{
			if (clut->clut_id == CLUT_id)
				break;
			pclut = &clut->next;
			clut = clut->next;
		}
		
		if (!clut)
		{
			*pclut = clut = new subtitle_clut;
			clut->next = 0;
		} else if (clut->CLUT_version_number == CLUT_version_number)
			break;
			
		clut->clut_id = CLUT_id;

			/* invalidate CLUT if updated. */			
		if ((sub->current_clut_page_id == page_id) && (sub->current_clut_id == CLUT_id))
			sub->current_clut_id = -1;
			
		clut->size = 0;
			
		//eDebug("new clut");
		while (processed_length < segment_length)
		{
			int CLUT_entry_id, entry_CLUT_flag, full_range;
			int v_Y, v_Cr, v_Cb, v_T;
			
			CLUT_entry_id = *segment++;
			entry_CLUT_flag = *segment >> 5;
			full_range = *segment++ & 1;
			processed_length += 2;
			
			if (full_range)
			{
				v_Y  = *segment++;
				v_Cr = *segment++;
				v_Cb = *segment++;
				v_T  = *segment++;
				processed_length += 4;
			} else
			{
				v_Y   = *segment & 0xFC;
				v_Cr  = (*segment++ & 3) << 6;
				v_Cr |= (*segment & 0xC0) >> 2;
				v_Cb  = (*segment & 0x3C) << 2;
				v_T   = (*segment++ & 3) << 6;
				processed_length += 2;
			}
			if (!(entry_CLUT_flag & 6)) // if no 4 or 16 color entry, skip it
			{
				eDebug("skipped 8bit CLUT entry");
				continue;
			}
			
			clut->entries[CLUT_entry_id].Y = v_Y; 
			clut->entries[CLUT_entry_id].Cr = v_Cr; 
			clut->entries[CLUT_entry_id].Cb = v_Cb; 
			clut->entries[CLUT_entry_id].T = v_T; 
			if (clut->size <= CLUT_entry_id)
				clut->size = CLUT_entry_id+1;
			//eDebug("  %04x %02x %02x %02x %02x", CLUT_entry_id, v_Y, v_Cb, v_Cr, v_T);
		}
		break;
	}
	case 0x13: // object data segment
	{
		int object_id, object_version_number, object_coding_method, non_modifying_color_flag;
		
		object_id  = *segment++ << 8;
		object_id |= *segment++;
		processed_length += 2;
		
		object_version_number = *segment >> 4;
		object_coding_method  = (*segment >> 2) & 3;
		non_modifying_color_flag = (*segment++ >> 1) & 1;
		processed_length++;
		
		//eDebug("object id %04x, version %d, object_coding_method %d (page_id %d)", object_id, object_version_number, object_coding_method, page_id);
		
		if (object_coding_method == 0)
		{
			int top_field_data_blocklength, bottom_field_data_blocklength;
			int i, line, linep;
			
			top_field_data_blocklength  = *segment++ << 8;
			top_field_data_blocklength |= *segment++;
			
			bottom_field_data_blocklength  = *segment++ << 8;
			bottom_field_data_blocklength |= *segment++;
			//eDebug("%d / %d bytes", top_field_data_blocklength, bottom_field_data_blocklength);
			processed_length += 4;
			
			i = 0;
			line = 0;
			linep = 0;
			while (i < top_field_data_blocklength)
			{
				int len;
				len = subtitle_process_pixel_data(sub, page, object_id, &line, &linep, segment);
				if (len < 0)
					return -1;
				segment += len;
				processed_length += len;
				i += len;
			}
			
			line = 1;
			linep = 0;

			if (bottom_field_data_blocklength)
			{
				i = 0;
				while (i < bottom_field_data_blocklength)
				{
					int len;
					len = subtitle_process_pixel_data(sub, page, object_id, &line, &linep, segment);
					if (len < 0)
						return -1;
					segment += len;
					processed_length += len;
					i += len;
				}
			} 
			else if (top_field_data_blocklength)
				eDebug("!!!! unimplemented: no bottom field! (%d : %d)", top_field_data_blocklength, bottom_field_data_blocklength);

			if ((top_field_data_blocklength + bottom_field_data_blocklength) & 1)
			{
				segment++; processed_length++;
			}
		} 
		else if (object_coding_method == 1)
		{
			int len = *segment++;
			processed_length++;
			subtitle_process_string(sub, page, object_id, segment, len*2);
			segment += len;
			processed_length += len;
		}
		break;
	}
	case 0x80: // end of display set segment
	{
//		eDebug("end of display set segment");
		if (sub->screen_enabled)
			subtitle_redraw_all(sub);
		sub->isdrawn = 1;
		break;
	}
	case 0xFF: // stuffing
		break;
	default:
		eDebug("unhandled segment type %02x", segment_type);
	}
	
	return segment_length + 6;
}

void subtitle_process_pes(struct subtitle_ctx *sub, void *buffer, int len)
{
	__u8 *pkt = (__u8*)buffer;
	if (pkt[0] || pkt[1] || (pkt[2] != 1))
	{
		//eDebug("%s: invalid startcode %02x %02x %02x", __FUNCTION__, pkt[0], pkt[1], pkt[2]);
		return;
	}
	if (pkt[3] != 0xBD)
	{
		//eDebug("%s: invalid stream_id %02x", __FUNCTION__, pkt[3]);
		return;
	}
	pkt += 6; len -= 6;
	// skip PES header
	pkt++; len--;
	pkt++; len--;
	
	int hdr_len = *pkt++; len--;

	pkt+=hdr_len; len-=hdr_len;

	if (*pkt != 0x20)
	{
		//eDebug("data identifier is 0x%02x, but not 0x20", *pkt);
		return;
	}
	pkt++; len--; // data identifier
	*pkt++; len--; // stream id;
	
	if (len <= 0)
	{
		//eDebug("no data left (%d)", len);
		return;
	}
	
	while (len && *pkt == 0x0F)
	{
		int l = subtitle_process_segment(sub, pkt);
		if (l < 0)
			break;
		pkt += l;
		len -= l;
	}
//	if (len && *pkt != 0xFF)
//		eDebug("strange data at the end");
}

void subtitle_clear_screen(struct subtitle_ctx *sub)
{
		/* clear bbox */
	int y;

	//eDebug("BBOX clear %d:%d -> %d:%d", sub->bbox_left, sub->bbox_top, sub->bbox_right, sub->bbox_bottom);

	// do not draw when anyone has locked the 
	// framebuffer ( non enigma plugins... )
	sub->bbox_right = 720;
	if (sub->bbox_right > sub->bbox_left && !fbClass::getInstance()->islocked())
		for (y=sub->bbox_top; y < sub->bbox_bottom; ++y)
			memset(sub->screen_buffer + y * sub->screen_width, 0, sub->bbox_right - sub->bbox_left);

	sub->bbox_right = 0;
	sub->bbox_left = sub->screen_width;
	sub->bbox_top = sub->screen_height;
	sub->bbox_bottom = 0;
}

void subtitle_redraw_all(struct subtitle_ctx *sub)
{
#if 1
	struct subtitle_page *page = sub->pages;
	if ( page )
		subtitle_clear_screen(sub);
	while(page)
	{
		subtitle_redraw(sub, page->page_id);
		page = page->next;
	}
#else
	struct subtitle_page *page = sub->pages;
	if ( page )
		subtitle_clear_screen(sub);
	//eDebug("----------- end of display set");
	//eDebug("active pages:");
	while (page)
	{
		//eDebug("  page_id %02x", page->page_id);
		//eDebug("  page_version_number: %d", page->page_version_number);
		//eDebug("  active regions:");
		{
			struct subtitle_page_region *region = page->page_regions;
			while (region)
			{
				//eDebug("    region_id: %04x", region->region_id);
				//eDebug("    region_horizontal_address: %d", region->region_horizontal_address);
				//eDebug("    region_vertical_address: %d", region->region_vertical_address);
				
				region = region->next;
			}
		}
		
		subtitle_redraw(sub, page->page_id);
		//eDebug("defined regions:");
		struct subtitle_region *region = page->regions;
		while (region)
		{
			//eDebug("  region_id %04x, version %d, %dx%d", region->region_id, region->region_version_number, region->region_width, region->region_height);
			
			struct subtitle_region_object *object = region->region_objects;
			while (object)
			{
				//eDebug("  object %02x, type %d, %d:%d", object->object_id, object->object_type, object->object_horizontal_position, object->object_vertical_position);
				object = object->next;
			}				
				
			region = region->next;
		}
		page = page->next;
	}
#endif
}

void subtitle_reset(struct subtitle_ctx *sub)
{
	while (struct subtitle_page *page = sub->pages)
	{
			/* free page regions */
		while (page->page_regions)
		{
			struct subtitle_page_region *p = page->page_regions->next;
			delete page->page_regions;
			page->page_regions = p;
		}
			/* free regions */
		while (page->regions)
		{
			struct subtitle_region *region = page->regions;
			
			while (region->region_objects)
			{
				struct subtitle_region_object *obj = region->region_objects;
				region->region_objects = obj->next;
				delete obj;
			}
			
			delete [] region->region_buffer;

			page->regions = region->next;
			delete region;
		}
			
			/* free CLUTs */
		while (page->cluts)
		{
			struct subtitle_clut *clut = page->cluts;
			page->cluts = clut->next;
			delete clut;
		}
		
		sub->pages = page->next;
		delete page;
	}
}

static __u32 * argb_palette( struct subtitle_clut *pal, int clut_id)
{
	static __u32 def_palette[16];
	static __u32 palette[16];
	static bool def_palette_initialized;
	static int prev_clut_id = -1;

	while (pal)
	{
		if (pal->clut_id == clut_id)
			break;
		pal = pal->next;
	}
	if ( !pal )// use default pallette
	{
		// eDebug("Subtitle: default palette");
		if ( !def_palette_initialized )  // fill default palette
		{
			for (int i=0; i < 16; ++i)
			{
				if (!i)
					def_palette[i] = 0x00000000;
				else if (i&8)
				{
					def_palette[i] = 0xFF000000;
					if (i & 1) def_palette[i] |= 0x80 << 16; // r
					if (i & 2) def_palette[i] |= 0x80 << 8; // g
					if (i & 4) def_palette[i] |= 0x80; // b
				}
				else
				{
					def_palette[i] = 0xFF000000;
					if (i & 1) def_palette[i] |= 0xFF << 16;
					if (i & 2) def_palette[i] |= 0xFF << 8;
					if (i & 4) def_palette[i] |= 0xFF;
				}
				// eDebug("%d %08x", i, def_palette[i]);
			}
			def_palette_initialized=1;
		}
		return(def_palette);
	}
	else
	{
		if (clut_id != prev_clut_id)
		{
			// eDebug("Subtitle: palette of %d colors, clut_id = %d", pal->size, clut_id);
			for (int i=0; i<pal->size; ++i)
			{
				int y = pal->entries[i].Y,
				    cr = pal->entries[i].Cr,
				    cb = pal->entries[i].Cb;
				if (y > 0)
				{
					y -= 16;
					cr -= 128;
					cb -= 128;
					palette[i] = 
				             	(MAX(MIN(((298 * y            + 460 * cr) / 256), 255), 0)) << 16 |
				             	(MAX(MIN(((298 * y -  55 * cb - 137 * cr) / 256), 255), 0)) <<  8 |
				             	(MAX(MIN(((298 * y + 543 * cb           ) / 256), 255), 0));
					if (palette[i])
						palette[i] |= (((pal->entries[i].T) & 0xFF) ^ 0xFF) << 24;
					else
						palette[i] |= (0x00) << 24;
					//palette[i].a = (pal->entries[i].T) & 0xFF;
				}
				else
				{
					palette[i] = 0;
				}
				// eDebug("%d: %08x", i, palette[i]);
			}
			prev_clut_id = clut_id;
		}
		return(palette);
	}
}

void subtitle_redraw(struct subtitle_ctx *sub, int page_id)
{
	struct subtitle_page *page = sub->pages;
	int main_clut_id = -1;
	
	//eDebug("displaying page id %d", page_id);
	
	while (page)
	{
		if (page->page_id == page_id)
			break;
		page = page->next;
	}
	if (!page)
	{
		//eDebug("page not found");
		return;
	}
	
	//eDebug("iterating regions..");
		/* iterate all regions in this pcs */
	struct subtitle_page_region *region = page->page_regions;
	while (region)
	{
		//eDebug("region %d", region->region_id);
			/* find corresponding region */
		struct subtitle_region *reg = page->regions;
		while (reg)
		{
			if (reg->region_id == region->region_id)
				break;
			reg = reg->next;
		}
		if (reg)
		{
			int y;
			int clut_id = reg->clut_id;
			//eDebug("clut %d, %d", clut_id, main_clut_id);
			if (main_clut_id != -1)
			{
				if (main_clut_id != clut_id)
				{
					eDebug("MULTIPLE CLUTS IN USE! prepare for pixelmuell!");
//					exit(0);
				}
			}
			main_clut_id = clut_id;
				
			int x0 = region->region_horizontal_address;
			int y0 = region->region_vertical_address;
			int x1 = x0 + reg->region_width;
			int y1 = y0 + reg->region_height;

			//eDebug("copy region %d to %d, %d, size %d %d", region->region_id, x0, y0, x1, y1);

			if ((x0 < 0) || (y0 < 0) || (x0 > sub->screen_width) || (x0 > sub->screen_height))
				continue;

				/* adjust bbox */
			if (x0 < sub->bbox_left)
				sub->bbox_left = x0;
			if (y0 < sub->bbox_top)
				sub->bbox_top = y0;
			if (x1 > sub->bbox_right)
				sub->bbox_right = x1;
			if (y1 > sub->bbox_bottom)
				sub->bbox_bottom = y1;

			// start timeout... 
			sub->timeout_timer->start(page->page_time_out*1000, true);

			// do not draw when anyone has locked the 
			// framebuffer ( non enigma plugins... )
			if ( !fbClass::getInstance()->islocked() )
			{
				if (sub->bpp == 1) // PLi extension for 32bit skins
				{
					/* copy to screen */
					for (y=0; y < reg->region_height; ++y)
					{
						memcpy(sub->screen_buffer + 
							sub->screen_width * (y + region->region_vertical_address) + 
							region->region_horizontal_address, 
							reg->region_buffer + reg->region_width * y, reg->region_width);
					}
				}
				else
				{ // must be 32-bit...
					__u8 *srcptr=(__u8*) reg->region_buffer;
					__u8 *dstptr=(__u8*) (sub->screen_buffer +
								sub->stride * region->region_vertical_address +
								region->region_horizontal_address * 4);
					// Get a colortable
					__u32 *pal = argb_palette(page->cluts, main_clut_id);
				
					for (y = 0; y < reg->region_height; ++y)
					{
						__u8  *srcp = (__u8*)  srcptr;
						__u32 *dstp = (__u32*) dstptr;
						int width = reg->region_width;
						while (width--) {
							//eDebug("sub%d,%d: ind=%02x col=%08x", y, width, *srcp, pal[(*srcp)&0x0F]);
							*dstp++ = pal[(*srcp++) & 0x0F];
						}
						srcptr += reg->region_width;
						dstptr += sub->stride;
					}
				}
			}
		} 
		else
			eDebug("region not found");
		region = region->next;
	}
	//eDebug("schon gut.");
	
	if (main_clut_id == -1)
		return; /* most probably no display active */
	
	if ((sub->current_clut_id == main_clut_id) && (sub->current_clut_page_id == page_id))
		return;
	
	//eDebug("updating clut..");
			/* find corresponding clut */
	struct subtitle_clut *clut = page->cluts;
	while (clut)
	{
		//eDebug("have %d, want %d", clut->clut_id, main_clut_id);
		if (clut->clut_id == main_clut_id)
			break;
		clut = clut->next;
	}
	if (clut)
	{
		// do not change color palette when anyone has 
		// locked the framebuffer ( non enigma plugins )
		if (!fbClass::getInstance()->islocked())
			sub->set_palette(clut);
	}
	else
	{
		if (!fbClass::getInstance()->islocked())
			sub->set_palette(0);
	}
}

void subtitle_screen_enable(struct subtitle_ctx *sub, int enable)
{
	if (sub->screen_enabled == enable)
		return;
	sub->screen_enabled = enable;
	if (enable)
		subtitle_redraw_all(sub);
	else
		subtitle_clear_screen(sub);
}
