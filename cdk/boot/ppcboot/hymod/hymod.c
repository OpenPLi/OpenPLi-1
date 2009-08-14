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
 * Hacked for the Hymod board by Murray.Jensen@cmst.csiro.au, 20-Oct-00
 */

#include <ppcboot.h>
#include <mpc8260.h>
#include <ioports.h>

/* ------------------------------------------------------------------------- */

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	      conf ppar psor pdir podr pdat */
	/* PA31 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII COL */
	/* PA30 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII CRS */
	/* PA29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_ER */
	/* PA28 */ {   1,   1,   1,   1,   0,   0   }, /* FCC1 MII TX_EN */
	/* PA27 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_DV */
	/* PA26 */ {   1,   1,   1,   0,   0,   0   }, /* FCC1 MII RX_ER */
	/* PA25 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII MDIO */
	/* PA24 */ {   1,   0,   0,   1,   0,   0   }, /* FCC2 MII MDC */
	/* PA23 */ {   1,   0,   0,   1,   0,   0   }, /* FCC3 MII MDIO */
	/* PA22 */ {   1,   0,   0,   1,   0,   0   }, /* FCC3 MII MDC */
	/* PA21 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[3] */
	/* PA20 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[2] */
	/* PA19 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[1] */
	/* PA18 */ {   1,   1,   0,   1,   0,   0   }, /* FCC1 MII TxD[0] */
	/* PA17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[3] */
	/* PA16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[2] */
	/* PA15 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[1] */
	/* PA14 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RxD[0] */
	/* PA13 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII MDIO */
	/* PA12 */ {   1,   0,   0,   1,   0,   0   }, /* FCC1 MII MDC */
	/* PA11 */ {   1,   0,   0,   0,   0,   0   }, /* SEL_CD */
	/* PA10 */ {   1,   0,   0,   0,   0,   0   }, /* FLASH STS1 */
	/* PA9  */ {   1,   0,   0,   0,   0,   0   }, /* FLASH STS0 */
	/* PA8  */ {   1,   0,   0,   0,   0,   0   }, /* FLASH ~PE */
	/* PA7  */ {   1,   0,   0,   0,   0,   0   }, /* WATCH ~HRESET */
	/* PA6  */ {   1,   0,   0,   0,   0,   0   }, /* VC DONE */
	/* PA5  */ {   1,   0,   0,   0,   0,   0   }, /* VC INIT */
	/* PA4  */ {   1,   0,   0,   0,   0,   0   }, /* VC ~PROG */
	/* PA3  */ {   1,   0,   0,   0,   0,   0   }, /* VM ENABLE */
	/* PA2  */ {   1,   0,   0,   0,   0,   0   }, /* VM DONE */
	/* PA1  */ {   1,   0,   0,   0,   0,   0   }, /* VM INIT */
	/* PA0  */ {   1,   0,   0,   0,   0,   0   }  /* VM ~PROG */
    },

    /* Port B configuration */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PB31 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TX_ER */
	/* PB30 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_DV */
	/* PB29 */ {   1,   1,   1,   1,   0,   0   }, /* FCC2 MII TX_EN */
	/* PB28 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_ER */
	/* PB27 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII COL */
	/* PB26 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII CRS */
	/* PB25 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[3] */
	/* PB24 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[2] */
	/* PB23 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[1] */
	/* PB22 */ {   1,   1,   0,   1,   0,   0   }, /* FCC2 MII TxD[0] */
	/* PB21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[0] */
	/* PB20 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[1] */
	/* PB19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[2] */
	/* PB18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RxD[3] */
	/* PB17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_DV */
	/* PB16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_ER */
	/* PB15 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TX_ER */
	/* PB14 */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TX_EN */
	/* PB13 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII COL */
	/* PB12 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII CRS */
	/* PB11 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[3] */
	/* PB10 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[2] */
	/* PB9  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[1] */
	/* PB8  */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RxD[0] */
	/* PB7  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[3] */
	/* PB6  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[2] */
	/* PB5  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[1] */
	/* PB4  */ {   1,   1,   0,   1,   0,   0   }, /* FCC3 MII TxD[0] */
	/* PB3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PB0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PC31 */ {   1,   0,   0,   0,   0,   0   }, /* MEZ ~IACK */
	/* PC30 */ {   0,   0,   0,   0,   0,   0   },
	/* PC29 */ {   1,   1,   0,   0,   0,   0   }, /* CLK SCCx */
	/* PC28 */ {   1,   1,   0,   0,   0,   0   }, /* CLK4 */
	/* PC27 */ {   1,   1,   0,   0,   0,   0   }, /* CLK SCCF */
	/* PC26 */ {   1,   1,   0,   0,   0,   0   }, /* CLK 32K */
	/* PC25 */ {   1,   1,   0,   0,   0,   0   }, /* BRG4/CLK7 */
	/* PC24 */ {   0,   0,   0,   0,   0,   0   },
	/* PC23 */ {   1,   1,   0,   0,   0,   0   }, /* CLK SCCx */
	/* PC22 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII RX_CLK */
	/* PC21 */ {   1,   1,   0,   0,   0,   0   }, /* FCC1 MII TX_CLK */
	/* PC20 */ {   1,   1,   0,   0,   0,   0   }, /* CLK SCCF */
	/* PC19 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII RX_CLK */
	/* PC18 */ {   1,   1,   0,   0,   0,   0   }, /* FCC2 MII TX_CLK */
	/* PC17 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII RX_CLK */
	/* PC16 */ {   1,   1,   0,   0,   0,   0   }, /* FCC3 MII TX_CLK */
	/* PC15 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART ~CTS */
	/* PC14 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART ~CD */
	/* PC13 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 UART ~CTS */
	/* PC12 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 UART ~CD */
	/* PC11 */ {   1,   0,   0,   1,   0,   0   }, /* SCC1 UART ~DTR */
	/* PC10 */ {   1,   0,   0,   1,   0,   0   }, /* SCC1 UART ~DSR */
	/* PC9  */ {   1,   0,   0,   1,   0,   0   }, /* SCC2 UART ~DTR */
	/* PC8  */ {   1,   0,   0,   1,   0,   0   }, /* SCC2 UART ~DSR */
	/* PC7  */ {   1,   0,   0,   0,   0,   0   }, /* TEMP ~ALERT */
	/* PC6  */ {   1,   0,   0,   0,   0,   0   }, /* FCC3 INT */
	/* PC5  */ {   1,   0,   0,   0,   0,   0   }, /* FCC2 INT */
	/* PC4  */ {   1,   0,   0,   0,   0,   0   }, /* FCC1 INT */
	/* PC3  */ {   1,   1,   1,   1,   0,   0   }, /* SDMA IDMA2 ~DACK */
	/* PC2  */ {   1,   1,   1,   0,   0,   0   }, /* SDMA IDMA2 ~DONE */
	/* PC1  */ {   1,   1,   0,   0,   0,   0   }, /* SDMA IDMA2 ~DREQ */
	/* PC0  */ {   1,   1,   0,   1,   0,   0   }  /* BRG7 */
    },

    /* Port D */
    {   /*	      conf ppar psor pdir podr pdat */
	/* PD31 */ {   1,   1,   0,   0,   0,   0   }, /* SCC1 UART RxD */
	/* PD30 */ {   1,   1,   1,   1,   0,   0   }, /* SCC1 UART TxD */
	/* PD29 */ {   1,   1,   0,   1,   0,   0   }, /* SCC1 UART ~RTS */
	/* PD28 */ {   1,   1,   0,   0,   0,   0   }, /* SCC2 UART RxD */
	/* PD27 */ {   1,   1,   0,   1,   0,   0   }, /* SCC2 UART TxD */
	/* PD26 */ {   1,   1,   0,   1,   0,   0   }, /* SCC2 UART ~RTS */
	/* PD25 */ {   1,   0,   0,   0,   0,   0   }, /* SCC1 UART ~RI */
	/* PD24 */ {   1,   0,   0,   0,   0,   0   }, /* SCC2 UART ~RI */
	/* PD23 */ {   1,   0,   0,   1,   0,   0   }, /* CLKGEN PD */
	/* PD22 */ {   1,   0,   0,   0,   0,   0   }, /* USER3 */
	/* PD21 */ {   1,   0,   0,   0,   0,   0   }, /* USER2 */
	/* PD20 */ {   1,   0,   0,   0,   0,   0   }, /* USER1 */
	/* PD19 */ {   1,   1,   1,   0,   0,   0   }, /* SPI ~SEL */
	/* PD18 */ {   1,   1,   1,   0,   0,   0   }, /* SPI CLK */
	/* PD17 */ {   1,   1,   1,   0,   0,   0   }, /* SPI MOSI */
	/* PD16 */ {   1,   1,   1,   0,   0,   0   }, /* SPI MISO */
	/* PD15 */ {   1,   1,   1,   0,   0,   0   }, /* I2C SDA */
	/* PD14 */ {   1,   1,   1,   0,   0,   0   }, /* I2C SCL */
	/* PD13 */ {   1,   0,   0,   1,   0,   1   }, /* TEMP ~STDBY */
	/* PD12 */ {   1,   0,   0,   1,   0,   1   }, /* FCC3 ~RESET */
	/* PD11 */ {   1,   0,   0,   1,   0,   1   }, /* FCC2 ~RESET */
	/* PD10 */ {   1,   0,   0,   1,   0,   1   }, /* FCC1 ~RESET */
	/* PD9  */ {   1,   0,   0,   0,   0,   0   }, /* PD9 */
	/* PD8  */ {   1,   0,   0,   0,   0,   0   }, /* PD8 */
	/* PD7  */ {   1,   0,   0,   0,   0,   0   }, /* PD7 */
	/* PD6  */ {   1,   0,   0,   0,   0,   0   }, /* PD6 */
	/* PD5  */ {   1,   0,   0,   0,   0,   0   }, /* PD5 */
	/* PD4  */ {   1,   0,   0,   0,   0,   0   }, /* PD4 */
	/* PD3  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD2  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD1  */ {   0,   0,   0,   0,   0,   0   }, /* pin doesn't exist */
	/* PD0  */ {   0,   0,   0,   0,   0,   0   }  /* pin doesn't exist */
    }
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity: Hardwired to HYMOD
 */

int
checkboard(void)
{
    printf("HYMOD\n");

    return (0);
}

/* ------------------------------------------------------------------------- */

long
initdram(int board_type)
{
    volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
    volatile memctl8260_t *memctl = &immap->im_memctl;
    volatile uchar *ramaddr = (uchar *)(CFG_SDRAM_BASE + 0x8);
    ulong psdmr = CFG_PSDMR;
    int i;

    /*
     * Quote from 8260 UM (10.4.2 SDRAM Power-On Initialization, 10-35):
     *
     * "At system reset, initialization software must set up the
     *  programmable parameters in the memory controller banks registers
     *  (ORx, BRx, P/LSDMR). After all memory parameters are conÞgured,
     *  system software should execute the following initialization sequence
     *  for each SDRAM device.
     *
     *  1. Issue a PRECHARGE-ALL-BANKS command
     *  2. Issue eight CBR REFRESH commands
     *  3. Issue a MODE-SET command to initialize the mode register
     *
     *  The initial commands are executed by setting P/LSDMR[OP] and
     *  accessing the SDRAM with a single-byte transaction."
     *
     * The appropriate BRx/ORx registers have already been set when we
     * get here. The SDRAM can be accessed at the address CFG_SDRAM_BASE.
     */

    memctl->memc_psrt = CFG_PSRT;
    memctl->memc_mptpr = CFG_MPTPR;

    memctl->memc_psdmr = psdmr | PSDMR_OP_PREA; *ramaddr = 0;

    for (i = 0; i < 8; i++) {
	memctl->memc_psdmr = psdmr | PSDMR_OP_CBRR; *ramaddr = 0;
    }

    memctl->memc_psdmr = psdmr | PSDMR_OP_MRW; *ramaddr = 0;

    memctl->memc_psdmr = psdmr | PSDMR_OP_NORM; *ramaddr = 0;

    return (CFG_SDRAM_SIZE << 20);
}
