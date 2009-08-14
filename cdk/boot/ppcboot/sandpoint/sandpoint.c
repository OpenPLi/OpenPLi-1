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

#include <ppcboot.h>

int checkboard (void)
{
    printf("Sandpoint ");
    /*TODO: Check processor type*/
    printf("8240 Unity ##Test not implemented yet##\n");
    return 0;
}

int checkflash (void)
{
    /* TODO: XXX XXX XXX */
    printf ("8 MB ## Test not implemented yet ##\n");

    return (0);
}

long int dram_size (int board_type)
{
    /* No actual initialisation to do - done when setting up PICRs MCCRs ME/SARs etc
     * in cpu_init.c - done there to keep init sequence same as Dink. May be able to move some of
     * it here and refine the configuration defines to be more high-level, but I haven't had time
     * to investigate that yet
     */
     #if defined(CFG_MEM_TEST)

     register unsigned long reg;

     //write each mem addr with it's address
     for (reg = CFG_MEM_TEST_START; reg < CGF_MEM_TEST_END; reg+=4
         *reg = reg;

     for (reg = CFG_MEM_TEST_START; reg < CGF_MEM_TEST_END; reg+=4
     {
         if (*reg != reg)
           return -1;
     }
     #endif

     //TODO: calculate amount of dram..for now just return MEMTEST_END
     return CFG_MEMTEST_END;

}

long int initdram(int board_type)
{
return dram_size(board_type);
}



/*temporarlyily here: to be removed:*/


int eth_init(bd_t *bis)
{
  /* Initialize the device  */
  return 0;
}

int eth_send(volatile void *packet, int length)
{
  /* Send a packet */
  return 0;
}

int eth_rx(void)
{
  /* Check for received packets */
  return 0;
}

void eth_halt(void)
{
  /* stop ethernet     */
}
