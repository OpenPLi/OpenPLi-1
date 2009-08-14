/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
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
//------------------------------------------------------------------------------+
//
//       This source code has been made available to you by IBM on an AS-IS
//       basis.  Anyone receiving this source is licensed under IBM
//       copyrights to use it in any way he or she deems fit, including
//       copying it, modifying it, compiling it, and redistributing it either
//       with or without modifications.  No license under IBM patents or
//       patent applications is to be implied by the copyright license.
//
//       Any user of this software should understand that IBM cannot provide
//       technical support for this software and will not be responsible for
//       any consequences resulting from the use of this software.
//
//       Any person who transfers this source code or any derivative work
//       must include the IBM copyright notice, this paragraph, and the
//       preceding two paragraphs in the transferred software.
//
//       COPYRIGHT   I B M   CORPORATION 1995
//       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
//-------------------------------------------------------------------------------

/*  ppcboot - Startup Code for IBM 4xx PowerPC based Embedded Boards
 *
 *
 *  The processor starts at 0xfffffffc and the code is executed
 *  from flash/rom.
 *  in memory, but as long we don't jump around before relocating.
 *  board_init lies at a quite high address and when the cpu has
 *  jumped there, everything is ok.
 *  This works because the cpu gives the FLASH (CS0) the whole
 *  address space at startup, and board_init lies as a echo of
 *  the flash somewhere up there in the memorymap.
 *
 *  board_init will change CS0 to be positioned at the correct
 *  address and (s)dram will be positioned at address 0
 */
#include <config.h>
#include <mpc8xx.h>
#include <ppc4xx.h>
#include "version.h"

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

/* We don't want the  MMU yet.
*/
#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME  )	/* Machine Check */

/*
 * Set up GOT: Global Offset Table
 *
 * Use r14 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(_end)
	GOT_ENTRY(.bss)
	END_GOT

/*
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
	.text
	.long	0x27051956		/* PPCBOOT Magic Number			*/
	.globl	version_string
version_string:
	.ascii PPCBOOT_VERSION, " (", __DATE__, " - ", __TIME__, ")\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start
_start:
	li	r4, BOOTFLAG_COLD	/* Normal Power-On: Boot from FLASH	*/
	b	boot_cold

	. = EXC_OFF_SYS_RESET + 0x10

	.globl	_start_warm
_start_warm:
	li	r4, BOOTFLAG_WARM	/* Software reboot			*/
	b	boot_warm

boot_cold:
boot_warm:

/*****************************************************************************/
#ifdef CONFIG_IOP480
        //-----------------------------------------------------------------------
        // Set up some machine state registers.
        //-----------------------------------------------------------------------
        addi    r4,r0,0x0000          // initialize r4 to zero
        mtspr   esr,r4                 // clear Exception Syndrome Reg
        mttcr   r4                     // timer control register
        addis   r4,r0,0x0000
        mtexier r4                     // disable all interrupts
        addi    r4,r0,0x1000          // set ME bit (Machine Exceptions)
        oris    r4,r4,0x2             // set CE bit (Critical Exceptions)
        mtmsr   r4                     // change MSR
        addis   r4,r0,0xFFFF          // set r4 to 0xFFFFFFFF (status in the
        ori     r4,r4,0xFFFF          // dbsr is cleared by setting bits to 1)
        mtdbsr  r4                     // clear/reset the dbsr
        mtexisr r4                     // clear all pending interrupts
        addis   r4,r0,0x8000
        mtexier r4                     // enable critical exceptions
        addis   r4,r0,0x0000          // assume 403GCX - enable core clk
        ori     r4,r4,0x4020          // dbling (no harm done on GA and GC
        mtiocr  r4                     // since bit not used) & DRC to latch
                                        // data bus on rising edge of CAS
        //-----------------------------------------------------------------------
        // Clear XER.
        //-----------------------------------------------------------------------
        addis   r0,r0,0x0000
        mtxer   r0
        //-----------------------------------------------------------------------
        // Invalidate i-cache and d-cache TAG arrays.
        //-----------------------------------------------------------------------
        addi    r3,0,1024              // 1/4 of I-cache size, half of D-cache
        addi    r4,0,1024              // 1/4 of I-cache
..cloop:
        iccci   0,r3
        iccci   r4,r3
        dccci   0,r3
        addic.  r3,r3,-16             // move back one cache line
        bne     ..cloop                 // loop back to do rest until r3 = 0

        //
        // initialize IOP480 so it can read 1 MB code area for SRAM spaces
        // this requires enabling MA[17..0], by default only MA[12..0] are enabled.
        //

        // first copy IOP480 register base address into r3
        addis   r3,0,0x5000             // IOP480 register base address hi
        ori     r3,r3,0x0000           // IOP480 register base address lo

        // use r4 as the working variable
        // turn on CS3 (LOCCTL.7)
        lwz     r4,0x84(r3)            // LOCTL is at offset 0x84
        andi.   r4,r4,0xff7f           // make bit 7 = 0 -- CS3 mode
        stw     r4,0x84(r3)            // LOCTL is at offset 0x84
        
        // turn on MA16..13 (LCS0BRD.12 = 0)
        lwz     r4,0x100(r3)           // LCS0BRD is at offset 0x100
        andi.   r4,r4,0xefff           // make bit 12 = 0
        stw     r4,0x100(r3)           // LCS0BRD is at offset 0x100
        
        // make sure above stores all comlete before going on
        sync

        // last thing, set local init status done bit (DEVINIT.31)
        lwz     r4,0x80(r3)             // DEVINIT is at offset 0x80
        oris    r4,r4,0x8000           // make bit 31 = 1
        stw     r4,0x80(r3)            // DEVINIT is at offset 0x80

        // clear all pending interrupts and disable all interrupts
        li      r4,-1                   // set p1 to 0xffffffff
        stw     r4,0x1b0(r3)           // clear all pending interrupts
        stw     r4,0x1b8(r3)           // clear all pending interrupts
        li      r4,0                    // set r4 to 0
        stw     r4,0x1b4(r3)           // disable all interrupts
        stw     r4,0x1bc(r3)           // disable all interrupts
                
        // make sure above stores all comlete before going on
        sync
        
        //-----------------------------------------------------------------------
        // Enable two 128MB cachable regions.
        //-----------------------------------------------------------------------
        addis   r1,r0,0x8000
        addi    r1,r1,0x0001
        mticcr  r1                  // instruction cache

        addis   r1,r0,0x0000
        addi    r1,r1,0x0000
        mtdccr  r1                  // data cache

        addis   r1,r0,CFG_INIT_RAM_ADDR@h
       	ori	r1,r1,CFG_INIT_SP_OFFSET          /* set up the stack to SDRAM */
        
	GET_GOT			/* initialize GOT access			*/

	bl	board_init_f	/* run first part of init code (from Flash)	*/

#endif  /* CONFIG_IOP480 */

/*****************************************************************************/
#ifdef CONFIG_PPC405GP

        //-----------------------------------------------------------------------
        // Clear and set up some registers.
        //-----------------------------------------------------------------------
        addi    r4,r0,0x0000
        mtspr   sgr,r4
        mtspr   dcwr,r4
        mtesr   r4                     // clear Exception Syndrome Reg
        mttcr   r4                     // clear Timer Control Reg       
        mtxer   r4                     // clear Fixed-Point Exception Reg
        mtevpr  r4                     // clear Exception Vector Prefix Reg
#if 1  /* test-only */
        addi    r4,r0,0x1000          // set ME bit (Machine Exceptions)
        oris    r4,r4,0x0002          // set CE bit (Critical Exceptions)
        mtmsr   r4                     // change MSR
#else   /* problems in rom2ram (see test-only in rom2ram!!!) */
        addi    r4,r0,0x0000          // dont set (!!!) ME bit (Machine Exceptions)
        oris    r4,r4,0x0002          // set CE bit (Critical Exceptions)
        mtmsr   r4                     // change MSR
#endif
        addi    r4,r0,(0xFFFF-0x10000)          // set r4 to 0xFFFFFFFF (status in the
                                        // dbsr is cleared by setting bits to 1)
        mtdbsr  r4                     // clear/reset the dbsr

        //-----------------------------------------------------------------------
        // Invalidate I and D caches. Enable I cache for defined memory regions
	// to speed things up. Leave the D cache disabled for now. It will be 
	// enabled/left disabled later based on user selected menu options. 
	// Be aware that the I cache may be disabled later based on the menu  
        // options as well. See miscLib/main.c.
        //-----------------------------------------------------------------------
        bl	invalidate_icache
        bl      invalidate_dcache

        //-----------------------------------------------------------------------
        // Enable two 128MB cachable regions.
        //-----------------------------------------------------------------------
        addis   r4,r0,0x8000
        addi    r4,r4,0x0001
        mticcr  r4                  // instruction cache

        addis   r4,r0,0x0000
        addi    r4,r4,0x0000
        mtdccr  r4                  // data cache

        //-----------------------------------------------------------------------
        // Initialize the External Bus Controller for external peripherals
        //-----------------------------------------------------------------------
        bl      ext_bus_cntlr_init

        //-----------------------------------------------------------------------
        // Initialize the On Chip Memory (4k SRAM)
        //-----------------------------------------------------------------------
        addis   r4,r0,OCM_DATA_ADDR@h
        ori     r4,r4,OCM_DATA_ADDR@l
        mtdcr   ocmdsarc,r4
        addis   r4,r0,0xc000
        ori     r4,r4,0x0000
        mtdcr   ocmdscntl,r4

        //-----------------------------------------------------------------------
        // Initialize the Control 0 register for UART control. 
	// Set UART1 for CTS/RTS and set the UART0 and UART1 internal
	// clock enable to use the internal serial clock instead of an  
	// external clock. Set the FPGA control reg for UART1 to
	// select CTS/RTS.              
        //-----------------------------------------------------------------------
	addis	r3,r0,0x0000            // set CTS/RTS for UART1 and set int.
        ori     r3,r3,0x1022		  // clock for UART0 and UART1 (/ 18)
        mtdcr   cntrl0,r3                // set CNTRL0

        //-----------------------------------------------------------------------
        // Initialize SDRAM Controller
        //-----------------------------------------------------------------------
        bl      sdram_init

        addis   r1,r0,CFG_INIT_RAM_ADDR@h
       	ori	r1,r1,CFG_INIT_SP_OFFSET   /* set up the stack in internal SRAM */
        
	GET_GOT			/* initialize GOT access			*/

	bl	board_init_f	/* run first part of init code (from Flash)	*/

#endif  /* CONFIG_PPC405GP */

        
	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception.  "Never" generated on the 860. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_Alignment:
	.long	AlignmentException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

	/* No FPU on MPC8xx.  This exception is not supposed to happen.
	*/
	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)

	/* I guess we could implement decrementer, and may have
	 * to someday for timekeeping.
	 */
	STD_EXCEPTION(0x900, Decrementer, timer_interrupt)
	STD_EXCEPTION(0xa00, Trap_0a, UnknownException)
	STD_EXCEPTION(0xb00, Trap_0b, UnknownException)

	STD_EXCEPTION(0xc00, SystemCall, UnknownException)
	STD_EXCEPTION(0xd00, SingleStep, UnknownException)

	STD_EXCEPTION(0xe00, Trap_0e, UnknownException)
	STD_EXCEPTION(0xf00, Trap_0f, UnknownException)

	/* On the MPC8xx, this is a software emulation interrupt.  It occurs
	 * for all unimplemented and illegal instructions.
	 */
	STD_EXCEPTION(0x1000, PIT, PITException)

	STD_EXCEPTION(0x1100, InstructionTLBMiss, UnknownException)
	STD_EXCEPTION(0x1200, DataTLBMiss, UnknownException)
	STD_EXCEPTION(0x1300, InstructionTLBError, UnknownException)
	STD_EXCEPTION(0x1400, DataTLBError, UnknownException)

	STD_EXCEPTION(0x1500, Reserved5, UnknownException)
	STD_EXCEPTION(0x1600, Reserved6, UnknownException)
	STD_EXCEPTION(0x1700, Reserved7, UnknownException)
	STD_EXCEPTION(0x1800, Reserved8, UnknownException)
	STD_EXCEPTION(0x1900, Reserved9, UnknownException)
	STD_EXCEPTION(0x1a00, ReservedA, UnknownException)
	STD_EXCEPTION(0x1b00, ReservedB, UnknownException)

	STD_EXCEPTION(0x1c00, DataBreakpoint, UnknownException)
	STD_EXCEPTION(0x1d00, InstructionBreakpoint, UnknownException)
	STD_EXCEPTION(0x1e00, PeripheralBreakpoint, UnknownException)
	STD_EXCEPTION(0x1f00, DevPortBreakpoint, UnknownException)


	.globl	_end_of_vectors
_end_of_vectors:


	. = 0x2000

/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	stw	r22,_NIP(r21)
	lis	r22,MSR_POW@h
	andc	r23,r23,r22
	stw	r23,_MSR(r21)
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)
#if 0
	andi.	r23,r23,MSR_PR
	mfspr	r23,SPRG3		/* if from user, fix up tss.regs */
	beq	2f
	addi	r24,r1,STACK_FRAME_OVERHEAD
	stw	r24,PT_REGS(r23)
2:	addi	r2,r23,-TSS		/* set r2 to current */
	tovirt(r2,r2,r23)
#endif
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */
#if 0
	addi	r24,r2,TASK_STRUCT_SIZE /* check for kernel stack overflow */
	cmplw	0,r1,r2
	cmplw	1,r1,r24
	crand	1,1,4
	bgt	stack_ovf		/* if r2 < r1 < r2+TASK_STRUCT_SIZE */
#endif
	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	mtspr	SRR1,r20
	mtlr	r23
	SYNC
	rfi				/* jump to handler, enable MMU */

int_return:
	mfmsr	r29		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r29,r29,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r29
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi

/* Cache functions.
*/
invalidate_icache:
        iccci   r0,r0                // for 405, iccci invalidates the 
        blr                            //   entire I cache 

invalidate_dcache:    
        addi    r6,0,0x0000            // clear GPR 6
        addi    r7,r0, 128 		// do loop for # of dcache lines
					// NOTE: dccci invalidates both 
        mtctr   r7                     // ways in the D cache
..dcloop:
        dccci   0,r6                   // invalidate line
        addi    r6,r6, 32		// bump to next line 
        bdnz    ..dcloop
	blr

flush_dcache:
        addis   r9,r0,0x0002          // set mask for EE and CE msr bits
        ori     r9,r9,0x8000
        mfmsr   r12                    // save msr
        andc    r9,r12,r9
        mtmsr   r9                     // disable EE and CE
        addi    r10,r0,0x0001         // enable data cache for unused memory
        mfdccr  r9                     // region 0xF8000000-0xFFFFFFFF via
        or      r10,r10,r9           // bit 31 in dccr
        mtdccr  r10
        addi    r10,r0,128            // do loop for # of lines
        addi    r11,r0,4096           // D cache set size=4K
        mtctr   r10
        addi    r10,r0,(0xE000-0x10000)         // start at 0xFFFFE000
        add     r11,r10,r11          // add to get to other side of cache line
..flush_dcache_loop:
        lwz     r3,0(r10)             // least recently used side
        lwz     r3,0(r11)             // the other side
        dccci   r0,r11                // invalidate both sides
        addi    r10,r10,0x0020        // bump to next line (32 bytes)
        addi    r11,r11,0x0020        // bump to next line (32 bytes)
        bdnz    ..flush_dcache_loop
        sync                            // allow memory access to complete
        mtdccr  r9                     // restore dccr
        mtmsr   r12                    // restore msr
        blr
                
     	.globl	icache_enable
icache_enable:
        mflr    r8
        bl      invalidate_icache
        mtlr    r8
        isync
        addis   r3,r0, 0x8000         /* set bit 0 */
        mticcr  r3    
	blr

	.globl	icache_disable
icache_disable:
        addis   r3,r0, 0x0000         /* clear bit 0 */
        mticcr  r3
        isync
	blr

	.globl	icache_status
icache_status:
        mficcr  r3
	srwi	r3, r3, 31	/* >>31 => select bit 0 */
	blr

	.globl	dcache_enable
dcache_enable:
        mflr    r8
        bl      invalidate_dcache
        mtlr    r8
        isync
        addis   r3,r0, 0x8000         /* set bit 0 */
        mtdccr  r3    
	blr

	.globl	dcache_disable
dcache_disable:
        mflr    r8
        bl      flush_dcache
        mtlr    r8
        addis   r3,r0, 0x0000         /* clear bit 0 */
        mtdccr  r3    
	blr

	.globl	dcache_status
dcache_status:
        mfdccr  r3
	srwi	r3, r3, 31	/* >>31 => select bit 0 */
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

	.globl wr_pit
wr_pit:
	mtspr	pit, r3
	blr

	.globl wr_tcr
wr_tcr:
	mtspr	tcr, r3
	blr

//-------------------------------------------------------------------------------
// Function:     in8
// Description:  Input 8 bits
//-------------------------------------------------------------------------------
        .globl  in8
in8:
        lbz     r3,0x0000(r3)
        blr
        
//-------------------------------------------------------------------------------
// Function:     out8
// Description:  Output 8 bits
//-------------------------------------------------------------------------------
        .globl  out8
out8:
        stb     r4,0x0000(r3)
        blr

//-------------------------------------------------------------------------------
// Function:     out16
// Description:  Output 16 bits
//-------------------------------------------------------------------------------
        .globl  out16
out16:
        sth     r4,0x0000(r3)
        blr

//-------------------------------------------------------------------------------
// Function:     out16r
// Description:  Byte reverse and output 16 bits
//-------------------------------------------------------------------------------
        .globl  out16r
out16r:
        sthbrx  r4,r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     out32
// Description:  Output 32 bits
//-------------------------------------------------------------------------------
        .globl  out32
out32:
        stw     r4,0x0000(r3)
        blr

//-------------------------------------------------------------------------------
// Function:     out32r
// Description:  Byte reverse and output 32 bits
//-------------------------------------------------------------------------------
        .globl  out32r
out32r:
        stwbrx  r4,r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     in16
// Description:  Input 16 bits
//-------------------------------------------------------------------------------
        .globl  in16
in16:
        lhz     r3,0x0000(r3)
        blr

//-------------------------------------------------------------------------------
// Function:     in16r
// Description:  Input 16 bits and byte reverse
//-------------------------------------------------------------------------------
        .globl  in16r
in16r:
        lhbrx   r3,r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     in32
// Description:  Input 32 bits
//-------------------------------------------------------------------------------
        .globl  in32
in32:
        lwz     3,0x0000(3)
        blr

//-------------------------------------------------------------------------------
// Function:     in32r
// Description:  Input 32 bits and byte reverse
//-------------------------------------------------------------------------------
        .globl  in32r
in32r:
        lwbrx   r3,r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     ppcDcbf
// Description:  Data Cache block flush
// Input:        r3 = effective address
// Output:       none.
//-------------------------------------------------------------------------------
	.globl  ppcDcbf
ppcDcbf:
        dcbf    r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     ppcDcbi
// Description:  Data Cache block Invalidate
// Input:        r3 = effective address
// Output:       none.
//-------------------------------------------------------------------------------
	.globl  ppcDcbi
ppcDcbi:
        dcbi    r0,r3
        blr

//-------------------------------------------------------------------------------
// Function:     ppcSync
// Description:  Processor Synchronize
// Input:        none.
// Output:       none.
//-------------------------------------------------------------------------------
	.globl  ppcSync
ppcSync:
        sync
        blr

/*------------------------------------------------------------------------------*/

/*
 * void relocate_code (addr_sp, bd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = dest
 * r4 = src
 * r5 = length in bytes
 * r6 = cachelinesize
 */
	.globl	relocate_code
relocate_code:
	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Board Info pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CFG_FLASH_BASE@h		/* Source      Address	*/
	ori	r4, r4, CFG_FLASH_BASE@l
	lis	r5, CFG_MONITOR_LEN@h		/* Length in Bytes	*/
	ori	r5, r5, CFG_MONITOR_LEN@l
	li	r6, CFG_CACHELINE_SIZE		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CFG_FLASH_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r14, r14, r15
	/* the the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */
	
	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	7f		/* Protect against 0 count		*/
	mtctr	r0
	bge	cr1,2f

	la	r8,-4(r4)
	la	r7,-4(r3)
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b
	b	4f

2:	slwi	r0,r0,2
	add	r8,r4,r0
	add	r7,r3,r0
3:	lwzu	r0,-4(r8)
	stwu	r0,-4(r7)
	bdnz	3b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
4:	cmpwi	r6,0
	add	r5,r3,r5
	beq	7f		/* Always flush prefetch queue in any case */
	subi	r0,r6,1
	andc	r3,r3,r0
	mr	r4,r3
5:	cmplw	r4,r5
	dcbst	0,r4
	add	r4,r4,r6
	blt	5b
	sync			/* Wait for all dcbst to complete on bus */
	mr	r4,r3
6:	cmplw	r4,r5
	icbi	0,r4
	add	r4,r4,r6
	blt	6b
7:	sync			/* Wait for all icbi to complete on bus	*/
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */

	addi	r0, r10, in_ram - _start + EXC_OFF_SYS_RESET
	mtlr	r0
	blr

in_ram:

	/*
	 * Relocation Function, r14 point to got2+0x8000
	 *
         * Adjust got2 pointers, no need to check for 0, this code
         * already puts a few entries in the table.
	 */
	li	r0,__got2_entries@sectoff@l
	la	r3,GOT(_GOT2_TABLE_)
	lwz	r11,GOT(_GOT2_TABLE_)
	mtctr	r0
	sub	r11,r3,r11
	addi	r3,r3,-4
1:	lwzu	r0,4(r3)
	add	r0,r0,r11
	stw	r0,0(r3)
	bdnz	1b
	
	/*
         * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */	
2:	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	add	r0,r0,r11
	stw	r10,0(r3)
	stw	r0,0(r4)
	bdnz	3b
4:
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(.bss)
	lwz	r4,GOT(_end)

	cmplw	0, r3, r4
	beq	6f

	li	r0, 0
5:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	5b
6:

	mr	r3, r9		/* Board Info pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/* Problems accessing "end" in C, so do it here */
	.globl	get_endaddr
get_endaddr:
	lwz	r3,GOT(_end)
	blr

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	rlwinm	r9, r7, 0, 18, 31	/* _start & 0x3FFF	*/

	cmplw	0, r7, r8
	bgelr				/* return if r7>=r8 - just in case */

	mflr	r4			/* save link register		*/
1:
	lwz	r0, 0(r7)
	stw	r0, 0(r9)
	addi	r7, r7, 4
	addi	r9, r9, 4
	cmplw	0, r7, r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7, .L_MachineCheck - _start + EXC_OFF_SYS_RESET
	li	r8, Alignment - _start + EXC_OFF_SYS_RESET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_FPUnavailable - _start + EXC_OFF_SYS_RESET
	li	r8, _end_of_vectors - _start + EXC_OFF_SYS_RESET
3:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	3b

	mtlr	r4			/* restore link register	*/
	blr

	/*
	 * Function: relocate entries for one exception vector
	 */
trap_reloc:
	lwz	r0, 0(r7)		/* hdlr ...			*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 0(r7)

	lwz	r0, 4(r7)		/* int_return ...		*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 4(r7)

	blr


#ifdef CONFIG_PPC405GP

//-----------------------------------------------------------------------------
// Function:     ext_bus_cntlr_init
// Description:  Initializes the External Bus Controller for the external 
//		peripherals. IMPORTANT: For pass1 this code must run from 
//		cache since you can not reliably change a peripheral banks
//		timing register (pbxap) while running code from that bank.
//		For ex., since we are running from ROM on bank 0, we can NOT 
//		execute the code that modifies bank 0 timings from ROM, so
//		we run it from cache.
//	Bank 0 - Flash bank 0
//	Bank 1 - Flash bank 1
//	Bank 2 - CAN0, CAN1, CAN2, Codeswitch (0x000,0x100,0x200,0x400)
//	Bank 3 - IDE (CompactFlash)
//	Bank 4 - Quart
//	Bank 5 - not used
//	Bank 6 - not used
//	Bank 7 - not used
//-----------------------------------------------------------------------------
	.globl	ext_bus_cntlr_init
ext_bus_cntlr_init:
        addis   r4,r0,0x8000
        ori     r4,r4,0x0001

        mficcr  r9                     // get iccr value
        cmp     cr0,0,r9,r4          // check if caching already enabled
        beq     ..icache_on             // if not,
        mticcr  r4                     // enable caching 
..icache_on:
#if 0 /* test-only */
        addis   r3,0,ext_bus_cntlr_init@h    // store the address of the 
        ori     r3,r3,ext_bus_cntlr_init@l  // ext_bus_cntlr_init functn in r3
#else
        addis   r3,0,0xfffd    // store the address of the 
        ori     r3,r3,0x24ec   // ext_bus_cntlr_init functn in r3
#endif
        addi    r4,0,14                // set ctr to 10; used to prefetch
        mtctr   r4                     // 10 cache lines to fit this function
                                        // in cache (gives us 8x10=80 instrctns)
..ebcloop:
        icbt    r0,r3                 // prefetch cache line for addr in r3
        addi    r3,r3,32		// move to next cache line
        bdnz    ..ebcloop               // continue for 10 cache lines

        //-------------------------------------------------------------------
        // Delay to ensure all accesses to ROM are complete before changing
	// bank 0 timings. 200usec should be enough.
        //   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
        //-------------------------------------------------------------------
	addis	r3,0,0x0
        ori     r3,r3,0xA000          // ensure 200usec have passed since reset
        mtctr   r3
..spinlp:
        bdnz    ..spinlp                // spin loop

        //-----------------------------------------------------------------------
        // Memory Bank 0 (Flash Bank 0) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb0ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x9201
        ori     r4,r4,0x5480
        mtdcr   ebccfgd,r4

        addi    r4,0,pb0cr
        mtdcr   ebccfga,r4
//        addis   r4,0,0xFFE3           // BAS=0xFFE,BS=0x1(2MB),BU=0x3(R/W),
        addis   r4,0,0xFFC5           // BAS=0xFFC,BS=0x2(4MB),BU=0x3(R/W),
        ori     r4,r4,0xA000          // BW=0x1(16 bits)
        mtdcr   ebccfgd,r4

        //-----------------------------------------------------------------------
        // Memory Bank 1 (Flash Bank 1) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb1ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x9201         
        ori     r4,r4,0x5480
        mtdcr   ebccfgd,r4

        addi    r4,0,pb1cr
        mtdcr   ebccfga,r4
//        addis   r4,0,0xFFC3           // BAS=0xFFC,BS=0x1(2MB),BU=0x3(R/W),
        addis   r4,0,0xFF85           // BAS=0xFF8,BS=0x2(4MB),BU=0x3(R/W),
        ori     r4,r4,0xA000          // BW=0x1(16 bits)
        mtdcr   ebccfgd,r4

        //-----------------------------------------------------------------------
        // Memory Bank 2 (CAN0, 1, 2, Codeswitch) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb2ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x0100
        ori     r4,r4,0x53c0          // enable Ready, BEM=1
        mtdcr   ebccfgd,r4

        addi    r4,0,pb2cr
        mtdcr   ebccfga,r4
        addis   r4,0,0xF001            // BAS=0xF00,BS=0x0(1MB),BU=0x3(R/W),
        ori     r4,r4,0x8000            // BW=0x0(8 bits)
        mtdcr   ebccfgd,r4

        //-----------------------------------------------------------------------
        // Memory Bank 3 (CompactFlash IDE) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb3ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x0100         
        ori     r4,r4,0x53c0          // enable Ready, BEM=1
        mtdcr   ebccfgd,r4

        addi    r4,0,pb3cr
        mtdcr   ebccfga,r4
        addis   r4,0,0xF011           // BAS=0xF01,BS=0x0(1MB),BU=0x3(R/W),
        ori     r4,r4,0xA000          // BW=0x1(16 bits)
        mtdcr   ebccfgd,r4

        //-----------------------------------------------------------------------
        // Memory Bank 4 (NVRAM) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb4ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x0100
        ori     r4,r4,0x5280          // disable Ready, BEM=0
        mtdcr   ebccfgd,r4

        addi    r4,0,pb4cr
        mtdcr   ebccfga,r4
        addis   r4,0,0xF021            // BAS=0xF02,BS=0x0(1MB),BU=0x3(R/W),
        ori     r4,r4,0x8000            // BW=0x0(8 bits)
        mtdcr   ebccfgd,r4

        //-----------------------------------------------------------------------
        // Memory Bank 5 (Quart) initialization
        //-----------------------------------------------------------------------
        addi    r4,0,pb5ap
        mtdcr   ebccfga,r4
        addis   r4,0,0x0100
        ori     r4,r4,0x5280          // disable Ready, BEM=0
        mtdcr   ebccfgd,r4

        addi    r4,0,pb5cr
        mtdcr   ebccfga,r4
        addis   r4,0,0xF031            // BAS=0xF03,BS=0x0(1MB),BU=0x3(R/W),
        ori     r4,r4,0x8000            // BW=0x0(8 bits)
        mtdcr   ebccfgd,r4

        cmpi    cr0,0,r9,0x0 		// check if I cache was off when we
					// started 
        bne     ..ebc_done		// if it was on, leave on
        addis   r4,r0,0x0000		// if it was off, disable
        mticcr  r4                     // restore iccr
        isync
..ebc_done:
	nop				// pass2 DCR errata #8
        blr

//-----------------------------------------------------------------------------
// Function:     sdram_init
// Description:  Configures SDRAM memory banks.
//               Auto Memory Configuration option reads the SDRAM EEPROM 
//		via the IIC bus and then configures the SDRAM memory
//               banks appropriately. If Auto Memory Configuration is
//		is not used, it is assumed that a 32MB 12x8(2) non-ECC DIMM is
//		plugged, ie. the DIMM that shipped wih the Eval board.
//-----------------------------------------------------------------------------
        .globl  sdram_init

sdram_init:

	mflr	r31

        //-------------------------------------------------------------------
        // Set MB0CF for bank 0. (0-16MB) Address Mode 4 since 12x8(4)
        //-------------------------------------------------------------------
        addi    r4,0,mem_mb0cf
        mtdcr   memcfga,r4
        addis   r4,0,0x0004
        ori     r4,r4,0x6001
        mtdcr   memcfgd,r4

        //-------------------------------------------------------------------
        // Set the SDRAM Timing reg, SDTR1 and the refresh timer reg, RTR. 
        // To set the appropriate timings, we need to know the SDRAM speed. 
	// We can use the PLB speed since the SDRAM speed is the same as 
	// the PLB speed. The PLB speed is the FBK divider times the 
	// 405GP reference clock, which on the Walnut board is 33Mhz.
	// Thus, if FBK div is 2, SDRAM is 66Mhz; if FBK div is 3, SDRAM is 
	// 100Mhz; if FBK is 3, SDRAM is 133Mhz. 
	// NOTE: The Walnut board supports SDRAM speeds of 66Mhz, 100Mhz, and
	// maybe 133Mhz. 
        //-------------------------------------------------------------------
        mfdcr   r5,strap                 // determine FBK divider
                                          // via STRAP reg to calc PLB speed.
                                          // SDRAM speed is the same as the PLB
				          // speed.
        rlwinm  r4,r5,4,0x3             // get FBK divide bits

..chk_66:
        cmpi    %cr0,0,r4,0x1
        bne     ..chk_100
	addis	r6,0,0x0085		// SDTR1 value for 66Mhz
	ori     r6,r6,0x4005 
	addis	r7,0,0x03F8		// RTR value for 66Mhz
        b	..sdram_ok
..chk_100:
        cmpi    %cr0,0,r4,0x2
        bne     ..chk_133
        addis   r6,0,0x0086            // SDTR1 value for 100Mhz
        ori     r6,r6,0x400D 
        addis   r7,0,0x05F0            // RTR value for 100Mhz
        b       ..sdram_ok
..chk_133:
        addis   r6,0,0x0107            // SDTR1 value for 133Mhz
        ori     r6,r6,0x4015
        addis   r7,0,0x07F0            // RTR value for 133Mhz

..sdram_ok:
        //-------------------------------------------------------------------
        // Set SDTR1 
        //-------------------------------------------------------------------
        addi    r4,0,mem_sdtr1
        mtdcr   memcfga,r4
        mtdcr   memcfgd,r6

        //-------------------------------------------------------------------
        // Set RTR
        //-------------------------------------------------------------------
        addi    r4,0,mem_rtr  
        mtdcr   memcfga,r4
        mtdcr   memcfgd,r7

        //-------------------------------------------------------------------
        // Delay to ensure 200usec have elapsed since reset. Assume worst
        // case that the core is running 200Mhz:
        //   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
        //-------------------------------------------------------------------
        addis   r3,0,0x0000
        ori     r3,r3,0xA000          // ensure 200usec have passed since reset
        mtctr   r3
..spinlp2:
        bdnz    ..spinlp2               // spin loop

        //-------------------------------------------------------------------
        // Set memory controller options reg, MCOPT1.
	// Set DC_EN to '1' and BRD_PRF to '01' for 16 byte PLB Burst 
	// read/prefetch.
        //-------------------------------------------------------------------
        addi    r4,0,mem_mcopt1
        mtdcr   memcfga,r4
        addis   r4,0,0x8080             // set DC_EN=1
        ori     r4,r4,0x0000
        mtdcr   memcfgd,r4

        //-------------------------------------------------------------------
        // Delay to ensure 10msec have elapsed since reset. This is
        // required for the MPC952 to stabalize. Assume worst
        // case that the core is running 200Mhz:
        //   200,000,000 (cycles/sec) X .010 (sec) = 0x1E8480 cycles
        // This delay should occur before accessing SDRAM.
        //-------------------------------------------------------------------
        addis   r3,0,0x001E
        ori     r3,r3,0x8480          // ensure 10msec have passed since reset
        mtctr   r3
..spinlp3:
        bdnz    ..spinlp3                // spin loop

        mtlr    r31                     // restore lr
        blr

#endif  /* CONFIG_PPC405GP */
        
