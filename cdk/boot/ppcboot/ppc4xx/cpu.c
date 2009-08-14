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
 * m8xx.c
 *
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 */

#include <ppcboot.h>
#include <command.h>
#include <asm/cache.h>
#include <ppc4xx.h>



/* ------------------------------------------------------------------------- */

int checkcpu(long clock)
{
  uint pvr = get_pvr();
  char buf[32];

#ifdef CONFIG_PPC405GP
  PPC405_SYS_INFO sys_info;

  get_sys_info(&sys_info);

  printf("IBM PowerPC 405GP Rev. ");
  switch (pvr)
    {
    case PVR_405GP_RB:
      putc('B');
      break;
    case PVR_405GP_RC:
      putc('C');
      break;
    case PVR_405GP_RD:
      putc('D');
      break;
    default:
      printf("? (PVR=%08x)", pvr);
      break;
    }

  printf(" at %s MHz (PLB=%lu, OPB=%lu, EBC=%lu MHz)\n", strmhz(buf, clock),
         sys_info.freqPLB / 1000000,
         sys_info.freqPLB / sys_info.pllOpbDiv / 1000000,
         sys_info.freqPLB / sys_info.pllExtBusDiv / 1000000);

  if (mfdcr(strap) & PSR_PCI_ASYNC_EN)
    printf("           PCI async ext clock used, ");
  else
    printf("           PCI sync clock at %lu MHz, ",
           sys_info.freqPLB / sys_info.pllPciDiv / 1000000);
  if (mfdcr(strap) & PSR_PCI_ARBIT_EN)
    printf("internal PCI arbiter enabled\n");
  else
    printf("external PCI arbiter enabled\n");

  switch (pvr)
    {
    case PVR_405GP_RB:
    case PVR_405GP_RC:
    case PVR_405GP_RD:
      printf("           16 kB I-Cache 8 kB D-Cache");
      break;
    }

#endif

#ifdef CONFIG_IOP480
  printf("PLX IOP480 (PVR=%08x)", pvr);
  printf(" at %s MHz:", strmhz(buf, clock));
  printf(" %u kB I-Cache", 4);
  printf(" %u kB D-Cache", 2);
#endif

  printf("\n");

  return 0;
}


/* ------------------------------------------------------------------------- */

void do_reset (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
        /*
         * Initiate system reset in debug control register DBCR
         */
	__asm__ __volatile__(" lis   3, 0x3000
                               mtspr 0x3f2, 3");
}


/*
 * Get timebase clock frequency
 */
unsigned long get_tbclk (void)
{
#ifdef CONFIG_PPC405GP
  PPC405_SYS_INFO sys_info;
  
  get_sys_info(&sys_info);
  return (sys_info.freqProcessor);
#endif  /* CONFIG_PPC405GP */
  
#ifdef CONFIG_IOP480
  return (66000000);
#endif  /* CONFIG_IOP480 */
}
