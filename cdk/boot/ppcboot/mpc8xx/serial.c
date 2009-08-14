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
#include <commproc.h>
#include <command.h>

#if !defined(CONFIG_8xx_CONS_NONE)	/* No Console at all */

#if defined(CONFIG_8xx_CONS_SMC1)	/* Console on SMC1 */
#define	SMC_INDEX	0
#undef 	SCC_INDEX
#define PROFF_SMC	PROFF_SMC1
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC1

#elif defined(CONFIG_8xx_CONS_SMC2)	/* Console on SMC2 */
#define SMC_INDEX	1
#undef 	SCC_INDEX
#define PROFF_SMC	PROFF_SMC2
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC2

#elif defined(CONFIG_8xx_CONS_SCC1)	/* Console on SCC1 */
#undef  SMC_INDEX
#define SCC_INDEX	0
#define PROFF_SCC	PROFF_SCC1
#define CPM_CR_CH_SCC	CPM_CR_CH_SCC1

#elif defined(CONFIG_8xx_CONS_SCC2)	/* Console on SCC2 */
#undef  SMC_INDEX
#define SCC_INDEX	1
#define PROFF_SCC	PROFF_SCC2
#define CPM_CR_CH_SCC	CPM_CR_CH_SCC2

#elif defined(CONFIG_8xx_CONS_SCC3)	/* Console on SCC3 */
#undef  SMC_INDEX
#define SCC_INDEX	2
#define PROFF_SCC	PROFF_SCC3
#define CPM_CR_CH_SCC	CPM_CR_CH_SCC3

#elif defined(CONFIG_8xx_CONS_SCC4)	/* Console on SCC4 */
#undef  SMC_INDEX
#define SCC_INDEX	3
#define PROFF_SCC	PROFF_SCC4
#define CPM_CR_CH_SCC	CPM_CR_CH_SCC4

#else /* CONFIG_8xx_CONS_? */
#error "console not correctly defined"
#endif

#if (defined (CONFIG_8xx_CONS_SMC1) || defined (CONFIG_8xx_CONS_SMC2))

/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

void
serial_init (ulong cpu_clock, int baudrate)
{
        volatile immap_t *im = (immap_t *)CFG_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8xx_t *cp = &(im->im_cpm);
#if (!defined(CONFIG_8xx_CONS_SMC1)) && (defined(CONFIG_MPC823) || defined(CONFIG_MPC850))
	volatile iop8xx_t *ip = (iop8xx_t *)&(im->im_ioport);
#endif
	uint	dpaddr, dpsize, size;

	/* initialize pointers to SMC */

	sp = (smc_t *) &(cp->cp_smc[SMC_INDEX]);
	up = (smc_uart_t *) &cp->cp_dparam[PROFF_SMC];

	/* Disable transmitter/receiver.
	*/
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* Enable SDMA.
	*/
	im->im_siu_conf.sc_sdcr = 1;

	/* clear error conditions */
#ifdef	CFG_SDSR
	im->im_sdma.sdma_sdsr = CFG_SDSR;
#else
	im->im_sdma.sdma_sdsr = 0x83;
#endif

	/* clear SDMA interrupt mask */
#ifdef	CFG_SDMR
	im->im_sdma.sdma_sdmr = CFG_SDMR;
#else
	im->im_sdma.sdma_sdmr = 0x00;
#endif

#if defined(CONFIG_8xx_CONS_SMC1)
	/* Use Port B for SMC1 instead of other functions.
	*/
	cp->cp_pbpar |=  0x000000c0;
	cp->cp_pbdir &= ~0x000000c0;
	cp->cp_pbodr &= ~0x000000c0;
#else	/* CONFIG_8xx_CONS_SMC2 */
# if defined(CONFIG_MPC823) || defined(CONFIG_MPC850)
	/* Use Port A for SMC2 instead of other functions.
	*/
	ip->iop_papar |=  0x00c0;
	ip->iop_padir &= ~0x00c0;
	ip->iop_paodr &= ~0x00c0;
# else	/* must be a 860 then */
	/* Use Port B for SMC2 instead of other functions.
	*/
	cp->cp_pbpar |=  0x00000c00;
	cp->cp_pbdir &= ~0x00000c00;
	cp->cp_pbodr &= ~0x00000c00;
# endif
#endif

#if defined(CONFIG_FADS)
	/* Enable RS232 */
#if defined(CONFIG_8xx_CONS_SMC1)
	*((uint *) BCSR1) &= ~BCSR1_RS232EN_1;
#else
	*((uint *) BCSR1) &= ~BCSR1_RS232EN_2;
#endif
#endif	/* CONFIG_FADS */

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

	dpaddr = CPM_DATAONLY_BASE;
	dpsize = CPM_DATAONLY_SIZE;

#if defined(CONFIG_MBX)
	board_serial_init();
#endif	/* CONFIG_MBX */

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

	rbdf = (cbd_t *)&cp->cp_dpmem[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram.
	*/
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr+sizeof(cbd_t);
	up->smc_rfcr = SMC_EB;
	up->smc_tfcr = SMC_EB;

	/* Updating dpram address and size
	*/
	size = ((sizeof(cbd_t)*2 + 2) + 15) & ~15;
	dpaddr += size;
	dpsize -= size;

	/* Initialize CPM
	*/
	m8xx_cpm_init(dpaddr, dpsize);

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	sp->smc_smcmr = smcr_mk_clen(9) |  SMCMR_SM_UART;

	/* Mask all interrupts and remove anything pending.
	*/
	sp->smc_smcm = 0;
	sp->smc_smce = 0xff;

	/* Set up the baud rate generator.
	*/
	serial_setbrg (cpu_clock, baudrate);

	/* Make the first buffer the only buffer.
	*/
	tbdf->cbd_sc |= BD_SC_WRAP;
	rbdf->cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* Single character receive.
	*/
	up->smc_mrblr = 1;
	up->smc_maxidl = 0;

	/* Initialize Tx/Rx parameters.
	*/

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_SMC, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.
	*/
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;
}

void
serial_setbrg (ulong cpu_clock, int baudrate)
{
        volatile immap_t *im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);

	/* Set up the baud rate generator.
	 * See 8xx_io/commproc.c for details.
	 *
	 * Wire BRG1 to SMC1 and BRG2 to SMC2.
	 */

	cp->cp_simode = 0x10000000;

#if defined(CONFIG_8xx_CONS_SMC1)
	cp->cp_brgc1 =			/* Console on SMC1 */
#else
	cp->cp_brgc2 =			/* Console on SMC2 */
#endif
		((((cpu_clock / 16) / baudrate)-1) << 1) | CPM_BRG_EN;
}

void
serial_putc(const char c)
{
	volatile cbd_t		*tbdf;
	volatile char		*buf;
	volatile smc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	if (c == '\n')
		serial_putc ('\r');

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	tbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_tbase];

	/* Wait for last character to go.
	*/

	buf = (char *)tbdf->cbd_bufaddr;
#if 0
	__asm__("eieio");
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__("eieio");
#endif

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
	__asm__("eieio");
#if 1
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__("eieio");
#endif
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int
serial_getc(void)
{
	volatile cbd_t		*rbdf;
	volatile unsigned char	*buf;
	volatile smc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	unsigned char		c;

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];

	/* Wait for character to show up.
	*/
	buf = (unsigned char *)rbdf->cbd_bufaddr;
	while (rbdf->cbd_sc & BD_SC_EMPTY)
		;
	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return(c);
}

int
serial_tstc()
{
	volatile cbd_t		*rbdf;
	volatile smc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];

	return(!(rbdf->cbd_sc & BD_SC_EMPTY));
}

#else	/* ! CONFIG_8xx_CONS_SMC1, CONFIG_8xx_CONS_SMC2 */

void
serial_init (ulong cpu_clock, int baudrate)
{
        volatile immap_t *im = (immap_t *)CFG_IMMR;
	volatile scc_t *sp;
	volatile scc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	uint	 dpaddr;
	volatile iop8xx_t *ip = (iop8xx_t *)&(im->im_ioport);

	/* initialize pointers to SCC */

	sp = (scc_t *) &(cp->cp_scc[SCC_INDEX]);
	up = (scc_uart_t *) &cp->cp_dparam[PROFF_SCC];

	/* Disable transmitter/receiver.
	*/
	sp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	/* Enable SDMA.
	*/
	im->im_siu_conf.sc_sdcr = 1;

	ip->iop_papar |=  ((3 << (2 * SCC_INDEX)));
	ip->iop_padir &= ~((3 << (2 * SCC_INDEX)));
	ip->iop_paodr &= ~((3 << (2 * SCC_INDEX)));

	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	dpaddr = 0x800;

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

	rbdf = (cbd_t *)&cp->cp_dpmem[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram.
	*/
	up->scc_genscc.scc_rbase = dpaddr;
	up->scc_genscc.scc_tbase = dpaddr+sizeof(cbd_t);
	up->scc_genscc.scc_rfcr  = SCC_EB;
	up->scc_genscc.scc_tfcr  = SCC_EB;


	/* Set SCC(x) clock mode to 16x
	 * Set up the baud rate generator.
	 * See 8xx_io/commproc.c for details.
	 *
	 * Wire BRG1 to SCC
	 */

	/* Set up the baud rate generator.
	*/

	sp->scc_gsmrl |= (SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16);
	cp->cp_simode = SICR_UART_CLKRT;
	cp->cp_brgc1 =
		((((cpu_clock / 16) / baudrate)-1) << 1) | CPM_BRG_EN;

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	sp->scc_gsmrl |= SCC_GSMRL_MODE_UART;
	sp->scc_pmsr  |= SCU_PMSR_CL;

	/* Mask all interrupts and remove anything pending.
	*/
	sp->scc_sccm = 0;
	sp->scc_scce = 0xffff;

	/* Make the first buffer the only buffer.
	*/
	tbdf->cbd_sc |= BD_SC_WRAP;
	rbdf->cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* Single character receive.
	*/
	up->scc_genscc.scc_mrblr  = 1;
	up->scc_maxidl = 0;

	/* Initialize Tx/Rx parameters.
	*/

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_SCC, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.
	*/
	sp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);
}


void
serial_putc(const char c)
{
	volatile cbd_t		*tbdf;
	volatile char		*buf;
	volatile scc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	if (c == '\n')
		serial_putc ('\r');

	up = (scc_uart_t *)&cpmp->cp_dparam[PROFF_SCC];

	tbdf = (cbd_t *)&cpmp->cp_dpmem[up->scc_genscc.scc_tbase];

	/* Wait for last character to go.
	*/

	buf = (char *)tbdf->cbd_bufaddr;
#if 0
	__asm__("eieio");
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__("eieio");
#endif

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
	__asm__("eieio");
#if 1
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__("eieio");
#endif
}

void
serial_putstr (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int
serial_getc(void)
{
	volatile cbd_t		*rbdf;
	volatile unsigned char	*buf;
	volatile scc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	unsigned char		c;

	up = (scc_uart_t *)&cpmp->cp_dparam[PROFF_SCC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->scc_genscc.scc_rbase];

	/* Wait for character to show up.
	*/
	buf = (unsigned char *)rbdf->cbd_bufaddr;
	while (rbdf->cbd_sc & BD_SC_EMPTY)
		;
	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return(c);
}

int
serial_tstc()
{
	volatile cbd_t		*rbdf;
	volatile scc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	up = (scc_uart_t *)&cpmp->cp_dparam[PROFF_SCC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->scc_genscc.scc_rbase];

	return(!(rbdf->cbd_sc & BD_SC_EMPTY));
}

#endif	/* CONFIG_8xx_CONS_SMC1, CONFIG_8xx_CONS_SMC2 */


#if (CONFIG_COMMANDS & CFG_CMD_KGDB)

void
kgdb_serial_init(void)
{
#ifdef	CONFIG_8xx_CONS_SMC1
	serial_printf("[on SMC1] ");
#endif
#ifdef	CONFIG_8xx_CONS_SMC2
	serial_printf("[on SMC2] ");
#endif
#ifdef	CONFIG_8xx_CONS_SCC3
	printf("[on SCC3] ");
#endif
}

void
putDebugChar (int c)
{
	serial_putc (c);
}

void
putDebugStr (const char *str)
{
	serial_puts (str);
}

int
getDebugChar (void)
{
	return serial_getc();
}

void
kgdb_interruptible (int yes)
{
	return;
}
#endif	/* CFG_CMD_KGDB	*/

#endif	/* CONFIG_8xx_CONS_NONE */
