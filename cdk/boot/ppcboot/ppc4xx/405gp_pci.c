/*-----------------------------------------------------------------------------+
|
|       This source code has been made available to you by IBM on an AS-IS
|       basis.  Anyone receiving this source is licensed under IBM
|       copyrights to use it in any way he or she deems fit, including
|       copying it, modifying it, compiling it, and redistributing it either
|       with or without modifications.  No license under IBM patents or
|       patent applications is to be implied by the copyright license.
|
|       Any user of this software should understand that IBM cannot provide
|       technical support for this software and will not be responsible for
|       any consequences resulting from the use of this software.
|
|       Any person who transfers this source code or any derivative work
|       must include the IBM copyright notice, this paragraph, and the
|       preceding two paragraphs in the transferred software.
|
|       COPYRIGHT   I B M   CORPORATION 1995
|       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+-----------------------------------------------------------------------------*/

#include <ppcboot.h>
#include <command.h>
#include <cmd_boot.h>
#include <405gp_pci.h>
#include <asm/processor.h>

#ifdef CONFIG_PPC405GP

#ifdef CONFIG_PCI_PNP

/*#define DEBUG*/

/*------------------------------------------------------------------------
| These are the lowest addresses allowed for PCI configuration.
| They correspond to lowest available I/O and Memory addresses.
| In the case where where multiple PMM regs are being used to map
| different PLB to PCI regions, each region should have it's own
| minimum address.
+-----------------------------------------------------------------------*/
unsigned long    LowestMemAddr1 = MIN_PCI_MEMADDR1;
unsigned long    LowestMemAddr2 = MIN_PCI_MEMADDR2;
unsigned long    LowestIOAddr   = MIN_PCI_PCI_IOADDR;

unsigned long    MaxBusNum = 0;

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


/*-----------------------------------------------------------------------------+
| pci_init.  Initializes the 405GP PCI Configuration regs.
+-----------------------------------------------------------------------------*/
void pci_init(void)
{
   unsigned short temp_short;

   /*--------------------------------------------------------------------------+
   | 405GP PCI Master configuration.
   | Map one 512 MB range of PLB/processor addresses to PCI memory space.
   |   PLB address 0x80000000-0x9FFFFFFF ==> PCI address 0x80000000-0x9FFFFFFF
   |   Use byte reversed out routines to handle endianess.
   +--------------------------------------------------------------------------*/
   out32r(PMM0MA,    0x00000000);        /* ensure disabled b4 setting PMM0LA */
   out32r(PMM0LA,    0x80000000);
   out32r(PMM0PCILA, 0x80000000);
   out32r(PMM0PCIHA, 0x00000000);
   out32r(PMM0MA,    0xE0000001);        /* no prefetching, and enable region */

   /*--------------------------------------------------------------------------+
   | Map one 512 MB range of PLB/processor addresses to PCI memory space.
   |   PLB address 0xA0000000-0xBFFFFFFF ==> PCI address 0x00000000-0x1FFFFFFF
   |   Use byte reversed out routines to handle endianess.
   | This space is for the VGA card.
   +--------------------------------------------------------------------------*/
   out32r(PMM1MA,    0x00000000);        /* ensure disabled b4 setting PMM1LA */
   out32r(PMM1LA,    0xA0000000);
   out32r(PMM1PCILA, 0x00000000);
   out32r(PMM1PCIHA, 0x00000000);
   out32r(PMM1MA,    0xE0000001);        /* no prefetching, and enable region */

   /*--------------------------------------------------------------------------+
   | PMM2 is not used.  Initialize them to zero.
   +--------------------------------------------------------------------------*/
   out32r(PMM2MA,    0x00000000);        /* ensure disabled b4 setting PMM2LA */
   out32r(PMM2LA,    0x00000000);
   out32r(PMM2PCILA, 0x00000000);
   out32r(PMM2PCIHA, 0x00000000);
   out32r(PMM2MA,    0x00000000);         /* not enabled */

   /*--------------------------------------------------------------------------+
   | 405GP PCI Target configuration.  (PTM1)
   | Map one 2 GB range of PCI addresses to PLB/processor address space.
   |   PCI address 0x00000000-0x7FFFFFFF ==> PLB address 0x00000000-0x7FFFFFFF
   |   The 0x00000008 default value of the 405GP PTM1 Base Address Register
   |   (in the PCI config header) is correct for this mapping.
   | Note: PTM1MS is hardwire enabled but we set the enable bit anyway.
   +--------------------------------------------------------------------------*/
   out32r(PTM1LA,    0x00000000);
   out32r(PTM1MS,    0x80000001);       /* 2GB, enable bit is hard-wired to 1 */

   /*--------------------------------------------------------------------------+
   | 405GP PCI Target configuration.  (PTM2) Not used here.
   | It is possible that the enable bit in PTM2MS could be set at power
   | up.  The ROM monitor only needs to use PTM1, so we must make sure that
   | PTM2 is disabled to avoid PCI target conflicts.
   | Note: PTM2MS must be enabled to write PTM 2 BAR.
   | Zero out PTM 2 BAR, then disable via PTM2MS.
   +--------------------------------------------------------------------------*/
   out32r(PTM2LA,    0x00000000);
   out32r(PTM2MS,    0x00000001);        /* set enable bit */
   PCI_Write_CFG_Reg(PCIDEVID_405GP, PCIBASEADDR2, 0x00000000, 4);
   out32r(PTM2MS,    0x00000000);        /* disable */

   /*--------------------------------------------------------------------------+
   | Write the 405GP PCI Configuration regs.
   |   Enable 405GP to be a master on the PCI bus (PMM).
   |   Enable 405GP to act as a PCI memory target (PTM).
   +--------------------------------------------------------------------------*/
   temp_short = PCI_Read_CFG_Reg(PCIDEVID_405GP, PCICMD, 2);
   PCI_Write_CFG_Reg(PCIDEVID_405GP, PCICMD, temp_short | BM_EN | MEM_EN, 2);

#if 0  /* test-only: no need yet! */
   /*--------------------------------------------------------------------------+
   | If PCI speed = 66Mhz, set 66Mhz capable bit.
   +--------------------------------------------------------------------------*/
   if (board_cfg.pci_speed==66666666) {
      temp_short = PCI_Read_CFG_Reg(PCIDEVID_405GP, PCISTATUS, 2);
      PCI_Write_CFG_Reg(PCIDEVID_405GP,PCISTATUS,(temp_short|CAPABLE_66MHZ), 2);
   }
#endif

#if 0  /* test-only: no need yet! */
   /*--------------------------------------------------------------------------+
   | Default value of the Bridge Options1 register is OK (0xFF60).
   | Default value of the Bridge Options2 register is OK (0x0100).
   | No need to change them in pass 2 405GP.
   | Low subsequent target latency timer values are not supported in pass 1.
   |      STLD = '1111' if asynchronous PCI
   |      STLD = '0111' if synchronous PCI
   +--------------------------------------------------------------------------*/
   /* The following should only be required for PASS 1 405GP because of       */
   /* errata #26                                                              */
   if (ppcMfpvr() == PVR_405GP_PASS1) 
     {
      temp_short = PCI_Read_CFG_Reg(PCIDEVID_405GP, PCIBRDGOPT2, 2);
      if (ppcMfstrap() & PCI_ASYNC_MODE_EN) {
        PCI_Write_CFG_Reg(PCIDEVID_405GP, PCIBRDGOPT2,(temp_short | 0x0F00), 2);
      }
      else {
        PCI_Write_CFG_Reg(PCIDEVID_405GP, PCIBRDGOPT2,(temp_short | 0x0700), 2);
      }
   }
#endif

   /*--------------------------------------------------------------------------+
   | Scan the PCI bus and configure devices found.
   +--------------------------------------------------------------------------*/
   PCI_Scan(0);

}

/*-----------------------------------------------------------------------------+
|  Subroutine:  PCI_Read_CFG_Reg
|  Description: Read a PCI configuration register
|  Inputs:
|               BusDevFunc      PCI Bus+Device+Function number
|               Reg             Configuration register number
|               Width           Number of bytes to read (1, 2, or 4)
|  Return value:
|               (unsigned int)  Value of the configuration register read.
|                      For reads shorter than 4 bytes, return value
|                      is LSB-justified
+-----------------------------------------------------------------------------*/
unsigned int     PCI_Read_CFG_Reg(int BusDevFunc, int Reg, int Width)
{
   unsigned int    RegAddr;

   /*--------------------------------------------------------------------------+
   | bit 31 must be 1 and bits 1:0 must be 0 (note Little Endian bit notation)
   +--------------------------------------------------------------------------*/
   RegAddr = 0x80000000 | ((Reg|BusDevFunc) & 0xFFFFFFFC);

   /*--------------------------------------------------------------------------+
   | Write reg to PCI Config Address
   +--------------------------------------------------------------------------*/
   out32r(PCICFGADR, RegAddr);

   /*--------------------------------------------------------------------------+
   | Read reg value from PCI Config Data
   +--------------------------------------------------------------------------*/
   switch (Width)
   {
      case 1: return ((unsigned int) in8(PCICFGDATA | (Reg & 0x3)));
      case 2: return ((unsigned int) in16r(PCICFGDATA | (Reg & 0x3)));
      case 4: return (in32r(PCICFGDATA | (Reg & 0x3)));
   }

   return 0; /* not reached: just to satisfy the compiler */
}

/*-----------------------------------------------------------------------------+
|  Subroutine:  PCI_Write_CFG_Reg
|  Description: Write a PCI configuration register.
|  Inputs:
|               BusDevFunc      PCI Bus+Device+Function number
|               Reg             Configuration register number
|               Value           Configuration register value
|               Width           Number of bytes to write (1, 2, or 4)
|  Return value:
|               0       Successful
| Updated for pass2 errata #6. Need to disable interrupts and clear the
| PCICFGADR reg after writing the PCICFGDATA reg.  
+-----------------------------------------------------------------------------*/
int    PCI_Write_CFG_Reg(int BusDevFunc, int Reg, unsigned int Value, int Width)
{
   unsigned int    RegAddr;
   unsigned int    msr;

   /*--------------------------------------------------------------------------+
   | Ensure interrupts disabled for pass2 errata #6.
   +--------------------------------------------------------------------------*/
   msr = get_msr();
   set_msr(msr & ~(MSR_EE|MSR_CE));

   /*--------------------------------------------------------------------------+
   | bit 31 must be 1 and bits 1:0 must be 0 (note Little Endian bit notation)
   +--------------------------------------------------------------------------*/
   RegAddr = 0x80000000 | ((Reg|BusDevFunc) & 0xFFFFFFFC);

   /*--------------------------------------------------------------------------+
   | Write reg to PCI Config Address
   +--------------------------------------------------------------------------*/
   out32r(PCICFGADR, RegAddr);

   /*--------------------------------------------------------------------------+
   | Write reg value to PCI Config Data
   +--------------------------------------------------------------------------*/
   switch (Width)
   {
      case 1: out8(PCICFGDATA | (Reg & 0x3), (unsigned char)(Value & 0xFF));
              break;
      case 2: out16r(PCICFGDATA | (Reg & 0x3),(unsigned short)(Value & 0xFFFF));
              break;
      case 4: out32r(PCICFGDATA | (Reg & 0x3), Value);
              break;
   }

   /*--------------------------------------------------------------------------+
   | Write PCI Config Address after writing PCICFGDATA for pass2 errata #6.
   +--------------------------------------------------------------------------*/
   out32r(PCICFGADR, 0x00000000);

   /*--------------------------------------------------------------------------+
   | Restore msr (for pass2 errata #6).
   +--------------------------------------------------------------------------*/
   set_msr(msr);

   return (0);
}

/*-----------------------------------------------------------------------
|
|  Subroutine:  PCI_Scan
|
|  Prototype:   void    PCI_Scan(int BusNum)
|
|  Description: Scan through all 16 allowable PCI IDs and configure
|               those for which the vendor ID indicates there is a
|               device present.
|
|  Inputs:
|               BusNum  Bus number where scanning begins
|
|  Return value:
|               None
|
+----------------------------------------------------------------------*/
void    PCI_Scan(int BusNum)
{
   int                     Device;
   int                     BusDevFunc;

   /*--------------------------------------------------------------------------+
   | Start with device 1, the 405GP is device 0.  MCG 01/04/99
   +--------------------------------------------------------------------------*/
   for (Device = 1; Device < MAX_PCI_DEVICES; Device++)
   {
      BusDevFunc = (BusNum << 16) | (Device << 11);

      if (PCI_Read_CFG_Reg(BusDevFunc, PCIVENDORID,2) != 0xFFFF)
      {
#ifdef DEBUG
         printf("Device %d is present\n",Device);
#endif
         switch( PCI_Read_CFG_Reg(BusDevFunc, PCICLASSCODE, 2) )
         {

            case 0x0604:   /*  PCI-PCI Bridge */
               PCI_Config_Device(BusDevFunc, 2);
               PCI_Config_Bridge(BusDevFunc);
               break;

            case 0x0300:   /*  VGA Display controller */
            case 0x0001:   /*  VGA Display controller (pre PCI rev 2.0)*/
               PCI_Config_VGA_Device(BusDevFunc, 6);
               break;

            default:
               PCI_Config_Device(BusDevFunc, 6);
         }
      }
      else
      {
#ifdef DEBUG
         printf("Device %d not present\n",Device);
#endif
      }
   }
}

/*-----------------------------------------------------------------------
|  Subroutine:  PCI_Config_Device
|
|  Description: Configure a PCI device by examining its I/O and memory
|               address space needs and allocating address space to it by
|               programming the address decoders in the Base Address Registers.
|
|  Inputs:
|               BusDevFunc    Bus+Device+Function number
|               NumBaseAddr   Number of base address registers to
|                             configure
|  Return value:
|               None
+----------------------------------------------------------------------*/
void    PCI_Config_Device(int BusDevFunc, int NumBaseAddr)
{
    int     AddrSlot, i;
    unsigned long  AddrDesc, AddrProg, Min_Gnt_Val, int_line;

    for (AddrSlot = 0; AddrSlot < NumBaseAddr; AddrSlot++)
    {
       PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot), 0xFFFFFFFF,4);

       AddrDesc = PCI_Read_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot), 4);

       if (AddrDesc == 0)                   /* unimplemented, stop looking */
          continue;                         /* 01/04/99 MCG */

#ifdef DEBUG
       printf("Read Base Addr Reg %d = 0x%08x\n",AddrSlot,AddrDesc);
#endif

       if ((AddrDesc & 1) == 0)        /* Memory */
       {
          AddrDesc &= 0xFFFFFFF0;

          for (i = 0; (AddrDesc & 1) != 1; i++)
             AddrDesc = AddrDesc >> 1;

          AddrDesc = 1 << i;

          if ((unsigned long)AddrDesc < 4096)
             AddrDesc = 4096;
#ifdef DEBUG
          printf("    PCI Memory space = 0x%x bytes \n",AddrDesc);
#endif
          for (AddrProg = MIN_PCI_MEMADDR1; AddrProg < LowestMemAddr1; AddrProg += AddrDesc);

          PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot),AddrProg,4);
          LowestMemAddr1 = AddrProg + AddrDesc;
       }
       else                            /* I/O */
       {
          AddrDesc &= 0xFFFFFFFC;

          for (i = 0; (AddrDesc & 1) != 1; i++)
             AddrDesc = AddrDesc >> 1;

          AddrDesc = 1 << i;
#ifdef DEBUG
          printf("    PCI I/O space = 0x%x bytes \n",AddrDesc);
#endif
          for (AddrProg = MIN_PCI_PCI_IOADDR; AddrProg < LowestIOAddr; AddrProg += AddrDesc);

          PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot),AddrProg,4);
          LowestIOAddr = AddrProg + AddrDesc;
       }

    }

    Min_Gnt_Val = 0x80;
    PCI_Write_CFG_Reg(BusDevFunc, PCILATENCYTIM, Min_Gnt_Val, 1);

    /*
     * Write pci interrupt line register (cpci405 specific)
     */
    switch ((BusDevFunc >> 11) & 0x03)
      {
      case 0:
        int_line = 27 + 2;
        break;
      case 1:
        int_line = 27 + 3;
        break;
      case 2:
        int_line = 27 + 0;
        break;
      case 3:
        int_line = 27 + 1;
        break;
      }
    PCI_Write_CFG_Reg(BusDevFunc, PCIINTLINE, int_line, 1);

    /*
     * Enable i/o space, memory space and master on this device
     */
    PCI_Write_CFG_Reg(BusDevFunc, PCICMD, 7, 2);

}

/*-----------------------------------------------------------------------
|  Subroutine:  PCI_Config_VGA_Device
|
|  Description: Configure a PCI VGA device by examining its I/O and memory
|               address space needs and allocating address space to it by
|               programming the address decoders in the Base Address Registers.
|
|  Inputs:
|               BusDevFunc    Bus+Device+Function number
|               NumBaseAddr   Number of base address registers to
|                             configure
|  Return value:
|               None
+----------------------------------------------------------------------*/
void    PCI_Config_VGA_Device(int BusDevFunc, int NumBaseAddr)
{
    int     AddrSlot, i;
    unsigned long  AddrDesc, AddrProg, Min_Gnt_Val;

    for (AddrSlot = 0; AddrSlot < NumBaseAddr; AddrSlot++)
    {
       PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot), 0xFFFFFFFF,4);

       AddrDesc = PCI_Read_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot), 4);

       if (AddrDesc == 0)                   /* unimplemented, stop looking */
          continue;                         /* 01/04/99 MCG */

#ifdef DEBUG
       printf("Read Base Addr Reg %d = 0x%08x\n",AddrSlot,AddrDesc);
#endif

       if ((AddrDesc & 1) == 0)        /* Memory */
       {
          AddrDesc &= 0xFFFFFFF0;

          for (i = 0; (AddrDesc & 1) != 1; i++)
             AddrDesc = AddrDesc >> 1;

          AddrDesc = 1 << i;

          if ((unsigned long)AddrDesc < 4096)
             AddrDesc = 4096;
#ifdef DEBUG
          printf("    PCI Memory space = 0x%x bytes \n",AddrDesc);
#endif
          for (AddrProg = MIN_PCI_MEMADDR2; AddrProg < LowestMemAddr2; AddrProg += AddrDesc);

          PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot),AddrProg,4);
          LowestMemAddr2 = AddrProg + AddrDesc;
       }
       else                            /* I/O */
       {
          AddrDesc &= 0xFFFFFFFC;

          for (i = 0; (AddrDesc & 1) != 1; i++)
             AddrDesc = AddrDesc >> 1;

          AddrDesc = 1 << i;
#ifdef DEBUG
          printf("    PCI I/O space = 0x%x bytes \n",AddrDesc);
#endif
          for (AddrProg = MIN_PCI_PCI_IOADDR; AddrProg < LowestIOAddr; AddrProg
+= AddrDesc);

          PCI_Write_CFG_Reg(BusDevFunc, PCIBASEADDR0 + (4*AddrSlot),AddrProg,4);
          LowestIOAddr = AddrProg + AddrDesc;
       }

    }

    Min_Gnt_Val = 0x80;
    PCI_Write_CFG_Reg(BusDevFunc, PCILATENCYTIM, Min_Gnt_Val, 1);
}

/*-----------------------------------------------------------------------
|
|  Subroutine:  PCI_Config_Bridge
|
|  Prototype:   void    PCI_Config_Bridge(int BusDevFunc)
|
|  Description: Configure a PCI-PCI bridge
|
|  Inputs:
|               BusDevFunc      Bus+Device+Function number
|
|  Return value:
|               None
|
+----------------------------------------------------------------------*/
void    PCI_Config_Bridge(int BusDevFunc)
{
   int     SecondaryBus;
   int     PrimaryBus;
   int     CommandReg_Val;
   int     InitialLowestIOAddr, InitialLowestMemAddr;
   int     IOBase, MemBase;
   int     IOLimit, MemLimit;

   InitialLowestIOAddr = LowestIOAddr;
   InitialLowestMemAddr = LowestMemAddr1;

   CommandReg_Val = PCI_Read_CFG_Reg(BusDevFunc, PCICMD, 2);

   /* Configure bridge's base address registers */

   PCI_Config_Device(BusDevFunc, 2);

   if ( LowestIOAddr > InitialLowestIOAddr )    /* bridge uses IO space?     */
      CommandReg_Val |= 0x01;                   /* enable I/O Space          */

   if ( LowestMemAddr1 > InitialLowestMemAddr )  /* bridge uses memory space? */
      CommandReg_Val |= 0x02;                   /* enable Memory Space       */

   PrimaryBus = (BusDevFunc >> 16) & 0xFF;
   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_PRIMARYBUS, PrimaryBus, 1);

   SecondaryBus = ++MaxBusNum;
   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_SECONDARYBUS, SecondaryBus, 1);

   /* Start with max. possible value for subordinate bus number                    */
   /* Later, after any additional child busses are found, we'll update this        */

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_SUBORDINATEBUS, 0xFF, 1);

   /* IO Base must be on 4Kb boundary.  Adjust if needed */

   if ((LowestIOAddr % 4096) != 0)
      LowestIOAddr += 4096 - (LowestIOAddr % 4096);

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_IOBASE, (LowestIOAddr>>8) & 0xF0, 1);
   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_IOBASEUPPER16, (LowestIOAddr>>16) & 0xFFFF, 2);

   IOBase = LowestIOAddr;

   /* Mem Base must be on 1 MB boundary.  adjust if needed */
   if ((LowestMemAddr1 % 0x100000) != 0)
      LowestMemAddr1 += 0x100000 - (LowestMemAddr1 % 0x100000);

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_MEMBASE, (LowestMemAddr1>>16) & 0xFFF0, 2);
   MemBase = LowestMemAddr1;

   PCI_Scan(SecondaryBus);

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_SUBORDINATEBUS, MaxBusNum, 1);

   IOLimit = LowestIOAddr;
   if (LowestIOAddr > IOBase)           /* IO space used on secondary bus?   */
   {
      CommandReg_Val |= 0x01;           /*   enable IO Space                 */
      IOLimit--;                        /*   IOLimit is highest used address */
   }

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_IOLIMIT, ((IOLimit)>>8) & 0xF0, 1);
   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_IOLIMITUPPER16, ((IOLimit)>>16) & 0xFFFF, 2);

   /* IOLIMIT is the starting address of a 4K block forwarded by the bridge.       */
   /* Round LowestIOAddr up to the next 4K boundary if IO space is enabled.        */

   if ((CommandReg_Val & 0x01) == 0x01)
      LowestIOAddr = (IOLimit | 0xFFF) + 1;

   MemLimit = LowestMemAddr1;
   if ( LowestMemAddr1 > MemBase )    /* mem. space is used on secondary bus? */
   {
      CommandReg_Val |= 0x02;        /*   enable Memory Space                */
      MemLimit--;                    /*   MemLimit is highest used address   */
   }

   PCI_Write_CFG_Reg(BusDevFunc, PCIPCI_MEMLIMIT, ((MemLimit)>>16) & 0xFFF0, 2);

   /* MEMLIMIT is the starting address of a 1M block forwarded by the bridge.      */
   /* Round LowestMemAddr up to the next 1M boundary if Memory space is enabled.   */

   if ( (CommandReg_Val & 0x02) == 0x02 )
           LowestMemAddr1 = (MemLimit | 0xFFFFF) + 1;

   /* Enable Bus Master on secondary bus */
   CommandReg_Val |= 0x04;

   PCI_Write_CFG_Reg(BusDevFunc, PCICMD, CommandReg_Val, 2);

}

/*-----------------------------------------------------------------------
|  Subroutine:  PCI_Find_Device
|
|  Prototype:   int     PCI_Find_Device(hword VendorID, hword DeviceID);
|
|  Description:
|               Locate a PCI device by vendor and device number
|
|  Inputs:
|               VendorID        Value of the device's Vendor ID field
|               DeviceID        Value of the device's Device ID field
|
|  Return value:
|               < 0     Device not found
|               (int)   PCI Bus+Device+Function number
+----------------------------------------------------------------------*/
int     PCI_Find_Device(unsigned short VendorID, unsigned short DeviceID)
{
   int     Device;
   int     BusDevFunc;
   int     BusNum;

   for (BusNum = MaxBusNum; BusNum >= 0; BusNum--)
      for (Device = 0; Device < MAX_PCI_DEVICES; Device++)
      {
         BusDevFunc = (BusNum << 16) | (Device << 11);

         if (PCI_Read_CFG_Reg(BusDevFunc, PCIVENDORID, 2) == VendorID
          && PCI_Read_CFG_Reg(BusDevFunc, PCIDEVICEID, 2) == DeviceID)
            return (BusDevFunc);
      }

   return (-1);
}


#if (CONFIG_COMMANDS & CFG_CMD_PCI)

void
do_pciinfo(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
  int bus_no = 0;

  if (argc == 2)
    {
      bus_no = (int)simple_strtoul(argv[1], NULL, 10);
    }

  pciinfo(bus_no);
}


/*******************************************************************************
*
* pciinfo - print information about PCI devices
*
*/
void
pciinfo(int bus_no)
{
  int device_no;
  unsigned short vendor_id;
  int BusDevFunc;
  int device_no_start = 0;
  
  printf ("Scanning function 0 of each PCI device on bus %d\n", bus_no);

  if (bus_no == 0)
    device_no_start = 1;
  for (device_no=device_no_start; device_no < MAX_PCI_DEVICES; device_no++)
    {
      BusDevFunc = (bus_no << 16) | (device_no << 11);
      vendor_id = PCI_Read_CFG_Reg(BusDevFunc, PCIVENDORID, 2);

      if (vendor_id != 0xffff)
        {
          printf("\nFound PCI device %d:\n", device_no);
          pciHeaderShow(BusDevFunc);
        }
    }
}


/*******************************************************************************
*
* pciHeaderShow - print a header of the specified PCI device
*
* This routine prints a header of the PCI device specified by BusDevFunc.
*
*/
void
pciHeaderShow(int BusDevFunc)
{
  PCI_HEADER_DEVICE headerDevice;
  PCI_HEADER_BRIDGE headerBridge;
  PCI_HEADER_DEVICE * pD = &headerDevice;
  PCI_HEADER_BRIDGE * pB = &headerBridge;
  
  pD->headerType = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_HEADER_TYPE, 1);

  if (pD->headerType & 0x01)		/* PCI-to-PCI bridge */
    {
      pB->vendorId      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_VENDOR_ID, 2);
      pB->deviceId      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_DEVICE_ID, 2);
      pB->command       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_COMMAND, 2);
      pB->status        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_STATUS, 2);
      pB->revisionId    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_REVISION, 1);
      pB->progIf        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PROGRAMMING_IF, 1);
      pB->subClass      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SUBCLASS, 1);
      pB->classCode     = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_CLASS, 1);
      pB->cacheLine     = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_CACHE_LINE_SIZE, 1);
      pB->latency       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_LATENCY_TIMER, 1);
      pB->headerType    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_HEADER_TYPE, 1);
      pB->bist          = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BIST, 1);
      pB->base0         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_0, 4);
      pB->base1         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_1, 4);
      pB->priBus        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PRIMARY_BUS, 1);
      pB->secBus        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SECONDARY_BUS, 1);
      pB->subBus        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SUBORDINATE_BUS, 1);
      pB->secLatency    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SEC_LATENCY, 1);
      pB->ioBase        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_IO_BASE, 1);
      pB->ioLimit       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_IO_LIMIT, 1);
      pB->secStatus     = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SEC_STATUS, 2);
      pB->memBase       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_MEM_BASE, 2);
      pB->memLimit      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_MEM_LIMIT, 2);
      pB->preBase       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PRE_MEM_BASE, 2);
      pB->preLimit      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PRE_MEM_LIMIT, 2);
      pB->preBaseUpper  = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PRE_MEM_BASE_U, 4);
      pB->preLimitUpper = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PRE_MEM_LIMIT_U, 4);
      pB->ioBaseUpper   = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_IO_BASE_U, 2);
      pB->ioLimitUpper  = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_IO_LIMIT_U, 2);
      pB->romBase       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_ROM_BASE, 4);
      pB->intLine       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BRG_INT_LINE, 1);
      pB->intPin        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BRG_INT_PIN, 1);
      pB->control       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BRIDGE_CONTROL, 2);
      pciBheaderPrint(pB);
    }
  else					/* PCI device */
    {
      pD->vendorId      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_VENDOR_ID, 2);
      pD->deviceId      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_DEVICE_ID, 2);
      pD->command       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_COMMAND, 2);
      pD->status        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_STATUS, 1);
      pD->revisionId    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_REVISION, 1);
      pD->progIf        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_PROGRAMMING_IF, 1);
      pD->subClass      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SUBCLASS, 1);
      pD->classCode     = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_CLASS, 1);
      pD->cacheLine     = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_CACHE_LINE_SIZE, 1);
      pD->latency       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_LATENCY_TIMER, 1);
      pD->headerType    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_HEADER_TYPE, 1);
      pD->bist          = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BIST, 1);
      pD->base0         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_0, 4);
      pD->base1         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_1, 4);
      pD->base2         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_2, 4);
      pD->base3         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_3, 4);
      pD->base4         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_4, 4);
      pD->base5         = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_BASE_ADDRESS_5, 4);
      pD->cis           = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_CIS, 4);
      pD->subVendorId   = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SUB_VENDER_ID, 2);
      pD->subSystemId   = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_SUB_SYSTEM_ID, 2);
      pD->romBase       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_EXPANSION_ROM, 4);
      pD->intLine       = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_DEV_INT_LINE, 1);
      pD->intPin        = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_DEV_INT_PIN, 1);
      pD->minGrant      = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_MIN_GRANT, 1);
      pD->maxLatency    = PCI_Read_CFG_Reg(BusDevFunc, PCI_CFG_MAX_LATENCY, 1);
      pciDheaderPrint(pD);
    }
}

/*******************************************************************************
*
* pciDheaderPrint - print a PCI device header
*
* This routine prints a PCI device header.
*
*/
void
pciDheaderPrint(PCI_HEADER_DEVICE * pD)
{
  printf ("  vendor ID =                   0x%.4x\n", (ushort)pD->vendorId);
  printf ("  device ID =                   0x%.4x\n", (ushort)pD->deviceId);
  printf ("  command register =            0x%.4x\n", (ushort)pD->command);
  printf ("  status register =             0x%.4x\n", (ushort)pD->status);	
  printf ("  revision ID =                 0x%.2x\n", (uchar)pD->revisionId);
  printf ("  class code =                  0x%.2x\n", (uchar)pD->classCode);	
  printf ("  sub class code =              0x%.2x\n", (uchar)pD->subClass);
  printf ("  programming interface =       0x%.2x\n", (uchar)pD->progIf);	
  printf ("  cache line =                  0x%.2x\n", (uchar)pD->cacheLine);
  printf ("  latency time =                0x%.2x\n", (uchar)pD->latency);
  printf ("  header type =                 0x%.2x\n", (uchar)pD->headerType);
  printf ("  BIST =                        0x%.2x\n", (uchar)pD->bist);	
  printf ("  base address 0 =              0x%.8x\n", pD->base0);	
  printf ("  base address 1 =              0x%.8x\n", pD->base1);	
  printf ("  base address 2 =              0x%.8x\n", pD->base2);	
  printf ("  base address 3 =              0x%.8x\n", pD->base3);	
  printf ("  base address 4 =              0x%.8x\n", pD->base4);	
  printf ("  base address 5 =              0x%.8x\n", pD->base5);	
  printf ("  cardBus CIS pointer =         0x%.8x\n", pD->cis);	
  printf ("  sub system vendor ID =        0x%.4x\n", (ushort)pD->subVendorId);
  printf ("  sub system ID =               0x%.4x\n", (ushort)pD->subSystemId);
  printf ("  expansion ROM base address =  0x%.8x\n", pD->romBase);
  printf ("  interrupt line =              0x%.2x\n", (uchar)pD->intLine);
  printf ("  interrupt pin =               0x%.2x\n", (uchar)pD->intPin);	
  printf ("  min Grant =                   0x%.2x\n", (uchar)pD->minGrant);
  printf ("  max Latency =                 0x%.2x\n", (uchar)pD->maxLatency);
}

/*******************************************************************************
*
* pciBheaderPrint - print a PCI-to-PCI bridge header
*
* This routine prints a PCI-to-PCI bridge header.
*
*/
void
pciBheaderPrint(PCI_HEADER_BRIDGE * pB)
{
  printf ("  vendor ID =                   0x%.4x\n", (ushort)pB->vendorId);
  printf ("  device ID =                   0x%.4x\n", (ushort)pB->deviceId);
  printf ("  command register =            0x%.4x\n", (ushort)pB->command);
  printf ("  status register =             0x%.4x\n", (ushort)pB->status);	
  printf ("  revision ID =                 0x%.2x\n", (uchar)pB->revisionId);
  printf ("  class code =                  0x%.2x\n", (uchar)pB->classCode);	
  printf ("  sub class code =              0x%.2x\n", (uchar)pB->subClass);
  printf ("  programming interface =       0x%.2x\n", (uchar)pB->progIf);	
  printf ("  cache line =                  0x%.2x\n", (uchar)pB->cacheLine);
  printf ("  latency time =                0x%.2x\n", (uchar)pB->latency);
  printf ("  header type =                 0x%.2x\n", (uchar)pB->headerType);
  printf ("  BIST =                        0x%.2x\n", (uchar)pB->bist);
  printf ("  base address 0 =              0x%.8x\n", pB->base0);	
  printf ("  base address 1 =              0x%.8x\n", pB->base1);	
  printf ("  primary bus number =          0x%.2x\n", (uchar)pB->priBus);	
  printf ("  secondary bus number =        0x%.2x\n", (uchar)pB->secBus);	
  printf ("  subordinate bus number =      0x%.2x\n", (uchar)pB->subBus);	
  printf ("  secondary latency timer =     0x%.2x\n", (uchar)pB->secLatency);
  printf ("  IO base =                     0x%.2x\n", (uchar)pB->ioBase);
  printf ("  IO limit =                    0x%.2x\n", (uchar)pB->ioLimit);
  printf ("  secondary status =            0x%.4x\n", (ushort)pB->secStatus);
  printf ("  memory base =                 0x%.4x\n", (ushort)pB->memBase);
  printf ("  memory limit =                0x%.4x\n", (ushort)pB->memLimit);	
  printf ("  prefetch memory base =        0x%.4x\n", (ushort)pB->preBase);
  printf ("  prefetch memory limit =       0x%.4x\n", (ushort)pB->preLimit);
  printf ("  prefetch memory base upper =  0x%.8x\n", pB->preBaseUpper);
  printf ("  prefetch memory limit upper = 0x%.8x\n", pB->preLimitUpper);
  printf ("  IO base upper 16 bits =       0x%.4x\n", (ushort)pB->ioBaseUpper);
  printf ("  IO limit upper 16 bits =      0x%.4x\n", (ushort)pB->ioLimitUpper);
  printf ("  expansion ROM base address =  0x%.8x\n", pB->romBase);
  printf ("  interrupt line =              0x%.2x\n", (uchar)pB->intLine);
  printf ("  interrupt pin =               0x%.2x\n", (uchar)pB->intPin);	
  printf ("  bridge control =              0x%.4x\n", (ushort)pB->control);
}

#endif /* CONFIG_COMMANDS & CFG_CMD_PCI */

#endif /* CONFIG_PCI_PNP */

#endif /* CONFIG_PPC405GP */
