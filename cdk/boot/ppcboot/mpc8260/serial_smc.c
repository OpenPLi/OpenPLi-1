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
 *
 * Hacked for MPC8260 by Murray.Jensen@cmst.csiro.au, 19-Oct-00, with
 * changes based on the file arch/ppc/mbxboot/m8260_tty.c from the
 * Linux/PPC sources (m8260_tty.c had no copyright info in it).
 */

/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

#include <ppcboot.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>

#if defined(CONFIG_CONS_ON_SMC)

#if CONFIG_CONS_INDEX == 1	/* Console on SMC1 */

#define SMC_INDEX		0
#define PROFF_SMC_BASE		PROFF_SMC1_BASE
#define PROFF_SMC		PROFF_SMC1
#define CPM_CR_SMC_PAGE		CPM_CR_SMC1_PAGE
#define CPM_CR_SMC_SBLOCK	CPM_CR_SMC1_SBLOCK
#define CMXSMR_MASK		(CMXSMR_SMC1|CMXSMR_SMC1CS_MSK)
#define CMXSMR_VALUE		CMXSMR_SMC1CS_BRG1

#elif CONFIG_CONS_INDEX == 2	/* Console on SMC2 */

#define SMC_INDEX		1
#define PROFF_SMC_BASE		PROFF_SMC2_BASE
#define PROFF_SMC		PROFF_SMC2
#define CPM_CR_SMC_PAGE		CPM_CR_SMC2_PAGE
#define CPM_CR_SMC_SBLOCK	CPM_CR_SMC2_SBLOCK
#define CMXSMR_MASK		(CMXSMR_SMC2|CMXSMR_SMC2CS_MSK)
#define CMXSMR_VALUE		CMXSMR_SMC2CS_BRG2

#else

#error "console not correctly defined"

#endif

void
serial_init (ulong cpu_clock, int baudrate)
{
        volatile immap_t *im = (immap_t *)CFG_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile iop8260_t *io = &im->im_ioport;
	volatile cpm8260_t *cp = &(im->im_cpm);
	uint	dpaddr;

	/* initialize pointers to SMC */

	sp = (smc_t *) &(im->im_smc[SMC_INDEX]);
	*(ushort *)(&im->im_dprambase[PROFF_SMC_BASE]) = PROFF_SMC;
	up = (smc_uart_t *)&im->im_dprambase[PROFF_SMC];

	/* Disable transmitter/receiver.
	*/
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

#if CONFIG_CONS_INDEX == 1
	/* Use Port D (PD8 & PD9) for SMC1 instead of other functions.
	*/
	io->iop_ppard |= 0x00c00000;
	io->iop_pdird |= 0x00400000;
	io->iop_pdird &= ~0x00800000;
	io->iop_psord &= ~0x00c00000;
#else
	/* Use Port A (PA8 & PA9) for SMC2 instead of other functions.
	*/
	io->iop_ppard |= 0x00c00000;
	io->iop_pdird |= 0x00400000;
	io->iop_pdird &= ~0x00800000;
	io->iop_psord &= ~0x00c00000;
#endif

	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	dpaddr = 0x800;

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rbdf = (cbd_t *)&im->im_dprambase[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram.
	*/
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr+sizeof(cbd_t);
	up->smc_rfcr = CPMFCR_EB;
	up->smc_tfcr = CPMFCR_EB;
	up->smc_brklen = 0;
	up->smc_brkec = 0;
	up->smc_brkcr = 0;

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

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_SMC_PAGE, CPM_CR_SMC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;

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
	init_data_t *idata =
		(init_data_t *)(CFG_INIT_RAM_ADDR+CFG_INIT_DATA_OFFSET);
	ulong i;

	/* put the SMC channel into NMSI (non multiplexd serial interface)
	 * mode and wire either BRG1 to SMC1 or BRG2 to SMC2 (15-17).
	 */
	im->im_cpmux.cmx_smr = (im->im_cpmux.cmx_smr&~CMXSMR_MASK)|CMXSMR_VALUE;

	/* configure the appropriate baud rate generator (16-2).
	 * the uart must be configured for 16x oversampling (hence the
	 * divide by 16 below). I also round all divisions, rather than
	 * truncate - hopefully that will provide a slightly more accurate
	 * clock divider.
	 */
	i = (idata->brg_clk + 15) / 16;

	i = (i + baudrate - 1) / baudrate;

	i = (((i - 1) & 0xfff) << 1) | CPM_BRG_EN;

#if CONFIG_CONS_INDEX == 1
	im->im_brgc1 = i;	/* Console on SMC1 */
#else
	im->im_brgc2 = i;	/* Console on SMC2 */
#endif
}

void
serial_putc(const char c)
{
	volatile cbd_t		*tbdf;
	volatile char		*buf;
	volatile smc_uart_t	*up;
        volatile immap_t	*im = (immap_t *)CFG_IMMR;

	if (c == '\n')
		serial_putc ('\r');

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);

	tbdf = (cbd_t *)&im->im_dprambase[up->smc_tbase];

	/* Wait for last character to go.
	*/
	buf = (char *)tbdf->cbd_bufaddr;
	while (tbdf->cbd_sc & BD_SC_READY)
		;

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
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
	unsigned char		c;

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);

	rbdf = (cbd_t *)&im->im_dprambase[up->smc_rbase];

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

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);

	rbdf = (cbd_t *)&im->im_dprambase[up->smc_rbase];

	return(!(rbdf->cbd_sc & BD_SC_EMPTY));
}

#endif	/* CONFIG_CONS_ON_SMC */
