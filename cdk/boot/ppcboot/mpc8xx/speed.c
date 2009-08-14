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
#include <mpc8xx.h>
#include <asm/processor.h>
#include "speed.h"

#if !defined(CONFIG_8xx_GCLK_FREQ)
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
#endif

/* ------------------------------------------------------------------------- */

/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 *
 * (Approx. GCLK frequency in Hz)
 *
 * Initializes timer 2 and PIT, but disables them before return.
 * [Use timer 2, because MPC823 CPUs mask 0.x do not have timers 3 and 4]
 */

ulong get_gclk_freq (void)
{
#ifndef	CONFIG_8xx_GCLK_FREQ
  volatile immap_t        *immap = (immap_t *)CFG_IMMR;
  volatile cpmtimer8xx_t *timerp = &immap->im_cpmtimer;
  ulong timer2_val;
  ulong msr_val;

  /* Reset + Stop Timer 2, no cascading
   */
  timerp->cpmt_tgcr &= ~(TGCR_CAS2 | TGCR_RST2);

  /* Keep stopped, halt in debug mode
   */
  timerp->cpmt_tgcr |=  (TGCR_FRZ2 | TGCR_STP2);

  /* Timer 2 setup:
   * Output ref. interrupt disable, int. clock / 16
   */
  timerp->cpmt_tmr2 = (SPEED_TMR2_PS << TMR_PS_SHIFT) | TMR_ICLK_IN_GEN_DIV16;

  timerp->cpmt_tcn2 = 0;			/* reset state		*/
  timerp->cpmt_tgcr |= TGCR_RST2;		/* enable timer 2	*/

  immap->im_sitk.sitk_pitck  = KAPWR_KEY;	/* PIT initialization	*/
  immap->im_sit.sit_pitc    = SPEED_PITC;

  immap->im_sitk.sitk_piscrk = KAPWR_KEY;
  immap->im_sit.sit_piscr    = CFG_PISCR;

  /*
   * Start measurement - disable interrupts, just in case
   */
  msr_val = get_msr();
  set_msr (msr_val & ~MSR_EE);

  immap->im_sit.sit_piscr |= PISCR_PTE;
  timerp->cpmt_tgcr &= ~TGCR_STP2;		/* Start Timer 2	*/
  while ((immap->im_sit.sit_piscr & PISCR_PS) == 0)
	;
  timerp->cpmt_tgcr |=  TGCR_STP2;		/* Stop  Timer 2	*/

  /* re-enable external interrupts if they were on */
  set_msr (msr_val);

  /* Disable timer and PIT
   */
  timer2_val = timerp->cpmt_tcn2;		/* save before restting	*/

  timerp->cpmt_tgcr &= ~(TGCR_RST2 | TGCR_FRZ2 | TGCR_STP2);
  immap->im_sit.sit_piscr &= ~PISCR_PTE;

  return (timer2_val * 400000L);		/* convert to Hz	*/

#else	/* CONFIG_8xx_GCLK_FREQ */

	/*
	 * If for some reason measuring the gclk frequency won't work,
	 * we return the hardwired value.
	 * (For example, the cogent CMA286-60 CPU module has no
	 * separate oscillator for PITRTCLK)
	 */

	return (CONFIG_8xx_GCLK_FREQ);

#endif	/* CONFIG_8xx_GCLK_FREQ */
}

/* ------------------------------------------------------------------------- */

ulong get_bus_freq (ulong gclk_freq)
{
    immap_t *immr = (immap_t *)CFG_IMMR;

    if ((immr->im_clkrst.car_sccr & SCCR_EBDF11) == 0)	/* No Bus Divider active */
	return (gclk_freq);

    return (gclk_freq / 2);	/* The MPC8xx has only one BDF: half clock speed */
}

/* ------------------------------------------------------------------------- */
