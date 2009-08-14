/*
 *   flash.c - nokia/sagem/philips flash driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 a lot of the developers ...
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   $Log: flash.c,v $
 *   Revision 1.8  2001/12/16 08:57:52  gillem
 *   - add removed AMD support
 *
 *   Revision 1.7  2001/12/12 20:22:31  derget
 *   unnoetiges entfernt um platzt zu sparen
 *
 *   Revision 1.6  2001/11/16 10:38:52  derget
 *   *wart* zu . gemacht in strata un/protect
 *
 *   Revision 1.5  2001/11/15 23:58:05  derget
 *   grossen bug gefixxt
 *   das bloede array vom ppcboot wurde nicht aktualisier
 *   beim un/protecten ....
 *
 *   Revision 1.4  2001/11/13 01:34:00  derget
 *   strata Protect und unprotect implementiert
 *   protect und unprotect debug messages rausgenommen ==> schneller ...
 *
 *   Revision 1.3  2001/11/11 22:40:42  derget
 *   added FAST Strata flash write
 *   thanks to Marko
 *
 *
 *   $Revision: 1.8 $
 *   
 */

#include <ppcboot.h>
#include <mpc8xx.h>

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/
unsigned char flash_bus_width;

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
static void flash_get_offsets (ulong base, flash_info_t *info);
static int  flash_protect (int flag, ulong from, ulong to, flash_info_t *info);
static int strata_buffer_write (flash_info_t *info, uchar *src, ulong addr, ulong cnt);
/*-----------------------------------------------------------------------
 */

void flash_put(vu_long *addr, ulong offs, ulong val) {

    vu_short *addr16 = (vu_short*)addr;
    
    if (flash_bus_width == 2)
	addr16[offs] = (ushort)(val & 0xFFFF);
    else  
	addr[offs] = val;

}

ulong flash_get(vu_long *addr, ulong offs) {

    vu_short *addr16 = (vu_short*)addr;
    
    if (flash_bus_width == 2)
	return (addr16[offs] & 0xFFFF);
    else  
	return addr[offs];

}

ulong flash_mask(ulong val) {

    if (flash_bus_width == 2)
	return (val & 0xFFFF);
    else  
	return val;

}

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */
	
	flash_bus_width = 4;
	
	size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);
	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
	    flash_bus_width = 2;
  	    size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);
	}

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	/* Remap FLASH according to real size */

//  Doesnt work for Sagem! FIXME!!!

//	memctl->memc_or0 = CFG_OR_TIMING_FLASH | (-size_b0 & 0xFFFF8000);
//	memctl->memc_br0 = (CFG_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_V;

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *)CFG_FLASH_BASE, &flash_info[0]);
	flash_get_offsets (CFG_FLASH_BASE, &flash_info[0]);

	/* monitor protection ON by default */
	   (void)flash_protect(FLAG_PROTECT_SET,
			    CFG_FLASH_BASE,
			    CFG_FLASH_BASE+CFG_MONITOR_LEN-1,
			    &flash_info[0]);

        flash_info[1].flash_id = FLASH_UNKNOWN;
	flash_info[1].sector_count = -1;

	flash_info[0].size = size_b0;
	flash_info[1].size = 0;
	
	return (size_b0);
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
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start adress table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		if ((info->flash_id & FLASH_TYPEMASK) != FLASH_INT640B) {
		    info->start[0] = base + 0x00000000;
		    info->start[1] = base + 0x00008000;
		    info->start[2] = base + 0x0000C000;
		    info->start[3] = base + 0x00010000;
		    for (i = 4; i < info->sector_count; i++) {
			    info->start[i] = base + (i * 0x00020000) - 0x00060000;
		    }
		} else {
		    for (i = 0; i < info->sector_count; i++) {
			    info->start[i] = base + (i * 0x00020000);
		    }
		}    
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}

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

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_INTEL:   printf ("INTEL ");              break;
        default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM323B:	printf ("AM29LV323B (32 Mbit, bottom boot sector)\n");
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_INT320B:	printf ("28F320-B  (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_INT320T:	printf ("28F320-T  (32 Mbit, top boot sector)\n");
				break;
	case FLASH_INT640B:	printf ("28F640-B  (64 Mbit, bottom boot sect)\n");
				break;
        default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
                if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
                {
                	volatile unsigned long *addr = (volatile unsigned long *)info->start[i];
                        flash_put(addr, 0, 0x00900090);               /* read configuration */
                        info->protect[i]= flash_get(addr, 2) & 1;
                        flash_put(addr, 0, 0x00FF00FF);               /* read array */
                }
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value;
	ulong base = (ulong)addr;


	/* Write auto select command: read Manufacturer ID */
	flash_put(addr, 0x0555, 0x00AA00AA);
	flash_put(addr, 0x02AA, 0x00550055);
	flash_put(addr, 0x0555, 0x00900090);

	value = flash_get(addr, 0);

	if (flash_bus_width == 2)
	    value |= value << 16;
	    
	switch (value) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
        case INT_MANUFACT:
                info->flash_id = FLASH_MAN_INTEL;
                break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = flash_get(addr, 1);			/* device ID		*/

	if (flash_bus_width == 2)
	    value |= value << 16;

	switch (value) {
#if 0
	case AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
#endif
	case AMD_ID_LV323B:
		info->flash_id += FLASH_AM323B;
		info->sector_count = 63+4;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
        case INT_ID_28F320B:
                info->flash_id |= FLASH_BTYPE;
        case INT_ID_28F320T:
                info->flash_id += FLASH_INT320T;
                info->sector_count = 63+4;
                info->size = 0x00800000;
                break;
        case INT_ID_28F640B:
                info->flash_id |= FLASH_BTYPE;
        case INT_ID_28F640T:
                info->flash_id += FLASH_INT640T;
                info->sector_count = 64;
                info->size = 0x00800000;
                break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start adress table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		if ((info->flash_id & FLASH_TYPEMASK) != FLASH_INT640B) {
		
       		    info->start[0] = base + 0x00000000;
       		    info->start[1] = base + 0x00004000;
       		    info->start[2] = base + 0x0000C000;
      		    info->start[3] = base + 0x00010000;
		    for (i = 4; i < info->sector_count; i++) {
			    info->start[i] = base + (i * 0x00020000) - 0x00060000;
		    }
		} else {
		    for (i = 0; i < info->sector_count; i++) {
			    info->start[i] = base + (i * 0x00020000);
		    }
		}    
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		info->protect[i] = flash_get(addr, 2) & 1;
		//printf("\n bla %d",flash_get(addr, 2));
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *)info->start[0];

                if ((info->flash_id & FLASH_VENDMASK)==FLASH_MAN_INTEL)
                	flash_put(addr, 0, 0x00FF00FF);	/* read array */
                else
        	        flash_put(addr, 0, 0x00F000F0);	/* reset bank */
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

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    ((info->flash_id > FLASH_AMD_COMP) && (info->flash_id & FLASH_VENDMASK)!=FLASH_MAN_INTEL)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
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

        if ((info->flash_id & FLASH_VENDMASK)!=FLASH_MAN_INTEL)
        {
        	flag = disable_interrupts();
        	flash_put(addr, 0x0555, 0x00AA00AA);
	        flash_put(addr, 0x02AA, 0x00550055);
        	flash_put(addr, 0x0555, 0x00800080);
        	flash_put(addr, 0x0555, 0x00AA00AA);
        	flash_put(addr, 0x02AA, 0x00550055);

        	/* Start erase on unprotected sectors */
        	for (sect = s_first; sect<=s_last; sect++) {
        		if (info->protect[sect] == 0) {	/* not protected */
        			addr = (vu_long*)(info->start[sect]);
        			flash_put(addr, 0, 0x00300030);
        			l_sect = sect;
        		}
        	}
        	/* re-enable interrupts if necessary */
	        if (flag)
		        enable_interrupts();

        } else
        {
                flash_put(addr, 0, 0x00500050);     /* clear status register */
        	/* Start erase on unprotected sectors */
        	for (sect = s_first; sect<=s_last; sect++) 
                {
                        printf("\r sector %d ...", sect);
        		if (info->protect[sect] == 0) 
        	        {
        			addr = (vu_long*)(info->start[sect]);
        			flash_put(addr, 0, 0x00200020);           /* erase setup */
                                flash_put(addr, 0, 0x00D000D0);           /* erase confirm */
        			l_sect = sect;
                                flash_put(addr, 0, 0x00700070);     /* read status register */
                                start = get_timer (0);
                        	last  = start;
                          	while ((flash_get(addr, 0) & 0x00800080) != flash_mask(0x00800080)) 
                                {
                       		        if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) 
                       		        {
               	        		        printf ("Timeout\n");
               		        	        return;
               		        	}
               		           		/* show that we're waiting */
               		                if ((now - last) > 1000) 
               		                {	/* every second */
                               			putc ('.');
                               			last = now;
                               		}

               		        }
                        }
        	}
        }


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
        if ((info->flash_id & FLASH_VENDMASK)==FLASH_MAN_INTEL)
                flash_put(addr, 0, 0x00700070);     /* read status register */
       	while ((flash_get(addr, 0) & 0x00800080) != flash_mask(0x00800080)) 
        {
       		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) 
                {
       			printf ("Timeout\n");
       			return;
       		}
       		/* show that we're waiting */
       		if ((now - last) > 1000) {	/* every second */
       			putc ('.');
       			last = now;
       		}
       	}

        if (flash_get(addr, 0) != flash_mask(0x00800080))
        {
                if ((info->flash_id & FLASH_VENDMASK)==FLASH_MAN_INTEL)
                        printf("erase failed: %02x\n", addr[0]);
        }
DONE:
	/* reset to read mode */
	addr = (volatile unsigned long *)info->start[0];
        if ((info->flash_id & FLASH_VENDMASK)==FLASH_MAN_INTEL)
        	flash_put(addr, 0, 0x00FF00FF);	/* read array */
        else
	        flash_put(addr, 0, 0x00F000F0);	/* reset bank */

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
		if ((info->flash_id & FLASH_TYPEMASK)==FLASH_INT640B) {
		  if ((i = strata_buffer_write(info, src, addr, len)) != 0) {
		    return (i);
		  }
		}
		else {
		  if ((i = write_buff(info, src, addr, len)) != 0) {
		    return (i);
		  }
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

	wp = (addr & ~(flash_bus_width - 1));	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<flash_bus_width && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<flash_bus_width; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += flash_bus_width;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= flash_bus_width) {
		data = 0;
		for (i=0; i<flash_bus_width; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += flash_bus_width;
		cnt -= flash_bus_width;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<flash_bus_width && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<flash_bus_width; ++i, ++cp) {
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
	if ((flash_get((vu_long *)dest, 0) & data) != flash_mask(data)) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

        if ((info->flash_id & FLASH_VENDMASK)!=FLASH_MAN_INTEL)
        {
        	flash_put(addr, 0x0555, 0x00AA00AA);
        	flash_put(addr, 0x02AA, 0x00550055);
        	flash_put(addr, 0x0555, 0x00A000A0);
        	flash_put((vu_long *)dest, 0, data);
        } else
        {
        	flash_put((vu_long *)dest, 0, 0x00400040);
        	flash_put((vu_long *)dest, 0, data);
        }
	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

        if ((info->flash_id & FLASH_VENDMASK)!=FLASH_MAN_INTEL)
        {
        	/* data polling for D7 */
        	start = get_timer (0);
        	while ((flash_get((vu_long *)dest, 0) & 0x00800080) != (data & flash_mask(0x00800080))) {
        		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
        			return (1);
        		}
        	}
        } else
        {
        	flash_put((vu_long *)dest, 0, 0x00700070);        /* read status */
                start = get_timer (0);
                while ((flash_get(addr, 0) & 0x00800080) != flash_mask(0x00800080))
                        if (get_timer(start) > (CFG_FLASH_WRITE_TOUT*5))
                        {
                                flash_put(addr, 0, 0x00FF00FF);     /* read array */
                                return (1);
                        }
                if (flash_get(addr, 0) != flash_mask(0x00800080))
                {
                        printf("flash error: status %x\n", addr[0]);
                        flash_put(addr, 0, 0x00FF00FF);
                        return (1);
                }
        }
        if ((info->flash_id & FLASH_VENDMASK)==FLASH_MAN_INTEL)
        {
                flash_put(addr, 0, 0x00FF00FF);     /* read array */
        }
    	return (0);
}
/*
 *
 */

static int strata_buffer_write (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
  int buffer_size = 32; /* bytes */
  int word_count;        /* word count */
  int byte_count;
  vu_long *cmd_adr;
  ulong value;
  vu_long *write_adr = (vu_long*)addr;
  vu_long *read_adr = (vu_long*)src;
  int sector;
  int z, flag;
  static int tot_cnt = 0;
  static int Mbytes = 0;

  printf("\nFlashed\t%dMB", Mbytes); 

  /* clear status register */
  flash_put(cmd_adr, 0, 0x00500050);

  while (cnt > 0) {

    /* find the sector the adress is in  

    sector = 0;
    while (!(info->start[sector+1] > (ulong)write_adr))
      sector++; 

    cmd_adr = (vu_long*) ((info->start[sector]) & ~(buffer_size-1));
    */

    cmd_adr = (vu_long*) ((ulong)write_adr & ~(buffer_size - 1)); 

    /* printf("\ncmd_adr: %lx\twrite_adr: %lx\tread_adr: %lx", cmd_adr, write_adr, read_adr); */

    flag = disable_interrupts();
    
    z=0;
    /* issue write command */
    flash_put(cmd_adr, 0, 0x00E800E8);
    /* read extended status register */
    flash_put(cmd_adr, 0, 0x00700070);
    for(;;) {
      if ((flash_get(cmd_adr, 0) & 0x00800080) == flash_mask(0x00800080))
	break;
      
      udelay(5);
      
      if(++z > 20) {
	flash_put(cmd_adr, 0, 0x00FF00FF);
	if (flag)
		enable_interrupts();
	return 1;
      }
   }

    /* write word count */  

    byte_count = (((ulong)write_adr+32) & ~(buffer_size-1)) - (ulong)write_adr;

    if (byte_count > cnt)
      byte_count = cnt;

     
    word_count = (byte_count / flash_bus_width - 1) * 0x10000 + (byte_count / flash_bus_width - 1);
    flash_put(cmd_adr, 0, word_count);

    for(z=0; z< byte_count / flash_bus_width; z++) {
      value = flash_get(read_adr, 0);
      flash_put(write_adr, 0, value);
      (char*)read_adr += flash_bus_width;
      (char*)write_adr += flash_bus_width;
    }

    /*  confirm buffer write */
    /* flash_put(cmd_adr, 0, 0x00D000D0); */
    flash_put(write_adr-1, 0, 0x00D000D0);
    //flash_put(write_adr-1, 0, 0x00010001);
    z=0;
    flash_put(cmd_adr, 0, 0x00700070);
    for(;;) {
      if ((flash_get(cmd_adr, 0) & 0x00800080) == flash_mask(0x00800080))
	break;
      
      udelay(5);
      
      if(++z > 20) {
	flash_put(cmd_adr, 0, 0x00FF00FF);
	if (flag)
	  enable_interrupts();
	printf("Failure after flash\n");

	return 1;
      }
    }
    
    if (flag)
      enable_interrupts();	
    
    /* move section */
    cnt -= byte_count;

    tot_cnt += byte_count;
    while (tot_cnt > 1048575) {
      Mbytes++;
      tot_cnt-= 1048575;
      printf("\rFlashed\t%dMB", Mbytes); 
    }

  }
  
  printf("\n");
  
  flash_put(cmd_adr, 0, 0x00FF00FF);

  if (flag)
	enable_interrupts();	

  return 0;
}

void flash_protect_sector(flash_info_t *info, int i, int p)  /* is nur für 28f540j3 strata flash */ 
{
                        volatile unsigned long *addr = (volatile unsigned long *)info->start[i];
                        flash_put(addr, 0, 0x00500050);
                        flash_put(addr, 0, 0x00900090);
			flash_put(addr, 0, 0x00600060);
			switch (p) {	   
				case 1:		
                        		flash_put(addr, 0, 0x00010001); /* protekten */
					 info->protect[i]=1;
					break;
				case 0:
                                        flash_put(addr, 0, 0x00D000D0);	/* unprotekten */
					info->protect[i]=0;
					break;
				default:      
					break;
				   }

                         flash_put(addr, 0, 0x00900090);               /* read configuration einschalten */
                         for(;;) { if (flash_get(addr, 7)) printf("."); break; }  /* warte bis das flash wieder bereit ist */
                         							      /* . ist als zeitverzögerung da ! */
			 flash_put(addr, 0, 0x00FF00FF);	      /* in normalen modus schalten */
}
