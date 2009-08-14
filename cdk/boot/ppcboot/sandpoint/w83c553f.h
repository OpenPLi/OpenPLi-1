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

 /* winbond access routines and defines*/

/* from the winbond data sheet -
 The W83C553F SIO controller with PCI arbiter is a multi-function PCI device.
 Function 0 is the ISA bridge, and Function 1 is the bus master IDE controller.
*/

/*ISA bridge configuration space*/

#define WINBOND_PCICONTR  0x40  /*pci control reg*/
#define WINBOND_SGBAR     0x41  /*scatter/gather base address reg*/
#define WINBOND_LBCR      0x42  /*Line Buffer Control reg*/
#define WINBOND_IDEIRCR   0x43  /*IDE Interrupt Routing Control  Reg*/
#define WINBOND_PCIIRCR   0x44  /*PCI Interrupt Routing Control Reg*/
#define WINBOND_BTBAR     0x46  /*BIOS Timer Base Address Register*/
#define WINBOND_IPADCR    0x48  /*ISA to PCI Address Decoder Control Register*/
#define WINBOND_IRADCR    0x49  /*ISA ROM Address Decoder Control Register*/
#define WINBOND_IPMHSAR   0x4a  /*ISA to PCI Memory Hole STart Address Register*/
#define WINBOND_IPMHSR    0x4b  /*ISA to PCI Memory Hols Size Register*/
#define WINBOND_CDR       0x4c  /*Clock Divisor Register*/
#define WINBOND_CSCR      0x4d  /*Chip Select Control Register*/
#define WINBOND_ATSCR     0x4e  /*AT System Control register*/
#define WINBOND_ATBCR     0x4f  /*AT Bus ControL Register*/
#define WINBOND_IRQBEE0R  0x60  /*IRQ Break Event Enable 0 Register*/
#define WINBOND_IRQBEE1R  0x61  /*IRQ Break Event Enable 1 Register*/
#define WINBOND_ABEER     0x62  /*Additional Break Event Enable Register*/
#define WINBOND_DMABEER   0x63  /*DMA Break Event Enable Register*/


