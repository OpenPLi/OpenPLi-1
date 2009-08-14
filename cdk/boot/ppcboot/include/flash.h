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


/*-----------------------------------------------------------------------
 * FLASH Info: contains chip specific data, per FLASH bank
 */

typedef struct {
	ulong	size;			/* total bank size in bytes		*/
	ushort	sector_count;		/* number of erase units		*/
	ulong	flash_id;		/* combined device & manufacturer code	*/
	ulong	start[CFG_MAX_FLASH_SECT];   /* physical sector start addresses	*/
	uchar	protect[CFG_MAX_FLASH_SECT]; /* sector protection status	*/
} flash_info_t;

/* Prototypes */

unsigned long flash_init (void);
void	flash_print_info (flash_info_t *);
void 	flash_protect_sector(flash_info_t *, int, int);
void	flash_erase	 (flash_info_t *, int, int);

/*-----------------------------------------------------------------------
 * Device IDs for AMD and Fujitsu FLASH
 */

#define AMD_MANUFACT	0x00010001	/* AMD     manuf. ID in D23..D16, D7..D0 */
#define FUJ_MANUFACT	0x00040004	/* FUJITSU manuf. ID in D23..D16, D7..D0 */
#define STM_MANUFACT	0x00200020	/* STM (Thomson) manuf. ID in D23.. -"-	*/
#define SST_MANUFACT	0x00BF00BF	/* SST     manuf. ID in D23..D16, D7..D0 */
#define MT_MANUFACT	0x00890089	/* MT      manuf. ID in D23..D16, D7..D0 */
#define INT_MANUFACT    0x00890089      /* INTEL   manuf. ID in D23..D16, D7..D0 */

#define INT_ID_28F800T  0x88C088C0      /* 28F800C3 ID ( 1 M, top boot sector)  */
#define INT_ID_28F800B  0x88C188C1      /* 28F800C3 ID ( 1 M, bottom boot sect) */
#define INT_ID_28F160T  0x88C288C2      /* 28F160C3 ID ( 2 M, top boot sector)  */
#define INT_ID_28F160B  0x88C388C3      /* 28F160C3 ID ( 2 M, bottom boot sect) */
#define INT_ID_28F320T  0x88C488C4      /* 28F320C3 ID ( 4 M, top boot sector)  */
#define INT_ID_28F320B  0x88C588C5      /* 28F320C3 ID ( 4 M, bottom boot sect) */
#define INT_ID_28F640B  0x00170017      /* 28F640C3 ID ( 8 M, top boot sector)  */
#define INT_ID_28F640T  0x88CD88CD      /* 28F640C3 ID ( 8 M, bottom boot sect) */

#define	MT_ID_28F400_T	0x44704470	/* 28F400B3 ID ( 4 M, top boot sector)	*/
#define MT_ID_28F400_B	0x44714471	/* 28F400B3 ID ( 4 M, bottom boot sect)	*/

#define AMD_ID_F040B	0xA4		/* 29F040B ID  ( 4 M, bottom boot sect)	*/
#define AMD_ID_F080B	0xD5		/* 29F0B0  ID  ( 1 M)			*/

#define AMD_ID_LV400T	0x22B922B9	/* 29LV400T ID ( 4 M, top boot sector)	*/
#define AMD_ID_LV400B	0x22BA22BA	/* 29LV400B ID ( 4 M, bottom boot sect)	*/

#define AMD_ID_LV800T	0x22DA22DA	/* 29LV800T ID ( 8 M, top boot sector)	*/
#define AMD_ID_LV800B	0x225B225B	/* 29LV800B ID ( 8 M, bottom boot sect)	*/

#define AMD_ID_LV160T	0x22C422C4	/* 29LV160T ID (16 M, top boot sector)	*/
#define AMD_ID_LV160B	0x22492249	/* 29LV160B ID (16 M, bottom boot sect)	*/

/* 29LV320 device IDs are not yet available */
#define AMD_ID_LV320T	0xDEADBEEF	/* 29LV320T ID (32 M, top boot sector)	*/
#define AMD_ID_LV320B	0xDEADBEEF	/* 29LV320B ID (32 M, bottom boot sect)	*/

#define AMD_ID_LV323B	0x22532253	/* 29LV322B ID (16 M, bottom boot sect)	*/

#define SST_ID_xF200A	0x27892789	/* 39xF200A ID ( 2M = 128K x 16	)	*/
#define SST_ID_xF400A	0x27802780	/* 39xF400A ID ( 4M = 256K x 16	)	*/
#define SST_ID_xF800A	0x27812781	/* 39xF800A ID ( 8M = 512K x 16	)	*/
#define SST_ID_xF160A	0x27822782	/* 39xF800A ID (16M =   1M x 16 )	*/

#define STM_ID_x800AB	0x005B005B	/* M29W800AB ID (8M = 512K x 16	)	*/

/*-----------------------------------------------------------------------
 * Internal FLASH identification codes
 */

#define FLASH_AM040T	0x0000		/* AMD AM29F040			*/
#define FLASH_AM040B	0x0001
#define FLASH_AM400T	0x0002		/* AMD AM29LV400			*/
#define FLASH_AM400B	0x0003
#define FLASH_AM800T	0x0004		/* AMD AM29LV800			*/
#define FLASH_AM800B	0x0005
#define FLASH_INT800T   0x0034
#define FLASH_INT800B   0x0035
#define FLASH_AM160T	0x0006		/* AMD AM29LV160			*/
#define FLASH_AM160B	0x0007
#define FLASH_INT160T	0x0036
#define FLASH_INT160B	0x0037
#define FLASH_AM320T	0x0008		/* AMD AM29LV320			*/
#define FLASH_AM320B	0x0009
#define FLASH_AM323B	0x0041

#define FLASH_INT320T	0x0038
#define FLASH_INT320B	0x0039
#define FLASH_INT640T	0x003A
#define FLASH_INT640B	0x003B

#define	FLASH_SST200A	0x000A		/* SST 39xF200A ID ( 2M = 128K x 16 )	*/
#define	FLASH_SST400A	0x000B		/* SST 39xF400A ID ( 4M = 256K x 16 )	*/
#define	FLASH_SST800A	0x000C		/* SST 39xF800A ID ( 8M = 512K x 16 )	*/
#define FLASH_SST160A	0x000D		/* SST 39xF160A ID (16M =   1M x 16 )	*/

#define	FLASH_STM800AB	0x0011		/* STM M29WF800AB ID ( 8M = 512K x 16 )	*/

#define FLASH_28F400_T	0x0022		/* MT  28F400B3 ID ( 4M = 256K x 16 )	*/
#define FLASH_28F400_B	0x0023		/* MT  28F400B3 ID ( 4M = 256K x 16 )	*/

#define FLASH_28F008S5	0x0050		/* Intel 28F008S5  ( 1M =  64K x 16 )	*/
#define FLASH_28F800_B	0x0051		/* Intel E28F800B ( 1M = ? )		*/
#define FLASH_AM29F800B	0x0052		/* AMD Am29F800BB ( 1M = ? )		*/

#define FLASH_UNKNOWN	0xFFFF		/* unknown flash type			*/


#define FLASH_MAN_AMD	0x00000000	/* manufacturer offsets			*/
#define FLASH_MAN_FUJ	0x00010000
#define FLASH_MAN_SST	0x00100000
#define FLASH_MAN_STM	0x00200000
#define	FLASH_MAN_INTEL	0x00300000
#define FLASH_MAN_MT	0x00400000


#define FLASH_TYPEMASK	0x0000FFFF	/* extract FLASH type   information	*/
#define FLASH_VENDMASK	0xFFFF0000	/* extract FLASH vendor information	*/

#define FLASH_AMD_COMP	0x000FFFFF	/* Up to this ID, FLASH is compatible	*/
					/* with AMD, Fujitsu and SST		*/
					/* (JEDEC standard commands ?)		*/

#define FLASH_BTYPE	0x01		/* mask for bottom boot sector type	*/

/*-----------------------------------------------------------------------
 * Timeout constants:
 *
 * We can't find any specifications for maximum chip erase times,
 * so these values are guestimates.
 */
#define FLASH_ERASE_TIMEOUT	120000	/* timeout for erasing in ms		*/
#define FLASH_WRITE_TIMEOUT	500	/* timeout for writes  in ms		*/

