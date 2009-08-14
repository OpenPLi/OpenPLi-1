/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <ppcboot.h>
#include <command.h>
#include "net.h"
#include "bootp.h"
#include "rarp.h"
#include "tftp.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET)

#define TIMEOUT		5		/* Seconds before trying BOOTP again */


int		RarpTry;
static ulong	lAddr;

/*
 *	Handle a RARP received packet.
 */
static void
RarpHandler(uchar * dummi0, unsigned dummi1, unsigned dummi2, unsigned dummi3)
{
	printf("Got good RARP\n");

	TftpStart(lAddr);
}


/*
 *	Timeout on BOOTP request.  Try again, forever.
 */
static void
RarpTimeout(void)
{
	RarpRequest(BootFile, lAddr);
}


void
RarpRequest(char *fileName, ulong loadAdr)
{
	int i;
	volatile uchar *pkt;
	ARP_t *	rarp;

	lAddr = loadAdr;

	printf("RARP broadcast %d\n", ++RarpTry);
	pkt = NetTxPacket;

	NetSetEther(pkt, NetBcastAddr, PROT_RARP);
	pkt += ETHER_HDR_SIZE;

	rarp = (ARP_t *)pkt;

	rarp->ar_hrd = ARP_ETHER;
	rarp->ar_pro = PROT_IP;
	rarp->ar_hln = 6;
	rarp->ar_pln = 4;
	rarp->ar_op  = RARPOP_REQUEST;
	NetCopyEther(&rarp->ar_data[0], NetOurEther);	/* source ET addr */
	*(IPaddr_t *)(&rarp->ar_data[6]) = NetOurIP;	/* source IP addr */
	NetCopyEther(&rarp->ar_data[10], NetOurEther);	/* dest ET addr = source ET addr ??*/
	/* dest. IP addr set to broadcast */
	for (i = 0; i <= 3; i++) rarp->ar_data[16 + i] = 0xff;



	/* store boot file name for TFTP */
	strcpy(BootFile, fileName);

	NetSendPacket(NetTxPacket, ETHER_HDR_SIZE + ARP_HDR_SIZE);

	NetSetTimeout(TIMEOUT * CFG_HZ, RarpTimeout);
	NetSetHandler(RarpHandler);
}

#endif /* CFG_CMD_NET */
