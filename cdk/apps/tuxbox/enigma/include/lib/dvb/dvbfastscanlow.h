#ifndef __src_lib_dvb_dvbfastscanlow_h
#define __src_lib_dvb_dvbfastscanlow_h

//
// hack to get macros we like to use.
// probably could be done using other includes...
// somethign for a next revision of the fastscan code
//

#include <set>
#include <byteswap.h>
#include <endian.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define r16(p)		(*(const uint16_t * const)(p))
#define r32(p)		(*(const uint32_t * const)(p))
#define r64(p)		(*(const uint64_t * const)(p))
#define w16(p,v)	do { *(uint16_t * const)(p) = ((const uint16_t)v) } while (0)
#define w32(p,v)	do { *(uint32_t * const)(p) = ((const uint32_t)v) } while (0)
#define w64(p,v)	do { *(uint64_t * const)(p) = ((const uint64_t)v) } while (0)
#else
#define r16(p)		bswap_16(*(const uint16_t * const)p)
#define r32(p)		bswap_32(*(const uint32_t * const)p)
#define r64(p)		bswap_64(*(const uint64_t * const)p)
#define w16(p,v)	do { *(uint16_t * const)(p) = bswap_16((const uint16_t)v) } while (0)
#define w32(p,v)	do { *(uint32_t * const)(p) = bswap_32((const uint32_t)v) } while (0)
#define w64(p,v)	do { *(uint64_t * const)(p) = bswap_64((const uint64_t)v) } while (0)
#endif

#define DVB_LENGTH(p)	(r16(p) & 0x0fff)
#define DVB_PID(p)	(r16(p) & 0x1fff)


typedef struct {
	u_char table_id				: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char section_syntax_indicator		: 1;
	u_char					: 3;
	u_char section_length_hi		: 4;
#else
	u_char section_length_hi		: 4;
	u_char					: 3;
	u_char section_syntax_indicator	:	1;
#endif

	u_char section_length_lo		: 8;

	u_char operator_network_id_hi		: 8;
	u_char operator_network_id_lo		: 8;

#if BYTE_ORDER == BIG_ENDIAN
	u_char					: 2;
	u_char version_number			: 5;
	u_char current_next_indicator		: 1;
#else
	u_char current_next_indicator		: 1;
	u_char version_number			: 5;
	u_char					: 2;
#endif

	u_char section_number			: 8;
	u_char last_section_number		: 8;
} FST_t;
#endif
