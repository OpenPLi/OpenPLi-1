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
#include <commproc.h>

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 * initialize the UPM's
 */
void
cpu_init_f (volatile immap_t *immr)
{
#ifndef CONFIG_MBX
    volatile	memctl8xx_t *memctl = &immr->im_memctl;
    ulong	reg;
#endif

    /* SYPCR - contains watchdog control (11-9) */

    immr->im_siu_conf.sc_sypcr = CFG_SYPCR;

#if defined(CONFIG_WATCHDOG)
    reset_8xx_watchdog(immr);
#endif /* CONFIG_WATCHDOG */

    /* SIUMCR - contains debug pin configuration (11-6) */

    immr->im_siu_conf.sc_siumcr |= CFG_SIUMCR;

    /* initialize timebase status and control register (11-26) */
    /* unlock TBSCRK */

    immr->im_sitk.sitk_tbscrk = KAPWR_KEY;
    immr->im_sit.sit_tbscr = CFG_TBSCR;

    /* initialize the PIT (11-31) */

    immr->im_sitk.sitk_piscrk = KAPWR_KEY;
    immr->im_sit.sit_piscr = CFG_PISCR;

    /* PLL (CPU clock) settings (15-30) */

    immr->im_clkrstk.cark_plprcrk = KAPWR_KEY;

#ifndef CONFIG_MBX	/* MBX board does things different */

    /* If CFG_PLPRCR (set in the various *_config.h files) tries to
     * set the MF field, then just copy CFG_PLPRCR over car_plprcr,
     * otherwise OR in CFG_PLPRCR so we do not change the currentMF
     * field value.
     */
#if ((CFG_PLPRCR & PLPRCR_MF_MSK) != 0)
    reg = CFG_PLPRCR;			/* reset control bits	*/
#else
    reg = immr->im_clkrst.car_plprcr;
    reg &= PLPRCR_MF_MSK;		/* isolate MF field	*/
    reg |= CFG_PLPRCR;			/* reset control bits	*/
#endif
    immr->im_clkrst.car_plprcr = reg;

    /* System integration timers. Don't change EBDF! (15-27) */

    immr->im_clkrstk.cark_sccrk = KAPWR_KEY;
    reg = immr->im_clkrst.car_sccr;
    reg &= SCCR_MASK;
    reg |= CFG_SCCR;
    immr->im_clkrst.car_sccr = reg;

    /*
     * Memory Controller:
     */
#ifndef CONFIG_DBOX
    /* perform BR0 reset that MPC850 Rev. A can't guarantee */
    reg = memctl->memc_br0;
    reg &= BR_PS_MSK;		/* Clear everything except Port Size bits */
    reg |= BR_V;		/* then add just the "Bank Valid" bit     */
    memctl->memc_br0 = reg;

    /* Map banks 0 and 1 to the FLASH banks 0 and 1 at preliminary
     * addresses - these have to be modified later when FLASH size
     * has been determined
     */

#if defined(CONFIG_SPD823TS) || defined(CONFIG_IVMS8) || \
   (defined(CONFIG_MPC860T) && defined(CONFIG_FADS))
    /* XXX - FIXME - XXX
     * I still don't understand why some systems work only with this
     * statement here, and others work only without it.
     * I offer a free beer to anyone who can explain that to me - wd
     */
    memctl->memc_br0 = CFG_BR0_PRELIM;	/* XXX ??? XXX ??? XXX */
#endif

#if defined(CFG_OR0_REMAP)
    memctl->memc_or0 = CFG_OR0_REMAP;
#endif
#if defined(CFG_OR1_REMAP)
    memctl->memc_or1 = CFG_OR1_REMAP;
#endif

    /* now restrict to preliminary range */
    memctl->memc_br0 = CFG_BR0_PRELIM;
    memctl->memc_or0 = CFG_OR0_PRELIM;

#if (defined(CFG_OR1_PRELIM) && defined(CFG_BR1_PRELIM))
    memctl->memc_or1 = CFG_OR1_PRELIM;
    memctl->memc_br1 = CFG_BR1_PRELIM;
#endif

#if defined(CFG_OR2_PRELIM) && defined(CFG_BR2_PRELIM)
    memctl->memc_or2 = CFG_OR2_PRELIM;
    memctl->memc_br2 = CFG_BR2_PRELIM;
#endif

#if defined(CFG_OR3_PRELIM) && defined(CFG_BR3_PRELIM)
    memctl->memc_or3 = CFG_OR3_PRELIM;
    memctl->memc_br3 = CFG_BR3_PRELIM;
#endif
#endif  /* ! CONFIG_DBOX */

#endif	/* ! CONFIG_MBX */


#ifndef CONFIG_DBOX
    /*
     * Reset CPM
     */
    immr->im_cpm.cp_cpcr = CPM_CR_RST;
    do {				/* Spin until command processed		*/
	__asm__ ("eieio");
    } while (immr->im_cpm.cp_cpcr & CPM_CR_FLG);
#endif
#ifdef CONFIG_MBX
    /*
     * on the MBX, things are a little bit different:
     * - we need to read the VPD to get board information
     * - the plprcr is set up dynamically
     * - the memory controller is set up dynamically
     */
    mbx_init();
#endif	/* CONFIG_MBX */
}

/*
 * initialize higher level parts of CPU like timers
 */
void
cpu_init_r  (bd_t *bd)
{
#if defined(CFG_RTCSC) || defined(CFG_RCCR) || defined(CFG_RMDS)
    volatile immap_t *immr = (volatile immap_t *)(bd->bi_immr_base);
#endif

#ifdef CFG_RTCSC
    /* Unlock RTSC register */
    immr->im_sitk.sitk_rtcsck = KAPWR_KEY;
    /* write config value */
    immr->im_sit.sit_rtcsc = CFG_RTCSC;
#endif

#ifdef CFG_RCCR
    /* write config value */
    immr->im_cpm.cp_rccr = CFG_RCCR;
#endif

#ifdef CFG_RMDS
    /* write config value */
    immr->im_cpm.cp_rmds = CFG_RMDS;
#endif
}
