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

#include <ppcboot.h>
#include <commproc.h>


/*****************************************************************************/
#ifdef CONFIG_IOP480

#define SPU_BASE         0x40000000

#define spu_LineStat_rc  0x00     // Line Status Register (Read/Clear)
#define spu_LineStat_w   0x04     // Line Status Register (Set)
#define spu_Handshk_rc   0x08     // Handshake Status Register (Read/Clear)
#define spu_Handshk_w    0x0c     // Handshake Status Register (Set)
#define spu_BRateDivh    0x10     // Baud rate divisor high
#define spu_BRateDivl    0x14     // Baud rate divisor low
#define spu_CtlReg       0x18     // Control Register
#define spu_RxCmd        0x1c     // Rx Command Register
#define spu_TxCmd        0x20     // Tx Command Register
#define spu_RxBuff       0x24     // Rx data buffer
#define spu_TxBuff       0x24     // Tx data buffer

/*-----------------------------------------------------------------------------+
| Line Status Register.
+-----------------------------------------------------------------------------*/
#define asyncLSRport1           0x40000000
#define asyncLSRport1set        0x40000004
#define asyncLSRDataReady             0x80
#define asyncLSRFramingError          0x40
#define asyncLSROverrunError          0x20
#define asyncLSRParityError           0x10
#define asyncLSRBreakInterrupt        0x08
#define asyncLSRTxHoldEmpty           0x04
#define asyncLSRTxShiftEmpty          0x02

/*-----------------------------------------------------------------------------+
| Handshake Status Register.
+-----------------------------------------------------------------------------*/
#define asyncHSRport1           0x40000008
#define asyncHSRport1set        0x4000000c
#define asyncHSRDsr                   0x80
#define asyncLSRCts                   0x40

/*-----------------------------------------------------------------------------+
| Control Register.
+-----------------------------------------------------------------------------*/
#define asyncCRport1            0x40000018
#define asyncCRNormal                 0x00
#define asyncCRLoopback               0x40
#define asyncCRAutoEcho               0x80
#define asyncCRDtr                    0x20
#define asyncCRRts                    0x10
#define asyncCRWordLength7            0x00
#define asyncCRWordLength8            0x08
#define asyncCRParityDisable          0x00
#define asyncCRParityEnable           0x04
#define asyncCREvenParity             0x00
#define asyncCROddParity              0x02
#define asyncCRStopBitsOne            0x00
#define asyncCRStopBitsTwo            0x01
#define asyncCRDisableDtrRts          0x00

/*-----------------------------------------------------------------------------+
| Receiver Command Register.
+-----------------------------------------------------------------------------*/
#define asyncRCRport1           0x4000001c
#define asyncRCRDisable               0x00
#define asyncRCREnable                0x80
#define asyncRCRIntDisable            0x00
#define asyncRCRIntEnabled            0x20
#define asyncRCRDMACh2                0x40
#define asyncRCRDMACh3                0x60
#define asyncRCRErrorInt              0x10
#define asyncRCRPauseEnable           0x08

/*-----------------------------------------------------------------------------+
| Transmitter Command Register.
+-----------------------------------------------------------------------------*/
#define asyncTCRport1           0x40000020
#define asyncTCRDisable               0x00
#define asyncTCREnable                0x80
#define asyncTCRIntDisable            0x00
#define asyncTCRIntEnabled            0x20
#define asyncTCRDMACh2                0x40
#define asyncTCRDMACh3                0x60
#define asyncTCRTxEmpty               0x10
#define asyncTCRErrorInt              0x08
#define asyncTCRStopPause             0x04
#define asyncTCRBreakGen              0x02

/*-----------------------------------------------------------------------------+
| Miscellanies defines.
+-----------------------------------------------------------------------------*/
#define asyncTxBufferport1      0x40000024
#define asyncRxBufferport1      0x40000024
#define asyncDLABLsbport1       0x40000014
#define asyncDLABMsbport1       0x40000010
#define asyncXOFFchar                 0x13
#define asyncXONchar                  0x11


/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

void
serial_init(ulong cpu_clock, int baudrate)
{
  volatile char  val;
  unsigned short br_reg;

  /*
   * Check for sanity
   */
  if ((baudrate < CFG_BAUDRATE_MIN) || (baudrate > CFG_BAUDRATE_MAX))
    baudrate = CFG_BAUDRATE_DEFAULT;
  br_reg = ((((CONFIG_CPUCLOCK * 1000000)/16) / baudrate)-1);

  /*
   * Init onboard UART
   */
  out8(SPU_BASE + spu_LineStat_rc, 0x78); // Clear all bits in Line Status Reg
  out8(SPU_BASE + spu_BRateDivl, (br_reg & 0x00ff));   // Set baud rate divisor...
  out8(SPU_BASE + spu_BRateDivh, ((br_reg & 0xff00) >> 8));   // ...
  out8(SPU_BASE + spu_CtlReg, 0x08);      // Set 8 bits, no parity and 1 stop bit
  out8(SPU_BASE + spu_RxCmd, 0xb0);       // Enable Rx
  out8(SPU_BASE + spu_TxCmd, 0x9c);       // Enable Tx
  out8(SPU_BASE + spu_Handshk_rc, 0xff);  // Clear Handshake
  val = in8(SPU_BASE + spu_RxBuff);       // Dummy read, to clear receiver
}


void
serial_setbrg (ulong cpu_clock, int baudrate)
{
  unsigned short br_reg;

  /*
   * Check for sanity
   */
  if ((baudrate < CFG_BAUDRATE_MIN) || (baudrate > CFG_BAUDRATE_MAX))
    baudrate = CFG_BAUDRATE_DEFAULT;
  br_reg = ((((CONFIG_CPUCLOCK * 1000000)/16) / baudrate)-1);

  out8(SPU_BASE + spu_BRateDivl, (br_reg & 0x00ff));   // Set baud rate divisor...
  out8(SPU_BASE + spu_BRateDivh, ((br_reg & 0xff00) >> 8));   // ...
}


void
serial_putc(const char c)
{
  if (c == '\n')
    serial_putc ('\r');
  
  /* load status from handshake register */
  if (in8(SPU_BASE + spu_Handshk_rc) != 00)
    out8(SPU_BASE + spu_Handshk_rc, 0xff);  // Clear Handshake
  
  out8(SPU_BASE + spu_TxBuff, c);  // Put char
  
  while ((in8(SPU_BASE + spu_LineStat_rc) & 04) != 04)
    {
      if (in8(SPU_BASE + spu_Handshk_rc) != 00)
        out8(SPU_BASE + spu_Handshk_rc, 0xff);  // Clear Handshake
    }
}


void
serial_puts (const char *s)
{
  while (*s)
    {
      serial_putc (*s++);
    }
}


int serial_getc()
{
  unsigned char status = 0;

  while(1)
    {
      status=in8(asyncLSRport1);
      if ((status&asyncLSRDataReady)!=0x0)
        {
          break;
        }
      else if ((status&(asyncLSRFramingError|asyncLSROverrunError|
                        asyncLSRParityError|asyncLSRBreakInterrupt))!=0)
        {
          (void)out8(asyncLSRport1, asyncLSRFramingError|asyncLSROverrunError|
                     asyncLSRParityError|asyncLSRBreakInterrupt);
        }
    }
  return(0x000000ff&(int)in8(asyncRxBufferport1));
}


int
serial_tstc()
{
   unsigned char status;
   
   status=in8(asyncLSRport1);
   if ((status&asyncLSRDataReady)!=0x0) 
     {
       return(1);
     }
   if ((status&(asyncLSRFramingError|asyncLSROverrunError|asyncLSRParityError|
                asyncLSRBreakInterrupt))!=0) 
     {
       (void)out8(asyncLSRport1, asyncLSRFramingError|asyncLSROverrunError|
                  asyncLSRParityError|asyncLSRBreakInterrupt);
     }
   return 0;
}

#endif  /* CONFIG_IOP480 */


/*****************************************************************************/
#ifdef CONFIG_PPC405GP

#define UART_BASE_PTR   0xF800FFFC;                  /* pointer to uart base */

#define UART0_BASE  0xef600300
#define UART1_BASE  0xef600400

#define UART_RBR    0x00
#define UART_THR    0x00
#define UART_IER    0x01
#define UART_IIR    0x02
#define UART_FCR    0x02
#define UART_LCR    0x03
#define UART_MCR    0x04
#define UART_LSR    0x05
#define UART_MSR    0x06
#define UART_SCR    0x07
#define UART_DLL    0x00
#define UART_DLM    0x01

/*-----------------------------------------------------------------------------+
| Line Status Register.
+-----------------------------------------------------------------------------*/
//#define asyncLSRport1           UART0_BASE+0x05
#define asyncLSRDataReady1            0x01
#define asyncLSROverrunError1         0x02
#define asyncLSRParityError1          0x04
#define asyncLSRFramingError1         0x08
#define asyncLSRBreakInterrupt1       0x10
#define asyncLSRTxHoldEmpty1          0x20
#define asyncLSRTxShiftEmpty1         0x40
#define asyncLSRRxFifoError1          0x80

/*-----------------------------------------------------------------------------+
| Miscellanies defines.
+-----------------------------------------------------------------------------*/
//#define asyncTxBufferport1      UART0_BASE+0x00
//#define asyncRxBufferport1      UART0_BASE+0x00


/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

void
serial_init(ulong cpu_clock, int baudrate)
{
  volatile char val;
  unsigned short br_reg;

  /*
   * Check for sanity
   */
  if ((baudrate < CFG_BAUDRATE_MIN) || (baudrate > CFG_BAUDRATE_MAX))
    baudrate = CFG_BAUDRATE_DEFAULT;
  br_reg = ((((CONFIG_CPUCLOCK * 1000000)/16)/18) / baudrate);

  /*
   * Init onboard 16550 UART
   */
  out8(UART0_BASE + UART_LCR, 0x80);  /* set DLAB bit */
  out8(UART0_BASE + UART_DLL, (br_reg & 0x00ff));  /* set divisor for 9600 baud */
  out8(UART0_BASE + UART_DLM, ((br_reg & 0xff00) >> 8));  /* set divisor for 9600 baud */
  out8(UART0_BASE + UART_LCR, 0x03);  /* line control 8 bits no parity */
  out8(UART0_BASE + UART_FCR, 0x00);  /* disable FIFO */
  out8(UART0_BASE + UART_MCR, 0x00);  /* no modem control DTR RTS */
  val = in8(UART0_BASE + UART_LSR);   /* clear line status */
  val = in8(UART0_BASE + UART_RBR);   /* read receive buffer */
  out8(UART0_BASE + UART_SCR, 0x00);  /* set scratchpad */
  out8(UART0_BASE + UART_IER, 0x00);  /* set interrupt enable reg */
}


void
serial_setbrg (ulong cpu_clock, int baudrate)
{
  unsigned short br_reg;

  /*
   * Check for sanity
   */
  if ((baudrate < CFG_BAUDRATE_MIN) || (baudrate > CFG_BAUDRATE_MAX))
    baudrate = CFG_BAUDRATE_DEFAULT;
  br_reg = ((((CONFIG_CPUCLOCK * 1000000)/16)/18) / baudrate);

  out8(UART0_BASE + UART_LCR, 0x80);  /* set DLAB bit */
  out8(UART0_BASE + UART_DLL, (br_reg & 0x00ff));  /* set divisor for 9600 baud */
  out8(UART0_BASE + UART_DLM, ((br_reg & 0xff00) >> 8));  /* set divisor for 9600 baud */
  out8(UART0_BASE + UART_LCR, 0x03);  /* line control 8 bits no parity */
}


void
serial_putc(const char c)
{
  if (c == '\n')
    serial_putc ('\r');
  
  out8(UART0_BASE + UART_THR, c);    /* put character out */
  
  /* check THRE bit, wait for transfer done */
  while ((in8(UART0_BASE + UART_LSR) & 0x20) != 0x20)
    ;
}


void
serial_puts (const char *s)
{
  while (*s)
    {
      serial_putc (*s++);
    }
}


int serial_getc()
{
  unsigned char        status=0;
  
  while(1)
    {
      status = in8(UART0_BASE + UART_LSR);
      if ((status&asyncLSRDataReady1)!=0x0)
        {
          break;
        }
      else if ((status&(asyncLSRFramingError1|asyncLSROverrunError1|
                        asyncLSRParityError1|asyncLSRBreakInterrupt1))!=0)
        {
          out8(UART0_BASE + UART_LSR, asyncLSRFramingError1|asyncLSROverrunError1|
               asyncLSRParityError1|asyncLSRBreakInterrupt1);
        }
    }
  return(0x000000ff&(int)in8(UART0_BASE));
}


int
serial_tstc()
{
  unsigned char        status;
  
  status = in8(UART0_BASE + UART_LSR);
  if ((status&asyncLSRDataReady1)!=0x0)
    {
      return(1);
    }
  if ((status&(asyncLSRFramingError1|asyncLSROverrunError1|asyncLSRParityError1|
               asyncLSRBreakInterrupt1))!=0)
    {
      out8(UART0_BASE + UART_LSR, asyncLSRFramingError1|asyncLSROverrunError1|
           asyncLSRParityError1|asyncLSRBreakInterrupt1);
    }
  return 0;
}

#endif  /* CONFIG_PPC405GP */
