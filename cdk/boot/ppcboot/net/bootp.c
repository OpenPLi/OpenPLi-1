/*
 *	Based on LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 */

/* #define DEBUG_BOOTP_EXT	1	/ * Debug received vendor fields	*/

#include <ppcboot.h>
#include <command.h>
#include "net.h"
#include "bootp.h"
#include "tftp.h"
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#define	BOOTP_VENDOR_MAGIC	0x63825363 	/* RFC1048 Magic Cookie 	*/

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define TIMEOUT		5		/* Seconds before trying BOOTP again	*/

#define PORT_BOOTPS	67		/* BOOTP server UDP port		*/
#define PORT_BOOTPC	68		/* BOOTP client UDP port		*/

ulong		BootpID;
char		BootFile[128];
int		BootpTry;
static ulong	lAddr;

static int BootpExtended (u8 *e);

static void BootpVendorFieldProcess(u8 *ext)
{
    int size = *(ext+1) ;

#ifdef DEBUG_BOOTP_EXT
    printf("[BOOTP] Processing extension %d... (%d bytes)\n", *ext, *(ext+1));
#endif

    NetBootFileSize = 0;

    switch (*ext) {
    /* Fixed length fields */
	case 1:		/* Subnet mask					*/
		if (NetOurSubnetMask == 0)
		    memcpy(&NetOurSubnetMask, ext+2, 4);
		break;
	case 2:		/* Time offset - Not yet supported		*/
		break;
    /* Variable length fields */
	case 3:		/* Gateways list				*/
		if (NetOurGatewayIP == 0) {
		    memcpy(&NetOurGatewayIP, ext+2, 4);
		}    
		break;
	case 4:		/* Time server - Not yet supported		*/
		break;
	case 5:		/* IEN-116 name server - Not yet supported	*/
		break;
	case 6:
		if (NetOurDNSIP == 0) {
		    memcpy(&NetOurDNSIP, ext+2, 4);
		}
		break;
	case 7:		/* Log server - Not yet supported		*/
		break;
	case 8:		/* Cookie/Quote server - Not yet supported	*/
		break;
	case 9:		/* LPR server - Not yet supported		*/
		break;
	case 10:	/* Impress server - Not yet supported		*/
		break;
	case 11:	/* RPL server - Not yet supported		*/
		break;
	case 12:	/* Host name					*/
		if (NetOurHostName[0] == 0) {
		    memcpy(&NetOurHostName, ext+2, size);
		    NetOurHostName[size] = 0 ;
		}
		break;
	case 13:	/* Boot file size				*/
		memcpy(&NetBootFileSize, ext+2, size);
		break;
	case 14:	/* Merit dump file - Not yet supported		*/
		break;
	case 15:	/* Domain name - Not yet supported		*/
		break;
	case 16:	/* Swap server - Not yet supported		*/
		break;
	case 17:	/* Root path					*/
		if (NetOurRootPath[0] == 0) {
		    memcpy(&NetOurRootPath, ext+2, size);
		    NetOurRootPath[size] = 0 ;
		}
		break;
	case 18:	/* Extension path - Not yet supported		*/
		/*
                 * This can be used to send the informations of the
                 * vendor area in another file that the client can
                 * access via TFTP.
		 */
		break;
    /* IP host layer fields */
	case 40:	/* NIS Domain name				*/
		if (NetOurNISDomain[0] == 0) {
		    memcpy(&NetOurNISDomain, ext+2, size);
		    NetOurNISDomain[size] = 0 ;
		}
		break;
    /* Application layer fields */
	case 43:	/* Vendor specific info - Not yet supported	*/
		/*
                 * Binary informations to exchange specific
                 * product information.
		 */
		break;
    /* Reserved (custom) fields (128..254) */
    }
}

static void BootpVendorProcess(u8 *ext, int size)
{
    u8 *end = ext + size ;
#ifdef DEBUG_BOOTP_EXT
    printf("[BOOTP] Checking extension (%d bytes)...\n", size);
#endif
    while ((ext < end) && (*ext != 0xff)) {
	if (*ext == 0) {
	    ext ++ ;
	} else {
		u8 *opt = ext ;
		ext += ext[1] + 2 ;
		if (ext <= end)
		    BootpVendorFieldProcess (opt) ;
	}
    }

#ifdef DEBUG_BOOTP_EXT
    printf("[BOOTP] Received fields: \n");
    if (NetOurSubnetMask) {
	puts ("NetOurSubnetMask	: ");
	NetPrintIPaddr (NetOurSubnetMask);
	putc('\n');
    }
/*    
    if (NetOurGatewaysIP[0]) {
	puts ("NetOurGatewaysIP	: ");
	NetPrintIPaddr (NetOurGatewaysIP[0]);
	putc('\n');
    }
*/
    if (NetBootFileSize) {
	printf("NetBootFileSize : %d\n", NetBootFileSize);
    }
    
    if (NetOurHostName[0]) {
	printf("NetOurHostName  : %s\n", NetOurHostName);
    }
	
    if (NetOurRootPath[0]) {
	printf("NetOurRootPath  : %s\n", NetOurRootPath);
    }
    
    if (NetOurBootPath[0]) {
	printf("NetOurBootPath  : %s\n", NetOurBootPath);
    }

    if (NetOurNISDomain[0]) {
        printf("NetOurNISDomain : %s\n", NetOurNISDomain);
    }
#endif
}

void netboot_update_env(void); // quick'n dirty proto
void process_macros (char *input, char *output, char delim);

/*
 *	Handle a BOOTP received packet.
 */
static void
BootpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	Bootp_t *	bp;
	char *s1, *s2;

#ifdef DEBUG
	printf("got BOOTP packet (src=%d, dst=%d, len=%d want_len=%d)\n",
		src, dest, len, sizeof (Bootp_t));
#endif /* DEBUG */

	bp = (Bootp_t *)pkt;

	if (dest != PORT_BOOTPC || src != PORT_BOOTPS)
		return;
	if (len < sizeof (Bootp_t))
		return;
	if (bp->bp_op != OP_BOOTREPLY)
		return;
	if (bp->bp_htype != HWT_ETHER)
		return;
	if (bp->bp_hlen != HWL_ETHER)
		return;
	if (bp->bp_id != BootpID)
		return;

	/*
	 *	Got a good BOOTP reply.  Copy the data into our variables.
	 */
#ifdef CONFIG_STATUS_LED
	status_led_set (STATUS_LED_OFF);
#endif
	NetOurIP = bp->bp_yiaddr;
	NetServerIP = bp->bp_siaddr;
	NetCopyEther(NetServerEther, ((Ethernet_t *)NetRxPkt)->et_src);

	/* The following is somewhat strange but if I send
	     %(bootpath)/tftpboot/kernel
	   to my bootpd bootpd will send back
	     /%(bootpath)/tftpboot/blabla/tftpboot/ppcboot
           instead of
	     /blabla/tftpboot/ppcboot
           as dhcpd does
	*/
	bp->bp_file[sizeof(bp->bp_file)-1]=0; // keep str... save
	s1=strchr(BootFile, '/');
	s2=strchr(bp->bp_file, '/');
	if(s1!=BootFile && s2==bp->bp_file)
	  // we've got a new / first
	  s2++;
        else
	  s2=bp->bp_file;
	if(s1 && s2 && !strncmp(BootFile, s2, s1-BootFile))
	    /* if we have send a bootfile name and we asume we've got bootfile+normal bootfile back */
	    memcpy(NetOurBootPath, bp->bp_file+(strrchr(BootFile, '/')-BootFile+1), sizeof(bp->bp_file)-(strrchr(BootFile, '/')-BootFile+1));
	else
	    // We have got the same filename as used for ppcboot
	    memcpy(NetOurBootPath, bp->bp_file, sizeof(bp->bp_file));
	NetOurBootPath[sizeof(NetOurBootPath)-1]=0;
	s1=strrchr(NetOurBootPath, '/');
	if(s1) {
	    *s1=0;
	    s1=strrchr(NetOurBootPath, '/');
	    if(s1)
		*s1=0;
	}
	/* Retrieve extended informations (we must parse the vendor area) */
	if ((*(uint *)bp->bp_vend) == BOOTP_VENDOR_MAGIC)
	    BootpVendorProcess(&bp->bp_vend[4], len);
	netboot_update_env();
	NetSetTimeout(0, (thand_f *)0);

#ifdef DEBUG
	printf("Got good BOOTP\n");
#endif /* DEBUG */

	TftpStart(lAddr);
}


/*
 *	Timeout on BOOTP request.  Try again, forever.
 */
static void
BootpTimeout(void)
{
	BootpRequest(BootFile, lAddr);
}

/*
 *	Initialize BOOTP extension fields in the request.
 *
 *	Warning: no field size check - change CONFIG_BOOTP_MASK at your own risk!
 */
static int BootpExtended (u8 *e)
{
    u8 *start = e ;
    
    *e++ =  99;		/* RFC1048 Magic Cookie */
    *e++ = 130;
    *e++ =  83;
    *e++ =  99;

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_SUBNETMASK)
    *e++ =  1;		/* Subnet mask request */
    *e++ =  4;
     e  +=  4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_GATEWAY)
    *e++ =  3;		/* Default gateway request */
    *e++ =  4;
     e  +=  4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_DNS)
    *e++ =  6;		/* Domain Name Server */
    *e++ =  4;
     e  +=  4;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_HOSTNAME)
    *e++ = 12;		/* Host name request */
    *e++ = 32;
     e  += 32;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTFILESIZE)
    *e++ = 13;		/* Boot file size */
    *e++ =  2;
     e  +=  2;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_NISDOMAIN)
    *e++ = 40;		/* NIS Domain name request */
    *e++ = 32;
     e  += 32;
#endif

#if (CONFIG_BOOTP_MASK & CONFIG_BOOTP_BOOTPATH)
    *e++ = 17;		/* Boot path */
    *e++ = 32;
     e  += 32;
#endif

    *e = 255;		/* End of the list */

    return e - start ;
}

void
BootpRequest(char *fileName, ulong loadAdr)
{
	volatile uchar *pkt;
	Bootp_t *bp;
	int ext_len;

	lAddr = loadAdr;

	printf("BOOTP broadcast %d\n", ++BootpTry);
	pkt = NetTxPacket;

	NetSetEther(pkt, NetBcastAddr, PROT_IP);
	pkt += ETHER_HDR_SIZE;

	NetSetIP(pkt, 0xffffffffL, PORT_BOOTPS, PORT_BOOTPC, sizeof (Bootp_t));
	pkt += IP_HDR_SIZE;

	bp = (Bootp_t *)pkt;
	bp->bp_op = OP_BOOTREQUEST;
	bp->bp_htype = HWT_ETHER;
	bp->bp_hlen = HWL_ETHER;
	bp->bp_hops = 0;
	bp->bp_secs = SWAP16( get_timer(0) / CFG_HZ);
	bp->bp_ciaddr = 0;
	bp->bp_yiaddr = 0;
	bp->bp_siaddr = 0;
	bp->bp_giaddr = 0;
	NetCopyEther(bp->bp_chaddr, NetOurEther);
	strcpy(bp->bp_file, fileName);

	/* Request additional information from the BOOTP server */
	ext_len = BootpExtended (bp->bp_vend) - sizeof(bp->bp_vend);
	if (ext_len < 0) {
		ext_len = 0;
	}

	/* store boot file name for repetitions in case of bootp timeout */
	strcpy(BootFile, fileName);
	/*
	 *	Bootp ID is the lower 4 bytes of our ethernet address
	 *	plus the current time in HZ.
	 */
	BootpID = ((ulong)NetOurEther[2] << 24)
		| ((ulong)NetOurEther[3] << 16)
		| ((ulong)NetOurEther[4] << 8)
		| (ulong)NetOurEther[5];
	BootpID += get_timer(0);
	bp->bp_id = BootpID;

	NetSendPacket(NetTxPacket, BOOTP_SIZE + ext_len);

	NetSetTimeout(TIMEOUT * CFG_HZ, BootpTimeout);
	NetSetHandler(BootpHandler);
}

#endif /* CFG_CMD_NET */
