/*
 * $Id: Remuxer.cpp,v 1.2 2002/08/31 01:06:19 obi Exp $
 *
 * MPEG2 remuxer
 *
 * Copyright (C) 2001 Peter Niemayer et al.
 * See AUTHORS for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Log: Remuxer.cpp,v $
 * Revision 1.2  2002/08/31 01:06:19  obi
 * sync with grab v1.40
 *
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 
#include "Remuxer.h"

// #################################################################################

#define DEBUG_RESYNC_DROPS 0
#define DEBUG_AV_DESYNC 0

// #################################################################################
//
// everything below takes some C++ knowledge to understand :-)
//
// ##################################################################################

const unsigned long known_audio_frame_sizes[] = {
//	MPG/192	AC3/5+1	MPG/64
	576,	896*2,	167
};


// ac3 tables by Aaron Holtzman - May 1999

struct ac3_frmsize_s
{
	unsigned short bit_rate;
	unsigned short frm_size[3];
};

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

static int find_next_pes_packet(unsigned char * data, unsigned long valid,
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
	static unsigned long last_audio_frame_size = ~0;
	unsigned long audio_frame_size = 0;
	
	for (unsigned int i = 0; i < sizeof(known_audio_frame_sizes)/sizeof(unsigned long); i++) {
		if (orig_es_size % known_audio_frame_sizes[i] == 0) {
			audio_frame_size = known_audio_frame_sizes[i];
			wanted_audio_per_pes = (max_audio_per_pes / audio_frame_size) * audio_frame_size;
		}
	}
	
	if (audio_frame_size != last_audio_frame_size) {
		last_audio_frame_size = audio_frame_size;
		fprintf(stderr,"detected audio frame size: %ld. ES data size: %ld                  \n",
		        audio_frame_size, orig_es_size);
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
			fprintf(stderr, "audio_packets buffer overrun, dropping some data\n");			
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

			unsigned long long pts64 = (unsigned long long)orig_pts;
			double part_done = 1.0 - (((double)es_left)/((double)orig_es_size));
			pts64 += (unsigned long long)(part_done * duration);
			
			ap[7] = 0x80; // PTS, only
			ap[8] = 0x05; // 5 bytes for the PTS
			
			ap[9]  = 0x20 | ((pts64 & 0x1c0000000ULL) >> 29) | 0x01;
			ap[10] =        ((pts64 & 0x03fc00000ULL) >> 22)       ;
			ap[11] =        ((pts64 & 0x0003f8000ULL) >> 14) | 0x01;
			ap[12] =        ((pts64 & 0x000007f80ULL) >>  7)       ;
			ap[13] =        ((pts64 & 0x00000007fULL) <<  1) | 0x01;
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
	
	//fprintf(stderr,"created %ld PESes from 1 original audio PES\n",
	//        peses);
	
	return 0;
}

// ##################################################################


bool Remuxer::audio_packet_wanted(unsigned char * pes) {
	
	if (wanted_audio_stream == 0) {
		// make a guess: just take the first type of audio stream we find...
		
		if (   pes[3] == STREAM_PRIVATE_1
		    || pes[3] == STREAM_AUDIO
		    || pes[3] == STREAM_REST_AUDIO
		    || pes[3] == 0xc1
		   )
		{
			wanted_audio_stream = pes[3];
			return true;
		}
	}
	
	if (pes[3] == wanted_audio_stream) {
		return true;
	}

	return false;
}

// ##################################################################

int Remuxer::supply_audio_data(void * data, unsigned long len) {
	
		
	if (abuf_valid + len > abuf_size) {
		fprintf(stderr, "abuf overrun, dropping some data\n");
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
		
		if (!pes_len) {
			// no packet found...
			
			offset += pes_start;
			break;
		}			
		
		
		unsigned char * pes_packet = abuf + offset + pes_start; // what we just found

		if (pes_start + pes_len > abuf_valid-offset) { *((long *)0) = 0; }


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

static int find_next_video_packet(unsigned char * data, unsigned long valid, unsigned long & vp_start, unsigned long & vp_len, double * pts)
{
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
	
	if (valid < 4) return 0;
	
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
			
			if (   data[vp_start+2] == 0x01
			    && data[vp_start+3] == 0xe0
		      ) {
				
				// found video PES header, 
				if (pts) {
					*pts = pes_pts(data+vp_start);
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
	
	if (valid < 12) {
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

static unsigned long find_pic_end(unsigned char * data, unsigned long len, unsigned long offset) {
	
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
		// fprintf(stderr,"frame_pts adjustment: %s\n", pts_to_hms(adjust));
		
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
	if (pic_end != vp_len) {
		stamp = true;
		// real pic_end is at the _end_ of the 1. pic
		pic_end = find_pic_end(vp_packet, vp_len, pic_end+1);		
	}
	
	unsigned long pic_len  = pic_end - offset;
	unsigned long pic_left = pic_end - offset;
	
	bool v_seq_start = true; // we need to mark the v_seq_start PES packets... 
	
	for (;;) {
		
		unsigned long pes_payload = pic_end - offset;
		if (pes_payload > max_video_per_pes) {
			pes_payload = max_video_per_pes;
		}		
		
		if (video_packets_avail >= max_video_packets) {
			fprintf(stderr, "video_packets buffer overrun, dropping some data\n");			
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
			
			unsigned long long frame_pts64 = (unsigned long long)frame_pts;

			double part_done = 1.0 - (((double)pic_left)/((double)pic_len));
			frame_pts64 += (unsigned long long)(part_done * 90000.0 / frame_rate);
				
			vp[7] = 0x80; // PTS, only
			vp[8] = 0x05; // 5 bytes for the PTS
			
			vp[9]  = 0x20 | ((frame_pts64 & 0x1c0000000ULL) >> 29) | 0x01;
			vp[10] =        ((frame_pts64 & 0x03fc00000ULL) >> 22)       ;
			vp[11] =        ((frame_pts64 & 0x0003f8000ULL) >> 14) | 0x01;
			vp[12] =        ((frame_pts64 & 0x000007f80ULL) >>  7)       ;
			vp[13] =        ((frame_pts64 & 0x00000007fULL) <<  1) | 0x01;
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
	
	
	//fprintf(stderr,"created %ld PESes from %ld pics in  %ld bytes\n",
	//        peses, pics, vp_len);
	
	
	
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
	if (frame_pts <= 0.0) {
		fprintf(stderr,"total_vframes = %ld, aux_packets_avail = %ld          \n", total_vframes, aux_packets_avail);
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
	

	unsigned long start_aux = (unsigned long)rint(((double)aux_packets_avail) * before_part);
	unsigned long our_aux   = (unsigned long)rint(((double)aux_packets_avail) * our_part);
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
		
		if (our_aux) avg_diff /= (double)our_aux;
		//fprintf(stderr,"biggest frame_pts diff: %s, avg_diff: %f\n",
		//		  pts_to_hms(biggest_diff), avg_diff
		//		 );
		
		if (good_cnt >= bad_cnt) {
			// ok, we can stay with our frame_pts

			if (one_pts_per_gop) {
				our_aux = 1;
			}
			
			remove_aux_packets(start_aux + our_aux);
			return;
		}
		
		fprintf(stderr,"frame_pts drifted off from aux data (%ld/%ld, %s), will try to readjust\n",
			     bad_cnt, good_cnt, pts_to_hms(avg_diff)
			    );
		
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
		
		/*
		fprintf(stderr,"readjusted frame_pts to %s                                           \n",
				  pts_to_hms(frame_pts)
				 );
		*/
		fprintf(stderr,"readjusted frame_pts                                       \n");
		
		remove_aux_packets(start_aux + our_aux);
		return;
	}
	
	// we better try to find a new frame_pts next time.. so we stay with the old one (if any)...

//fprintf(stderr,"no frame_pts, total_vframes=%ld, aux_packets_avail=%ld\n",
//        total_vframes, aux_packets_avail);

	remove_aux_packets(start_aux + our_aux);
	return;
}

// ##################################################################

int Remuxer::supply_video_data(void * data, unsigned long len) {
	
	if (vbuf_valid + len > vbuf_size) {
		fprintf(stderr, "vbuf overrun, dropping some data\n");
		vbuf_valid = 0;
		if (len > vbuf_size) {
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
		
		if (vp_len) {
			
			if (vp_start + vp_len > vbuf_valid-offset) { *((long *)0) = 0; }

			unsigned char * vp_packet = vbuf + offset + vp_start; // what we just found
			
			adjust_frame_pts(vp_packet, vp_len);
			if (frame_pts >= 0.0) {
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
		
		if (aux_packets_avail >= max_aux_packets) {
			fprintf(stderr, "aux_packets buffer overrun, dropping some data\n");
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
			
			unsigned long long scro = (unsigned long long)pts_diff(scr, system_clock_ref_start);
			scro += (unsigned long long)playtime_offset;
			
			if (scr_duration > 0.0) {
				double part_done = 1.0 - (((double)orig_es_avail)/((double)orig_es_size));
				scro += (unsigned long long)(part_done * scr_duration);
			}
			
			pp[4]  = 0x40 | ((scro & 0x1c0000000ULL) >> 27) | 0x04 | ((scro & 0x030000000ULL) >> 28);
			pp[5]  =        ((scro & 0x00ff00000ULL) >> 20);
			pp[6]  =        ((scro & 0x0000f8000ULL) >> 12) | 0x04 | ((scro & 0x000006000ULL) >> 13);
			pp[7]  =        ((scro & 0x000001fe0ULL) >> 5);
			pp[8]  =        ((scro & 0x00000001fULL) << 3) | 0x04;
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
		
		unsigned long long cur_pes_pts = ~0ULL;
		
		unsigned long cur_pes_header_size = 9;
		
		if (orig_es_avail == orig_es_size) {
			// maybe we should put a PTS in the PES header...
			
			if (orig_pes_time >= 0.0) {
				if (stamp) {
					cur_pes_header_size += 5;
					cur_pes_pts = (unsigned long long)pts_diff(orig_pes_time, system_clock_ref_start);
					cur_pes_pts += (unsigned long long)playtime_offset;
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

							cur_pes_pts = (unsigned long long)pts_diff(orig_pes_time + part_done * pes_duration,
							                                           system_clock_ref_start);
							cur_pes_pts += (unsigned long long)playtime_offset;
							cur_pes_pts += scr_play_offset ; // show it in 1/3 seconds..
//fprintf(stderr,"current_pes_pts = %lld\n", cur_pes_pts);					
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
		pp[7] = (cur_pes_pts != ~0ULL)? 0x80 : 0x00; // flags for header extensions
		pp[8] = cur_pes_header_size - 9 + cur_pes_stuff_bytes;
		
		if (cur_pes_pts != ~0ULL) {
			pp[9]  = 0x20 | ((cur_pes_pts & 0x1c0000000ULL) >> 29) | 0x01;
			pp[10] =        ((cur_pes_pts & 0x03fc00000ULL) >> 22)       ;
			pp[11] =        ((cur_pes_pts & 0x0003f8000ULL) >> 14) | 0x01;
			pp[12] =        ((cur_pes_pts & 0x000007f80ULL) >>  7)       ;
			pp[13] =        ((cur_pes_pts & 0x00000007fULL) <<  1) | 0x01;
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
			
			if (1 != fwrite(pp_buffer, pp_valid, 1, mpgfile)) {
				fprintf(stderr, "\nerror while writing to MPEG-2 output file\n");
				return -1;
			}
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

	if (1 != fwrite(pp_buffer, max_out_pp, 1, mpgfile)) {
		fprintf(stderr, "\nerror while flushing to MPEG-2 output file\n");
		return -1;
	}
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

//fprintf(stderr,"resync - a=%ld, v=%ld\n", audio_packets_avail, video_packets_avail);
		
		while (audio_packets_avail && video_packets_avail) {
			
			unsigned char * ap = audio_packets[0];
			
			double audio_pts = pes_pts(ap);
			if (audio_pts < 0) {
				// cannot use this...
				remove_audio_packets(1);
if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - no audio pts\n");
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
if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - no ac3 start\n");
					continue;
				}
			}
			
			
			unsigned char * vp = video_packets[0];
						
			double video_pts = pes_pts(vp);
			if (video_pts < 0) {
				// cannot use this...
				remove_video_packets(1);
if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - no video pts\n");
				continue;
			}
			
			// we need to make sure our first video data comes
			// before our first audio data...
			
			if (audio_pts <= video_pts) { 
				remove_audio_packets(1);
if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - audio before video\n");
				continue;
			}

			if (0 == (vp[6] & 0x01)) {
				// this is not a video sequence start... cannot use this...
				remove_video_packets(1);
if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - no video sequence start\n");
				continue;				
			}
			
			// check for the PTS difference...
			double av_pts_diff = pts_diff(audio_pts, video_pts);
			
			if (fabs(av_pts_diff) < (90000.0/22)) {
				// ha - we found a place...
				
				playtime_offset += pts_diff(system_clock_ref, system_clock_ref_start);
				
				double smaller_pts = audio_pts;
				if (video_pts < smaller_pts) smaller_pts = video_pts;
				
				fprintf(stderr, "resynced at playtime %s                                 \n",
				        pts_to_hms(playtime_offset));
				
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

if (DEBUG_RESYNC_DROPS) fprintf(stderr,"drop - a/v pts diff too big\n");

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
		if (   fabs(pts_diff(vp_pts[0], ap_pts[0]))        > 1*90000.0
		    || fabs(pts_diff(vp_pts[0], system_clock_ref)) > 1*90000.0
		    || fabs(vp_dur[0])                                    > 1*90000.0
			 || fabs(ap_dur[0])                                    > 1*90000.0
			) {
			
			// oops, our data seems to be severely broken. need a resync...
			
			fprintf(stderr,"ap_pts[0]=%f, ap_pts[1]=%f, ap_dur[0]=%f             \n",
			        ap_pts[0], ap_pts[1], ap_dur[0]);

			fprintf(stderr,"vp_pts[0]=%f, vp_pts[1]=%f, vp_dur[0]=%f             \n",
			        vp_pts[0], vp_pts[1], vp_dur[0]);
			
			fprintf(stderr,"vp_pts[0]-ap_pts[0] =%f                             \n",
			        pts_diff(vp_pts[0], ap_pts[0]));

			fprintf(stderr,"vp_pts[0]-system_clock_ref =%f                             \n",
			        pts_diff(vp_pts[0], system_clock_ref));
					  
			fprintf(stderr,"resyncing due to big time differences in packet buffers     \n");
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
					fprintf(stderr, "\nerror while flushing to pp\n");
					return -1;
				}
			}
			
last_video_pts = vp_pts[0];
			

			double pes_duration = vp_dur[0];
			
			double scr_duration = pts_diff(f2_pts, vp_pts[0]);						
			
			if (write_pes_in_pp(video_packets[0], mpgfile, system_clock_ref, seqstart, false,
				                 seqstart, pes_duration, scr_duration )) {
				fprintf(stderr, "error while writing video pes to pp\n");
				return -1;
			}

			remove_video_packets(1);

		} else {

			// write audio frame

last_audio_pts = ap_pts[0];

			bool seqstart = (((audio_packets[0])[6] & 0x01) == 0x01)? true : false;
			
			if (seqstart) {
				if (flush_pp(mpgfile)) {
					fprintf(stderr, "\nerror while flushing to pp\n");
					return -1;
				}
			}
			
			double pes_duration = ap_dur[0];
			
			system_clock_ref = ap_pts[0];
			
			double scr_duration = pts_diff(f2_pts, ap_pts[0]);						
			
			if (write_pes_in_pp(audio_packets[0], mpgfile, system_clock_ref, true, true,
				                 false, pes_duration, scr_duration )) {
				fprintf(stderr, "error while writing video pes to pp\n");
				return -1;
			}
			
			remove_audio_packets(1);

		}


if (DEBUG_AV_DESYNC && last_video_pts>=0 && last_audio_pts>=0 && fabs(last_video_pts-last_audio_pts) > 0000) {
fprintf(stderr,"Remuxer detected some a/v desync: ap=%ld, vp=%ld, scr=%.0f, audio=%.0f, video=%.0f, sv=%.0f, sa=%.0f\n",
     audio_packets_avail, video_packets_avail,
	  system_clock_ref-system_clock_ref_start,
	  last_audio_pts-system_clock_ref_start,
	  last_video_pts-system_clock_ref_start,
	  (pes_pts(video_packets[0])+scr_play_offset) - system_clock_ref,
	  (pes_pts(audio_packets[0])+scr_play_offset) - system_clock_ref
	 );
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

		if (!mpp_started) {

			// make sure to start with a magic sync word...

			if (orig_pes[3] == STREAM_PRIVATE_1) {
				if (orig_es_size < 4 || orig_es_data[0] != 0x0b || orig_es_data[1] != 0x77) {
					return 0;
				}
			} else {
				// MPEG audio
				if (orig_es_size < 4 || orig_es_data[0] != 0xff || (orig_es_data[1] & 0xe0) != 0xe0) {
					return 0;
				}
			}

			mpp_started = true;
		}


		if (1 != fwrite(orig_es_data, orig_es_size, 1, mppfile)) {
			fprintf(stderr, "\nerror while writing to .mpp output file\n");
			return -1;
		}
	}

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

		if (1 != fwrite(orig_es_data, orig_es_size, 1, mpvfile)) {
			fprintf(stderr, "\nerror while writing to .mpv output file\n");
			return -1;
		}
	}

	return 0;
}

// ##################################################################

