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
#include <command.h>
#include <cmd_boot.h>
#include <asm/processor.h>
#include <ppc4xx.h>
#include <ppc_asm.tmpl>
#include <commproc.h>

/****************************************************************************/

unsigned decrementer_count;		/* count value for 1e6/HZ microseconds */

/****************************************************************************/

/*
 * CPM interrupt vector functions.
 */
struct	irq_action {
	 interrupt_handler_t *handler;
	 void *arg;
	 int count;
};

static struct irq_action irq_vecs[32];

/****************************************************************************/

static __inline__ unsigned long get_msr(void)
{
    unsigned long msr;

    asm volatile("mfmsr %0" : "=r" (msr) :);
    return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
    asm volatile("mtmsr %0" : : "r" (msr)); 
}

static __inline__ unsigned long get_dec(void)
{
    unsigned long val;

    asm volatile("mfdec %0" : "=r" (val) :);
    return val;
}


static __inline__ void set_dec(unsigned long val)
{
    asm volatile("mtdec %0" : : "r" (val)); 
}


static __inline__ void set_pit(unsigned long val)
{
    asm volatile("mtpit %0" : : "r" (val)); 
}


static __inline__ void set_tcr(unsigned long val)
{
    asm volatile("mttcr %0" : : "r" (val)); 
}


static __inline__ void set_evpr(unsigned long val)
{
    asm volatile("mtevpr %0" : : "r" (val));
}


void enable_interrupts (void)
{
	set_msr (get_msr() | MSR_EE);
}

/* returns flag if MSR_EE was set before */
int disable_interrupts (void)
{
	ulong msr = get_msr();
	set_msr (msr & ~MSR_EE);
	return ((msr & MSR_EE) != 0);
}

/****************************************************************************/

void
interrupt_init (bd_t *bd)
{
  int vec;

  /*
   * Mark all irqs as free
   */
  for (vec=0; vec<32; vec++)
    {
      irq_vecs[vec].handler = NULL;
      irq_vecs[vec].arg = NULL;
      irq_vecs[vec].count = 0;
    }

#ifdef CONFIG_PPC405GP
  /*--------------------------------------------------------------------------+
    | Interrupt controller setup for the Walnut board.
    | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
    |       IRQ 16    405GP internally generated; active low; level sensitive
    |       IRQ 17-24 RESERVED
    |       IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
    |       IRQ 26 (EXT IRQ 1) CAN1; active low; level sensitive
    |       IRQ 27 (EXT IRQ 2) PCI SLOT 0; active low; level sensitive
    |       IRQ 28 (EXT IRQ 3) PCI SLOT 1; active low; level sensitive
    |       IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
    |       IRQ 30 (EXT IRQ 5) PCI SLOT 3; active low; level sensitive
    |       IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
    | Note for Walnut board:
    |       An interrupt taken for the FPGA (IRQ 25) indicates that either   
    |       the Mouse, Keyboard, IRDA, or External Expansion caused the 
    |       interrupt. The FPGA must be read to determine which device
    |       caused the interrupt. The default setting of the FPGA clears
    |          
    +--------------------------------------------------------------------------*/
  mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */
  mtdcr(uicer, 0x00000000);        /* disable all ints */
  mtdcr(uiccr, 0x00000000);        /* set all to be non-critical*/
  mtdcr(uicpr, 0xFFFFFF81);        /* set int polarities */
  mtdcr(uictr, 0x10000000);        /* set int trigger levels */
  mtdcr(uicvcr, 0x00000001);       /* set vect base=0,INT0 highest priority*/
  mtdcr(uicsr, 0xFFFFFFFF);        /* clear all ints */

  /*
   * Init PIT
   */
  set_pit(200000);
#endif

#ifdef CONFIG_ADCIOP
  /*
   * Init PIT
   */
  set_pit(66000);
#endif

  /*
   * Enable PIT
   */
  set_tcr(0x04400000);
  
  /*
   * Set EVPR to 0
   */
  set_evpr(0x00000000);
  
  /*
   * Enable external interrupts (including PIT)
   */
  set_msr (get_msr() | MSR_EE);
}

/****************************************************************************/

/*
 * Handle external interrupts
 */
void external_interrupt(struct pt_regs *regs)
{
  ulong uic_msr;
  ulong msr_shift;
  int vec;

  /*
   * Read masked interrupt status register to determine interrupt source
   */
  uic_msr = mfdcr(uicmsr);
  msr_shift = uic_msr;
  vec = 0;
  
  while (msr_shift != 0)
    {
      if (msr_shift & 0x80000000)
        {
          /*
           * Increment irq counter (for debug purpose only)
           */
          irq_vecs[vec].count++;

          if (irq_vecs[vec].handler != NULL)
            (*irq_vecs[vec].handler)(irq_vecs[vec].arg);      /* call isr */
          else
            {
              mtdcr(uicer, mfdcr(uicer) & ~(0x80000000 >> vec));
              printf ("Masking bogus interrupt vector 0x%x\n", vec);
            }
          
          /*
           * After servicing the interrupt, we have to remove the status indicator.
           */
          mtdcr(uicsr, (0x80000000 >> vec));
        }
      
      /*
       * Shift msr to next position and increment vector
       */
      msr_shift <<= 1;
      vec++;
    }
}


/****************************************************************************/

/*
 * Install and free a interrupt handler.
 */

void
irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
  if (irq_vecs[vec].handler != NULL) {
    printf ("Interrupt vector %d: handler 0x%x replacing 0x%x\n",
            vec, (uint)handler, (uint)irq_vecs[vec].handler);
  }
  irq_vecs[vec].handler = handler;
  irq_vecs[vec].arg     = arg;
  mtdcr(uicer, mfdcr(uicer) | (0x80000000 >> vec));
#if 0
  printf ("Install interrupt for vector %d ==> %p\n", vec, handler);
#endif
}

void
irq_free_handler(int vec)
{
#if 0
  printf ("Free interrupt for vector %d ==> %p\n",
          vec, irq_vecs[vec].handler);
#endif
  mtdcr(uicer, mfdcr(uicer) & ~(0x80000000 >> vec));
  irq_vecs[vec].handler = NULL;
  irq_vecs[vec].arg     = NULL;
}

/****************************************************************************/


volatile ulong timestamp = 0;

/*
 * timer_interrupt - gets called when the decrementer overflows,
 * with interrupts disabled.
 * Trivial implementation - no need to be really accurate.
 */
void timer_interrupt(struct pt_regs *regs)
{
#if 0
	printf ("*** Timer Interrupt *** ");
#endif
	timestamp++;
}

/****************************************************************************/

void reset_timer (void)
{
	timestamp = 0;
}

ulong get_timer (ulong base)
{
	return (timestamp - base);
}

void set_timer (ulong t)
{
	timestamp = t;
}

/****************************************************************************/


#if (CONFIG_COMMANDS & CFG_CMD_IRQ)

/*******************************************************************************
*
* irqinfo - print information about PCI devices
*
*/
void
do_irqinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
  int vec;
  
  printf ("\nInterrupt-Information:\n");
  printf ("  Nr  Routine   Arg       Count\n");

  for (vec=0; vec<32; vec++)
    {
      if (irq_vecs[vec].handler != NULL)
        printf("  %02d  %08lx  %08lx  %d\n",
               vec, (ulong)irq_vecs[vec].handler, (ulong)irq_vecs[vec].arg,
               irq_vecs[vec].count);
    }
}


#endif  /* CONFIG_COMMANDS & CFG_CMD_IRQ */
