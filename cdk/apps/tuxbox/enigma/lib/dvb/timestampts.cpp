/*
 * $Id: timestampts.cpp,v 1.7 2009/05/29 17:53:26 dbluelle Exp $
 *
 * (C) 2008 by Dr. Best  <dr.best@dreambox-tools.info>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <lib/dvb/timestampts.h>
#include <lib/base/eerror.h>
#include <lib/system/econfig.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#ifndef BYTE_ORDER
#error no byte order defined!
#endif

eTimeStampParserTS::eTimeStampParserTS(const char* filename): pktptr(0), pid(-1), needNextPacket(0), MovieCurrentTime(0),
	MovieBeginTime(0), MovieEndTime(0),MovieDuration(0),filelength(-1),sec_duration(-1),sec_currentpos(-1)
{
	init_eTimeStampParserTS(filename);
}
void eTimeStampParserTS::init_eTimeStampParserTS(const char* filename)
{
	basefilename = filename;
	// VPID ermitteln
	int fd=open(basefilename.c_str(), O_RDONLY|O_LARGEFILE);
	if (fd >= 0)
	{
		__u8 packet[188];
		do
		{
			if (::read(fd, packet, 188) != 188)
				break;
			if ((packet[0] != 0x47) || (packet[1] != 0x40) || (packet[2] != 0x1f) || (packet[3] != 0x10))
				break;
			int nameoffset = 6;
			if (memcmp(packet+0x15, "ENIGMA", 6))
			{
				if (!memcmp(packet+0x15, "NEUTRINONG", 10))
					nameoffset = 10;
				else
					break;
			}
			__u8 *descriptor=packet+0x13;
			int len=descriptor[1];
			len-=nameoffset;
			descriptor+=2+nameoffset;
			for (int i=0; i<len; i+=descriptor[i+1]+2)
			{
				int tag=descriptor[i];
				if  (tag == eServiceDVB::cVPID)
				{
						pid= ((descriptor[i+2]<<8)|(descriptor[i+3]));
						break;
				}
			}
		} while (0);
		close(fd);
	}	

	int fd_begin=::open(basefilename.c_str(), O_RDONLY|O_LARGEFILE);
	if (fd_begin >= 0)
	{

		// Anfangszeit setzen!
		char tmp[65424];
		int i=0;

		pktptr= 0;
		needNextPacket = 0;
		while ( i < 10 && !MovieBeginTime)
		{
			int rd = ::read(fd_begin, tmp, 65424);
			parseData(tmp,rd,1);
			i++;
		}
		close(fd_begin);
		if (MovieBeginTime)
		{
			RefreshEndTime();
		}
	}
	pktptr= 0;
	needNextPacket = 0;
}
void eTimeStampParserTS::RefreshEndTime()
{
	struct stat64 s;
	int slice = 0;
	eString tfilename;
	filelength = 0;
	while (!stat64((basefilename + (slice ? eString().sprintf(".%03d", slice) : eString(""))).c_str(), &s))
	{
		filelength += s.st_size;
		tfilename = basefilename + (slice ? eString().sprintf(".%03d", slice) : eString(""));
		slice++;
	}
eDebug("RefreshEndTime:%s,%s",basefilename.c_str(), tfilename.c_str());
	int fd_end=::open(tfilename.c_str(), O_RDONLY|O_LARGEFILE);
	if (fd_end>= 0)
	{
		// Endzeit setzen!
		pktptr= 0;
		needNextPacket = 0;
		off64_t posbegin=::lseek64(fd_end,0, SEEK_END);
		::lseek64(fd_end, posbegin - (off64_t)654240, SEEK_SET);
		char p[65424];
		int rd =1;
		while ( rd > 0 )
		{
			rd = ::read(fd_end, p, 65424);
			parseData(p,rd,2);
		}
		close(fd_end);
	}
	pktptr= 0;
	needNextPacket = 0;
}
int eTimeStampParserTS::processPacket(const unsigned char *pkt, int type)
{
	if (!wantPacket(pkt))
		printf("ne irgendwas ist da falsch\n");

	const unsigned char *end = pkt + 188;
	
	if (!(pkt[3] & 0x10))
		return 0;
	
	if (pkt[3] & 0x20)
		pkt += pkt[4] + 4 + 1;
	else
		pkt += 4;

	if (pkt > end)
		return 0;
	
	if (pkt[0] || pkt[1] || (pkt[2] != 1))
		return 0;
	
	Timestamp pts = 0;
	int ptsvalid = 0;
	
	if (pkt[7] & 0x80) 
	{
		pts  = ((unsigned long long)(pkt[ 9]&0xE))  << 29;
		pts |= ((unsigned long long)(pkt[10]&0xFF)) << 22;
		pts |= ((unsigned long long)(pkt[11]&0xFE)) << 14;
		pts |= ((unsigned long long)(pkt[12]&0xFF)) << 7;
		pts |= ((unsigned long long)(pkt[13]&0xFE)) >> 1;
		ptsvalid = 1;


		int sec = pts / 90000;
		int min = sec / 60;
		sec %= 60;
		int hr = min / 60;
		min %= 60;
		
		tm tempTime;
		tempTime.tm_hour = hr;
  		tempTime.tm_min = min;
  		tempTime.tm_sec = sec;

		tempTime.tm_year = 70;
  		tempTime.tm_mon = 0;
		tempTime.tm_mday = 1;

		switch(type)
		{
			case 0:
			{
				movie_current = tempTime;
				MovieCurrentTime = 1;
				break;
			}
			case 1:
			{
				movie_begin = tempTime;
				MovieBeginTime = 1;
				break;
			}
			case 2:
			{
				movie_end = tempTime;
				MovieEndTime = 1;
				break;
			}
			default:
				break;
		}

		if ( MovieCurrentTime && MovieBeginTime)
		{
			tm tt = movie_current;
			if (tt.tm_hour < movie_begin.tm_hour) // Korrektur
				tt.tm_mday = 2;
			time_t t1 = mktime(&movie_begin);
			time_t t2 = mktime(&tt);
			sec_currentpos = t2 - t1; 
		}
		if (MovieBeginTime && MovieEndTime && !MovieDuration )
		{
			MovieDuration = 1;
			tm tt = movie_end;
			if (movie_begin.tm_hour > tt.tm_hour) // Korrektur
				tt.tm_mday = 2;
			time_t t1 = mktime(&tt);
			time_t t2 = mktime(&movie_begin);
			sec_duration = t1 - t2; 
		}
	}
	return 0;
}

inline int eTimeStampParserTS::wantPacket(const unsigned char *hdr) const
{
	if (hdr[0] != 0x47)
		return 0;

	int ppid = ((hdr[1]&0x1F) << 8) | hdr[2];

	if (ppid != pid)
		return 0;

	if (needNextPacket)
		return 1;
	
	if (hdr[1] & 0x40)
		return 1;

	return 0;
}

void eTimeStampParserTS::parseData(const void *data, unsigned int len, int type)
{
	if (!type)
	{
		pktptr= 0;
		needNextPacket = 0;
	}
	const unsigned char *packet = (const unsigned char*)data;
	while (len)
	{
		int skipped = 0;
		while (!pktptr && len)
		{
			if (packet[0] == 0x47)
				break;
			len--;
			packet++;
			skipped++;
		}

		if (!len)
			break;
		
		if (pktptr)
		{
			if (pktptr < 0)
			{
				unsigned int skiplen = -pktptr;
				if (skiplen > len)
					skiplen = len;
				packet += skiplen;
				len -= skiplen;
				pktptr += skiplen;
				continue;
			} else if (pktptr < 4)
			{
				unsigned int storelen = 4 - pktptr;
				if (storelen > len)
					storelen = len;
				memcpy(pkt + pktptr, packet,  storelen);
				
				pktptr += storelen;
				len -= storelen;
				packet += storelen;
				
				if (pktptr == 4)
					if (!wantPacket(pkt))
					{
						packet += 184;
						len -= 184;
						pktptr = 0;
						continue;
					}
			}
			unsigned int storelen = 188 - pktptr;
			if (storelen > len)
				storelen = len;
			memcpy(pkt + pktptr, packet,  storelen);
			pktptr += storelen;
			len -= storelen;
			packet += storelen;
			
			if (pktptr == 188)
			{
				needNextPacket = processPacket(pkt,type);
				pktptr = 0;
			}
		} else if (len >= 4)
		{
			if (wantPacket(packet))
			{
				if (len >= 188)
				{
					needNextPacket = processPacket(packet,type);
				} else
				{
					memcpy(pkt, packet, len);
					pktptr = len;
				}
			}
			int sk = len;
			if (sk >= 188)
				sk = 188;
			else if (!pktptr)
				pktptr = sk - 188;

			len -= sk;
			packet += sk;
		} else
		{
			memcpy(pkt, packet, len);
			pktptr = len;
			packet += len;
			len = 0;
		}
	}
}
