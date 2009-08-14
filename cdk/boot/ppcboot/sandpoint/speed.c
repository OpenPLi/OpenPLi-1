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
#include <mpc8240.h>
#include <asm/processor.h>
#include "speed.h"

/*use UART2 to measure bus speed*/
#include "ns16550.h"


/* Access functions for the Machine State Register */
static __inline__ unsigned long get_msr(void)
{
    unsigned long msr;

    asm volatile("mfmsr %0" : "=r" (msr) :);
    return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
    asm volatile("mtmsr %0" : : "r" (msr));
}

/* ------------------------------------------------------------------------- */

ulong get_bus_freq (ulong ignore)
{
    /* measure the bus frequency using UART2 */
    /* 1st initilise it to run at 9600
       it takes approx 10*(1/9600) us to sens an 8N1 character at 9600bps
       but due to chrystal inaccuracies on sandpoint we use 1145us */

    int count=0, start, end;
    int i;
    volatile struct NS16550 *uart2;

    uart2=NS16550_init(1,115200/9600);


    for (i =0; i< 10 ; i++)
    {
       NS16550_putc(uart2,0); /*send NUL*/
       start=mfspr(DEC);
       NS16550_putc(uart2,0); /*send NUL*/
       end=mfspr(DEC);
       count +=(start-end);
    }

    count /=10; /*average number of decrements per char send*/
    count *=4;  /*1 decrement per 4 bus cycles*/

    /*time taken to do count cycles = 1145 us
      therefore number of cycles per second is (1/(1145*10-6))*count
      */
    return (1000000/1145)*count;
}

/* ------------------------------------------------------------------------- */

/*
 * Measure CPU clock speed
 */

/*table to convert pllratio to actual processor clock scaling factor (*10)*/
/* if you are using a a different processor with your sandpoint,
   have a look at pmc.c in the dink source for values, or
   figure it out from the hardware book*/

#ifdef CONFIG_MPC8240
short pllratio_to_factor[] = {
    00, 00, 00, 10, 20, 20, 25, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 10, 00, 00, 00, 45, 30, 00, 40, 00, 00, 00, 35, 00,
};
#endif

ulong get_gclk_freq (void)
{
   uint hid1;
   hid1=mfspr(HID1);
   #ifdef CONFIG_MPC8240
   hid1=(hid1 >> (32-5)) & 0x1f; /* 5 bits for PLL ration on 8240*/
   #else
   hid1=(hid1 >> (32-4)) & 0xf; /* 4 bits on everythings else*/
   #endif

   return (pllratio_to_factor[hid1] * get_bus_freq(0))/10;

}

/* ------------------------------------------------------------------------- */

