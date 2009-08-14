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
#include <405gp_enet.h>


/*
 * Breath some life into the CPU...
 *
 * On 4xx: allready done in start.S
 */
void
cpu_init_f (void)
{
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
void
cpu_init_r  (bd_t *bd)
{
    unsigned long reg;

    /*
     * Write Ethernetaddress into on-chip register
     */
    reg = 0x00000000;
    reg |= bd->bi_enetaddr[0];           /* set high address */
    reg = reg << 8;
    reg |= bd->bi_enetaddr[1];
    out32 (EMAC_IAH, reg);
    
    reg = 0x00000000;
    reg |= bd->bi_enetaddr[2];           /* set low address  */
    reg = reg << 8;
    reg |= bd->bi_enetaddr[3];
    reg = reg << 8;
    reg |= bd->bi_enetaddr[4];
    reg = reg << 8;
    reg |= bd->bi_enetaddr[5];
    out32 (EMAC_IAL, reg);

}
