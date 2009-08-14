


/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8240      1
#define CONFIG_SANDPOINT    1

#define CONFIG_BAUDRATE     9600
#define CONFIG_DRAM_SPEED   100   /* MHz      */
#define CONFIG_BOOTCOMMAND  "bootm FE020000"    /* autoboot command */
#define CONFIG_BOOTARGS     " "

#define CONFIG_COMMANDS  (CONFIG_CMD_DFL & ~CFG_CMD_NET)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


/*
 * Miscellaneous configurable options
 */
#undef  CFG_LONGHELP            /* undef to save memory     */
#define CFG_PROMPT      ":>"        /* Monitor Command Prompt   */
#define CFG_CBSIZE      256     /* Console I/O Buffer Size  */
#define CFG_PBSIZE      (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS     8       /* max number of command args   */
#define CFG_BARGSIZE    CFG_CBSIZE  /* Boot Argument Buffer Size    */
#define CFG_LOAD_ADDR   0x00100000  /* default load address */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE      0x00000000
#define CFG_FLASH_BASE      0xFFF00000
#define CFG_FLASH_SIZE      ((uint)(512 * 1024))   /* sandpoint has tiny eeprom */

#ifdef DEBUG
  #define CFG_MONITOR_BASE    CFG_SDRAM_BASE
#else
  #define CFG_MONITOR_BASE    CFG_FLASH_BASE
#endif

#ifdef DEBUG
#define CFG_MONITOR_LEN     (4 <<20)	/* if we're running in ram, give us plenty of space for debug info*/
#else
#define CFG_MONITOR_LEN     (512 << 10)	/* Reserve 512 kB for Monitor   */
#endif
#define CFG_MALLOC_LEN      (512 << 10)	/* Reserve 512 kB for malloc()  */

#define CFG_TFTP_LOADADDR   0x00100000	/* default load address */

#define CFG_MEMTEST_START   0x00000000	/* memtest works on */
#define CFG_MEMTEST_END     0x02000000	/* 0 ... 32 MB in DRAM   */

#define CFG_EUMB_ADDR       0xFC000000

#define CFG_ISA_MEM         0xFD000000
#define CFG_ISA_IO          0xFE000000

#define FLASH_BASE0_PRELIM  0xFFF00000  /* sandpoint flash    */
#define FLASH_BASE1_PRELIM  0xFF000000  /* PMC onboard flash*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR CFG_SDRAM_BASE + CFG_MONITOR_LEN
#define CFG_INIT_RAM_END   0x3000  /* End of used area in DPRAM  */
#define CFG_INIT_DATA_SIZE  64  /* size in bytes reserved for initial data */
#define CFG_INIT_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_INIT_DATA_SIZE)
#define CFG_INIT_SP_OFFSET  CFG_INIT_DATA_OFFSET


#define CFG_WINBOND_83C553      1           /*has a winbond bridge  */
#define CFG_USE_WINBOND_IDE     0           /*use winbond 83c553 internal ide controller */
#define CFG_WINBOND_ISA_CFG_ADDR    0x80005800  /*pci-isa bridge config addr */
#define CFG_WINBOND_IDE_CFG_ADDR    0x80005900  /*ide config addr */
#define CFG_NS87308_BADDR_10    1


#define CFG_NS_PC87308UL        1           /* Nat Semi super-io controller on ISA bus */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */


#define CFG_ROMNAL          8       /*rom/flash next access time*/
#define CFG_ROMFAL          16      /*rom/flash access time*/

#define CFG_REFINT          430     /* no of clock cycles between CBR refresh cycles*/
/* the following are for SDRAM only*/
#define CFG_BSTOPRE         604     /* Burst To Precharge, sets open page interval */
#define CFG_REFREC          8       /* Refresh to activate interval */
#define CFG_RDLAT           4       /* data latancy from read command*/
#define CFG_PRETOACT        3       /* Precharge to activate interval */
#define CFG_ACTTOPRE        5       /* Activate to Precharge interval */
#define CFG_SDMODE_CAS_LAT  3       /* SDMODE CAS latancy */
#define CFG_SDMODE_WRAP     0       /* SDMODE wrap type */
#define CFG_SDMODE_BURSTLEN 2       /* SDMODE Burst length 2=4, 3=8 */

#define CFG_REGISTERD_TYPE_BUFFER   1

/* memory bank settings*/
/* only bits 20-29 are actually used from these vales to set the stare/end address
   the upper two bits will be 0, and the lower 20 bits will be set to
   0x00000 for a start address, or 0xfffff for an end address*/

#define CFG_BANK0_START     0x00000000

#define CFG_BANK0_END       0x01ffffff
#define CFG_BANK0_ENABLE    1
#define CFG_BANK1_START     0x01000000
#define CFG_BANK1_END       0x00ffffff
#define CFG_BANK1_ENABLE    0
#define CFG_BANK2_START     0x02000000
#define CFG_BANK2_END       0x02ffffff
#define CFG_BANK2_ENABLE    0
#define CFG_BANK3_START     0x03000000
#define CFG_BANK3_END       0x03ffffff
#define CFG_BANK3_ENABLE    0
#define CFG_BANK4_START     0x04000000
#define CFG_BANK4_END       0x04ffffff
#define CFG_BANK4_ENABLE    0
#define CFG_BANK5_START     0x05000000
#define CFG_BANK5_END       0x05ffffff
#define CFG_BANK5_ENABLE    0
#define CFG_BANK6_START     0x06000000
#define CFG_BANK6_END       0x06ffffff
#define CFG_BANK6_ENABLE    0
#define CFG_BANK7_START     0x07000000
#define CFG_BANK7_END       0x07ffffff
#define CFG_BANK7_ENABLE    0

#define CFG_ODCR            0xff /* configures line driver impedances,
                                    see 8240 book for bit definitions */
#define CFG_PGMAX           0x32 /* how long the 8240 reatins the currently accessed page in memory
                                    see 8240 book for details*/
#define CFG_IBAT0L  FLASH_BASE0_PRELIM | BATL_CACHEINHIBIT | BATL_PP_10
#define CFG_IBAT0U  FLASH_BASE0_PRELIM | BATU_BL_16M | BATU_VS | BATU_VP
#define CFG_IBAT1L  0x00000000 | BATL_CACHEINHIBIT | BATL_PP_10
#define CFG_IBAT1U  0x00000000 | BATU_BL_128M | BATU_VS | BATU_VP
#define CFG_IBAT2L  CFG_ISA_MEM | BATL_MEMCOHERENCE | BATL_PP_10
#define CFG_IBAT2U  CFG_ISA_MEM | BATU_BL_16M | BATU_VS | BATU_VP
#define CFG_IBAT3L  CFG_ISA_IO | BATL_MEMCOHERENCE | BATL_PP_10
#define CFG_IBAT3U  CFG_ISA_IO | BATU_BL_16M | BATU_VS | BATU_VP
#define CFG_DBAT0L  FLASH_BASE0_PRELIM | BATL_MEMCOHERENCE | BATL_WRITETHROUGH | BATL_PP_10
#define CFG_DBAT0U  FLASH_BASE0_PRELIM | BATU_BL_16M | BATU_VS | BATU_VP
#define CFG_DBAT1L  0x00000000 | BATL_MEMCOHERENCE | BATL_WRITETHROUGH | BATL_PP_10
#define CFG_DBAT1U  0x00000000 | BATU_BL_128M | BATU_VS | BATU_VP
#define CFG_DBAT2L  CFG_ISA_MEM | BATL_MEMCOHERENCE | BATL_PP_10
#define CFG_DBAT2U  CFG_ISA_MEM | BATU_BL_16M | BATU_VS | BATU_VP
#define CFG_DBAT3L  CFG_ISA_IO | BATL_MEMCOHERENCE | BATL_PP_10
#define CFG_DBAT3U  CFG_ISA_IO | BATU_BL_16M | BATU_VS | BATU_VP


/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ       (8 << 20)   /* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS 2           /* max number of memory banks       */
#define CFG_MAX_FLASH_SECT  8           /* max number of sectors on one chip    */

#define CFG_FLASH_ERASE_TOUT    120000  /* Timeout for Flash Erase (in ms)  */
#define CFG_FLASH_WRITE_TOUT    500     /* Timeout for Flash Write (in ms)  */

#define CFG_FLASH_ENV_ALIGN 15  /*  Bitshift for Environment Sector */
#define CFG_FLASH_ENV_SIZE  0x4000  /* Total Size of Environment Sector */
/* the other CS:s are determined by looking at parameters in BCSRx */

/* values according to the manual */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE  16          /* For all MPC8xx CPUs          */



/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD           0x01        /* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM           0x02        /* Software reboot          */


/* values according to the manual */

#define CONFIG_DRAM_50MHZ       1
#define CONFIG_SDRAM_50MHZ

#undef	NR_8259_INTS
#define NR_8259_INTS    1


#define CONFIG_DISK_SPINUP_TIME 1000000


#endif  /* __CONFIG_H */
