#ifndef DISABLE_FILE

#include "lib/codecs/codecmpg.h"
#include <lib/dvb/decoder.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <lib/base/eerror.h>
#include <lib/system/econfig.h>

inline void eDemux::refill()
{
	if (input.read(&last, 4) != 4)
	{
		eDebug("read error ! :))");
		last = 0x1b9;		// simulate end of program stream.
	}
	else
		last = htonl(last);
	remaining = 32;
}

unsigned long eDemux::getBits(unsigned int num)
{
	unsigned long res = 0;
	unsigned int d;

	if (num <= remaining)
	{
		res = (last >> (remaining - num)) & ~(-1 << num);
		remaining -= num;
		return res;
	}
	
	while (num)
	{
		if (!remaining)
			refill();
		d = num;
		if (d > remaining)
			d = remaining;
		if (res)
			res <<= d;
		res |= (last >> (remaining - d)) & ~(-1 << d);
		remaining -= d;
		num -= d;
	}
	return res;
}

inline void eDemux::syncBits()
{
		// round UP. so we re-read the last octet.
		// that's ok, since syncBits() only does something
		// when we're out of sync. and in that case, we might have
		// already read one bit of the startcode.
	remaining +=  7;
	remaining &= ~7;
}

eDemux::eDemux(eIOBuffer &input, eIOBuffer &video, eIOBuffer &audio, int fd, int sourcefd)
	:input(input), video(video), audio(audio), minFrameLength(4096),
	mpegtype(-1), curAudioStreamID(0), synced(0), fd(fd), sheader(0)

{
	init_eDemux(sourcefd);
}
void eDemux::init_eDemux(int sourcefd)
{
	remaining=0;
	memset(&pcmsettings, 0, sizeof(pcmsettings));

	// extract first and last timestamp from file, if available
	if (sourcefd >= 0)
	{
		off64_t oldpos=::lseek64(sourcefd, 0, SEEK_CUR);
		::lseek64(sourcefd, 0, SEEK_SET);
		unsigned char tmp[65424];
		int i=0;
		int ok = 0;
		while ( i < 10)
		{
			int rd = ::read(sourcefd, tmp, 65424);
			if (ok = readTimestamp(tmp,rd,&movie_begin))
				break;
			i++;
		}
		if (ok)
		{
			filelength=::lseek64(sourcefd,0, SEEK_END);
			::lseek64(sourcefd, filelength - (off64_t)654240, SEEK_SET);
			i = 0;
			ok = 0;
			while ( i < 10)
			{
				int rd = ::read(sourcefd, tmp, 65424);
				if (readTimestamp(tmp,rd,&movie_end))
					ok = 1;
				i++;
			}
			if (ok)
			{
				sec_duration = movie_end - movie_begin; 
				eDebug("[TimeStampMPEG]set duration:%d,%d,%d",sec_duration, movie_begin,movie_end);
			}
			::lseek64(sourcefd, oldpos, SEEK_SET);
		}
	}

}
void eDemux::setCurrentTime(unsigned char* data, int len)
{
	if (sec_duration < 0)
		return;
	int movie_current;
	if (readTimestamp(data,len,&movie_current))
	{
		sec_currentpos = movie_current - movie_begin;
	}
}
int eDemux::readTimestamp(unsigned char* data, int len, int* seconds)
{
	unsigned char* pos = data;
	while (pos-data<len)
	{

		int x  = (((int)pos[0])<<16) +(((int)pos[1])<<8) +((int)pos[2]);
		pos++;
		if ( x != 0x1)
			continue;
		pos += 2;
		int streamid = *pos++;
		switch(streamid)
		{
			case 0xC0 ... 0xCF:
			case 0xE0 ... 0xEF:
			{
				unsigned char* pos2 = pos;
				int plen = (((int)pos[0])<<8) +((int)pos[1]);// packet length
				pos += 2;
				if (parseData(pos,plen,seconds))
					return 1;
				pos = pos2 + plen;
				break;
			}
		}
	}
	return 0;
}
int eDemux::parseData(unsigned char* data, int len, int* seconds)
{
	unsigned char* pos = data;
	while (pos-data<len)
	{
		if (((*pos)&0xc0) != 0xc0)
			break;
		pos++;
	}	
	int haspts = 0;
	switch ((*pos)&0xc0)
	{
		case 0x40: // MPEG-1
			pos+=2;
			haspts = (*pos)&0x20;
			break;
		case 0x80: // MPEG-2
			pos++;
			haspts = (*pos)&0x80;
			pos+=2;
			if (haspts) haspts = (*pos)&0xe0 == 0x20;
			break;
		case 0x00: // MPEG-1 ohne Indikator
			haspts = (*pos)&0x20;
			break;
	}
	if (pos-data<len-5 && haspts)
	{
		unsigned long long pts;
		pts  = ((unsigned long long)(pos[0]&0xE))  << 29;
		pts |= ((unsigned long long)(pos[1]&0xFF)) << 22;
		pts |= ((unsigned long long)(pos[2]&0xFE)) << 14;
		pts |= ((unsigned long long)(pos[3]&0xFF)) << 7;
		pts |= ((unsigned long long)(pos[4]&0xFE)) >> 1;
		*seconds = pts / 90000;
		return 1;
	}
	return 0;
}
eDemux::~eDemux()
{
	eConfig::getInstance()->setKey("/ezap/audio/prevAudioStreamID", curAudioStreamID);
	delete [] sheader;
}

int eMPEGDemux::decodeMore(int last, int maxsamples, Signal1<void,unsigned int>*newastreamid)
{
//	eDebug("decodeMore");
	int written = 0;
	(void)last;

	while (written < maxsamples)
	{
		unsigned int code = 0;
		while (1)	// search for start code.
		{
			if (input.size() < 4096)
			{
				return written; // not enough data to get going. Give enigma a change to do something else
			}
			syncBits();
			// need a sequenc of at least 2 zero bytes.
			if (getBits(8))
				continue;
			if (getBits(8))
				continue;
			int c;
			while (!(c = getBits(8)));
			// followed by a 1.
			if (c != 1)
				continue;
			if (!maxsamples)
				break;
			code = getBits(8);
			switch (code)
			{
				case 0xb9: // MPEG_program_end_code
				{
					eDebug("program_end_code");
					goto finish;
				}
				case 0xba: // pack_start_code
				{
					int type=getBits(2);
					//eDebug("[codecMPG] pack_start_code %02x type=%d", code, type);
					if ( type != mpegtype )
					{
						switch (type)
						{
							case 1:
								Decoder::SetStreamType(TYPE_PES);
								break;
							default:
								Decoder::SetStreamType(TYPE_MPEG1);
								break;
						}
						mpegtype=type;
						//eDebug("[codecMPG] set %s", type == 1 ? "MPEG-2" : "MPEG-1" );
					}
					if (type != 1)
					{
						getBits(6);
						getBits(16);
						getBits(16);
						getBits(16);
						getBits(8);
						continue;
					}
					int scr_base0, scr_base, scr_ext;
					scr_base0=getBits(3);
					scr_base=scr_base0<<30;
					scr_base0>>=2;
					if (!getBits(1))
						continue;
					scr_base|=getBits(15)<<15;
					if (!getBits(1))
						continue;
					scr_base|=getBits(15);
					if (!getBits(1))
						continue;
					scr_ext=getBits(9);
					if (!getBits(1))
						continue;
					/* int program_mux_rate= */ getBits(22);
					if (getBits(2) != 3)
						continue;
					getBits(5);
					int pack_stuffing_length = getBits(3);
					while (pack_stuffing_length--)
						if (getBits(8) != 0xFF)
							break;
					if (pack_stuffing_length >= 0)
						continue;
					// eDebug("scr: %d / 300 90kHz:%02d 27MHz, program mux rate = %d 50bytes/second", scr_base, scr_ext, program_mux_rate);
					break;
				}
				case 0xbb:  // system_header_start_code
				case 0xBE:  // Padding Stream
				case 0xBF:  // Private Stream 2 (???)
				case 0xF0:
				case 0xF1:
				case 0xF2:
				case 0xF3:
				case 0xFF:
				{
					// eDebug("[codecMPG] system_header %02x", code);
					int length = getBits(16);
					while (length && remaining)
					{
						getBits(8);
						--length;
					}
					if (length)
					{
						char buffer[length];
						if ( input.read(buffer, length) != length )
							eDebug("read Error in skip");
					}
					break;
				}
				case 0xbc: // program_stream_map
				{
#if 0
					int program_stream_map_length=getBits(16);
					eDebug("program stream map!\n");
					int current_next_indicator=getBits(1);
					getBits(2);
					int program_stream_map_version=getBits(5);
					getBits(7);
					if (!getBits(1))
						continue;
					int program_stream_info_length=getBits(16);
					for (int r=0; r<program_stream_info_length; )
					{
						int tag=getBits(8);
						length=getBits(8);
						r+=length+2;
						eDebug("tag: %02x %02x ", tag, length);
						while (length--)
							eDebug("%02lx ", getBits(8));
						eDebug("\n");
					}
					int elementary_stream_map_length=getBits(16);
					for (int r=0; r < elementary_stream_map_length; )
					{
						int stream_type=getBits(8);
						int elementary_stream_id=getBits(8);
						int elementary_stream_info_length=getBits(16);
						for (int a=0; a < elementary_stream_info_length; )
						{
							int tag=getBits(8);
							int length=getBits(8);
							eDebug("elementary: %02x %02x ", tag, length);
							while (length--)
								eDebugNoNewLine("%02x ", getBits(8));
							eDebug("\n");
						}
						r+=elementary_stream_info_length+4;
					}
					getBits(32);
#endif
					break;
				}
				case 0xC0 ... 0xCF:  // Audio Stream
				case 0xD0 ... 0xDF:
				{
					int &cnt = audiostreams[code];
					if ( cnt < 10 )
					{
						cnt++;
						if ( cnt == 10 )
						{
//							eDebug("/*emit*/ (*newastreamid)(%02x)", code);
							if ( !curAudioStreamID )
							{
								Decoder::parms.audio_type = DECODE_AUDIO_MPEG;
								Decoder::Set();
								curAudioStreamID = code;
							}
							/*emit*/ (*newastreamid)(code);
						}
					}
					// fall through to process packet
				}
				case 0xBD:  // Private Stream 1 (AC3 or ttx)
				case 0xE0 ... 0xEF:  // Video Stream
				{
					int length = getBits(16);
					if ((length + 6) > minFrameLength)
					{

						if ((minFrameLength + 2048) > (length + 6))
							minFrameLength = length + 6;
						else
							minFrameLength += 2048;
						eDebug("[codecMPG] minFrameLength now %d", minFrameLength);
					}
					unsigned char buffer[6 + length];
					int p = 0;

					buffer[p++] = 0;
					buffer[p++] = 0;
					buffer[p++] = 1;
					buffer[p++] = code;

					buffer[p++] = length >> 8;
					buffer[p++] = length & 0xFF;

					while (length && remaining)
					{
						buffer[p++] = getBits(8);
						--length;
					}

					if (length)
					{
						int rd = input.read(buffer + p, length);
						setCurrentTime(buffer, 6+rd);
						if ( rd != length )
						{  // buffer empty.. put all data back in input buffer
							// eDebug("[codecMPG] input buffer emtpy, want %d got %d return buffer to input", length, rd);
							input.write(buffer, p + rd);
							return written;
						}
/*						else
							eDebug("[codecMPG] read %04x bytes", length);*/
						p += length;
					}

					if (code == 0xBD)
					{
						int offs = buffer[8];
						int subid = buffer[8+offs+1];

//						eDebug("offs = %02x, subid = %02x",offs, subid);


//						if (offs == 0x24 && subid == 0x10) // TTX stream...
//							break;

						if (subid < 0x80)
							break;

//						if (subid > 0xA7) // 0xA0 .. 0xA7  (LPCM)
//							break;

						if (subid > 0x87) // 0x88 .. 0x89   (DTS)
							break;
/*
						for (int i=0; i < 32; ++i)
							eDebugNoNewLine("%02x ", buffer[i]);
						eDebug("");*/

						// here we have subid 0x80 .. 0x87
						code |= (subid << 8);
						int &cnt = audiostreams[code];
						if (cnt < 10)
						{
							cnt++;
							if ( cnt == 10 )
							{
//								eDebug("found new AC3 stream subid %02x", subid);
//								eDebug("/*emit*/ (*newastreamid)(%04x)", code);
								if (!curAudioStreamID)
								{
									Decoder::parms.audio_type = DECODE_AUDIO_AC3_VOB;
									Decoder::Set();
									curAudioStreamID = code;
								}
								/*emit*/ (*newastreamid)(code);
							}
						}
					}
					// check old audiopackets in syncbuffer
					if (syncbuffer.size())
					{
						unsigned int VideoPTS = 0xFFFFFFFF;
						Decoder::getVideoPTS(VideoPTS);
						if (VideoPTS != 0xFFFFFFFF)
						{
							std::list<syncAudioPacket>::iterator it(syncbuffer.begin());
							for (; it != syncbuffer.end(); ++it)
							{
								if (abs(VideoPTS - it->pts) <= 0x1000)
								{
									eDebug("[codecMPG] synced2");
									break;
								}
							}
							if (it != syncbuffer.end())
							{
								synced = 1;
								// write data from syncbuffer to audio device
								for (; it != syncbuffer.end(); ++it)
									audio.write(it->data, it->len);
								// cleanup syncbuffer
								for (it = syncbuffer.begin(); it != syncbuffer.end(); ++it)
									delete [] it->data;
								syncbuffer.clear();
							}
						}
					}
					if (code > 0xDF && code < 0xF0)
					{
						videostreams.insert(code);
						if (code != 0xE0 && videostreams.find(0xE0) != videostreams.end())
							; // dont play video streams != 0xE0 when 0xE0 is avail...
						else
						{
							bool sent = false;
							if (synced == 1 && sheader)
							{
								int pos = 6;
								while (pos < p)
								{
									if (buffer[pos++])
										continue;
									if (buffer[pos++])
										continue;
									while (!buffer[pos])
										pos++;
									if (buffer[pos++] != 1)
										continue;
									if (buffer[pos++] != 0xb8) // group start code
										continue;
									pos -= 4; // before GROUP START
									int length = (buffer[4] << 8) | buffer[5];
									length += sheader_len;
									buffer[4] = length >> 8;
									buffer[5] = length & 0xFF;
									video.write(buffer, pos);
									video.write(sheader, sheader_len);
									video.write(buffer + pos, p - pos);
									synced++;
									sent = true;
									break;
								}
							}
							if (!sent)
								video.write(buffer, p);
							written += p;
						}
					}
					else if (code == curAudioStreamID)
					{
						// check current audiopacket
						if (!synced)
						{
							unsigned int AudioPTS = 0xFFFFFFFF,
									VideoPTS = 0xFFFFFFFF,
									pos = 5;
							while (buffer[++pos] == 0xFF);  // stuffing überspringen
							if ((buffer[pos] & 0xC0) == 0x40) // buffer scale size
								pos += 2;
							if (((buffer[pos] & 0xF0) == 0x20) ||  //MPEG1
								((buffer[pos] & 0xF0) == 0x30) ||  //MPEG1
								((buffer[pos] & 0xC0) == 0x80))   //MPEG2
							{
								int readPTS = 1;
								if ((buffer[pos] & 0xC0) == 0x80) // we must skip many bytes
								{
									if ((c & 0x30) != 0)
										eDebug("[codecMPG] warning encrypted multiplex not handled!!!");
									++pos; // flags
									if (((buffer[pos] & 0xC0) != 0x80) &&
										((buffer[pos] & 0xC0) != 0xC0))
										readPTS = 0;
									pos += 2;
								}
								if (readPTS)
								{
									AudioPTS = (buffer[pos++] >> 1) << 29;
									AudioPTS |= buffer[pos++] << 21;
									AudioPTS |=(buffer[pos++] >> 1) << 14;
									AudioPTS |= buffer[pos++] << 6;
									AudioPTS |= buffer[pos] >> 2;
//									eDebug("[codecMPG] APTS %08x", AudioPTS);
								}
							}
							Decoder::getVideoPTS(VideoPTS);
							if (VideoPTS != 0xFFFFFFFF && abs(VideoPTS - AudioPTS) <= 0x1000)
							{
								synced = 1;
								eDebug("[codecMPG] synced1");
								// cleanup syncbuffer.. we don't need content of it
								std::list<syncAudioPacket>::iterator it( syncbuffer.begin() );
								for (; it != syncbuffer.end(); ++it)
									delete [] it->data;
								syncbuffer.clear();
							}
							else if ((AudioPTS > VideoPTS) || VideoPTS == 0xFFFFFFFF)
							{
								syncAudioPacket pack;
								pack.pts = AudioPTS;
								pack.len = p;
								pack.data = new __u8[p];
								memcpy( pack.data, buffer, pack.len );
								syncbuffer.push_back( pack );
								//Debug("[codecMPG] PTSA = %08x PTSV = %08x DIFF = %08x AUDIO queued", AudioPTS, VideoPTS, abs(AudioPTS-VideoPTS));
							}
						}
						if (synced)
							audio.write(buffer, p);
						written += p;
					}
					break;
				}
				default:
				{
					if ( audio.size() || video.size() )
						eDebug("unhandled code... but already data in buffers!!");
					for (std::map<int,int>::iterator it(audiostreams.begin());
						it != audiostreams.end();)
					{
						if ( it->second < 10 )
							audiostreams.erase(it++);
						else
							++it;
					}
					eDebug("unhandled code %02x", code);
				}
			}
		}
	}
finish:
	return written;
}

int ePVADemux::decodeMore(int last, int maxsamples, Signal1<void,unsigned int>*newastreamid )
{
//	eDebug("decodeMore");
	int written=0;
	(void)last;

	while (written < maxsamples)
	{
		bool readedPTS=false;
		unsigned char tmp[8];
		unsigned char pts[4];
		while (1)	// search for start code.
		{
			if (input.size() < 4096)
			{
				maxsamples=0;
				break;
			}
			syncBits();

			tmp[0]=getBits(8);

			if (tmp[0] != 0x41)
				continue;

			tmp[1]=getBits(8);

			if (tmp[1] != 0x56)
				continue;

			tmp[2]=getBits(8);
			tmp[3]=getBits(8);
			tmp[4]=getBits(8);

			if ( tmp[4] != 0x55 )
			{
				unsigned int cnt=0;
				unsigned char backbuff[input.size()+4+3];
				backbuff[cnt++]=tmp[2];
				backbuff[cnt++]=tmp[3];
				backbuff[cnt++]=tmp[4];
				while (remaining)
					backbuff[cnt++]=getBits(8);
				cnt+=input.read(backbuff+cnt, input.size());
				input.write(backbuff, cnt);
				continue;
			}

			tmp[5]=getBits(8);
			tmp[6]=getBits(8);
			tmp[7]=getBits(8);
			int length=tmp[6]<<8|tmp[7];
			int rd=8;

			unsigned int code = tmp[2];

			if ( code == 1 && tmp[5] & 0x10 )
			{
				pts[0]=getBits(8);
				pts[1]=getBits(8);
				pts[2]=getBits(8);
				pts[3]=getBits(8);
				length-=4;
				rd+=4;
				readedPTS=true;
			}
			else
				readedPTS=false;

			if (code == 2)
				code = 0xC0;
			else if (code == 1)
				code = 0xE0;
			switch (code)
			{
				case 0x80 ... 0x87:  // AC3
				{
					code <<= 8;
					code |= 0xBD;
				}
				case 0xC0 ... 0xCF:  // Audio Stream
				case 0xD0 ... 0xDF:
				{
					int &cnt = audiostreams[code];
					if ( cnt < 10 )
					{
						cnt++;
						if ( cnt == 10 )
						{
//							eDebug("/*emit*/ (*newastreamid)(%02x)", code);
							if ( !curAudioStreamID )
							{
								Decoder::parms.audio_type = (code&0xFF==0xBD) ?
									DECODE_AUDIO_AC3_VOB : DECODE_AUDIO_MPEG;
								Decoder::Set();
								curAudioStreamID = code;
//								eDebug("set %02x", code);
							}
							/*emit*/ (*newastreamid)(code);
						}
					}
				}
				case 0xE0 ... 0xEF:  // Video Stream
				{
					if ( (length+rd) > minFrameLength )
					{
						if ( (minFrameLength+2048) > (length+rd) )
							minFrameLength=length+rd;
						else
							minFrameLength+=2048;
						eDebug("minFrameLength now %d", minFrameLength );
					}
					unsigned char buffer[length];
					unsigned int p=0;
					while ( length && remaining )
					{
						buffer[p++]=getBits(8);
						--length;
					}
					if ( length )
					{
						int rd = input.read(buffer+p, length);
						if ( rd != length )
						{  // buffer empty.. put all data back in input buffer
							input.write(tmp, 8);  // put pva header back into buffer
							if (readedPTS) // pts read?
								input.write(pts,4); // put pts bytes back into buffer
							input.write(buffer, rd); // put all readed bytes back into buffer
							return written;
						}
//						else
//							eDebug("read %04x bytes", length);
						p+=length;
					}

		// check old audiopackets in syncbuffer
					unsigned int VideoPTS=0xFFFFFFFF;
					Decoder::getVideoPTS(VideoPTS);
					if ( syncbuffer.size() )
					{
						if ( VideoPTS != 0xFFFFFFFF )
						{
							std::list<syncAudioPacket>::iterator it( syncbuffer.begin() );
							for (;it != syncbuffer.end(); ++it )
							{
								if ( abs(VideoPTS - it->pts) <= 0x1000 )
								{
									eDebug("synced2 :)");
									break;
								}
							}
							if ( it != syncbuffer.end() )
							{
								synced=1;
		// write data from syncbuffer to audio device
								for (;it != syncbuffer.end(); ++it )
									audio.write( it->data, it->len );
		// cleanup syncbuffer
								for (it=syncbuffer.begin();it != syncbuffer.end(); ++it )
									delete [] it->data;
								syncbuffer.clear();
							}
						}
					}
					if (code > 0xDF && code < 0xF0)
					{
						videostreams.insert(code);
						if ( code != 0xE0 && videostreams.find(240) != videostreams.end() )
							; // dont play video streams != 1 when 0xE0 is avail...
						else
						{
							int headerPos=-1;
							if ( synced==1 && sheader)
							{
								unsigned int pos = 0;
								while(pos < p)
								{
									if ( buffer[pos++] )
										continue;
									if ( buffer[pos++] )
										continue;
									while ( !buffer[pos] )
										pos++;
									if ( buffer[pos++] != 1 )
										continue;
									if ( buffer[pos++] != 0xb8 ) // group start code
										continue;
									pos-=4; // before GROUP START
									headerPos=pos;
									break;
								}
							}
							if ( readedPTS )
							{
								VideoPTS=pts[0]<<24|pts[1]<<16|pts[2]<<8|pts[3];
								int payload_length = tmp[6]<<8 | tmp[7];
								payload_length+=3;
								if (headerPos!=-1)
									payload_length+=sheader_len;
								char buf[14] = { 0, 0, 1, code&0xFF,
									(payload_length & 0xFF00)>>8,
									payload_length&0xFF,
									0x80, 0x80, 0x05,
									0x20|((VideoPTS&0xC0000000)>>29)|1,
									(VideoPTS&0x3FC00000)>>22,
									((VideoPTS&0x3F8000)>>14)|1,
									(VideoPTS&0x7F80)>>7,
									((VideoPTS&0x7F)<<1)|1};
								video.write(buf, 14);
							}
							else
							{
								int payload_length = tmp[6]<<8 | tmp[7];
								payload_length+=3;
								if (headerPos!=-1)
									payload_length+=sheader_len;
								char buf[9] = { 0, 0, 1, code&0xFF,
									(payload_length & 0xFF00)>>8,
									payload_length&0xFF,
									0x80, 0, 0 };
								video.write(buf, 9);
							}
							if (headerPos != -1)
							{
								video.write(buffer,headerPos);
								video.write(sheader,sheader_len);
								video.write(buffer+headerPos, p-headerPos);
								synced++;
							}
							else
								video.write(buffer, p);
							written+=p;
						}
					}
					else if ( code == curAudioStreamID )
					{
		// check current audiopacket
						if (!synced)
						{
							unsigned int AudioPTS = 0xFFFFFFFF,
											VideoPTS = 0xFFFFFFFF,
											pos=5;
							while( buffer[++pos] == 0xFF );  // stuffing überspringen
							if ( (buffer[pos] & 0xC0) == 0x40 ) // buffer scale size
								pos+=2;
							if ( ((buffer[pos] & 0xF0) == 0x20) ||  //MPEG1
									 ((buffer[pos] & 0xF0) == 0x30) ||  //MPEG1
									 ((buffer[pos] & 0xC0) == 0x80) )   //MPEG2
							{
								int type=0;
								int readPTS=1;
								if ((buffer[pos] & 0xC0) == 0x80) // we must skip many bytes
								{
									type=1;
									++pos; // flags
									if ( ((buffer[pos]&0xC0) != 0x80) &&
											 ((buffer[pos]&0xC0) != 0xC0) )
										readPTS=0;
									pos+=2;
								}
								if ( type != mpegtype )
								{
									if ( type == 1 )
										Decoder::SetStreamType(TYPE_PES);
									else
										Decoder::SetStreamType(TYPE_MPEG1);
									mpegtype=type;
//									eDebug("set %s", type == 1 ? "MPEG-2" : "MPEG-1" );
								}
								if (readPTS)
								{
									AudioPTS = (buffer[pos++] >> 1) << 29;
									AudioPTS |= buffer[pos++] << 21;
									AudioPTS |=(buffer[pos++] >> 1) << 14;
									AudioPTS |= buffer[pos++] << 6;
									AudioPTS |= buffer[pos] >> 2;
//									eDebug("APTS %08x", AudioPTS);
								}
							}
							Decoder::getVideoPTS(VideoPTS);
//							eDebug("VPTS %08x", VideoPTS);
							if ( VideoPTS != 0xFFFFFFFF && abs(VideoPTS - AudioPTS) <= 0x1000 )
							{
								synced=1;
								eDebug("synced1 :)");
		// cleanup syncbuffer.. we don't need content of it
								std::list<syncAudioPacket>::iterator it( syncbuffer.begin() );
								for (;it != syncbuffer.end(); ++it )
									delete [] it->data;
								syncbuffer.clear();
							}
							else if ( (AudioPTS > VideoPTS) || VideoPTS == 0xFFFFFFFF )
							{
								syncAudioPacket pack;
								pack.pts = AudioPTS;
								pack.len = p;
								pack.data = new __u8[p];
								memcpy( pack.data, buffer, pack.len );
								syncbuffer.push_back( pack );
//								eDebug("PTSA = %08x\nPTSV = %08x\nDIFF = %08x",
//									AudioPTS, VideoPTS, abs(AudioPTS-VideoPTS) );
							}
						}
						if ( synced )
							audio.write(buffer, p);
						written+=p;
					}
					break;
				}
				default:
					/*eDebug("unhandled code %02x", tmp[2])*/;
			}
		}
	}
	return written;
}

void eDemux::resync()
{
	remaining=synced=0;
	// clear syncbuffer
	std::list<syncAudioPacket>::iterator it( syncbuffer.begin() );
	for (;it != syncbuffer.end(); ++it )
		delete [] it->data;
	syncbuffer.clear();
}

int eDemux::getMinimumFramelength()
{
	return minFrameLength;
}

int eDemux::getAverageBitrate()
{
	if (sec_duration > 0 && filelength > 0)
		return (filelength/sec_duration)*8;
	return 3*1024*1024;
}

void eDemux::setAudioStream( unsigned int id )
{
	if ( curAudioStreamID != id && audiostreams.find(id) != audiostreams.end() )
	{
		if ( (id&0xFF) == 0xBD)
		{
			// not thread safe !!
			Decoder::parms.audio_type = DECODE_AUDIO_AC3_VOB;
			Decoder::Set();
		}
		else
		{
			// not thread safe !!
			Decoder::parms.audio_type = DECODE_AUDIO_MPEG;
			Decoder::Set();
		}
		curAudioStreamID = id;
	}
}

void eDemux::extractSequenceHeader( unsigned char *buffer, unsigned int len )
{
	if (!sheader) // we have no sequence header
	{
		unsigned int pos = 0;

		while(pos < len)
		{
			if ( buffer[pos++] )
				continue;
			if ( pos >=len )
				return;
			if ( buffer[pos++] )
				continue;
			if ( pos >=len )
				return;
			while ( !buffer[pos] )
			{
				pos++;
				if ( pos >=len )
					return;
			}
			if ( buffer[pos++] != 1 )
				continue;
			if ( pos >=len )
				return;
			if ( buffer[pos++] != 0xB3 )
				continue;
			if ( pos >=len )
				return;
			pos+=7;
			if ( pos >=len )
				return;
			sheader_len=12;
			if ( buffer[pos] & 2 ) // intra matrix avail?
			{
				pos+=64;
				if ( pos >=len )
					return;
				sheader_len+=64;
			}
			if ( buffer[pos] & 1 ) // non intra matrix avail?
			{
				pos+=64;
				if ( pos >=len )
					return;
				sheader_len+=64;
			}
			pos+=1;
			if ( pos+3 >=len )
				return;
			// extended start code
			if ( !buffer[pos] && !buffer[pos+1] && buffer[pos+2] && buffer[pos+3] == 0xB5 )
			{
				pos+=3;
				sheader_len+=3;
				do
				{
					pos+=1;
					++sheader_len;
					if (pos+2 > len)
						return;
				}
				while( buffer[pos] || buffer[pos+1] || !buffer[pos+2] );
			}
			if ( pos+3 >=len )
				return;
			if ( !buffer[pos] && !buffer[pos+1] && buffer[pos+2] && buffer[pos+3] == 0xB2 )
			{
				pos+=3;
				sheader_len+=3;
				do
				{
					pos+=1;
					++sheader_len;
					if (pos+2 > len)
						return;
				}
				while( buffer[pos] || buffer[pos+1] || !buffer[pos+2] );
			}
			sheader = new unsigned char[sheader_len];
			memcpy(sheader, &buffer[pos-sheader_len], sheader_len);
//			for (unsigned int i=0; i < sheader_len; ++i)
//				eDebugNoNewLine("%02x", sheader[i]);
//			eDebug("");
			return;
		}
	}
}

#endif // DISABLE_FILE

// lpcm
// lpcm dvd
// frame rate 600Hz (48/96kHz)
// 16/20/24 bits
// 8ch
// max 6.144Mbps
// [0] private stream sub type
// [1] number_of_frame_headers
// [2-3] first_access_unit_pointer (offset from [3])
// [4] audio_frame_number
//          (of first access unit (wraps at 19 ?))
//          (20 frames at 1/600Hz = 1/30 (24 frames for 25fps?)
// [5]:
//      b7-b6: quantization_word_length,
//            0: 16bits, 1: 20bits, 2: 24bits, 3: reserved
//      b5: reserved
//      b4: audio_sampling_frequency,
//            0: 48 kHz, 1: 96 kHz
//      b3: reserved
//      b2-b0: number_of_audio_channels
//            0: mono (2ch ? dual-mono: L=R)
//            1: 2ch (stereo)
//            2: 3 channel
//            3: 4 ch
//            4: 5 ch
//            5: 6 ch
//            6: 7 ch
//            7: 8 ch
// [6]: dynamic_range
