/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *
 * History
 *	9/16/00	  bor  adapted to TQM823L/STK8xxL board, RARP/TFTP boot added
 */

#include <ppcboot.h>
#include <command.h>
#include "net.h"
#include "bootp.h"
#include "tftp.h"
#include "rarp.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET)

/** BOOTP EXTENTIONS **/

IPaddr_t	NetOurSubnetMask=0;		/* Our subnet mask (0=unknown)	*/
IPaddr_t	NetOurGatewayIP=0;		/* Our gateways IP address	*/
IPaddr_t	NetOurDNSIP=0;			/* Our DNS IP address		*/
char		NetOurNISDomain[32]={0,};	/* Our NIS domain		*/
char		NetOurHostName[32]={0,};	/* Our hostname			*/
char		NetOurRootPath[32]={0,};	/* Our bootpath			*/
char		NetOurBootPath[128]={0,};	/* Our bootpath			*/
ushort		NetBootFileSize=0;		/* Out bootfile size in blocks	*/

/** END OF BOOTP EXTENTIONS **/

uchar		NetOurEther[6];		/* Our ethernet address			*/
uchar		NetServerEther[6];	/* Boot server enet address		*/
IPaddr_t	NetOurIP;		/* Our IP addr (0 = unknown)		*/
IPaddr_t	NetServerIP;		/* Our IP addr (0 = unknown)		*/
volatile uchar *NetRxPkt;		/* Current receive packet		*/
int		NetRxPktLen;		/* Current rx packet length		*/
unsigned	NetIPID;		/* IP packet ID				*/
uchar		NetBcastAddr[6] =	/* Ethernet bcast address		*/
			{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
int		NetState;		/* Network loop state			*/

volatile uchar	PktBuf[(PKTBUFSRX+1) * PKTSIZE + PKTALIGN];

volatile uchar *NetRxPackets[PKTBUFSRX]; /* Receive packets			*/

static rxhand_f *packetHandler;		/* Current RX packet handler		*/
static thand_f *timeHandler;		/* Current timeout handler		*/
static ulong	timeValue;		/* Current timeout value		*/
volatile uchar *NetTxPacket = 0;	/* THE transmit packet			*/


/**********************************************************************/
/*
 *	Main network processing loop.
 */

int
NetLoop(bd_t *bis, proto_t protocol, char *fileName, ulong loadAdr)
{
    char	*s, *e;
    ulong	reg;

	if (!NetTxPacket) {
		int	i;

		/*
		 *	Setup packet buffers, aligned correctly.
		 */
		NetTxPacket = &PktBuf[0] + (PKTALIGN - 1);
		NetTxPacket -= (ulong)NetTxPacket % PKTALIGN;
		for (i = 0; i < PKTBUFSRX; i++) {
			NetRxPackets[i] = NetTxPacket + (i+1)*PKTSIZE;
		}
	}

	eth_halt();
	eth_init(bis);

	NetCopyEther(NetOurEther, bis->bi_enetaddr);

	/* initialize our IP adr to 0 in order to accept ANY IP addr
	   assigned to us by the BOOTP server
	*/

	NetOurIP = 0;
	BootpTry = 0;
	RarpTry	 = 0;

restart:
	NetState = NETLOOP_CONTINUE;

	/*
	 *	Start the ball rolling with the given start function.  From
	 *	here on, this code is a state machine driven by received
	 *	packets and timer events.
	 */
	switch (protocol) {

	case TFTP:
		NetCopyEther(NetServerEther, NetBcastAddr);
		strcpy(BootFile, fileName);
		NetOurIP = bis->bi_ip_addr;
		NetServerIP = 0;
		s = getenv ("serverip");
		for (reg=0; reg<4; ++reg) {
			ulong val = s ? simple_strtoul(s, &e, 10) : 0;
			NetServerIP <<= 8;
			NetServerIP |= (val & 0xFF);
			if (s) s = (*e) ? e+1 : e;
		}
		if (NetOurIP & NetServerIP) {
			TftpStart(loadAdr);
		} else {
			printf ("\n"
				"Environment variables `ipaddr' and `serverip'"
				" needed for this command\n"
				);
			return 0;
		}
		break;

	case RARP:
		RarpRequest(fileName, loadAdr);
		break;

	default:
		BootpRequest(fileName, loadAdr);
		break;
	}


	/*
	 *	Main packet reception loop.  Loop receiving packets until
	 *	someone sets `NetQuit'.
	 */
	for (;;) {
		/*
		 *	Check the ethernet for a new packet.  The ethernet
		 *	receive routine will process it.
		 */
			eth_rx();

		/*
		 *	Check the keyboard for a Key.  Quit if we get one.
		 */
		if (tstc()) {
			printf("\nAbort\n");
			return 0;
		}


		/*
		 *	Check for a timeout, and run the timeout handler
		 *	if we have one.
		 */
		if (timeHandler && (get_timer(0) > timeValue)) {
			thand_f *x;

			x = timeHandler;
			timeHandler = (thand_f *)0;
			(*x)();
		}


		switch (NetState) {

		case NETLOOP_RESTART:
			goto restart;

		case NETLOOP_SUCCESS:
			eth_halt();
			return 1;

		case NETLOOP_FAIL:
			return 0;
		}
	}
}

/**********************************************************************/

static void
startAgainTimeout(void)
{
	NetState = NETLOOP_RESTART;
}


static void
startAgainHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	/* Totally ignore the packet */
}


void
NetStartAgain(void)
{
	NetServerIP = 0;
	NetOurIP = 0;
	NetSetTimeout(10 * CFG_HZ, startAgainTimeout);
	NetSetHandler(startAgainHandler);
}

/**********************************************************************/
/*
 *	Miscelaneous bits.
 */

void
NetSetHandler(rxhand_f * f)
{
	packetHandler = f;
}


void
NetSetTimeout(int iv, thand_f * f)
{
	if (iv == 0) {
		timeHandler = (thand_f *)0;
	} else {
		timeHandler = f;
		timeValue = get_timer(0) + iv;
	}
}


void
NetSendPacket(volatile uchar * pkt, int len)
{
	(void) eth_send(pkt, len);
}



void
NetReceive(volatile uchar * pkt, int len)
{
	Ethernet_t *et;
	IP_t	*ip;
	ARP_t	*arp;
	int	x;


	NetRxPkt = pkt;
	NetRxPktLen = len;
	et = (Ethernet_t *)pkt;

	x = SWAP16(et->et_protlen);

	if (x < 1514) {
		/*
		 *	Got a 802 packet.  Check the other protocol field.
		 */
		x = SWAP16(et->et_prot);
		ip = (IP_t *)(pkt + E802_HDR_SIZE);
		len -= E802_HDR_SIZE;
	} else {
		ip = (IP_t *)(pkt + ETHER_HDR_SIZE);
		len -= ETHER_HDR_SIZE;
	}

#ifdef ET_DEBUG
	printf("Receive from protocol 0x%x\n", x);
#endif

	switch (x) {

	case PROT_ARP:
		/*
		 *	The only type of ARP packet we deal with is a request
		 *	for our ethernet address.  We can only respond if we
		 *	know our IP address.
		 */
#ifdef ET_DEBUG
		printf("Got ARP\n");
#endif
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}
		if (SWAP16(arp->ar_op) != ARPOP_REQUEST)
			return;
		if (SWAP16(arp->ar_hrd) != ARP_ETHER)
			return;
		if (SWAP16(arp->ar_pro) != PROT_IP)
			return;
		if (arp->ar_hln != 6)
			return;
		if (arp->ar_pln != 4)
			return;

		if (NetOurIP == 0 ||
		    *((IPaddr_t *)&arp->ar_data[16]) != NetOurIP) {
			return;
		}

		NetSetEther((uchar *)et, et->et_src, PROT_ARP);
		arp->ar_op = SWAP16(ARPOP_REPLY);
		NetCopyEther(&arp->ar_data[10], &arp->ar_data[0]);
		NetCopyEther(&arp->ar_data[0], NetOurEther);
		*(IPaddr_t *)(&arp->ar_data[16]) =
					*(IPaddr_t *)(&arp->ar_data[6]);
		*(IPaddr_t *)(&arp->ar_data[6]) = NetOurIP;
		NetSendPacket((uchar *)et, ((uchar *)arp - pkt) + ARP_HDR_SIZE);
		break;

	case PROT_RARP:
#ifdef ET_DEBUG
		printf("Got RARP\n");
#endif
		arp = (ARP_t *)ip;
		if (len < ARP_HDR_SIZE) {
			printf("bad length %d < %d\n", len, ARP_HDR_SIZE);
			return;
		}

		if ((SWAP16(arp->ar_op) != RARPOP_REPLY) ||
			(SWAP16(arp->ar_hrd) != ARP_ETHER)   ||
			(SWAP16(arp->ar_pro) != PROT_IP)     ||
			(arp->ar_hln != 6) || (arp->ar_pln != 4)) {

			printf("invalid RARP header\n");
		} else {
			NetOurIP = *((IPaddr_t *)&arp->ar_data[16]);
			NetServerIP = *((IPaddr_t *)&arp->ar_data[6]);
			NetCopyEther(NetServerEther, &arp->ar_data[0]);

			(*packetHandler)(0,0,0,0);
		}
		break;

	case PROT_IP:
#ifdef ET_DEBUG
		printf("Got IP\n");
#endif
		if (len < IP_HDR_SIZE)
			return;
		if (len < SWAP16(ip->ip_len)) {
			printf("len bad %d < %d\n", len, SWAP16(ip->ip_len));
			return;
		}
		len = SWAP16(ip->ip_len);
#ifdef ET_DEBUG
		printf("len=%d, v=%02x\n", len, ip->ip_hl_v & 0xff);
#endif
		if ((ip->ip_hl_v & 0xf0) != 0x40)
			return;
		if (ip->ip_off & SWAP16c(0x1fff)) /* Can't deal w/ fragments */
			return;
		if (!NetCksumOk((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2)) {
			printf("checksum bad\n");
			return;
		}
		if (NetOurIP && ip->ip_dst != NetOurIP)
			return;
		if (ip->ip_p != 17)		/* Only UDP packets */
			return;


                /*
                 * At this point we still use the ET broadcast
                 * address; copy the server ET adress to the req.
                 * location. This happens when issuing a TFTP request
                 * with the known server IP address and the ET
                 * broadcast address as destination.
		 */
		if (memcmp(NetServerEther, NetBcastAddr, 6) == 0)
		NetCopyEther(NetServerEther, et->et_src);

		/*
		 *	IP header OK.  Pass the packet to the current handler.
		 */
		(*packetHandler)((uchar *)ip +IP_HDR_SIZE,
						SWAP16(ip->udp_dst),
						SWAP16(ip->udp_src),
						SWAP16(ip->udp_len) - 8);

		break;
	}
}


/**********************************************************************/

int
NetCksumOk(uchar * ptr, int len)
{
	return !((NetCksum(ptr, len) + 1) & 0xfffe);
}


unsigned
NetCksum(uchar * ptr, int len)
{
	ulong	xsum;

	xsum = 0;
	while (len-- > 0)
		xsum += *((ushort *)ptr)++;
	xsum = (xsum & 0xffff) + (xsum >> 16);
	xsum = (xsum & 0xffff) + (xsum >> 16);
	return (xsum & 0xffff);
}


void
NetCopyEther(volatile uchar * to, uchar * from)
{
	int	i;

	for (i = 0; i < 6; i++)
		*to++ = *from++;
}


void
NetSetEther(volatile uchar * xet, uchar * addr, uint prot)
{
	volatile Ethernet_t *et = (Ethernet_t *)xet;

	NetCopyEther(et->et_dest, addr);
	NetCopyEther(et->et_src, NetOurEther);
	et->et_protlen = SWAP16(prot);
}


void
NetSetIP(volatile uchar * xip, IPaddr_t dest, int dport, int sport, int len)
{
	volatile IP_t *ip = (IP_t *)xip;

	/*
	 *	If the data is an odd number of bytes, zero the
	 *	byte after the last byte so that the checksum
	 *	will work.
	 */
	if (len & 1)
		xip[IP_HDR_SIZE + len] = 0;

	/*
	 *	Construct an IP and UDP header.
			(need to set no fragment bit - XXX)
	 */
	ip->ip_hl_v  = 0x45;		/* IP_HDR_SIZE / 4 (not including UDP) */
	ip->ip_tos   = 0;
	ip->ip_len   = SWAP16(IP_HDR_SIZE + len);
	ip->ip_id    = SWAP16(NetIPID++);
	ip->ip_off   = SWAP16c(0x4000);	/* No fragmentation */
	ip->ip_ttl   = 255;
	ip->ip_p     = 17;		/* UDP */
	ip->ip_sum   = 0;
	ip->ip_src   = NetOurIP;
	ip->ip_dst   = dest;
	ip->udp_src  = SWAP16(sport);
	ip->udp_dst  = SWAP16(dport);
	ip->udp_len  = SWAP16(8 + len);
	ip->udp_xsum = 0;
	ip->ip_sum   = ~NetCksum((uchar *)ip, IP_HDR_SIZE_NO_UDP / 2);
}


void NetIPaddr (IPaddr_t x, char *s)
{
    x = SWAP32(x);
    sprintf (s,"%d.%d.%d.%d",
    	(int)((x >> 24) & 0xff),
	(int)((x >> 16) & 0xff),
	(int)((x >>  8) & 0xff),
	(int)((x >>  0) & 0xff)
    );
}

void NetPrintIPaddr(IPaddr_t x)
{
    char tmp[12];

    NetIPaddr(x, tmp);

    puts(tmp);
}

#endif /* CFG_CMD_NET */
