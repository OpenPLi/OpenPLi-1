/*
 *	LiMon - TFTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 */

#include <ppcboot.h>
#include <command.h>
#include "net.h"
#include "tftp.h"
#include "bootp.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define WELL_KNOWN_PORT	69		/* Well known TFTP port #		*/
#define TIMEOUT		2		/* Seconds to timeout for a lost pkt	*/
#define TIMEOUT_COUNT	10		/* # of timeouts before giving up	*/
					/* (for checking the image size)	*/
#define NDOTS		65		/* Number of "loading" dots		*/

/*
 *	TFTP operations.
 */
#define TFTP_RRQ	1
#define TFTP_WRQ	2
#define TFTP_DATA	3
#define TFTP_ACK	4
#define TFTP_ERROR	5



static int	TftpServerPort;		/* The UDP port at their end		*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static unsigned	TftpBlock;
static unsigned	TftpLastBlock;
static int	TftpState;
#define STATE_RRQ	1
#define STATE_DATA	2
#define STATE_TOO_LARGE	3
#define STATE_BAD_MAGIC	4

static __inline__ void
store_block (unsigned block, uchar * src, unsigned len)
{
	(void)memcpy((void *)(load_addr + block * 512), src, len);
}

static void TftpTimeout (void);

/**********************************************************************/

static void
TftpSend (void)
{
	volatile uchar *	pkt;
	volatile uchar *	xp;
	int			len;

	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + ETHER_HDR_SIZE + IP_HDR_SIZE;

	switch (TftpState) {

	case STATE_RRQ:
		xp = pkt;
		*((ushort *)pkt)++ = SWAP16c(TFTP_RRQ);
		strcpy ((char *)pkt, &BootFile[0]);
		pkt += strlen(BootFile) + 1;
		strcpy ((char *)pkt, "octet");
		pkt += 5 /*strlen("octet")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_DATA:
		xp = pkt;
		*((ushort *)pkt)++ = SWAP16c(TFTP_ACK);
		*((ushort *)pkt)++ = SWAP16(TftpBlock);
		len = pkt - xp;
		break;

	case STATE_TOO_LARGE:
		xp = pkt;
		*((ushort *)pkt)++ = SWAP16c(TFTP_ERROR);
		*((ushort *)pkt)++ = SWAP16(3);
		strcpy ((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;

	case STATE_BAD_MAGIC:
		xp = pkt;
		*((ushort *)pkt)++ = SWAP16c(TFTP_ERROR);
		*((ushort *)pkt)++ = SWAP16(2);
		strcpy ((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}

	NetSetEther (NetTxPacket, NetServerEther, PROT_IP);
	NetSetIP (NetTxPacket + ETHER_HDR_SIZE, NetServerIP,
					TftpServerPort, TftpOurPort, len);
	NetSendPacket (NetTxPacket, ETHER_HDR_SIZE + IP_HDR_SIZE + len);
}


static void
TftpHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	if (dest != TftpOurPort)
		return;
	if (TftpState != STATE_RRQ && src != TftpServerPort)
		return;

	if (len < 2)
		return;
	len -= 2;
	switch (SWAP16(*((ushort *)pkt)++)) {

	case TFTP_RRQ:
	case TFTP_WRQ:
	case TFTP_ACK:
	default:
		break;

	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = SWAP16(*(ushort *)pkt);
		if (((TftpBlock - 1) % 10) == 0) putc ('#');

		if (TftpState == STATE_RRQ) {
			TftpState = STATE_DATA;
			TftpServerPort = src;
			TftpLastBlock = 0;

			if (TftpBlock != 1) {	/* Assertion */
				printf ("\nTFTP error: "
					"First block is not block 1 (%d)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain ();
				break;
			}
		}

		if (TftpBlock == TftpLastBlock) {
			/*
			 *	Same block again; ignore it.
			 */
			break;
		}

		TftpLastBlock = TftpBlock;
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);

		/* ImageSize += len; */
		store_block (TftpBlock - 1, pkt + 2, len);

		/*
		 *	Acknoledge the block just received, which will prompt
		 *	the server for the next one.
		 */
		TftpSend ();

		if (len < 512) {
			/*
			 *	We received the whole thing.  Try to
			 *	run it.
			 */
			puts ("\ndone\n");
			NetState = NETLOOP_SUCCESS;
		}
		break;

	case TFTP_ERROR:
		printf ("\nTFTP error: '%s' (%d)\n",
					pkt + 2, SWAP16(*(ushort *)pkt));
		//puts ("Starting again\n\n");
		//NetStartAgain ();		// quick and durty , wenn tftp fails dann gehts einfach weiter ...
		// sorry, aber ein bisschen feedback braucht der programmierer
		NetState = NETLOOP_FAIL;
		break;
	}
}


static void
TftpTimeout (void)
{
	if (++TftpTimeoutCount >= TIMEOUT_COUNT) {
		puts ("\nRetry count exceeded; starting again\n");
		NetStartAgain ();
	} else {
		puts ("T ");
		NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
		TftpSend ();
	}
}


void process_macros (char *input, char *output, char delim);

void
TftpStart (ulong loadAdr)
{
char BootFile2[CFG_CBSIZE];

#ifdef ET_DEBUG
	printf ("\nServer ethernet address %02x:%02x:%02x:%02x:%02x:%02x\n",
		NetServerEther[0],
		NetServerEther[1],
		NetServerEther[2],
		NetServerEther[3],
		NetServerEther[4],
		NetServerEther[5]
	);
#endif /* DEBUG */

	puts ("TFTP from server ");	NetPrintIPaddr (NetServerIP);
	puts ("; our IP address is ");	NetPrintIPaddr (NetOurIP);

	// Check if we need to send across this subnet
	if (NetOurGatewayIP && NetOurSubnetMask) {
	    IPaddr_t OurNet 	= NetOurIP    & NetOurSubnetMask;
	    IPaddr_t ServerNet 	= NetServerIP & NetOurSubnetMask;

	    if (OurNet != ServerNet) {
		puts ("; sending throught gateway ");
		NetPrintIPaddr (NetOurGatewayIP) ;
	    }
	}
	putc ('\n');

	if (BootFile[0] == '\0') {
		int	i;

		for (i = 0; i < 4; i++) {
			static char hex[] = "0123456789ABCDEF";
			int	x;

			x = (NetOurIP >> (i * 8)) & 0xff;
			BootFile[i*2+0] = hex[x >>  4];
			BootFile[i*2+1] = hex[x & 0xf];
		}
		strcpy (&BootFile[8], ".img");

		printf ("No file name; using '%s'.", BootFile);
	} else {
                process_macros (BootFile, BootFile2, '%');
                strncpy(BootFile, BootFile2, sizeof(BootFile));
		BootFile[sizeof(BootFile)-1]=0;
		printf ("Filename '%s'.", BootFile);
	}

	if (NetBootFileSize)
	    printf (" Size is %d%s kB => %x Bytes",
		NetBootFileSize/2,
		(NetBootFileSize%2) ? ".5" : "",
		NetBootFileSize<<9);

	putc ('\n');

	if (loadAdr == ~0) {
		load_addr = CFG_LOAD_ADDR;
		printf ("No load address; using 0x%lx\n", load_addr);
	} else {
		load_addr = loadAdr;
		printf ("Load address: 0x%lx\n", load_addr);
	}

	puts ("Loading: *\b");

	NetSetTimeout (TIMEOUT * CFG_HZ, TftpTimeout);
	NetSetHandler (TftpHandler);

	TftpServerPort = WELL_KNOWN_PORT;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpOurPort = 1024 + (get_timer(0) % 3072);

	TftpSend ();
}

#endif /* CFG_CMD_NET */
