/*
 * (C) Copyright 2000
 * Rob Taylor. Flying Pig Systems. robt@flyingpig.com.
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
#include <asm/processor.h>
#include <mpc8240.h>

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers,
 */
void
cpu_init_f (void)
{
    register unsigned long val;
    CONFIG_WRITE_WORD( PCICR, 0x06); /* Bus Master, respond to PCI memory space acesses*/
/*    CONFIG_WRITE_HALFWORD(PCISR, 0xffff); *//*reset PCISR*/

    CONFIG_READ_WORD(PICR1, val);
    CONFIG_WRITE_WORD( PICR1,
       (val & (PICR1_ADDRESS_NAP | PICR1_RCS0)) |
               PIRC1_MSK | PICR1_PROC_TYPE_603E |
               PICR1_FLASH_WR_EN | PICR1_MCP_EN |
               PICR1_CF_DPARK | PICR1_EN_PCS |
               PICR1_CF_APARK );
    CONFIG_READ_WORD(PICR2, val);
    val= val & ~ (PICR2_CF_SNOOP_WS_MASK | PICR2_CF_APHASE_WS_MASK); /*mask off waitstate bits*/
    CONFIG_WRITE_WORD(PICR2, val | PICR2_CF_SNOOP_WS_1WS | PICR2_CF_APHASE_WS_1WS); /*1 wait state*/
    CONFIG_WRITE_WORD(EUMBBAR, CFG_EUMB_ADDR);
#ifndef DEBUG
    CONFIG_WRITE_WORD(MCCR1, (CFG_ROMNAL << MCCR1_ROMNAL_SHIFT) |
                             (CFG_ROMFAL << MCCR1_ROMFAL_SHIFT));

    CONFIG_WRITE_WORD(MCCR2, CFG_REFINT << MCCR2_REFINT_SHIFT);

    CONFIG_WRITE_WORD(MCCR3,
        (((CFG_BSTOPRE & 0x003c) >> 2) << MCCR3_BSTOPRE2TO5_SHIFT) |
        (CFG_REFREC << MCCR3_REFREC_SHIFT) |
        (CFG_RDLAT  << MCCR3_RDLAT_SHIFT));

    CONFIG_WRITE_WORD(MCCR4,
        (CFG_PRETOACT << MCCR4_PRETOACT_SHIFT) |
        (CFG_ACTTOPRE << MCCR4_ACTTOPRE_SHIFT) |
        MCCR4_BIT21 |
        (CFG_REGISTERD_TYPE_BUFFER ? MCCR4_REGISTERED: 0) |
        ((CFG_BSTOPRE & 0x0003) <<MCCR4_BSTOPRE0TO1_SHIFT ) |
        ((CFG_SDMODE_CAS_LAT <<4) | (CFG_SDMODE_WRAP <<3) | (CFG_SDMODE_BURSTLEN) <<MCCR4_SDMODE_SHIFT) |
        ((CFG_BSTOPRE & 0x03c0) <<MCCR4_BSTOPRE6TO9_SHIFT ));

    CONFIG_WRITE_WORD(MSAR1,
        ( CFG_BANK0_START & MICR_ADDR_MASK) |
        ((CFG_BANK1_START & MICR_ADDR_MASK) << 8) |
        ((CFG_BANK2_START & MICR_ADDR_MASK) << 16) |
        ((CFG_BANK3_START & MICR_ADDR_MASK) << 24));
    CONFIG_WRITE_WORD(EMSAR1,
        ( CFG_BANK0_START & MICR_EADDR_MASK) |
        ((CFG_BANK1_START & MICR_EADDR_MASK) << 8) |
        ((CFG_BANK2_START & MICR_EADDR_MASK) << 16) |
        ((CFG_BANK3_START & MICR_EADDR_MASK) << 24));
    CONFIG_WRITE_WORD(MSAR2,
        ( CFG_BANK4_START & MICR_ADDR_MASK) |
        ((CFG_BANK5_START & MICR_ADDR_MASK) << 8) |
        ((CFG_BANK6_START & MICR_ADDR_MASK) << 16) |
        ((CFG_BANK7_START & MICR_ADDR_MASK) << 24));
    CONFIG_WRITE_WORD(EMSAR2,
        ( CFG_BANK4_START & MICR_EADDR_MASK) |
        ((CFG_BANK5_START & MICR_EADDR_MASK) << 8) |
        ((CFG_BANK6_START & MICR_EADDR_MASK) << 16) |
        ((CFG_BANK7_START & MICR_EADDR_MASK) << 24));
    CONFIG_WRITE_WORD(MEAR1,
        ( CFG_BANK0_END & MICR_ADDR_MASK) |
        ((CFG_BANK1_END & MICR_ADDR_MASK) << 8) |
        ((CFG_BANK2_END & MICR_ADDR_MASK) << 16) |
        ((CFG_BANK3_END & MICR_ADDR_MASK) << 24));
    CONFIG_WRITE_WORD(EMEAR1,
        ( CFG_BANK0_END & MICR_EADDR_MASK) |
        ((CFG_BANK1_END & MICR_EADDR_MASK) << 8) |
        ((CFG_BANK2_END & MICR_EADDR_MASK) << 16) |
        ((CFG_BANK3_END & MICR_EADDR_MASK) << 24));
    CONFIG_WRITE_WORD(MEAR2,
        ( CFG_BANK4_END & MICR_ADDR_MASK) |
        ((CFG_BANK5_END & MICR_ADDR_MASK) << 8) |
        ((CFG_BANK6_END & MICR_ADDR_MASK) << 16) |
        ((CFG_BANK7_END & MICR_ADDR_MASK) << 24));
    CONFIG_WRITE_WORD(EMEAR2,
        ( CFG_BANK4_END & MICR_EADDR_MASK) |
        ((CFG_BANK5_END & MICR_EADDR_MASK) << 8) |
        ((CFG_BANK6_END & MICR_EADDR_MASK) << 16) |
        ((CFG_BANK7_END & MICR_EADDR_MASK) << 24));

    CONFIG_WRITE_BYTE(ODCR, CFG_ODCR);
    CONFIG_WRITE_BYTE(MBER,
         CFG_BANK0_ENABLE |
        (CFG_BANK1_ENABLE << 1) |
        (CFG_BANK2_ENABLE << 2) |
        (CFG_BANK3_ENABLE << 3) |
        (CFG_BANK4_ENABLE << 4) |
        (CFG_BANK5_ENABLE << 5) |
        (CFG_BANK6_ENABLE << 6) |
        (CFG_BANK7_ENABLE << 7));

    //! Wait 200us before initialize other registers
    /*FIXME: write a decent udelay wait */
    __asm__ __volatile__(
      " mtctr   %0 \n \
       0: bdnz   0b\n"
      :
      : "r" (0x10000));

   CONFIG_READ_WORD(MCCR1, val);
   CONFIG_WRITE_WORD(MCCR1, val | MCCR1_MEMGO); //set memory access going
   __asm__ __volatile__("eieio");
#endif

   /*map BAT areas - I think this should go somewhere else,
    *but it'll be fine here for now
    */
    mtspr (IBAT0L, CFG_IBAT0L);
    mtspr (IBAT0U, CFG_IBAT0U);
    mtspr (IBAT1L, CFG_IBAT1L);
    mtspr (IBAT1U, CFG_IBAT1U);
    mtspr (IBAT2L, CFG_IBAT2L);
    mtspr (IBAT2U, CFG_IBAT2U);
    mtspr (IBAT3L, CFG_IBAT3L);
    mtspr (IBAT3U, CFG_IBAT3U);
    mtspr (DBAT0L, CFG_DBAT0L);
    mtspr (DBAT0U, CFG_DBAT0U);
    mtspr (DBAT1L, CFG_DBAT1L);
    mtspr (DBAT1U, CFG_DBAT1U);
    mtspr (DBAT2L, CFG_DBAT2L);
    mtspr (DBAT2U, CFG_DBAT2U);
    mtspr (DBAT3L, CFG_DBAT3L);
    mtspr (DBAT3U, CFG_DBAT3U);

   /*initialise TLB
    */
    for (val = 0; val < 0x20000; val+=0x1000)
      tlbie(val);

}

/*
 * initialize higher level parts of CPU like time base and timers
 */
void
cpu_init_r  (bd_t *bd)
{

}
