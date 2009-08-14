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

#include <mpc8240.h>
#include <ppcboot.h>
#include <command.h>

int checkcpu(long clock)
{
  unsigned int pvr = get_pvr();
  unsigned int version = pvr >>16;
  unsigned int revision = pvr &0xffff;

  switch(version)
  {
    case 0x0081:
      printf("MPC8240");
      break;
    default:
      return -1; /*not valid for this source*/
  }

  switch(revision)
  {
    default: /*currently no info on revision numbers*/
      printf(" Revsion %d.%d",revision>>8, revision & 0xff);
  }

  printf(" at %lu MHz:", clock);

  printf(" %u kB I-Cache", checkicache() >> 10);
  printf(" %u kB D-Cache", checkdcache() >> 10);
  return 0;
}

/* ------------------------------------------------------------------------- */
/* L1 i-cache                                                                */

int checkicache(void)
{
    /*TODO*/
  return 128*4*32;
};
/* ------------------------------------------------------------------------- */
/* L1 d-cache                                                                */

int checkdcache(void)
{
    /*TODO*/
  return 128*4*32;

};

/*------------------------------------------------------------------- */

void do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
    ulong msr, addr;


    /* Interrupts and MMU off */
    __asm__ ("mtspr    81, 0
          mfmsr    %0": "=r"(msr) : );
    msr &= ~0x1030;
    __asm__ ("mtmsr    %0"::"r"(msr) );

    /* jump to reset vector*/
    addr = 0xfff0100;
    ((void (*)(void ))addr)();

}

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 *
 */
unsigned long get_tbclk (void)
{
	ulong tbclk;
	/* Pointer to initial global data area */
	init_data_t *idata =
		(init_data_t *)(CFG_INIT_RAM_ADDR+CFG_INIT_DATA_OFFSET);

	tbclk = (idata->cpu_clk + 3L) / 4L;

	return (tbclk);
}

/* ------------------------------------------------------------------------- */
