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

#ifndef	_PCMCIA_H
#define _PCMCIA_H

#include <ppcboot.h>
#include <config.h>

#ifdef CONFIG_IDE_PCMCIA
/*
 * Allow configuration to select PCMCIA slot,
 * or try to generate a useful default
 */
#if !defined(CONFIG_PCMCIA_SLOT_A) && !defined(CONFIG_PCMCIA_SLOT_B)

					/* The RPX series use SLOT_B	*/
#if defined(CONFIG_RPXCLASSIC) || defined(CONFIG_RPXLITE)
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_ADS)		/* The ADS  board use SLOT_A	*/
# define CONFIG_PCMCIA_SLOT_A
#elif defined(CONFIG_FADS)		/* The FADS series are a mess	*/
# if defined(CONFIG_MPC860T) || defined(CONFIG_MPC860) || defined(CONFIG_MPC821)
#  define CONFIG_PCMCIA_SLOT_A
# else
#  define CONFIG_PCMCIA_SLOT_B
# endif
#elif defined(CONFIG_TQM860L) || defined(CONFIG_TQM855L) /* The TQM8xxL modules */
# define	CONFIG_PCMCIA_SLOT_A	/* ... use SLOT_A on MPC860/855	*/
#elif defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L)
# define	CONFIG_PCMCIA_SLOT_B	/* ... and SLOT_B else		*/
#elif defined(CONFIG_SPD823TS)		/* The SPD8xx use SLOT_B	*/
# define CONFIG_PCMCIA_SLOT_B
#elif defined(CONFIG_IVMS8)		/* The IVMS8 use SLOT_A		*/
# define CONFIG_PCMCIA_SLOT_A
#else
# error "PCMCIA Slot not configured"
#endif

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
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_A
#else
# define _slot_			1
# define PCMCIA_SLOT_MSG	"SLOT_B"
# define PCMCIA_SLOT_x		PCMCIA_PSLOT_B
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

#if 0
static u_int *pcmcia_pgcrx[2] = {
	&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pgcra,
	&((immap_t *)CFG_IMMR)->im_pcmcia.pcmc_pgcrb,
};

#define PCMCIA_PGCRX(slot)	(*pcmcia_pgcrx[slot])
#endif

/*
 * This structure is used to address each window in the PCMCIA controller.
 *
 * Keep in mind that we assume that pcmcia_win_t[n+1] is mapped directly
 * after pcmcia_win_t[n]...
 */

typedef struct {
	ulong	br;
	ulong	or;
} pcmcia_win_t;

#endif /* CONFIG_IDE_PCMCIA */

/*
 * Definitions for PCMCIA control registers to operate in IDE mode
 *
 * All timing related setup (PCMCIA_SHT, PCMCIA_SST, PCMCIA_SL)
 * to be done later (depending on CPU clock)
 */

/* Window 0:
 *	Base: 0xFE100000	CS1
 *	Port Size:     2 Bytes
 *	Port Size:    16 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR0		0xFE100000
#define CFG_PCMCIA_POR0	    (	PCMCIA_BSIZE_2	\
			    |	PCMCIA_PPS_16	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 1:
 *	Base: 0xFE100080	CS1
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR1		0xFE100080
#define CFG_PCMCIA_POR1	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 2:
 *	Base: 0xFE100100	CS2
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR2		0xFE100100
#define CFG_PCMCIA_POR2	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 3:
 *	not used
 */
#define	CFG_PCMCIA_PBR3		0
#define	CFG_PCMCIA_POR3		0

/* Window 4:
 *	Base: 0xFE100C00	CS1
 *	Port Size:     2 Bytes
 *	Port Size:    16 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR4		0xFE100C00
#define CFG_PCMCIA_POR4	    (	PCMCIA_BSIZE_2	\
			    |	PCMCIA_PPS_16	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 5:
 *	Base: 0xFE100C80	CS1
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR5		0xFE100C80
#define CFG_PCMCIA_POR5	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 6:
 *	Base: 0xFE100D00	CS2
 *	Port Size:     8 Bytes
 *	Port Size:     8 Bit
 *	Common Memory Space
 */

#define	CFG_PCMCIA_PBR6		0xFE100D00
#define CFG_PCMCIA_POR6	    (	PCMCIA_BSIZE_8	\
			    |	PCMCIA_PPS_8	\
			    |	PCMCIA_PRS_MEM	\
			    |	PCMCIA_SLOT_x	\
			    |	PCMCIA_PV	\
			    )

/* Window 7:
 *	not used
 */
#define	CFG_PCMCIA_PBR7		0
#define	CFG_PCMCIA_POR7		0

#endif /* _PCMCIA_H */
