/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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

#include <config.h>

#include <mpc8240.h>
#include "ns87308.h"
#include "ns16550.h"   /*to configure the 87308's internal 16550's*/
#include <asm/mc146818rtc.h>  /* to configure 87308's RTC*/

void initialise_ns87308 (void)
{
    PNP_SET_DEVICE_BASE(LDEV_UART1, COM1);
    PNP_SET_DEVICE_BASE(LDEV_UART2, COM2);

    PNP_SET_DEVICE_BASE(LDEV_RTC_APC, RTC_PORT(0));
    PNP_ACTIVATE_DEVICE(LDEV_POWRMAN);

    /* set up the NVRAM access registers
    NVRAM's controlled by the configurable CS line from the 87308*/

    PNP_PGCS_CSLINE_BASE(0, 0x76);
    PNP_PGCS_CSLINE_CONF(0, 0x30);
    PNP_PGCS_CSLINE_BASE(1, 0x75);
    PNP_PGCS_CSLINE_CONF(1, 0x30);
    PNP_PGCS_CSLINE_BASE(2, 0x74);
    PNP_PGCS_CSLINE_CONF(2, 0x30);
}
/*
void write_pnp_config(unsigned char index, unsigned char data)
{
    unsigned char *io_index = (unsigned char *) IO_INDEX;
    unsigned char *io_data = (unsigned char *) IO_DATA;
    *io_index = index;
    *io_data = data;
}

void pnp_set_device(unsigned char dev)
{
    write_pnp_config(LOGICAL_DEVICE, dev);
}
*/
