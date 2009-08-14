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
 ********************************************************************
 *
 * Lots of code copied from:
 *
 * m8xx_pcmcia.c - Linux PCMCIA socket driver for the mpc8xx series.
 * (C) 1999-2000 Magnus Damm <damm@bitsmart.com>
 *
 * "The ExCA standard specifies that socket controllers should provide
 * two IO and five memory windows per socket, which can be independently
 * configured and positioned in the host address space and mapped to
 * arbitrary segments of card address space. " - David A Hinds. 1999
 *
 * This controller does _not_ meet the ExCA standard.
 *
 * m8xx pcmcia controller brief info:
 * + 8 windows (attrib, mem, i/o)
 * + up to two slots (SLOT_A and SLOT_B)
 * + inputpins, outputpins, event and mask registers.
 * - no offset register. sigh.
 *
 * Because of the lacking offset register we must map the whole card.
 * We assign each memory window PCMCIA_MEM_WIN_SIZE address space.
 * Make sure there is (PCMCIA_MEM_WIN_SIZE * PCMCIA_MEM_WIN_NO
 * * PCMCIA_SOCKETS_NO) bytes at PCMCIA_MEM_WIN_BASE.
 * The i/o windows are dynamically allocated at PCMCIA_IO_WIN_BASE.
 * They are maximum 64KByte each...
 */

/*
 * PCMCIA support
 */
#include <mpc8xx_irq.h>
#include <ppcboot.h>
#include <command.h>
#include <cmd_pcmcia.h>
#include <mpc8xx.h>


static int hardware_enable (int slot);
static int hardware_disable(int slot);
static int voltage_set(int slot, int vcc, int vpp);

static u_int m8xx_get_graycode(u_int size);
#if 0
static u_int m8xx_get_speed(u_int ns, u_int is_io);
#endif

/* ------------------------------------------------------------------------- */
/* Autoconfigure boards if no settings are defined                           */
#if defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L) || defined(CONFIG_TQM860L)
#define	CONFIG_TQM8xxL
#endif

#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)

/* The RPX series use SLOT_B */
#if defined(CONFIG_RPXCLASSIC) || defined(CONFIG_RPXLITE)
# define CONFIG_PCMCIA_SLOT_B
#endif

/* The ADS board use SLOT_A */
#ifdef CONFIG_ADS
# define CONFIG_PCMCIA_SLOT_A
#endif

/* The FADS series are a mess */
#ifdef CONFIG_FADS
# if defined(CONFIG_MPC860T) || defined(CONFIG_MPC860) || defined(CONFIG_MPC821)
#  define CONFIG_PCMCIA_SLOT_A
# else
#  define CONFIG_PCMCIA_SLOT_B
# endif
#endif

/* The TQM8xxL modules use SLOT_A on MPC860, SLOT_B else */
#ifdef	CONFIG_TQM8xxL
# if defined(CONFIG_MPC860T) || defined(CONFIG_MPC860)
#  define	CONFIG_PCMCIA_SLOT_A
# else	/* ! 860, 860T */
#  define	CONFIG_PCMCIA_SLOT_B
# endif	/* 860, 860T */
#endif	/* CONFIG_TQM8xxL */

#endif /* !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B) */

/* Make sure exactly one slot is defined - we support only one for now */
#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)
#error Neither CONFIG_PCMCIA_SLOT_A nor CONFIG_PCMCIA_SLOT_B configured
#endif
#if defined(CONFIG_PCMCIA_SLOT_A) && defined(CONFIG_PCMCIA_SLOT_B)
#error Both CONFIG_PCMCIA_SLOT_A and CONFIG_PCMCIA_SLOT_B configured
#endif

#define PCMCIA_SOCKETS_NO	1
#define PCMCIA_MEM_WIN_NO	4
#define PCMCIA_IO_WIN_NO	2

/* define _slot_ to be able to optimize macros */
#ifdef CONFIG_PCMCIA_SLOT_A
# define _slot_			0
# define PCMCIA_SLOT_MSG	"SLOT_A"
#else
# define _slot_			1
# define PCMCIA_SLOT_MSG	"SLOT_B"
#endif

/*
 * The TQM850L hardware has two pins swapped! Grrrrgh!
 */
#ifdef	CONFIG_TQM850L
#define __MY_PCMCIA_GCRX_CXRESET	PCMCIA_GCRX_CXOE
#define __MY_PCMCIA_GCRX_CXOE		PCMCIA_GCRX_CXRESET
#else
#define __MY_PCMCIA_GCRX_CXRESET	PCMCIA_GCRX_CXRESET
#define __MY_PCMCIA_GCRX_CXOE		PCMCIA_GCRX_CXOE
#endif

/* look up table for pgcrx registers */

static u_int *pcmcia_pgcrx[2] = {
	&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pgcra,
	&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pgcrb,
};

#define PCMCIA_PGCRX(slot)	(*pcmcia_pgcrx[slot])

/*
 * This structure is used to address each window in the PCMCIA controller.
 *
 * Keep in mind that we assume that pcmcia_win_t[n+1] is mapped directly
 * after pcmcia_win_t[n]...
 */

typedef struct {
	uint	br;
	uint	or;
} pcmcia_win_t;



/* ------------------------------------------------------------------------- */

void do_pinit (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int i;
	u_long reg, base;
	pcmcia_win_t *win;

	if (argc != 2) {
		printf ("Usage: pinit {on | off}\n");
		return;
	}
	if (strcmp(argv[1],"on") == 0) {
		printf ("Enable PCMCIA " PCMCIA_SLOT_MSG "\n");

		/* intialize the fixed memory windows */
		win = (pcmcia_win_t *)(&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pbr0);
		base = CFG_PCMCIA_MEM_ADDR;

		if((reg = m8xx_get_graycode(CFG_PCMCIA_MEM_SIZE)) == -1) {
			printf ("Cannot set window size to 0x%08x\n",
				CFG_PCMCIA_MEM_SIZE);
			return;
		}
#if 0
XXX XXX XXX
if(mem->flags & MAP_ATTRIB)
	reg |= 0x00000010;

if(mem->flags & MAP_WRPROT)
	reg |= 0x00000002;

if(mem->flags & MAP_16BIT)
	reg |= 0x00000040;

if(mem->flags & MAP_ACTIVE)
	reg |= 0x00000001;
XXX XXX XXX
#endif

		for (i=0; i<PCMCIA_MEM_WIN_NO; ++i) {
			win->br = base;
			win->or = 0;	/* set to not valid */
printf ("MemWin %d: Base 0x%08lX\n", i, base);
			base += CFG_PCMCIA_MEM_SIZE;
			++win;
		}

		/* turn off voltage */
		voltage_set(_slot_, 0, 0);

		/* Enable external hardware */
		hardware_enable(_slot_);

	} else if (strcmp(argv[1],"off") == 0) {
		printf ("Disable PCMCIA " PCMCIA_SLOT_MSG "\n");

		/* clear interrupt state, and disable interrupts */
		((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pscr =  PCMCIA_MASK(_slot_);
		((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_per &= ~PCMCIA_MASK(_slot_);

		/* turn off interrupt and disable CxOE */
		PCMCIA_PGCRX(_slot_) = __MY_PCMCIA_GCRX_CXOE;

		/* turn off memory windows */
		win = (pcmcia_win_t *)(&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pbr0);

		for (i=0; i<PCMCIA_MEM_WIN_NO; ++i) {
			/* disable memory window */
			win->or = 0;
			++win;
		}

		/* turn off voltage */
		voltage_set(_slot_, 0, 0);

		/* disable external hardware */
		printf ("Shutdown and Poweroff " PCMCIA_SLOT_MSG "\n");
		hardware_disable(_slot_);
	} else {
		printf ("Usage: pinit {on | off}\n");
		return;
	}

	return;
}

/* ---------------------------------------------------------------------------- */
/* board specific stuff:							*/
/* voltage_set(), hardware_enable() and hardware_disable()			*/
/* ---------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------- */
/* RPX Boards from Embedded Planet						*/
/* ---------------------------------------------------------------------------- */

#if defined(CONFIG_RPXCLASSIC) || defined(CONFIG_RPXLITE)

/* The RPX boards seems to have it's bus monitor timeout set to 6*8 clocks.
 * SYPCR is write once only, therefore must the slowest memory be faster
 * than the bus monitor or we will get a machine check due to the bus timeout.
 */

#define PCMCIA_BOARD_MSG "RPX CLASSIC or RPX LITE"

#undef PCMCIA_BMT_LIMIT
#define PCMCIA_BMT_LIMIT (6*8)

static int voltage_set(int slot, int vcc, int vpp)
{
	u_long reg = 0;

	switch(vcc) {
	case 0: break;
	case 33: reg |= BCSR1_PCVCTL4; break;
	case 50: reg |= BCSR1_PCVCTL5; break;
	default: return 1;
	}

	switch(vpp) {
	case 0: break;
	case 33:
	case 50:
		if(vcc == vpp)
			reg |= BCSR1_PCVCTL6;
		else
			return 1;
		break;
	case 120:
		reg |= BCSR1_PCVCTL7;
	default: return 1;
	}

	if(vcc == 120)
	   return 1;

	/* first, turn off all power */

	*((uint *)RPX_CSR_ADDR) &= ~(BCSR1_PCVCTL4 | BCSR1_PCVCTL5
				     | BCSR1_PCVCTL6 | BCSR1_PCVCTL7);

	/* enable new powersettings */

	*((uint *)RPX_CSR_ADDR) |= reg;

	return 0;
}

#define socket_get(_slot_) PCMCIA_SOCKET_KEY_5V
static int hardware_enable (int slot)
{
	return 0;	/* No hardware to enable */
}
static int hardware_disable(int slot)
{
	return 0;	/* No hardware to disable */
}
#endif /* CONFIG_RPXCLASSIC */

/* ---------------------------------------------------------------------------- */
/* (F)ADS Boards from Motorola							*/
/* ---------------------------------------------------------------------------- */

#if defined(CONFIG_ADS) || defined(CONFIG_FADS)

#ifdef CONFIG_ADS
#define PCMCIA_BOARD_MSG "ADS"
#define PCMCIA_GLITCHY_CD  /* My ADS board needs this */
#else
#define PCMCIA_BOARD_MSG "FADS"
#endif

static int voltage_set(int slot, int vcc, int vpp)
{
	u_long reg = 0;

	switch(vpp) {
	case 0: reg = 0; break;
	case 50: reg = 1; break;
	case 120: reg = 2; break;
	default: return 1;
	}

	switch(vcc) {
	case 0: reg = 0; break;
#ifdef CONFIG_ADS
	case 50: reg = BCSR1_PCCVCCON; break;
#endif
#ifdef CONFIG_FADS
	case 33: reg = BCSR1_PCCVCC0 | BCSR1_PCCVCC1; break;
	case 50: reg = BCSR1_PCCVCC1; break;
#endif
	default: return 1;
	}

	/* first, turn off all power */

#ifdef CONFIG_ADS
	*((uint *)BCSR1) |= BCSR1_PCCVCCON;
#endif
#ifdef CONFIG_FADS
	*((uint *)BCSR1) &= ~(BCSR1_PCCVCC0 | BCSR1_PCCVCC1);
#endif
	*((uint *)BCSR1) &= ~BCSR1_PCCVPP_MASK;

	/* enable new powersettings */

#ifdef CONFIG_ADS
	*((uint *)BCSR1) &= ~reg;
#endif
#ifdef CONFIG_FADS
	*((uint *)BCSR1) |= reg;
#endif

 	*((uint *)BCSR1) |= reg << 20;

	return 0;
}

#define socket_get(_slot_) PCMCIA_SOCKET_KEY_5V

static int hardware_enable(int slot)
{
	*((uint *)BCSR1) &= ~BCSR1_PCCEN;
	return 0;
}

static int hardware_disable(int slot)
{
	*((uint *)BCSR1) &= ~BCSR1_PCCEN;
	return 0;
}

#endif	/* (F)ADS */

/* ---------------------------------------------------------------------------- */
/* TQM8xxL Boards by TQ Components						*/
/* ---------------------------------------------------------------------------- */

#ifdef	CONFIG_TQM8xxL

#define PCMCIA_BOARD_MSG "TQM8xxL"

static int hardware_enable(int slot)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
	volatile pcmconf8xx_t	*pcmp;
	volatile sysconf8xx_t	*sysp;
	uint reg, mask;

printf ("hardware_enable: TQM8xxL Slot %c\n", 'A'+slot);

if (slot != 1) {
 printf ("## Only Slot B supported for now ##\n");
 return (1);
}

	immap = (immap_t *)CFG_IMMR;
	sysp  = (sysconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_siu_conf));
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));

	/*
	 * Configure SIUMCR to enable PCMCIA port B
	 * (VFLS[0:1] are not used for debugging, we connect FRZ# instead)
	 */
	sysp->sc_siumcr &= ~SIUMCR_DBGC11;	/* set DBGC to 00 */

	/* clear interrupt state, and disable interrupts */
	pcmp->pcmc_pscr =  PCMCIA_MASK(_slot_);
	pcmp->pcmc_per &= ~PCMCIA_MASK(_slot_);

	/* disable interrupts & DMA */
	PCMCIA_PGCRX(_slot_) = 0;

	/*
	 * Disable PCMCIA buffers (isolate the interface)
	 * and assert RESET signal
	 */
printf ("Disable PCMCIA buffers and assert RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |=  __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

	/*
	 * Configure Port C pins for
	 * 5 Volts Enable and 3 Volts enable
	 */
	immap->im_ioport.iop_pcpar &= ~(0x0002 | 0x0004);
	immap->im_ioport.iop_pcso  &= ~(0x0002 | 0x0004);
	/* remove all power */
	immap->im_ioport.iop_pcdat &= ~(0x0002 | 0x0004);

	/*
	 * Make sure there is a card in the slot, then configure the interface.
	 */
printf ("[%d] %s: PIPR(%p)=0x%x\n",__LINE__,__FUNCTION__,&(pcmp->pcmc_pipr),pcmp->pcmc_pipr);udelay(10000);
printf ("[%d] %s: PIPR(%p)=0x%x\n",__LINE__,__FUNCTION__,&(pcmp->pcmc_pipr),pcmp->pcmc_pipr);udelay(10000);
printf ("[%d] %s: PIPR(%p)=0x%x\n",__LINE__,__FUNCTION__,&(pcmp->pcmc_pipr),pcmp->pcmc_pipr);udelay(10000);
	if (pcmp->pcmc_pipr & 0x00001800) {
printf ("##### hardware_enable - no card in slot\n");
		return (-1);
	}
printf ("# hardware_enable - card found\n");

	/*
	 * Power On.
	 */
	mask = PCMCIA_VS1(slot) | PCMCIA_VS2(slot);
	reg  = pcmp->pcmc_pipr;
printf ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n", reg,
(reg&PCMCIA_VS1(slot))?"n":"ff", (reg&PCMCIA_VS2(slot))?"n":"ff");
	if ((reg & mask) == mask) {
		immap->im_ioport.iop_pcdat |= 0x0004;
printf ("PCMCIA Power on 5.0 Volt\n");
	} else {
		immap->im_ioport.iop_pcdat |= 0x0002;
printf ("PCMCIA Power on 3.3 Volt\n");
	}
	immap->im_ioport.iop_pcdir |=  (0x0002 | 0x0004);

	/*  VCC switch error flag, PCMCIA slot INPACK_ pin */
	cp->cp_pbdir &= ~(0x0020 | 0x0010);
	cp->cp_pbpar &= ~(0x0020 | 0x0010);
	udelay(500000);

printf ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg |=  __MY_PCMCIA_GCRX_CXRESET;		/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

printf ("# hardware_enable done\n");
	return (0);
}



static int hardware_disable(int slot)
{
	volatile immap_t	*immap;
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;

printf ("hardware_disable: TQM8xxL Slot %c\n", 'A'+slot);

if (slot != 1) {
 printf ("## Only Slot B supported for now ##\n");
 return (1);
}

	immap = (immap_t *)CFG_IMMR;
	pcmp = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));

	/* remove all power */
	immap->im_ioport.iop_pcdat &= ~(0x0002 | 0x0004);

	/* Configure PCMCIA General Control Register */
	PCMCIA_PGCRX(_slot_) = 0;

	/*
	 * Disable PCMCIA buffers (isolate the interface)
	 */
printf ("Disable PCMCIA buffers\n");
	PCMCIA_PGCRX(_slot_) &= ~__MY_PCMCIA_GCRX_CXOE;
	udelay(500);

printf ("Disable PCMCIA buffers and assert RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |=  __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	return (0);
}



static int voltage_set(int slot, int vcc, int vpp)
{
	volatile immap_t	*immap;
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;

printf ("voltage_set: TQM8xxL Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
'A'+slot,vcc/10,vcc%10,vpp/10,vcc%10);

if (slot != 1) {
 printf ("## Only Slot B supported for now ##\n");
 return (1);
}

	immap = (immap_t *)CFG_IMMR;
	pcmp = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	/*
	 * Disable PCMCIA buffers (isolate the interface)
	 * and assert RESET signal
	 */
printf ("Disable PCMCIA buffers and assert RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |=  __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

	/*
	 * Configure Port C pins for
	 * 5 Volts Enable and 3 Volts enable,
	 * Turn off all power
	 */
printf ("PCMCIA power OFF\n");
	immap->im_ioport.iop_pcpar &= ~(0x0002 | 0x0004);
	immap->im_ioport.iop_pcso  &= ~(0x0002 | 0x0004);
	immap->im_ioport.iop_pcdat &= ~(0x0002 | 0x0004);

	reg = 0;
	switch(vcc) {
	case  0: 		break;
	case 33: reg |= 0x0002;	break;
	case 50: reg |= 0x0004;	break;
	default: 		goto done;
	}

	/* Checking supported voltages */

printf ("PIPR: 0x%x --> %s\n", pcmp->pcmc_pipr,
(pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

	immap->im_ioport.iop_pcdat |= reg;
	immap->im_ioport.iop_pcdir |=  (0x0002 | 0x0004);
if (reg) {
printf ("PCMCIA powered at %sV\n",(reg&0x0004)?"5.0":"3.3");
} else {
printf ("PCMCIA powered down\n");
}

done:
printf ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg |=  __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

printf ("voltage_set: TQM8xxL Slot %c, DONE\n", slot+'A');
	return (0);
}

#endif	/* TQM8xxL */


/* ---------------------------------------------------------------------------- */
/* End of Board Specific Stuff							*/
/* ---------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------- */
/* MPC8xx Specific Stuff - should go to MPC8xx directory			*/
/* ---------------------------------------------------------------------------- */

/*
 * Search this table to see if the windowsize is
 * supported...
 */

#define M8XX_SIZES_NO 32

static const u_int m8xx_size_to_gray[M8XX_SIZES_NO] =
{ 0x00000001, 0x00000002, 0x00000008, 0x00000004,
  0x00000080, 0x00000040, 0x00000010, 0x00000020,
  0x00008000, 0x00004000, 0x00001000, 0x00002000,
  0x00000100, 0x00000200, 0x00000800, 0x00000400,

  0x0fffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x01000000, 0x02000000, 0xffffffff, 0x04000000,
  0x00010000, 0x00020000, 0x00080000, 0x00040000,
  0x00800000, 0x00400000, 0x00100000, 0x00200000 };


/* ---------------------------------------------------------------------------- */

static u_int m8xx_get_graycode(u_int size)
{
	u_int k;

	for (k = 0; k < M8XX_SIZES_NO; k++) {
		if(m8xx_size_to_gray[k] == size)
			break;
	}

	if((k == M8XX_SIZES_NO) || (m8xx_size_to_gray[k] == -1))
		k = -1;

	return k;
}

/* ------------------------------------------------------------------------- */

#if 0
static u_int m8xx_get_speed(u_int ns, u_int is_io)
{
	u_int reg, clocks, psst, psl, psht;

	if(!ns) {

		/*
		 * We get called with IO maps setup to 0ns
		 * if not specified by the user.
		 * They should be 255ns.
		 */

		if(is_io)
			ns = 255;
		else
			ns = 100;  /* fast memory if 0 */
	}

	/*
	 * In PSST, PSL, PSHT fields we tell the controller
	 * timing parameters in CLKOUT clock cycles.
	 * CLKOUT is the same as GCLK2_50.
	 */

/* how we want to adjust the timing - in percent */

#define ADJ 180 /* 80 % longer accesstime - to be sure */

	clocks = ((M8XX_BUSFREQ / 1000) * ns) / 1000;
	clocks = (clocks * ADJ) / (100*1000);

	if(clocks >= PCMCIA_BMT_LIMIT) {
		DEBUG(0, "Max access time limit reached\n");
		clocks = PCMCIA_BMT_LIMIT-1;
	}

	psst = clocks / 7;          /* setup time */
	psht = clocks / 7;          /* hold time */
	psl  = (clocks * 5) / 7;    /* strobe length */

	psst += clocks - (psst + psht + psl);

	reg =  psst << 12;
	reg |= psl  << 7;
	reg |= psht << 16;

	return reg;
}
#endif

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

