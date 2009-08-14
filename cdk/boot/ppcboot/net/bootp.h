/*
 *	Copied from LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Paolo Scaffardi
 */

#ifndef __BOOTP_H__
#define __BOOTP_H__

#ifndef __NET_H__
#include	"net.h"
#endif /* __NET_H__ */

/**********************************************************************/

/*
 *	BOOTP header.
 */
typedef struct
{
	uchar		bp_op;		/* Operation				*/
# define OP_BOOTREQUEST	1
# define OP_BOOTREPLY	2
	uchar		bp_htype;	/* Hardware type			*/
# define HWT_ETHER	1
	uchar		bp_hlen;	/* Hardware address length		*/
# define HWL_ETHER	6
	uchar		bp_hops;	/* Hop count (gateway thing)		*/
	ulong		bp_id;		/* Transaction ID			*/
	ushort		bp_secs;	/* Seconds since boot			*/
	ushort		bp_spare1;	/* Alignment				*/
	IPaddr_t	bp_ciaddr;	/* Client IP address			*/
	IPaddr_t	bp_yiaddr;	/* Your (client) IP address		*/
	IPaddr_t	bp_siaddr;	/* Server IP address			*/
	IPaddr_t	bp_giaddr;	/* Gateway IP address			*/
	uchar		bp_chaddr[16];	/* Client hardware address		*/
	char		bp_sname[64];	/* Server host name			*/
	char		bp_file[128];	/* Boot file name			*/
	char		bp_vend[64];	/* Vendor information			*/
}	Bootp_t;

#define BOOTP_HDR_SIZE	sizeof (Bootp_t)
#define BOOTP_SIZE	(ETHER_HDR_SIZE + IP_HDR_SIZE + BOOTP_HDR_SIZE)

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* bootp.c */
extern ulong	BootpID;		/* ID of cur BOOTP request		*/
extern char	BootFile[128];		/* Boot file name			*/
extern int	BootpTry;

/* Send a BOOTP request */
extern void	BootpRequest(char *fileName, ulong loadAdr);

/**********************************************************************/

#endif /* __BOOTP_H__ */
