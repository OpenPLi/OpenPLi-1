//
//  DBOXII Capture Filter
//  
//  Rev.0.0 Bernd Scharping 
//  bernd@transputer.escape.de
//
//  Origin: MPEG2 remuxer written by Peter Niemayer.
//
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <math.h>
#include "debug.h"
#include "Remuxer.h"
#include "ccircularbuffer.h"
#include "grab.h" 

#pragma warning (disable : 4244)

// #################################################################################

#define DEBUG_RESYNC_DROPS 0

// #################################################################################

const unsigned long known_audio_frame_sizes[] = {
//  MPG/192  AC3/5+1   MPG/64
      576,   896*2,     167
};

// ac3 tables by Aaron Holtzman - May 1999

struct ac3_frmsize_s
{
	unsigned short bit_rate;
	unsigned short frm_size[3];
};

static const int _AudioRateTable[4][16]=
        {
        {0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,0},
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0},
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0}
        };

static const int    _AudioFreqTable[4]={44100,48000,32000,0};

static const struct ac3_frmsize_s ac3_frmsizecod_tbl[64] = 
{
	{ 32  ,{64   ,69   ,96   } },
	{ 32  ,{64   ,70   ,96   } },
	{ 40  ,{80   ,87   ,120  } },
	{ 40  ,{80   ,88   ,120  } },
	{ 48  ,{96   ,104  ,144  } },
	{ 48  ,{96   ,105  ,144  } },
	{ 56  ,{112  ,121  ,168  } },
	{ 56  ,{112  ,122  ,168  } },
	{ 64  ,{128  ,139  ,192  } },
	{ 64  ,{128  ,140  ,192  } },
	{ 80  ,{160  ,174  ,240  } },
	{ 80  ,{160  ,175  ,240  } },
	{ 96  ,{192  ,208  ,288  } },
	{ 96  ,{192  ,209  ,288  } },
	{ 112 ,{224  ,243  ,336  } },
	{ 112 ,{224  ,244  ,336  } },
	{ 128 ,{256  ,278  ,384  } },
	{ 128 ,{256  ,279  ,384  } },
	{ 160 ,{320  ,348  ,480  } },
	{ 160 ,{320  ,349  ,480  } },
	{ 192 ,{384  ,417  ,576  } },
	{ 192 ,{384  ,418  ,576  } },
	{ 224 ,{448  ,487  ,672  } },
	{ 224 ,{448  ,488  ,672  } },
	{ 256 ,{512  ,557  ,768  } },
	{ 256 ,{512  ,558  ,768  } },
	{ 320 ,{640  ,696  ,960  } },
	{ 320 ,{640  ,697  ,960  } },
	{ 384 ,{768  ,835  ,1152 } },
	{ 384 ,{768  ,836  ,1152 } },
	{ 448 ,{896  ,975  ,1344 } },
	{ 448 ,{896  ,976  ,1344 } },
	{ 512 ,{1024 ,1114 ,1536 } },
	{ 512 ,{1024 ,1115 ,1536 } },
	{ 576 ,{1152 ,1253 ,1728 } },
	{ 576 ,{1152 ,1254 ,1728 } },
	{ 640 ,{1280 ,1393 ,1920 } },
	{ 640 ,{1280 ,1394 ,1920 } }
};


// ############################################################################

int find_next_pes_packet(unsigned char * data, unsigned long valid,
                                unsigned long & pes_start, unsigned long & pes_len) {
	
	// pes_len will be == 0 if no packet is found or ready yet
	pes_len = 0;
	
	// pes_start will be either the start of the found packet or
	// a pointer to the first non-junk byte in the buffer.
	// In any case, the caller may discard the bytes from 
	// "data" to "data+pes_start+pes_len"
	// after considering the packet
	
	
	// ------------------------------------
	// find start of next PES packet
	
	pes_start = 0;
		
search_again:	
	
	if (valid < 4) return 0;
	
	for (; pes_start < valid-3; pes_start++) {
		if (   data[pes_start]   == 0x00
		    && data[pes_start+1] == 0x00
			 && data[pes_start+2] == 0x01
			) {
			break;
		}
	}
	
	if (pes_start >= valid-3) {
		return 0;
	}
	
	// forget about data before first header
	
	unsigned char * a = data + pes_start;
	valid -= pes_start;
	
	// check for PES header validity

	if (valid < 6) {
		// ok, just not enough data, yet
		return 0;
	}
	
	unsigned char  stream_id  = a[3];
	unsigned long packet_len = pes_size(a);
	
	if (valid < packet_len) {
		// ok, just not enough data, yet
		return 0;
	}
	
	// ok, the whole PES packet is available in abuf
	
	if (   stream_id == STREAM_PRIVATE_2
	    || stream_id == STREAM_PADDING
		) {

		// ok, the rest of the packet is just data
		
		pes_len = packet_len;
		return 0;
	}
	
	if ( (a[6] & 0xc0) != 0x80) {
		// hmmm.. header looks invalid...
		pes_start += 1;
		goto search_again;
	}
	
	pes_len = packet_len;
	
	return 0;
}

// ##################################################################

int Remuxer::create_audio_pes(unsigned char * orig_pes, double duration) {
	


	// cut one audio PES in pieces so each fits into one program pack

	unsigned long   orig_pes_size = pes_size(orig_pes);
	unsigned long   orig_opt_header_size = orig_pes[8];
	unsigned long   orig_es_size = orig_pes_size - 9 - orig_opt_header_size;
	unsigned char * orig_es_data = orig_pes + 9 + orig_opt_header_size;
	double          orig_pts = pes_pts(orig_pes);

	
	unsigned long es_left   = orig_es_size;
	unsigned char * es_data = orig_es_data;
	
	
	                                             // pp   pes  header
	unsigned long max_audio_per_pes = max_out_pp - (14 + 9+5); // better not create bigger PES packets
	
	unsigned long wanted_audio_per_pes = max_audio_per_pes;
	
	// try to find a better wanted_audio_per_pes by trying known dividers
	static unsigned long last_audio_frame_size = ~(0UL);
	unsigned long audio_frame_size = 0;
	
	for (unsigned int i = 0; i < sizeof(known_audio_frame_sizes)/sizeof(unsigned long); i++) {
		if (orig_es_size % known_audio_frame_sizes[i] == 0) {
			audio_frame_size = known_audio_frame_sizes[i];
			wanted_audio_per_pes = (max_audio_per_pes / audio_frame_size) * audio_frame_size;
		}
	}
	
	if (audio_frame_size != last_audio_frame_size) {
		last_audio_frame_size = audio_frame_size;
		dprintf("detected audio frame size: %ld. ES data size: %ld",    audio_frame_size, orig_es_size);
	}
	
	unsigned long peses = 0;	
	
	bool stamp = true;
	
	bool a_seq_start = true; // we better remember whether this was the first part of an audio PES
	
	while (es_left) {
		
		unsigned long pes_payload = es_left;
		if (pes_payload > wanted_audio_per_pes) {
			pes_payload = wanted_audio_per_pes;
		}
		
		if (audio_packets_avail >= max_audio_packets) {
			dprintf( "audio_packets buffer overrun, dropping some data");			
			remove_audio_packets(1);
		}
		
		unsigned long total_len = 9 + ((stamp)? 5 : 0) + pes_payload;
		
		// create packet in a newly allocated memory buffer...
		unsigned char * ap = new unsigned char[total_len];
		audio_packets[audio_packets_avail] = ap;
		audio_packets_avail += 1;
		
		ap[0] = 0x00;
		ap[1] = 0x00;
		ap[2] = 0x01;
		ap[3] = orig_pes[3];
		ap[4] = ((total_len - 6) >> 8) & 0xff;
		ap[5] = (total_len - 6) & 0xff;
		ap[6] = 0x80 | ((a_seq_start)? 0x01 : 0x00); // dirty: we mark the a_seq_start this way...
		//a_seq_start = false;
		
		if (stamp) {
			//stamp = false;

			ULONGLONG pts64 = (ULONGLONG)orig_pts;
			double part_done = 1.0 - (((double)es_left)/((double)orig_es_size));
			pts64 += (ULONGLONG)(part_done * duration);
			
			ap[7] = 0x80; // PTS, only
			ap[8] = 0x05; // 5 bytes for the PTS
			
			ap[9]  = 0x20 | ((pts64 & 0x1c0000000ui64) >> 29) | 0x01;
			ap[10] =        ((pts64 & 0x03fc00000ui64) >> 22)       ;
			ap[11] =        ((pts64 & 0x0003f8000ui64) >> 14) | 0x01;
			ap[12] =        ((pts64 & 0x000007f80ui64) >>  7)       ;
			ap[13] =        ((pts64 & 0x00000007fui64) <<  1) | 0x01;
			ap += 14;
			
		} else {
			
			ap[7] = 0x00; // no PTS
			ap[8] = 0x00; //
			ap += 9;
		}
		
		memcpy(ap, es_data, pes_payload);
		
		es_data += pes_payload;
		es_left -= pes_payload;
		
		peses += 1;
		
	}

	return 0;
}

// ##################################################################


bool Remuxer::audio_packet_wanted(unsigned char * pes) {
	
	if (wanted_audio_stream == 0) {
		// make a guess: just take the first type of audio stream we find...
		
		if (   pes[3] == STREAM_PRIVATE_1
		    || pes[3] == STREAM_AUDIO
		    || ( (pes[3]>=STREAM_REST_AUDIO) && (pes[3]<=(STREAM_REST_AUDIO+0x1F)) )
		    //|| pes[3] == 0xc1 // EuroNews uses this...
		    //|| pes[3] == 0xc2 // Fritz, Sputnik Radio 
		    //|| pes[3] == 0xc3 // Radiomultikulti
		   ) {
			
			wanted_audio_stream = pes[3];
		}
	}
	
	if (pes[3] == wanted_audio_stream) {
		return true;
	}

	// PES packet did not belong to the stream we wanted..

	return false;
		

}

// ##################################################################

int Remuxer::supply_audio_data(void * data, unsigned long len) {
	
		
	if (abuf_valid + len > abuf_size) {
		dprintf( "abuf overrun, dropping some data");
		abuf_valid = 0;
		if (len > abuf_size) {
			len = abuf_size;
		}
	}
	
	memcpy(abuf+abuf_valid, data, len);
	abuf_valid += len;
	
	// work on as many PES packets as may be available now
	unsigned long offset = 0;
	while (offset < abuf_valid) {
		
		unsigned long  pes_start;
		unsigned long pes_len;
		
		find_next_pes_packet(abuf+offset, abuf_valid-offset,
                           pes_start, pes_len);
		

        if (oldAudioPts!=-1.0)
            {
            if (!resync)
                {
                if (fabs(pts_diff(oldAudioPts, pes_start)) > (3600.0 * 3))
                    {
                    dprintf("A DO_RESYNC ++");
                    do_resync=1;
                    }
                }
            }
		oldAudioPts=pes_start;

		if (!pes_len) {
			// no packet found...
			
			offset += pes_start;
			break;
		}			
		
		
		unsigned char * pes_packet = abuf + offset + pes_start; // what we just found

		if (audio_packet_wanted(pes_packet)) {

			// now find a second packet, we need it to compute the first packets
			// duration...

			unsigned long off2 = offset+pes_start+pes_len;
			unsigned char * ap2 = 0;

			while (off2 < abuf_valid) {
				unsigned long ap2_start;
				unsigned long ap2_len;
				
				find_next_pes_packet(abuf+off2, abuf_valid-off2, ap2_start, ap2_len);
				
				if (!ap2_len) {
					break;
				}
				
				unsigned char * p2 = abuf + off2 + ap2_start;
				if (audio_packet_wanted(p2)) {
					ap2 = p2;
					break;
				}
								
				off2 += ap2_start + ap2_len;
			}

			if (ap2) {
				// we found a second packet, so we can compute the duration
				double duration = pts_diff(pes_pts(ap2), pes_pts(pes_packet));
				create_audio_pes(pes_packet, duration);

			} else {
				// as there's no second audio packet, we can leave the loop
				// and wait for more audio data...
				offset += pes_start;
				break;
			}
		}

		// current pes was consumed/ignored...
		offset += pes_start + pes_len; // where to start searching for the next packet			
	}

	// move the remainder of the data in place for our next call...
	memmove(abuf, abuf+offset, abuf_valid-offset);
	abuf_valid -= offset;
	
	return 0;
}

// ############################################################################

int find_next_video_packet(unsigned char * data, unsigned long valid,
                                  unsigned long &vp_start, unsigned long &vp_len,
											 double * pts) {
	
	// vp_len will be == 0 if no packet is found or ready yet
	vp_len = 0;
	
	// vp_start will be either the start of the found packet or
	// a pointer to the first non-junk byte in the buffer.
	// In any case, the caller may discard the bytes from 
	// "data" to "data+vp_start+vp_len"
	// after considering the packet
		
	// ------------------------------------
	// find start of next MPEG-2 Video Sequence Header
	
	vp_start = 0;
	
	
search_again:	
	
//	if (valid < 4) return 0;
	if (valid < 64) return 0;

	for (; vp_start < valid-4; vp_start++) {

		if (   data[vp_start]   == 0x00
		    && data[vp_start+1] == 0x00
			) {
			
			if (   data[vp_start+2] == 0x01
			    && data[vp_start+3] == 0xb3
				) {
				// found a sequence start
				break;
			}
			
			if (data[vp_start+2] == 0x01)
                {
                BYTE val=data[vp_start+3];
			    if ( (val>=0xE0) && (val<=0xEF) )
		            {
				    // found video PES header, 
				    if (pts) {
					    *pts = pes_pts(data+vp_start);
				    }
                }
			}
		}
	}
	
	if (vp_start >= valid-4) {
		return 0;
	}
	
	// forget about data before first header
	
	unsigned char * a = data + vp_start;
	valid -= vp_start;
	
	
	// check for video header validity
	
//	if (valid < 12) 
	if (valid < 16) 
    {
		// ok, just not enough data, yet
		return 0;
	}
	
	if (   ((int)a[4]+(a[5] & 0xf0)) == 0
	    || ((a[5] & 0x0f)+a[6]) == 0
		 || (a[10] & 0x20) == 0
		 || (a[11] & 0x04) != 0
		) {
		
		// hmmm.. header looks invalid...
		vp_start += 1;
		goto search_again;		
	}
	
	
	// now we need to find the start of the next video packet to
	// determine to size of the just found one... too bad, but no other way...
	
	unsigned long vp_l = 12;
	
	for (; vp_l < valid-4; vp_l++) {
		if (   a[vp_l]   == 0x00
		    && a[vp_l+1] == 0x00
			 && a[vp_l+2] == 0x01
			 && a[vp_l+3] == 0xb3
			) {
			break;
		}
	}
	
	if (vp_l >= valid-4) {
		// ok, not enough data to determine video packet length
		return 0;
	}

	// ok, the whole video packet is available in vbuf
	
	vp_len = vp_l;	
	
	return 0;
}

// ##################################################################

unsigned long find_pic_end(unsigned char * data, unsigned long len, unsigned long offset) {
	
	for (unsigned long i = offset; i < len-4; i++) {
		if (   data[i] == 0x00
			 && data[i+1] == 0x00
			 && data[i+2] == 0x01
			 && data[i+3] == 0x00
			) {
			return i;
		}
	}
	
	return len;
}

// ##################################################################

double Remuxer::aux_to_framepts(unsigned long aux_start, unsigned long aux_len) {
	
	
	double base = aux_packets[aux_start];
	
	double adjust = 0.0;
	unsigned long divider = 0;
	
	for (unsigned long i = 1; i < aux_len; i++) {
		
		double i_pts = aux_packets[aux_start+i];
		
		double p_pts = base + ((double)i) * 90000.0 / frame_rate;
		
		if (fabs(pts_diff(i_pts, p_pts)) > 3*90000.0) {
			// hmmm.. we better ignore the deviant value...
		} else {
			
			adjust += pts_diff(i_pts, p_pts);
			divider += 1;
				
		}		
	}
	
	if (divider == 0) {
		return base;
	} else {
	
		adjust /= ((double)divider);
		return base+adjust;
	}
	
}

// ##################################################################

int Remuxer::create_video_pes(unsigned char * vp_packet, unsigned long vp_len) {
	
	// cut one MPEG-2 video sequence in pieces, wrap them in
	// PES packets, then store them in the video_packets list.
	
	// frame_pts has to be adjusted before calling this function!
	
	unsigned long max_video_per_pes = 0xffff - (9+5); // better not create bigger PES packets
		
		
	unsigned long pics = 0;	
	unsigned long peses = 0;	
	
	bool stamp = true;
	
	unsigned long offset = 0;
	unsigned long pic_end = find_pic_end(vp_packet, vp_len, 0);
//	if (pic_end != vp_len) {
		stamp = true;
		// real pic_end is at the _end_ of the 1. pic
		pic_end = find_pic_end(vp_packet, vp_len, pic_end+1);		
//	}
	
	unsigned long pic_len  = pic_end - offset;
	unsigned long pic_left = pic_end - offset;
	
	bool v_seq_start = true; // we need to mark the v_seq_start PES packets... 
	
	for (;;) {
		
		unsigned long pes_payload = pic_end - offset;
		if (pes_payload > max_video_per_pes) {
			pes_payload = max_video_per_pes;
		}		
		
		if (video_packets_avail >= max_video_packets) 
            {
			dprintf( "video_packets buffer overrun, dropping some data");			
			remove_video_packets(1);
		    }
		
		unsigned long total_len = 9 + ((stamp)? 5 : 0) + pes_payload;
		
		// create packet in a newly allocated memory buffer...
		unsigned char * vp = new unsigned char[total_len];
		video_packets[video_packets_avail] = vp;
		video_packets_avail += 1;
		
		vp[0] = 0x00;
		vp[1] = 0x00;
		vp[2] = 0x01;
		vp[3] = 0xE0;
		vp[4] = ((total_len - 6) >> 8) & 0xff;
		vp[5] = (total_len - 6) & 0xff;
		vp[6] = 0x80 | ((v_seq_start)? 0x01 : 0x00); // dirty: we mark the v_seq_start this way...
		v_seq_start = false;
		
		if (stamp) {
			//stamp = false;
			
			ULONGLONG frame_pts64 = (ULONGLONG)frame_pts;

			double part_done = 1.0 - (((double)pic_left)/((double)pic_len));
			frame_pts64 += (ULONGLONG)(part_done * 90000.0 / frame_rate);
				

            m_framePTS=(ULONGLONG)m_framePTS + (ULONGLONG)(part_done * 10000000.0 / frame_rate);

			vp[7] = 0x80; // PTS, only
			vp[8] = 0x05; // 5 bytes for the PTS
			
			vp[9]  = 0x20 | ((frame_pts64 & 0x1c0000000ui64) >> 29) | 0x01;
			vp[10] =        ((frame_pts64 & 0x03fc00000ui64) >> 22)       ;
			vp[11] =        ((frame_pts64 & 0x0003f8000ui64) >> 14) | 0x01;
			vp[12] =        ((frame_pts64 & 0x000007f80ui64) >>  7)       ;
			vp[13] =        ((frame_pts64 & 0x00000007fui64) <<  1) | 0x01;
			vp += 14;
			
		} else {
			vp[7] = 0x00; // no PTS
			vp[8] = 0x00; //
			vp += 9;
		}
		
		memcpy(vp, vp_packet + offset, pes_payload);
		
		offset   += pes_payload;
		pic_left -= pes_payload;
		
		if (pic_left == 0) {
			frame_pts += 90000.0/frame_rate;
			pics += 1;
		}
		
		peses += 1;
		
		if (offset >= vp_len) {
			break;
		}
		
		if (offset >= pic_end) {
			pic_end = find_pic_end(vp_packet, vp_len, offset + 1);
			pic_len  = pic_end - offset;
			pic_left = pic_end - offset;
			stamp = true;
		}
	}
	

	return 0;
}

// ##################################################################

unsigned long Remuxer::count_video_frames(unsigned char * data, unsigned long len) {
	
	unsigned long res = 0;
	
	if (len < 4) return 0;
	
	for (unsigned long i = 0; i < len-4; i++) {
		if (   data[i]   == 0x00
		    && data[i+1] == 0x00
		    && data[i+2] == 0x01
		    && data[i+3] == 0x00
			) {
			
			res += 1;
			i += 3;
		}
	}
	
	return res;
}

// ##################################################################

void Remuxer::adjust_frame_pts(unsigned char * vp_packet, unsigned long vp_len) {
	
	// the hard part here is to find out what aux data is associated
	// with the video sequence we want to wrap.
	// Seems there is only one chance to find out: If we have exactly
	// as much aux packets as we count video frames in the whole buffer,
	// we can be quite sure there's a direct correspondence.
	
	unsigned long total_vframes  = count_video_frames(vbuf, vbuf_valid);
	
	double before_part;
	double our_part;

#if (DEBUG_RESYNC_DROPS)  
//	if (frame_pts <= 0.0) 
        {
		dprintf("total_vframes = %ld, aux_packets_avail = %ld", total_vframes, aux_packets_avail);
	    }
#endif
	
	unsigned long our_vframes = 0;
	
	if (total_vframes) {
		
		our_vframes = count_video_frames(vp_packet, vp_len);
		unsigned long before_vframes = count_video_frames(vbuf, vp_packet-vbuf);

		before_part = ((double)before_vframes)/((double)total_vframes);
		our_part    = ((double)our_vframes)/((double)total_vframes);
	
	} else {
		// no vframes countable... so we have to make assumptions... :-(
		
		before_part = ((double)(vp_packet-vbuf)) / ((double)vbuf_valid);
		our_part  = ((double)vp_len) / ((double)vbuf_valid);
	}
	

//	unsigned long start_aux = (unsigned long)rint(((double)aux_packets_avail) * before_part);
//	unsigned long our_aux   = (unsigned long)rint(((double)aux_packets_avail) * our_part);

	unsigned long start_aux = (unsigned long)(((double)aux_packets_avail) * before_part + 0.5);
	unsigned long our_aux   = (unsigned long)(((double)aux_packets_avail) * our_part    + 0.5);

	if (start_aux + our_aux > aux_packets_avail) {
		our_aux -= 1;
	}
	
	
	if (frame_pts > 0.0) {
		
		// If our frame_pts had been synchronized before, we only
		// try to make sure it didn't drift off too far. If it did,
		// we'll have to find a new frame_pts from the aux data... 
		
		
		unsigned long good_cnt = 0;
		unsigned long bad_cnt = 0;
		
		//double biggest_diff = 0.0;
		double avg_diff = 0.0;
		
		unsigned long ptses_to_check = our_aux;
		if (one_pts_per_gop && ptses_to_check > 1) ptses_to_check = 1;
		
		for (unsigned long i = 0; i < ptses_to_check; i++) {
			
			double i_pts = aux_packets[start_aux+i];
			double p_pts = frame_pts + ((double)i) * 90000.0 / frame_rate;
			
			double d = pts_diff(i_pts, p_pts);
			
			avg_diff += d;
			//if (fabs(d) > biggest_diff) biggest_diff = fabs(d);
			
			if (d > allowed_frame_pts_skew) {
				bad_cnt += 1;
			} else {
				good_cnt += 1;
			}
		}
		
//        dprintf("good:%ld, bad:%ld",good_cnt,bad_cnt);

		if (our_aux) 
            avg_diff /= (double)our_aux;
		
		if (good_cnt >= bad_cnt) 
            {
			// ok, we can stay with our frame_pts
			if (one_pts_per_gop) 
                {
				our_aux = 1;
			    }
			
			remove_aux_packets(start_aux + our_aux);
			return;
		    }
		
		dprintf("frame_pts drifted off from aux data (%ld/%ld, %s), will try to readjust", bad_cnt, good_cnt, pts_to_hms(avg_diff));
		
	}
	
	
	// there's no frame_pts yet. Try to find one...
	if (
	    (   one_pts_per_gop
		  && our_vframes > 3 // need a plausible GOP size
		  && total_vframes == our_vframes + 1
		  && aux_packets_avail == 2
		 )
		 ||
		 (   !one_pts_per_gop
		  && total_vframes == aux_packets_avail
		  && our_aux > 0
		 )
		) {
		
		if (one_pts_per_gop) {
			our_aux = 1;
		}
		
		frame_pts = aux_to_framepts(start_aux, our_aux);
		
		dprintf("readjusted frame_pts ");
		
		remove_aux_packets(start_aux + our_aux);
		return;
	}
	
	// we better try to find a new frame_pts next time.. so we stay with the old one (if any)...

	remove_aux_packets(start_aux + our_aux);
	return;
}


// ##################################################################

int Remuxer::supply_video_data(void * data, unsigned long len) {
	
	if (vbuf_valid + len > vbuf_size) 
        {
		dprintf( "vbuf overrun, dropping some data");
		vbuf_valid = 0;
		if (len > vbuf_size) 
            {
			len = vbuf_size;
		    }
	    }
	
	memcpy(vbuf+vbuf_valid, data, len);
	vbuf_valid += len;
	
	// work on as many MPEG-II video packets as may be available now
	unsigned long offset = 0;
	while (offset < vbuf_valid) {
		
		unsigned long  vp_start;
		unsigned long  vp_len;
		
		find_next_video_packet(vbuf+offset, vbuf_valid-offset,
                             vp_start, vp_len, &frame_pts);
        if (oldFramePts!=-1.0)
            {
            if (!resync)
                {
                if (fabs(pts_diff(oldFramePts, frame_pts)) > (3600.0 * 25))
                    {
                    dprintf("V DO_RESYNC ++");
                    do_resync=1;
                    }
                }
            }
		oldFramePts=frame_pts;

		if (vp_len) {
			
            if (vp_start>0)
                {
                dprintf("VP_START:%ld",vp_start);
                }
			unsigned char * vp_packet = vbuf + offset + vp_start; // what we just found
			
			adjust_frame_pts(vp_packet, vp_len);
			if (frame_pts >= 0.0) 
                {
				create_video_pes(vp_packet, vp_len);
			    }
						
			offset += vp_start + vp_len; // where to start searching for the next packet
			
		} else {
			// no packet there...
			
			offset += vp_start;
			break;
		}
	}	

	// move the remainder of the data in place for our next call...
	memmove(vbuf, vbuf+offset, vbuf_valid-offset);
	vbuf_valid -= offset;
	
	return 0;
}

// ##################################################################

int Remuxer::supply_aux_data(void * data, unsigned long len) {
	
	unsigned char * d = (unsigned char *)data;
	
	// we have to assume that every 16 bytes there is one of the
	// PTS stamps we're interested in...
	
	for (unsigned long idx = 0; idx+15 < len; idx += 16) {
		
		if (aux_packets_avail >= max_aux_packets) 
            {
			dprintf( "aux_packets buffer overrun, dropping some data");
			remove_aux_packets(1);
		    }
		
		unsigned long p1 =   (d[idx+2] << 24)
		                   | (d[idx+3] << 16)
		                   | (d[idx+4] << 8)
		                   | (d[idx+5])
						      ;
		
		/*
		unsigned long p2 =   (d[idx+12] << 24)
					          | (d[idx+13] << 16)
					          | (d[idx+14] << 8)
					          | (d[idx+15])
						      ;
		*/
		aux_packets[aux_packets_avail] = (double)p1;
		aux_packets_avail += 1;
	}
	
	return 0;
}

// ##################################################################

int Remuxer::write_pes_in_pp(unsigned char * orig_pes, FILE * mpgfile,
                             double & scr, bool stamp, bool stamp_all, bool output_sys_header,
									  double pes_duration, double scr_duration) {
	
	
	unsigned long   orig_pes_size = pes_size(orig_pes);
	unsigned long   orig_opt_header_size = orig_pes[8];
	unsigned long   orig_es_size = orig_pes_size - 9 - orig_opt_header_size;
	unsigned char * orig_es_data = orig_pes + 9 + orig_opt_header_size;
	double          orig_pes_time = pes_pts(orig_pes);
	
	unsigned long orig_es_avail = orig_es_size;
	
	unsigned char * pp = pp_buffer + pp_valid;
	unsigned long pp_avail = max_out_pp - pp_valid;	
	
	while (orig_es_avail) {
		
		if (pp_valid == 0) {
			// setup pp header if we're at the start of a new output pp

			pp[0]  = 0x00;
			pp[1]  = 0x00;
			pp[2]  = 0x01;
			pp[3]  = 0xba;
			
			ULONGLONG scro = (ULONGLONG)pts_diff(scr, system_clock_ref_start);
			scro += (ULONGLONG)playtime_offset;
			
			if (scr_duration > 0.0) {
				double part_done = 1.0 - (((double)orig_es_avail)/((double)orig_es_size));
				scro += (ULONGLONG)(part_done * scr_duration);
			}
			
			pp[4]  = 0x40 | ((scro & 0x1c0000000ui64) >> 27) | 0x04 | ((scro & 0x030000000ui64) >> 28);
			pp[5]  =        ((scro & 0x00ff00000ui64) >> 20);
			pp[6]  =        ((scro & 0x0000f8000ui64) >> 12) | 0x04 | ((scro & 0x000006000ui64) >> 13);
			pp[7]  =        ((scro & 0x000001fe0ui64) >> 5);
			pp[8]  =        ((scro & 0x00000001fui64) << 3) | 0x04;
			pp[9]  = 0x01;
			
			unsigned long mr = mplex_rate / 400;

			pp[10] = (mr  & 0x3fc000) >> 14;
			pp[11] = (mr  & 0x003fc0) >> 6;
			pp[12] = ((mr & 0x00003f) << 2) | 0x03;
			pp[13] = 0x00;

			pp += 14;
			pp_valid += 14;
			pp_avail -= 14;
			
			// system header
			
			if (output_sys_header) {
				output_sys_header = false;
				pp[0]  = 0x00;
				pp[1]  = 0x00;
				pp[2]  = 0x01;
				pp[3]  = 0xbb;			
				pp[4]  = ((6+6+3+3) >> 8) & 0xff; // header length - 6
				pp[5]  =  (6+6+3+3) & 0xff;

				pp[6]  = 0x80 | (mr & 0x3f8000) >> 15;
				pp[7]  =        (mr & 0x007f80) >> 7;
				pp[8]  =        ((mr & 0x00007f) << 1) | 0x01;
				pp[9]  = (1 << 2);
				pp[10] = 0xe0 | 1;
				pp[11] = 0x7f; // bit 7 here is "rate restriction" flag... well... whatever that means..

				unsigned long max_video_buffer = 512*1024 / 1024;
				unsigned long max_audio_buffer =   8*1024 /  128;
				
				pp[12] = 0xe0;
				pp[13] = 0xc0 | 0x20 | ((max_video_buffer & 0x1f00) >> 8);
				pp[14] =               ((max_video_buffer & 0x00ff));
				
				pp[13] = wanted_audio_stream;
				pp[14] = 0xc0 | 0x00 | ((max_audio_buffer & 0x1f00) >> 8);
				pp[15] =              ((max_audio_buffer & 0x00ff));
				
				pp += 12 + 6;
				pp_valid += 12 + 6;
				pp_avail -= 12 + 6;
				
				if (flush_pp(mpgfile)) {
					return -1;
				}

				pp = pp_buffer;
				pp_valid = 0;
				pp_avail = max_out_pp;
				
				continue;
			}
		}
		
		ULONGLONG cur_pes_pts = ~0ui64;
		
		unsigned long cur_pes_header_size = 9;
		
		if (orig_es_avail == orig_es_size) {
			// maybe we should put a PTS in the PES header...
			
			if (orig_pes_time >= 0.0) {
				if (stamp) {
					cur_pes_header_size += 5;
					cur_pes_pts = (ULONGLONG)pts_diff(orig_pes_time, system_clock_ref_start);
					cur_pes_pts += (ULONGLONG)playtime_offset;
					cur_pes_pts += scr_play_offset ; // show it in 1/3 seconds..
				}
			}
		} else {
			
			if (stamp_all) {
				if (pes_duration >= 0.0) {
					if (orig_pes_time >= 0.0) {
						if (stamp) {
							cur_pes_header_size += 5;

							double part_done = 1.0 - (((double)orig_es_avail)/((double)orig_es_size));

							cur_pes_pts = (ULONGLONG)pts_diff(orig_pes_time + part_done * pes_duration,
							                                           system_clock_ref_start);
							cur_pes_pts += (ULONGLONG)playtime_offset;
							cur_pes_pts += scr_play_offset ; // show it in 1/3 seconds..
						}	
					}
				}
			}
		}
		
		unsigned long ac3_header_size = 0;
		if (orig_pes[3] == STREAM_PRIVATE_1) {
			// add that bizarre special 4 byte header for AC3...
			ac3_header_size = 4;
		}
		
		unsigned long cur_pes_payload = orig_es_avail;
		if (cur_pes_payload > pp_avail-(cur_pes_header_size + ac3_header_size)) {
			cur_pes_payload = pp_avail-(cur_pes_header_size + ac3_header_size);
		}
		
		unsigned long cur_pes_stuff_bytes = 0;
		if (pp_avail - (cur_pes_header_size+ac3_header_size+cur_pes_payload) < 20) {
			cur_pes_stuff_bytes = pp_avail - (cur_pes_header_size+ac3_header_size+cur_pes_payload);
		}
		
		unsigned long cur_pes_totlen = 
		    cur_pes_header_size + cur_pes_stuff_bytes + ac3_header_size + cur_pes_payload - 6;
		
		// create PES header
		
		pp[0] = 0x00;
		pp[1] = 0x00;
		pp[2] = 0x01;
		pp[3] = orig_pes[3]; // copy the stream id
		pp[4] = (cur_pes_totlen >> 8) & 0xff;
		pp[5] = (cur_pes_totlen) & 0xff;
		pp[6] = 0x80; // no wired stuff
		pp[7] = (cur_pes_pts != ~0ui64)? 0x80 : 0x00; // flags for header extensions
		pp[8] = cur_pes_header_size - 9 + cur_pes_stuff_bytes;
		
		if (cur_pes_pts != ~0ui64) {
			pp[9]  = 0x20 | ((cur_pes_pts & 0x1c0000000ui64) >> 29) | 0x01;
			pp[10] =        ((cur_pes_pts & 0x03fc00000ui64) >> 22)       ;
			pp[11] =        ((cur_pes_pts & 0x0003f8000ui64) >> 14) | 0x01;
			pp[12] =        ((cur_pes_pts & 0x000007f80ui64) >>  7)       ;
			pp[13] =        ((cur_pes_pts & 0x00000007fui64) <<  1) | 0x01;
		}
		
		pp += cur_pes_header_size;
		pp_valid += cur_pes_header_size;
		pp_avail -= cur_pes_header_size;
		
		
		if (cur_pes_stuff_bytes) {
			memset(pp, 0xff, cur_pes_stuff_bytes);
			pp       += cur_pes_stuff_bytes;
			pp_valid += cur_pes_stuff_bytes;
			pp_avail -= cur_pes_stuff_bytes;
		}
		
		if (ac3_header_size) {
			pp[0] = 0x80;
			pp[1] = 0x02;
			pp[2] = 0x00;
			pp[3] = 0x01;
			pp       += ac3_header_size;
			pp_valid += ac3_header_size;
			pp_avail -= ac3_header_size;
		}
		
		memcpy(pp, orig_es_data, cur_pes_payload);
		orig_es_data += cur_pes_payload;
		orig_es_avail -= cur_pes_payload;
		pp       += cur_pes_payload;
		pp_valid += cur_pes_payload;
		pp_avail -= cur_pes_payload;
		
		
		if (pp_avail == 0) {
			// output our new PES packets
			
            if (CMultiplexBuffer!=NULL)
                CMultiplexBuffer->Write(pp_buffer, pp_valid);

			total_bytes_written += pp_valid;
			
			pp = pp_buffer;
			pp_valid = 0;
			pp_avail = max_out_pp;
		}
		
		
		// next round - if some original data is left...
	}
	
	scr += scr_duration;
	
	return 0;
}

// ##################################################################

int Remuxer::flush_pp(FILE * mpgfile) {
	
	if (pp_valid == 0) {
		// no need to flush - everything is already out
		return 0;
	}
	
	unsigned char * pp = pp_buffer + pp_valid;
	unsigned long pp_avail = max_out_pp - pp_valid;
	
	// setup a dummy-packet for the rest of the pp_buffer
	
	unsigned long cur_pes_header_size = 6;
	unsigned long cur_pes_totlen = pp_avail - cur_pes_header_size;
	
	
	pp[0] = 0x00;
	pp[1] = 0x00;
	pp[2] = 0x01;
	pp[3] = STREAM_PADDING;
	pp[4] = (cur_pes_totlen >> 8) & 0xff;
	pp[5] = (cur_pes_totlen) & 0xff;
	
	memset(pp+cur_pes_header_size, 0xff, cur_pes_totlen);
	
	// output our new PES packets

    if (CMultiplexBuffer!=NULL)
        CMultiplexBuffer->Write(pp_buffer, max_out_pp);

	total_bytes_written += max_out_pp;
	
	pp_valid = 0;
	
	return 0;
}

// ##################################################################

int Remuxer::write_mpg(FILE * mpgfile) {
	
	// try to write out some multiplexed MPEG-II data
	

	if (resync) {
		
		// try to find a new point where the PTS of a video sequence start
		// and the PTS of the next audio frame are "near"...

        dprintf("resync - a=%ld, v=%ld", audio_packets_avail, video_packets_avail);

        if (video_packets_avail)
            gotVideo=TRUE;
    
        if (audio_packets_avail)
            gotAudio=TRUE;
		
		while (audio_packets_avail && video_packets_avail) {
			
			unsigned char * ap = audio_packets[0];
			
			double audio_pts = pes_pts(ap);
			if (audio_pts < 0) {
				// cannot use this...
				remove_audio_packets(1);
				continue;
			}
			
			// if AC3, make sure to start with a magic sync word...
			if (ap[3] == STREAM_PRIVATE_1) {
				unsigned long   orig_pes_size = pes_size(ap);
				unsigned long   orig_opt_header_size = ap[8];
				unsigned long   orig_es_size = orig_pes_size - 9 - orig_opt_header_size;
				unsigned char * orig_es_data = ap + 9 + orig_opt_header_size;
				if (orig_es_size < 4 || orig_es_data[0] != 0x0b || orig_es_data[1] != 0x77) {
					// cannot use this...
					remove_audio_packets(1);
					continue;
				}
			}
			
			
			unsigned char * vp = video_packets[0];
						
			double video_pts = pes_pts(vp);
			if (video_pts < 0) {
				// cannot use this...
				remove_video_packets(1);
				continue;
			}
						
			// we need to make sure our first video data comes
			// before our first audio data...
			
			if (audio_pts <= video_pts) { 
				remove_audio_packets(1);
				continue;
			}

			if (0 == (vp[6] & 0x01)) {
				// this is not a video sequence start... cannot use this...
				remove_video_packets(1);
				continue;				
			}
			
			// check for the PTS difference... 

			double av_pts_diff = pts_diff(audio_pts, video_pts);
			
			//if (fabs(av_pts_diff) < (90000.0/22)) { // /2 ?			
			if (fabs(av_pts_diff) < (3600.0*1.25)) { 		
				// ha - we found a place...
				
				playtime_offset += pts_diff(system_clock_ref, system_clock_ref_start);
				
				double smaller_pts = audio_pts;
				if (video_pts < smaller_pts) 
                    smaller_pts = video_pts;
				
				dprintf( "resynced at playtime %s", pts_to_hms(playtime_offset));
				OutputDebugString("ReSync !!!\r\n");
				system_clock_ref_start = smaller_pts;
				system_clock_ref       = smaller_pts;
				
				resyncs += 1;
				resync = false;
				break;
			}
			
			// alas, we cannot decide whether audio or video is incorrect -
			// so we have to throw away both packets...
			remove_audio_packets(1);

			remove_video_packets(1);

			// ... now try again with the next packets...
		}
		
		if (resync) {
			// well, we couldn't resync yet. Try again next time
			// with more new data...
			return 0;
		}
	}
	
	
	for (;;) {
		
		// look whether there's something to write out...
				
		// we need at least three packets with time-stamps
		// from both streamsto be able to
		// compute the duration of the next two packets...

		if (audio_packets_avail < 3) {
			// maybe next time...
			return 0;
		}
		
		if (video_packets_avail < 3) {
			// maybe next time...
			return 0;
		}

        static double last_audio_pts = -1;
        static double last_video_pts = -1;
		
		double      vp_pts[3];
		vp_pts[0] = pes_pts(video_packets[0]);
		vp_pts[1] = pes_pts(video_packets[1]);
		vp_pts[2] = pes_pts(video_packets[2]);
		
		double      vp_dur[2];		
		vp_dur[0] = pts_diff(vp_pts[1], vp_pts[0]);
		vp_dur[1] = pts_diff(vp_pts[2], vp_pts[1]);
		
		double      vp_mid[2];
		vp_mid[0] = vp_pts[0] + 0.5 * vp_dur[0];
		vp_mid[1] = vp_pts[1] + 0.5 * vp_dur[1];

		
		double      ap_pts[3];
		ap_pts[0] = pes_pts(audio_packets[0])+video_forerun;
		ap_pts[1] = pes_pts(audio_packets[1])+video_forerun;
		ap_pts[2] = pes_pts(audio_packets[2])+video_forerun;
		
		double      ap_dur[2];		
		ap_dur[0] = pts_diff(ap_pts[1], ap_pts[0]);
		ap_dur[1] = pts_diff(ap_pts[2], ap_pts[1]);
		
		double      ap_mid[2];
		ap_mid[0] = ap_pts[0] + 0.5 * ap_dur[0];
		ap_mid[1] = ap_pts[1] + 0.5 * ap_dur[1];
		

		// check for an emergency resync
		if (   fabs(pts_diff(vp_pts[0], ap_pts[0]))        > 0.50*90000.0
		    || fabs(pts_diff(vp_pts[0], system_clock_ref)) > 0.50*90000.0
		    || fabs(vp_dur[0])                             > 0.50*90000.0
		    || fabs(ap_dur[0])                             > 0.50*90000.0
		   ) 
        {
			// oops, our data seems to be severely broken. need a resync...
			dprintf("resyncing due to big time differences in packet buffers");
			perform_resync();
			return 0;
		}

        if (do_resync)
            {
			dprintf("DO_RESYNC");
			perform_resync();
			return 0;
            }
		
		bool f1_is_video = true;
		bool f2_is_video = true;
		
		double f2_pts = -1;
		double f2_mid = -1;
		double f2_dur = -1;
		
		
		if (pts_diff(ap_pts[0], vp_pts[0]) <= 0.0) {
			// place audio packet, first
			
			f1_is_video = false;
			
			if (pts_diff(ap_pts[1], vp_pts[0]) < 0.0) {
			
				f2_is_video = false;

				f2_pts = ap_pts[1];
				f2_mid = ap_mid[1];
				f2_dur = ap_dur[1];
								
			} else {

				f2_pts = vp_pts[0];
				f2_mid = vp_mid[0];
				f2_dur = vp_dur[0];
				
			}
			
		} else {
			// place video packet, first

			if (pts_diff(vp_pts[1], ap_pts[0]) < 0.0) {
			
				f2_pts = vp_pts[1];
				f2_mid = vp_mid[1];
				f2_dur = vp_dur[1];
			
			} else {
				
				f2_is_video = false;

				f2_pts = ap_pts[0];
				f2_mid = ap_mid[0];
				f2_dur = ap_dur[0];
			}
		}
		

		// what type of frame to write next?
		if (f1_is_video) {
			// write video frame
			
			bool seqstart = (((video_packets[0])[6] & 0x01) == 0x01)? true : false;
			
			if (seqstart) {
				if (flush_pp(mpgfile)) {
					dprintf( "error while flushing to pp");
					return -1;
				}
			}

		
            last_video_pts = vp_pts[0];
			

			double pes_duration = vp_dur[0];
			
			double scr_duration = pts_diff(f2_pts, vp_pts[0]);						

			if (write_pes_in_pp(video_packets[0], mpgfile, system_clock_ref, seqstart, false,
				                 seqstart, pes_duration, scr_duration )) 
                                 {
                				dprintf( "error while writing video pes to pp");
				                return -1;
                    			}

			remove_video_packets(1);

		} else {

			// write audio frame

            
            last_audio_pts = ap_pts[0];

			bool seqstart = (((audio_packets[0])[6] & 0x01) == 0x01)? true : false;
			
			if (seqstart) {
				if (flush_pp(mpgfile)) 
                {
					dprintf( "error while flushing to pp");
					return -1;
				}
			}
			
			double pes_duration = ap_dur[0];
			
			system_clock_ref = ap_pts[0];
			
			double scr_duration = pts_diff(f2_pts, ap_pts[0]);						
			
			if (write_pes_in_pp(audio_packets[0], mpgfile, system_clock_ref, true, true,
				                 false, pes_duration, scr_duration )) 
                {
				dprintf( "error while writing video pes to pp");
				return -1;
			    }
			
			remove_audio_packets(1);
		}


	// try to output some more...
	}
		
	
}

// ##################################################################

int Remuxer::write_mpp(FILE * mppfile) {
	
	for (unsigned long i = 0; i < audio_packets_avail; i++) {
		
		unsigned char * orig_pes = audio_packets[i];
		
		unsigned long   orig_pes_size = pes_size(orig_pes);
		unsigned long   orig_opt_header_size = orig_pes[8];
		unsigned long   orig_es_size = orig_pes_size - 9 - orig_opt_header_size;
		unsigned char * orig_es_data = orig_pes + 9 + orig_opt_header_size;
		

		if (!mpp_started) 
            {
            for(;;)
                {
                if (orig_es_size < 4)
                    break;
			    // make sure to start with a magic sync word...
			    if (orig_pes[3] == STREAM_PRIVATE_1) 
                    {
				    if ( (orig_es_data[0] == 0x0b) && (orig_es_data[1] == 0x77) ) 
                        {
            			mpp_started = true;
					    break;
				        }
			        } 
                 else 
                    {
				    // MPEG audio
				    //if (orig_es_data[0] == 0xff && ((orig_es_data[1] & 0xe0) != 0xe0)) 
				    if ( (orig_es_data[0] == 0xFF) && ((orig_es_data[1] & 0xF0) == 0xF0) ) 
                        {
                        int _layer=(orig_es_data[1]>>1)&0x03;
                        int _tmp=((orig_es_data[2])>>4)&0x0F;
                        int _bitrate=(_AudioRateTable[_layer][_tmp])*1000;
                        _tmp=((orig_es_data[2])>>2)&0x03;
                        int _frequency=_AudioFreqTable[_tmp];
                        audio_frequency=_frequency;
            			mpp_started = true;
					    break;
				        }
                    orig_es_data++;
                    orig_es_size--;
			        }
			    }
    		}
				
		if (mpp_started)
            {
            if (CMultiplexBuffer!=NULL)
                CMultiplexBuffer->Write(orig_es_data, orig_es_size);
            } 	    
    }

    remove_audio_packets(audio_packets_avail);

	return 0;
}

// ##################################################################

int Remuxer::write_mpv(FILE * mpvfile) {
	
	for (unsigned long i = 0; i < video_packets_avail; i++) {
		
		unsigned char * orig_pes = video_packets[i];
		
		unsigned long   orig_pes_size = pes_size(orig_pes);
		unsigned long   orig_opt_header_size = orig_pes[8];
		unsigned long   orig_es_size = orig_pes_size - 9 - orig_opt_header_size;
		unsigned char * orig_es_data = orig_pes + 9 + orig_opt_header_size;
				
        if (CMultiplexBuffer!=NULL)
            CMultiplexBuffer->Write(orig_es_data, orig_es_size);
	}

    remove_video_packets(video_packets_avail);
	
	return 0;
}

// ##################################################################


#pragma warning (default : 4244)
