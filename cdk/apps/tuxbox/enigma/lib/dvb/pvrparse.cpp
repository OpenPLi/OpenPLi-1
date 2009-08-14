#include <lib/dvb/pvrparse.h>
#include <lib/base/eerror.h>

#ifndef BYTE_ORDER
#error no byte order defined!
#endif

int eMPEGStreamInformation::save(const char *filename)
{
	FILE *f = fopen(filename, "wb");
	if (!f)
		return -1;
	
	for (std::map<off_t, Timestamp>::const_iterator i(accessPoints.begin()); i != accessPoints.end(); ++i)
	{
		unsigned long long d[2];
#if BYTE_ORDER == BIG_ENDIAN
		d[0] = i->first;
		d[1] = i->second;
#else
		d[0] = bswap64(i->first);
		d[1] = bswap64(i->second);
#endif
		fwrite(d, sizeof(d), 1, f);
	}
	fclose(f);
	
	return 0;
}

int eMPEGStreamInformation::load(const char *filename)
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return -1;
	accessPoints.clear();
	while (1)
	{
		unsigned long long d[2];
		if (fread(d, sizeof(d), 1, f) < 1)
			break;
		
#if BYTE_ORDER == LITTLE_ENDIAN
		d[0] = bswap64(d[0]);
		d[1] = bswap64(d[1]);
#endif
		accessPoints[d[0]] = d[1];
	}
	fclose(f);
	fixupDiscontinuties();
	return 0;
}

eMPEGStreamInformation::operator bool()
{
	return !!accessPoints.size();
}

void eMPEGStreamInformation::fixupDiscontinuties()
{
	timestampDeltas.clear();
	if (!accessPoints.size())
		return;
		
	eDebug("Fixing discontinuities ...");
	Timestamp currentDelta = accessPoints.begin()->second, lastTimestamp = 0;
	timestampDeltas[accessPoints.begin()->first] = accessPoints.begin()->second;
	for (std::map<off_t,Timestamp>::const_iterator i(accessPoints.begin()); i != accessPoints.end(); ++i)
	{
		Timestamp current = i->second - currentDelta;
		Timestamp diff = current - lastTimestamp;
		if (diff > (90000*5)) // 5sec diff
		{
			eDebug("%llx < %llx, have discont. new timestamp is %llx (diff is %llx)!", current, lastTimestamp, i->second, diff);
			currentDelta = i->second - lastTimestamp;
			eDebug("current delta now %llx, making current to %llx", currentDelta, i->second - currentDelta);
			timestampDeltas[i->first] = currentDelta;
		}
		lastTimestamp = i->second - currentDelta;
	}
	eDebug("ok, found %d disconts.", timestampDeltas.size());
}

eMPEGStreamInformation::Timestamp eMPEGStreamInformation::getDelta(eMPEGStreamInformation::off_t offset)
{
	if (!timestampDeltas.size())
		return 0;
	std::map<off_t,Timestamp>::iterator i = timestampDeltas.upper_bound(offset);

		/* i can be the first when you query for something before the first PTS */
	if (i != timestampDeltas.begin())
		--i;
	
	return i->second;
}

eMPEGStreamInformation::Timestamp eMPEGStreamInformation::getInterpolated(eMPEGStreamInformation::off_t offset)
{
		/* get the PTS values before and after the offset. */
	std::map<off_t,Timestamp>::iterator before, after;
	
	after = accessPoints.upper_bound(offset);
	before = after;

	eDebug("query %llx", offset);

	if (before != accessPoints.begin())
		--before;
	else	/* we query before the first known timestamp ... FIXME */
	{
		eDebug("query before");
		return 0;
	}

		/* empty... */
	if (before == accessPoints.end())
	{
		eDebug("empty");
		return 0;
	}

		/* if after == end, then we need to extrapolate ... FIXME */
	if ((before->first == offset) || (after == accessPoints.end()))
	{
		eDebug("exact match (or after end..)");
		return before->second - getDelta(offset);
	}
	
	Timestamp before_ts = before->second - getDelta(before->first);
	Timestamp after_ts = after->second - getDelta(after->first);
	
	Timestamp diff = after_ts - before_ts;
	off_t diff_off = after->first - before->first;
	eDebug("interpolating: %llx(%llx) .. %llx .. %llx(%llx)", before->first, before_ts, offset, after->first, after_ts),
	
	diff = (offset - before->first) * diff / diff_off;
	eDebug("result %llx + %llx = %llx", before_ts, diff, before_ts + diff);
	return before_ts + diff;
}
 
eMPEGStreamInformation::off_t eMPEGStreamInformation::getAccessPoint(eMPEGStreamInformation::Timestamp ts)
{
		/* FIXME: more efficient implementation */
	Timestamp delta = 0;
	off_t last = 0;
	for (std::map<off_t, Timestamp>::const_iterator i(accessPoints.begin()); i != accessPoints.end(); ++i)
	{
		std::map<off_t, Timestamp>::const_iterator d = timestampDeltas.find(i->first);
		if (d != timestampDeltas.end())
			delta = d->second;
		Timestamp c = i->second - delta;
		if (c > ts)
			break;
		last = i->first;
	}
	return last;
}

eMPEGStreamParserTS::eMPEGStreamParserTS(eMPEGStreamInformation &streaminfo): streaminfo(streaminfo), pktptr(0), pid(-1), needNextPacket(0), skip(0)
{
}

int eMPEGStreamParserTS::processPacket(const unsigned char *pkt, eMPEGStreamInformation::off_t offset)
{
	if (!wantPacket(pkt))
		printf("ne irgendwas ist da falsch\n");

	const unsigned char *end = pkt + 188;
	
	if (!(pkt[3] & 0x10))
	{
		eWarning("[TSPARSE] PUSI set but no payload.");
		return 0;
	}
	
	if (pkt[3] & 0x20) // adaption field present?
		pkt += pkt[4] + 4 + 1;  /* skip adaption field and header */
	else
		pkt += 4; /* skip header */

	if (pkt > end)
	{
		eWarning("[TSPARSE] dropping huge adaption field");
		return 0;
	}
	
		// ok, we now have the start of the payload, aligned with the PES packet start.
	if (pkt[0] || pkt[1] || (pkt[2] != 1))
	{
		eWarning("broken startcode");
		return 0;
	}
	
	
	eMPEGStreamInformation::Timestamp pts = 0;
	int ptsvalid = 0;
	
	if (pkt[7] & 0x80) // PTS present?
	{
		pts  = ((unsigned long long)(pkt[ 9]&0xE))  << 29;
		pts |= ((unsigned long long)(pkt[10]&0xFF)) << 22;
		pts |= ((unsigned long long)(pkt[11]&0xFE)) << 14;
		pts |= ((unsigned long long)(pkt[12]&0xFF)) << 7;
		pts |= ((unsigned long long)(pkt[13]&0xFE)) >> 1;
		ptsvalid = 1;

#if 0		
		int sec = pts / 90000;
		int frm = pts % 90000;
		int min = sec / 60;
		sec %= 60;
		int hr = min / 60;
		min %= 60;
		int d = hr / 24;
		hr %= 24;
		
		eDebug("pts: %016llx %d:%02d:%02d:%02d:%05d", pts, d, hr, min, sec, frm);
#endif
	}
	
		/* advance to payload */
	pkt += pkt[8] + 9;
	
		/* if startcode found */
	if (!(pkt[0] || pkt[1] || (pkt[2] != 1)))
	{
		if (pkt[3] == 0xb3) /* sequence header */
		{
			if (ptsvalid)
			{
				streaminfo.accessPoints[offset] = pts;
				eDebug("Sequence header at %llx, pts %llx", offset, pts);
			} else
				eDebug("Sequence header but no valid PTS value.");
		}
	}
	return 0;
}

inline int eMPEGStreamParserTS::wantPacket(const unsigned char *hdr) const
{
	if (hdr[0] != 0x47)
	{
		eDebug("missing sync!");
		return 0;
	}
	int ppid = ((hdr[1]&0x1F) << 8) | hdr[2];

	if (ppid != pid)
		return 0;
		
	if (needNextPacket)  /* next packet (on this pid) was required? */
		return 1;
	
	if (hdr[1] & 0x40)	 /* pusi set: yes. */
		return 1;

	return 0;
}

void eMPEGStreamParserTS::parseData(eMPEGStreamInformation::off_t offset, const void *data, unsigned int len)
{
	const unsigned char *packet = (const unsigned char*)data;
	const unsigned char *packet_start = packet;
	
			/* sorry for the redundant code here, but there are too many special cases... */
	while (len)
	{
		if (pktptr)
		{
				/* skip last packet */
			if (pktptr < 0)
			{
				unsigned int skiplen = -pktptr;
				if (skiplen > len)
					skiplen = len;
				packet += skiplen;
				len -= skiplen;
				pktptr += skiplen;
				continue;
			} else if (pktptr < 4) /* header not complete, thus we don't know if we want this packet */
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
							/* skip packet */
						packet += 184;
						len -= 184;
						pktptr = 0;
						continue;
					}
			}
				/* otherwise we complete up to the full packet */
			unsigned int storelen = 188 - pktptr;
			if (storelen > len)
				storelen = len;
			memcpy(pkt + pktptr, packet,  storelen);
			pktptr += storelen;
			len -= storelen;
			packet += storelen;
			
			if (pktptr == 188)
			{
				needNextPacket = processPacket(pkt, offset + (packet - packet_start));
				pktptr = 0;
			}
		} else if (len >= 4)  /* if we have a full header... */
		{
			if (wantPacket(packet))  /* decide wheter we need it ... */
			{
				if (len >= 188)          /* packet complete? */
				{
					needNextPacket = processPacket(packet, offset + (packet - packet_start)); /* process it now. */
				} else
				{
					memcpy(pkt, packet, len);  /* otherwise queue it up */
					pktptr = len;
				}
			}

				/* skip packet */
			int sk = len;
			if (sk >= 188)
				sk = 188;
			else if (!pktptr) /* we dont want this packet, otherwise pktptr = sk (=len) > 4 */
				pktptr = sk - 188;

			len -= sk;
			packet += sk;
		} else             /* if we don't have a complete header */
		{
			memcpy(pkt, packet, len);   /* complete header next time */
			pktptr = len;
			packet += len;
			len = 0;
		}
	}
}

void eMPEGStreamParserTS::setPid(int _pid)
{
	pktptr = 0;
	pid = _pid;
}
