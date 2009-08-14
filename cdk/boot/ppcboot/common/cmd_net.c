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

/*
 * Boot support
 */
#include <ppcboot.h>
#include <command.h>
#include <cmd_net.h>
#include <net.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET)

extern void do_bootm (cmd_tbl_t *, bd_t *, int, int, char *[]);

static void netboot_common (int, cmd_tbl_t *, bd_t *, int , char *[]);

void do_bootp (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	netboot_common (BOOTP, cmdtp, bd, argc, argv);
}

void do_tftpb (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	netboot_common (TFTP, cmdtp, bd, argc, argv);
}

void do_rarpb (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	netboot_common (RARP, cmdtp, bd, argc, argv);
}

void netboot_update_env(void)
{
    char tmp[12] ;

    if (NetOurGatewayIP) {
	NetIPaddr (NetOurGatewayIP, tmp);
	setenv("gatewayip", tmp);
    }

    if (NetOurSubnetMask) {
	NetIPaddr (NetOurSubnetMask, tmp);
	setenv("netmask", tmp);
    }

    if (NetOurHostName[0])
	setenv("hostname", NetOurHostName);

    if (NetOurRootPath[0])
	setenv("rootpath", NetOurRootPath);

    if (NetOurBootPath[0])
	setenv("bootpath", NetOurBootPath);

    if (NetOurIP) {
	NetIPaddr (NetOurIP, tmp);
	setenv("ipaddr", tmp);
    }

    if (NetServerIP) {
	NetIPaddr (NetServerIP, tmp);
	setenv("serverip", tmp);
    }

    if (NetOurDNSIP) {
	NetIPaddr (NetOurDNSIP, tmp);
	setenv("dnsip", tmp);
    }
}

static void
netboot_common (int proto, cmd_tbl_t *cmdtp, bd_t *bd, int argc, char *argv[])
{
	ulong	addr;
	int rc;
	char *s;

	switch (argc) {
	case 1:	rc = NetLoop(bd, proto, "", -1);
		break;
	case 2:	addr = simple_strtoul(argv[1], NULL, 16);
		rc = NetLoop(bd, proto, "", addr);
		break;
	case 3:	addr = simple_strtoul(argv[1], NULL, 16);
		rc = NetLoop(bd, proto, argv[2], addr);
		break;
	default: printf ("Usage:\n%s\n", cmdtp->usage);
		return;
	}

	if (rc == 0)
	    return;
	else
	    netboot_update_env();

	/* Loading ok, check if we should attempt an auto-start */
	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);

		do_bootm (cmdtp, bd, 0, 1, local_args);
	}
}

#endif	/* CFG_CMD_NET */
