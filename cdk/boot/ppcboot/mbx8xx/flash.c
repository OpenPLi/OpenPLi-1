/*
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * 
 * Flash Routines for AM290[48]0B devices
 * 
 *--------------------------------------------------------------------
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
#include "vpd.h"

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Protection Flags:
 */

#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02

/*-----------------------------------------------------------------------
 * Functions
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info);
int flash_write (uchar *, ulong, ulong);
flash_info_t *addr2info (ulong);

static int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static int  flash_protect (int flag, ulong from, ulong to, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
    unsigned long size, totsize;
    int i;
    ulong addr;

    /* Init: no FLASHes known */
    for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
	flash_info[i].flash_id = FLASH_UNKNOWN;
    }
    
    totsize = 0;
    addr = 0xfc000000;
    for(i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
	size = flash_get_size((vu_long *)addr, &flash_info[i]);
	if (flash_info[i].flash_id == FLASH_UNKNOWN) 
	  break;
	totsize += size;
	addr += size;
    }

    addr = 0xfe000000;
    for(i = 0; i < CFG_MAX_FLASH_BANKS; i++) {

	size = flash_get_size((vu_long *)addr, &flash_info[i]);
	if (flash_info[i].flash_id == FLASH_UNKNOWN) 
	  break;
	totsize += size;
	addr += size;
    }
    
    /* monitor protection ON by default */
    (void)flash_protect(FLAG_PROTECT_SET,
			CFG_FLASH_BASE,
			CFG_FLASH_BASE+CFG_MONITOR_LEN-1,
			&flash_info[0]);

    return (totsize);
}

/*-----------------------------------------------------------------------
 * Check or set protection status for monitor sectors
 *
 * The monitor always occupies the _first_ part of the _first_ Flash bank.
 */
static int  flash_protect (int flag, ulong from, ulong to, flash_info_t *info)
{
    ulong b_end = info->start[0] + info->size - 1;	/* bank end address */
    int rc    =  0;
    int first = -1;
    int last  = -1;
    int i;
    
    if (to < info->start[0]) {
	return (0);
    }
    
    for (i=0; i<info->sector_count; ++i) {
	ulong end;		/* last address in current sect	*/
	short s_end;
	
	s_end = info->sector_count - 1;
	
	end = (i == s_end) ? b_end : info->start[i + 1] - 1;
	
	if (from > end) {
	    continue;
	}
	if (to < info->start[i]) {
	    continue;
	}
	
	if (from == info->start[i]) {
	    first = i;
	    if (last < 0) {
		last = s_end;
	    }
	}
	if (to  == end) {
	    last  = i;
	    if (first < 0) {
		first = 0;
	    }
	}
    }
    
    for (i=first; i<=last; ++i) {
	if (flag & FLAG_PROTECT_CLEAR) {
	    info->protect[i] = 0;
	} else if (flag & FLAG_PROTECT_SET) {
	    info->protect[i] = 1;
	}
	if (info->protect[i]) {
	    rc = 1;
	}
    }
    return (rc);
}


/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
    int i;
    
    if (info->flash_id == FLASH_UNKNOWN) {
	printf ("missing or unknown FLASH type\n");
	return;
    }
    
    switch (info->flash_id >> 16) {
    case 0x1:	
	printf ("AMD ");		
	break;
    default:
	printf ("Unknown Vendor ");	
	break;
    }
    
    switch (info->flash_id & FLASH_TYPEMASK) {
    case AMD_ID_F040B:	
	printf ("AM29F040B (4 Mbit)\n");
	break;
    case AMD_ID_F080B:	
	printf ("AM29F080B (8 Mbit)\n");
	break;
    default:		
	printf ("Unknown Chip Type\n");
	break;
    }
    
    printf ("  Size: %ld MB in %d Sectors\n", 
	    info->size >> 20, info->sector_count);
    
    printf ("  Sector Start Addresses:");
    for (i=0; i<info->sector_count; ++i) {
	if ((i % 5) == 0)
	  printf ("\n   ");
	printf (" %08lX%s",
		info->start[i],
		info->protect[i] ? " (RO)" : "     "
		);
    }
    printf ("\n");
}

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
    short i;
    ulong vendor, devid;
    ulong base = (ulong)addr;
    
    /* Write auto select command: read Manufacturer ID */
    addr[0x0555] = 0xAAAAAAAA;
    addr[0x02AA] = 0x55555555;
    addr[0x0555] = 0x90909090;
   
    vendor = addr[0];
    devid = addr[1] & 0xff;

    /* only support AMD */
    if (vendor != 0x01010101) {
	return 0;
    }
      
    vendor &= 0xf;
    devid &= 0xff;

    if (devid == AMD_ID_F040B) {
	info->flash_id     = vendor << 16 | devid;
	info->sector_count = 8;
	info->size         = info->sector_count * 0x10000;
    }
    else if (devid == AMD_ID_F080B) {
	info->flash_id     = vendor << 16 | devid;
	info->sector_count = 4;
	info->size         = 4 * info->sector_count * 0x10000;  
    }
    else {
	printf ("## Unknown Flash Type: %08lx\n", devid);
	return 0;
    }
    
    /* check for protected sectors */
    for (i = 0; i < info->sector_count; i++) {
	/* sector base address */
	info->start[i] = base + i * (info->size / info->sector_count);
	/* read sector protection at sector address, (A7 .. A0) = 0x02 */
	/* D0 = 1 if protected */
	addr = (volatile unsigned long *)(info->start[i]);
	info->protect[i] = addr[2] & 1;
    }
    
    /*
     * Prevent writes to uninitialized FLASH.
     */
    if (info->flash_id != FLASH_UNKNOWN) {
	addr = (vu_long *)info->start[0];
	addr[0] = 0xF0;	/* reset bank */
    }
    
    return (info->size);
}


/*-----------------------------------------------------------------------
 */

void	flash_erase (flash_info_t *info, int s_first, int s_last)
{
    vu_long *addr = (vu_long*)(info->start[0]);
    int flag, prot, sect, l_sect;
    ulong start, now, last;
    
    if ((s_first < 0) || (s_first > s_last)) {
	if (info->flash_id == FLASH_UNKNOWN) {
	    printf ("- missing\n");
	} else {
	    printf ("- no sectors to erase\n");
	}
	return;
    }

    prot = 0;
    for (sect = s_first; sect <= s_last; sect++) {
	if (info->protect[sect]) {
	    prot++;
	}
    }
    
    if (prot) {
	printf ("- Warning: %d protected sectors will not be erased!\n",
		prot);
    } else {
	printf ("\n");
    }
    
    l_sect = -1;
    
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();
    
    addr[0x0555] = 0XAAAAAAAA;
    addr[0x02AA] = 0x55555555;
    addr[0x0555] = 0x80808080;
    addr[0x0555] = 0XAAAAAAAA;
    addr[0x02AA] = 0x55555555;
    
    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last; sect++) {
	if (info->protect[sect] == 0) {	/* not protected */
	    addr = (vu_long*)(info->start[sect]);
	    addr[0] = 0x30303030;
	    l_sect = sect;
	}
    }
    
    /* re-enable interrupts if necessary */
    if (flag)
      enable_interrupts();
    
    /* wait at least 80us - let's wait 1 ms */
    udelay (1000);
    
    /*
     * We wait for the last triggered sector
     */
    if (l_sect < 0)
      goto DONE;
    
    start = get_timer (0);
    last  = start;
    addr = (vu_long*)(info->start[l_sect]);
    while ((addr[0] & 0x80808080) != 0x80808080) {
	if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
	    printf ("Timeout\n");
	    return;
	}
	/* show that we're waiting */
	if ((now - last) > 1000) {	/* every second */
	    serial_putc ('.');
	    last = now;
	}
    }
    
    DONE:
    /* reset to read mode */
    addr = (volatile unsigned long *)info->start[0];
    addr[0] = 0xF0F0F0F0;	/* reset bank */
    
    printf (" done\n");
}

/*-----------------------------------------------------------------------
 */

flash_info_t *addr2info (ulong addr)
{
    flash_info_t *info;
    int i;
    
    for (i=0, info=&flash_info[0]; i<CFG_MAX_FLASH_BANKS; ++i, ++info) {
	if ((addr >= info->start[0]) &&
	    (addr < (info->start[0] + info->size)) ) {
	    return (info);
	}
    }
    
    return (NULL);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - target range includes protected sectors
 * 8 - target address not in Flash memory
 */
int flash_write (uchar *src, ulong addr, ulong cnt)
{
    int i;
    ulong         end        = addr + cnt - 1;
    flash_info_t *info_first = addr2info (addr);
    flash_info_t *info_last  = addr2info (end );
    flash_info_t *info;
    
    if (cnt == 0) {
	return (0);
    }
    
    if (!info_first || !info_last) {
	return (8);
    }
    
    for (info = info_first; info <= info_last; ++info) {
	ulong b_end = info->start[0] + info->size;	/* bank end addr */
	short s_end = info->sector_count - 1;
	for (i=0; i<info->sector_count; ++i) {
	    ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];
	    
	    if ((end >= info->start[i]) && (addr < e_addr) &&
		(info->protect[i] != 0) ) {
		return (4);
	    }
	}
    }
    
    /* finally write data to flash */
    for (info = info_first; info <= info_last && cnt>0; ++info) {
	ulong len;
	
	len = info->start[0] + info->size - addr;
	if (len > cnt)
	  len = cnt;
	if ((i = write_buff(info, src, addr, len)) != 0) {
	    return (i);
	}
	cnt  -= len;
	addr += len;
	src  += len;
    }
    return (0);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

static int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    int i, l, rc;
    
    wp = (addr & ~3);	/* get lower word aligned address */
    
    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0) {
	data = 0;
	for (i=0, cp=wp; i<l; ++i, ++cp) {
	    data = (data << 8) | (*(uchar *)cp);
	}
	for (; i<4 && cnt>0; ++i) {
	    data = (data << 8) | *src++;
	    --cnt;
	    ++cp;
	}
	for (; cnt==0 && i<4; ++i, ++cp) {
	    data = (data << 8) | (*(uchar *)cp);
	}
	
	if ((rc = write_word(info, wp, data)) != 0) {
	    return (rc);
	}
	wp += 4;
    }
    
    /*
     * handle word aligned part
     */
    while (cnt >= 4) {
	data = 0;
	for (i=0; i<4; ++i) {
	    data = (data << 8) | *src++;
	}
	if ((rc = write_word(info, wp, data)) != 0) {
	    return (rc);
	}
	wp  += 4;
	cnt -= 4;
    }
    
    if (cnt == 0) {
	return (0);
    }
    
    /*
     * handle unaligned tail bytes
     */
    data = 0;
    for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
	data = (data << 8) | *src++;
	--cnt;
    }
    for (; i<4; ++i, ++cp) {
	data = (data << 8) | (*(uchar *)cp);
    }
    
    return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
    vu_long *addr = (vu_long*)(info->start[0]);
    ulong start;
    int flag;
    
    /* Check if Flash is (sufficiently) erased */
    if ((*((vu_long *)dest) & data) != data) {
	return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();
    
    addr[0x0555] = 0xAAAAAAAA;
    addr[0x02AA] = 0x55555555;
    addr[0x0555] = 0xA0A0A0A0;
    
    *((vu_long *)dest) = data;
    
    /* re-enable interrupts if necessary */
    if (flag)
      enable_interrupts();
    
    /* data polling for D7 */
    start = get_timer (0);
    while ((*((vu_long *)dest) & 0x80808080) != (data & 0x80808080)) {
	if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	    return (1);
	}
    }
    return (0);
}

/*-----------------------------------------------------------------------
 */
