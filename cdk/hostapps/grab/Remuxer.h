/*
 * $Id: Remuxer.h,v 1.2 2002/08/31 01:06:19 obi Exp $
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
 * $Log: Remuxer.h,v $
 * Revision 1.2  2002/08/31 01:06:19  obi
 * sync with grab v1.40
 *
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */
 
#ifndef Remuxer_h
#define Remuxer_h

#include <cstring>
#include <cstdio>
#include <cmath>

// #######################################################

#define STREAM_PROGRAM_MAP	0xBC
#define STREAM_PRIVATE_1	0xBD
#define STREAM_PRIVATE_2	0xBF
#define STREAM_PADDING		0xBE
#define STREAM_AUDIO		0xBC	
#define STREAM_VIDEO		0xE0
#define STREAM_ECM		0xF0
#define STREAM_EMM		0xF1
#define STREAM_DSMCC		0xF2
#define STREAM_ITU_A		0xF4	
#define STREAM_ITU_B		0xF5	
#define STREAM_ITU_C		0xF6	
#define STREAM_ITU_D		0xF7	
#define STREAM_ITU_E		0xF8	
#define STREAM_MASK_AUDIO	0xE0
#define STREAM_REST_AUDIO	0xC0
#define STREAM_MASK_VIDEO	0xF0
#define STREAM_REST_VIDEO	0xE0

// #######################################################

// returns a PTS difference (l-r)
inline double pts_diff(double l, double r) {
	
	if (l < 0.0 || r < 0.0) return -1.0;
	
	double res = l - r;
	
	if (fabs(res) >= (double)(0x80000000UL)) {
		if (l > r) {
			if (l > 0x180000000ULL) {
				// assume that l wrapped around the 33-bit border
				l -= 0x200000000ULL;
			} else if (l > 0x80000000UL) {
				// assume that l wrapped around the 32-bit border
				l -= 0x100000000ULL;
			}
		} else {
			if (r > 0x180000000ULL) {
				// assume that r wrapped around the 33-bit border
				r -= 0x200000000ULL;
			} else if (r > 0x80000000UL) {
				// assume that r wrapped around the 32-bit border
				r -= 0x100000000ULL;
			}
		}
		res = l - r;
	}
	
	return res;	
}

// ########################################################

class Remuxer {
public:

	enum {
		max_audio_packets = 200,  // would be a max. of ~3MB
		max_video_packets = 200,
		max_aux_packets = 200,  // only one double each, holding the PTSes for the
		                       //  to-be-created video PES packets
		
		max_out_pp = 2*1024  // maximum size of MPEG-2 program pack we want to write
	};
	
	
	// ====================================================
	
	unsigned char * abuf;
	unsigned long abuf_size;
	
	unsigned long abuf_valid;
	
	// ----------------------------
	
	unsigned char * audio_packets[max_audio_packets];
	unsigned long audio_packets_avail;
	
	
	// ----------------------------
	
	unsigned char wanted_audio_stream;
	
	// ----------------------------
	// ====================================================


	// ====================================================
	
	unsigned char * vbuf;
	unsigned long vbuf_size;
	
	unsigned long vbuf_valid;
	
	// ----------------------------
	
	unsigned char * video_packets[max_video_packets];
	unsigned long video_packets_avail;
	
	double frame_pts; // our last synthesized PTS for video frames
	int frame_skew_counter;
	
	double aux_packets[max_aux_packets];
	unsigned long aux_packets_avail;
	
	bool one_pts_per_gop;
	
	// ----------------------------
	
	unsigned char pp_buffer[max_out_pp];
	unsigned long pp_valid;
	
	unsigned long mplex_rate;
	
	double frame_rate;
	
	bool resync;
	unsigned long long resyncs;
	
	double system_clock_ref_start;
	double system_clock_ref;
	double playtime_offset;  // playtime before the last resync
		
	unsigned long long total_bytes_written;
	
	unsigned long long scr_play_offset;
	double             video_forerun;
	double allowed_frame_pts_skew;	

	bool mpp_started; // for ::write_mpp, set once the first sync mark was found
	
	// ====================================================
	
	Remuxer(unsigned char _wanted_audio_stream = 0x00) {
		abuf_size = 1*1024*1024;
		abuf = new unsigned char[abuf_size];
		abuf_valid = 0;
		
		audio_packets_avail = 0;
		
		wanted_audio_stream = _wanted_audio_stream;

		// -------------
		
		vbuf_size = 2*1024*1024;
		vbuf = new unsigned char[vbuf_size];
		vbuf_valid = 0;
		
		video_packets_avail = 0;
		
		frame_pts = -1.0;
		frame_skew_counter = 0;
		
		// --------------
		
		aux_packets_avail = 0;
		
		one_pts_per_gop = false;
		
		// --------------
		
		pp_valid = 0;
		mplex_rate = 10080000;
		
		frame_rate = 25.0;
		
		resync = true;
		resyncs = 0;
		
		system_clock_ref_start = -1.0;
		system_clock_ref = -1.0;
		playtime_offset = 0.0;
		
		total_bytes_written = 0;
		
		video_forerun   = 19000.0;
		scr_play_offset = 8000 + (unsigned long long)video_forerun;
		allowed_frame_pts_skew = 0.1 * 90000.0;

		mpp_started = false;
	}
	
	~Remuxer(void) {
		delete [] abuf;
		delete [] vbuf;
		
		remove_audio_packets(audio_packets_avail);
		remove_video_packets(video_packets_avail);
		remove_aux_packets(aux_packets_avail);		
	}

	// ----------------------------
	
	double get_playtime(void) const {
		double r = playtime_offset;
		if (system_clock_ref >= 0.0) {
			r += pts_diff(system_clock_ref, system_clock_ref_start);
		}
		return r;
	}
	
	// ----------------------------
	
	void perform_resync(void) {
		
		remove_audio_packets(audio_packets_avail);
		remove_video_packets(video_packets_avail);
		remove_aux_packets(aux_packets_avail);
		
		wanted_audio_stream = 0x00;
		
		abuf_valid = 0;
		vbuf_valid = 0;
		
		frame_pts = -1.0;
		frame_skew_counter = 0;
		
		resync = true;
	}
	
	// ----------------------------
	
	int supply_audio_data(void * data, unsigned long len);
	
	bool audio_packet_wanted(unsigned char * pes);
	
	int create_audio_pes(unsigned char * orig_pes, double duration);
	
	void remove_audio_packets(unsigned long n) {
		
		if (!n) return;
		
		unsigned long i = 0;
		for (; i < n && i < audio_packets_avail; i++) {
			delete [] audio_packets[i];
		}
		if (i) {
			memmove(audio_packets, audio_packets+i, (audio_packets_avail-i)*sizeof(char *));
			audio_packets_avail -= i;
		}
	}

	// ----------------------------
	
	int supply_video_data(void * data, unsigned long len);
	
	double aux_to_framepts(unsigned long aux_start, unsigned long aux_len);
	
	unsigned long count_video_frames(unsigned char * data, unsigned long len);
	
	int create_video_pes(unsigned char * vp_packet, unsigned long vp_len);
	
	void adjust_frame_pts(unsigned char * vp_packet, unsigned long vp_len);
	
	void remove_video_packets(unsigned long n) {
		
		if (!n) return;
		
		unsigned long i = 0;
		for (; i < n && i < video_packets_avail; i++) {
			delete [] video_packets[i];
		}
		if (i) {
			memmove(video_packets, video_packets+i, (video_packets_avail-i)*sizeof(char *));
			video_packets_avail -= i;
		}
	}
	
	// ----------------------------
	
	int supply_aux_data(void * data, unsigned long len);

	void remove_aux_packets(unsigned long n) {
		if (!n) return;
		memmove(aux_packets, aux_packets+n, (aux_packets_avail-n)*sizeof(double));
		aux_packets_avail -= n;
	}
	
	// ----------------------------
	
	int write_mpg(FILE * mpgfile);
	int write_pes_in_pp(unsigned char * pes, FILE * mpgfile,
	                    double & scr, bool stamp, bool stamp_all, bool output_sys_header,
							  double pes_duration, double scr_duration);
	int flush_pp(FILE * mpgfile);
	
	// ----------------------------


	int write_mpp(FILE * mppfile);

	int write_mpv(FILE * mpvfile);

	// ----------------------------

};




// #####################################################
// #####################################################

// return the total size of the PES packet pointed to by p

inline unsigned long pes_size(const unsigned char * p) {
	unsigned long ps = (p[4] << 8) | p[5];
	if (ps == 0) ps = 0x10000;
	return 6 + ps;
}

// #####################################################

// get the Presentation Time Stamp from a PES packet
// the PTS value is in measured in 90kHz clock cycles 
// -1.0 is the result of pes_pts if there's no PTS available

inline double pes_pts(const unsigned char * p) {
	if (0 == (p[7] & 0x80)) {
		return -1.0; // no PTS available
	}
	
	double res = 0.0;
	if (p[9] & 0x08) {
		res += 0x100000000ULL; // for the 32. bit
	}
	
	unsigned long rl = 0;
	
	rl |= (p[9]  & 0x06) << 29;
	rl |= p[10]          << 22;
	rl |= (p[11] & 0xfe) << 14;
	rl |= p[12]          << 7;
	rl |= (p[13] & 0xfe) >> 1;
	
	res += (double) rl;
	
	return rl;
}

// #####################################################

// set the Presentation Time Stamp for a PES packet

inline void set_pes_pts(unsigned char * p, double pts) {
	
	// assumes that there _is_ a PTS in the header of the PES!
	
	unsigned long long ipts = (unsigned long long)pts;
	
	p[9]  = 0x20 | ((ipts & 0x1c0000000ULL) >> 29) | 0x01;
	p[10] =        ((ipts & 0x03fc00000ULL) >> 22)       ;
	p[11] =        ((ipts & 0x0003f8000ULL) >> 14) | 0x01;
	p[12] =        ((ipts & 0x000007f80ULL) >>  7)       ;
	p[13] =        ((ipts & 0x00000007fULL) <<  1) | 0x01;
	
}

// #######################################################

inline const char * pts_to_hms(double pts) {
	static char hmsbuf[20];
	
	bool minus = false;
	
	unsigned long long p = (unsigned long long)pts;
	
	if (pts < 0) {
		minus = true;
		p = (unsigned long long)(-pts);
	}
	
	
	p /= 900;
	
	int sh = p % 10;
	p /= 10;
	int sz = p % 10;
	p /= 10;
	int s1 = p % 10;
	p /= 10;
	int s10= p % 6;
	
	p /= 6;
	int m1 = p % 10;
	p /= 10;
	int m10 = p % 6;
	p /= 6;
	
	int h = p;
	
	if (h > 1325) h = 1325; // must be wrong...
	
	if (minus) {	
		sprintf(hmsbuf, "-%4d:%d%d:%d%d.%d%d", h, m10, m1, s10, s1, sz, sh);
	} else {
		sprintf(hmsbuf, " %4d:%d%d:%d%d.%d%d", h, m10, m1, s10, s1, sz, sh);	
	}
	return hmsbuf;
	
}

// #######################################################
#endif /* Remuxer_h */
