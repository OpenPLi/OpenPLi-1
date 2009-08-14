/*
 *	LiMon Monitor (LiMon) - Network.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *
 *
 * History
 *	9/16/00	  bor  adapted to TQM823L/STK8xxL board, RARP/TFTP boot added
 */

#ifndef __NET_H__
#define __NET_H__


/*
 *	The number of receive packet buffers, and the required packet buffer
 *	alignment in memory.
 *
 */

#define PKTBUFSRX	4
#define PKTALIGN	32

/****** from cpu_arch.h ************/

/* Byte swapping stuff (not needed on PPC). */

#define SWAP16(x)	(x)
#define SWAP16c(x)	(x)
#define SWAP32(x)	(x)

/****** end from cpu_arch.h **************/

typedef ulong		IPaddr_t;



/*
 * The current receive packet handler.  Called with a pointer to the
 * application packet, and a protocol type (PORT_BOOTPC or PORT_TFTP).
 * All other packets are dealt with without calling the handler.
 */
typedef void	rxhand_f(uchar *, unsigned, unsigned, unsigned);

/*
 *	A timeout handler.  Called after time interval has expired.
 */
typedef void	thand_f(void);



extern int eth_init(bd_t *bis);			/* Initialize the device	*/
extern int eth_send(volatile void *packet, int length);	   /* Send a packet	*/
extern int eth_rx(void);			/* Check for received packets	*/
extern void eth_halt(void);			/* stop SCC			*/




/**********************************************************************/
/*
 *	Protocol headers.
 */

/*
 *	Ethernet header
 */
typedef struct {
	uchar		et_dest[6];	/* Destination node		*/
	uchar		et_src[6];	/* Source node			*/
	ushort		et_protlen;	/* Protocol or length		*/
	uchar		et_dsap;	/* 802 DSAP			*/
	uchar		et_ssap;	/* 802 SSAP			*/
	uchar		et_ctl;		/* 802 control			*/
	uchar		et_snap1;	/* SNAP				*/
	uchar		et_snap2;
	uchar		et_snap3;
	ushort		et_prot;	/* 802 protocol			*/
} Ethernet_t;

#define ETHER_HDR_SIZE	14		/* Ethernet header size		*/
#define E802_HDR_SIZE	22		/* 802 ethernet header size	*/
#define PROT_IP		0x0800		/* IP protocol			*/
#define PROT_ARP	0x0806		/* IP ARP protocol		*/
#define PROT_RARP	0x8035		/* IP ARP protocol		*/

/*
 *	Internet Protocol (IP) header.
 */
typedef struct {
	uchar		ip_hl_v;	/* header length and version	*/
	uchar		ip_tos;		/* type of service		*/
	ushort		ip_len;		/* total length			*/
	ushort		ip_id;		/* identification		*/
	ushort		ip_off;		/* fragment offset field	*/
	uchar		ip_ttl;		/* time to live			*/
	uchar		ip_p;		/* protocol			*/
	ushort		ip_sum;		/* checksum			*/
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
	ushort		udp_src;	/* UDP source port		*/
	ushort		udp_dst;	/* UDP destination port		*/
	ushort		udp_len;	/* Length of UDP packet		*/
	ushort		udp_xsum;	/* Checksum			*/
} IP_t;

#define IP_HDR_SIZE_NO_UDP	(sizeof (IP_t) - 8)
#define IP_HDR_SIZE		(sizeof (IP_t))


/*
 *	Address Resolution Protocol (ARP) header.
 */
typedef struct
{
	ushort		ar_hrd;		/* Format of hardware address	*/
#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/
	ushort		ar_pro;		/* Format of protocol address	*/
	uchar		ar_hln;		/* Length of hardware address	*/
	uchar		ar_pln;		/* Length of protocol address	*/
	ushort		ar_op;		/* Operation			*/
#   define ARPOP_REQUEST    1		/* Request  to resolve  address	*/
#   define ARPOP_REPLY	    2		/* Response to previous request	*/

#   define RARPOP_REQUEST   3		/* Request  to resolve  address	*/
#   define RARPOP_REPLY	    4		/* Response to previous request */

	/*
         * The remaining fields are variable in size, according to
         * the sizes above, and are defined as appropriate for
         * specific hardware/protocol combinations.
	 */
	uchar		ar_data[0];
#if 0
	uchar		ar_sha[];	/* Sender hardware address	*/
	uchar		ar_spa[];	/* Sender protocol address	*/
	uchar		ar_tha[];	/* Target hardware address	*/
	uchar		ar_tpa[];	/* Target protocol address	*/
#endif /* 0 */
} ARP_t;

#define ARP_HDR_SIZE	(8+20)		/* Size assuming ethernet	*/


/*
 * Maximum packet size; used to allocate packet storage.
 * TFTP packets can be 524 bytes + IP header + ethernet header.
 * Lets be conservative, and go for 38 * 16.  (Must also be
 * a multiple of 32 bytes).
 */
/* #define PKTSIZE		1536 */
#define PKTSIZE		608

/*
 * Maximum receive ring size; that is, the number of packets
 * we can buffer before overflow happens. Basically, this just
 * needs to be enough to prevent a packet being discarded while
 * we are processing the previous one.
 */
#define RINGSZ		4
#define RINGSZ_LOG2	2

/**********************************************************************/
/*
 *	Globals.
 */

/* net.c */
/** BOOTP EXTENTIONS **/
extern IPaddr_t		NetOurGatewayIP;	/* Our gateway IP addresse	*/
extern IPaddr_t		NetOurSubnetMask;	/* Our subnet mask (0 = unknown)*/
extern IPaddr_t		NetOurDNSIP;	 /* Our Domain Name Server (0 = unknown)*/
extern char		NetOurNISDomain[32];	/* Our NIS domain		*/
extern char		NetOurHostName[32];	/* Our hostname			*/
extern char		NetOurRootPath[32];	/* Our root path		*/
extern char		NetOurBootPath[128];	/* directory from bootfile one up (/test/tftfpboot/ppcboot -> /test */
extern ushort		NetBootFileSize;	/* Our boot file size in blocks	*/
/** END OF BOOTP EXTENTIONS **/
extern uchar		NetOurEther[6];		/* Our ethernet address		*/
extern uchar		NetServerEther[6];	/* Boot server enet address	*/
extern IPaddr_t		NetOurIP;		/* Our    IP addr (0 = unknown)	*/
extern IPaddr_t		NetServerIP;		/* Server IP addr (0 = unknown)	*/
extern volatile uchar * NetTxPacket;		/* THE transmit packet		*/
extern volatile uchar * NetRxPackets[PKTBUFSRX];/* Receive packets		*/
extern volatile uchar * NetRxPkt;		/* Current receive packet	*/
extern int		NetRxPktLen;		/* Current rx packet length	*/
extern unsigned		NetIPID;		/* IP ID (counting)		*/
extern uchar		NetBcastAddr[6];	/* Ethernet boardcast address	*/

extern int		NetState;		/* Network loop state		*/
#define NETLOOP_CONTINUE	1
#define NETLOOP_RESTART		2
#define NETLOOP_SUCCESS		3
#define NETLOOP_FAIL		4


typedef enum { BOOTP, TFTP, RARP } proto_t;

/* Initialize the network adapter */
extern int	NetLoop(bd_t *bis, proto_t protocol, char *fileName, ulong loadAdr);

/* Shutdown adapters and cleanup */
extern void	NetStop(void);

/* Load failed.	 Start again. */
extern void	NetStartAgain(void);

/* Copy ethernet address */
extern void	NetCopyEther(volatile uchar *, uchar *);

/* Set ethernet header */
extern void	NetSetEther(volatile uchar *, uchar *, uint);

/* Set IP header */
extern void	NetSetIP(volatile uchar *, IPaddr_t, int, int, int);

/* Checksum */
extern int	NetCksumOk(uchar *, int);	/* Return true if cksum OK	*/
extern uint	NetCksum(uchar *, int);		/* Calculate the checksum	*/

/* Set callbacks */
extern void	NetSetHandler(rxhand_f *);	/* Set RX packet handler	*/
extern void	NetSetTimeout(int, thand_f *);	/* Set timeout handler		*/

/* Transmit "NetTxPacket" */
extern void	NetSendPacket(volatile uchar *, int);

/* Processes a received packet */
extern void	NetReceive(volatile uchar *, int);

/* Print an IP address on the console */
extern void	NetPrintIPaddr(IPaddr_t);

/* Convert a IP address to a string */
extern void	NetIPaddr (IPaddr_t x, char *s);

/**********************************************************************/

#endif /* __NET_H__ */
