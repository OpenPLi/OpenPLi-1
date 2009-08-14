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
/*-----------------------------------------------------------------------------+
  |
  |  File Name:  enetemac.c
  |
  |  Function:   Device driver for the ethernet EMAC3 macro on the 405GP.
  |
  |  Author:     Mark Wisner
  |
  |  Change Activity-
  |
  |  Date        Description of Change                                       BY
  |  ---------   ---------------------                                       ---
  |  05-May-99   Created                                                     MKW
  |  27-Jun-99   Clean up                                                    JWB
  |  16-Jul-99   Added MAL error recovery and better IP packet handling      MKW
  |  29-Jul-99   Added Full duplex support                                   MKW
  |  06-Aug-99   Changed names for Mal CR reg                                MKW
  |  23-Aug-99   Turned off SYE when running at 10Mbs                        MKW
  |  24-Aug-99   Marked descriptor empty after call_xlc                      MKW
  |  07-Sep-99   Set MAL RX buffer size reg to ENET_MAX_MTU_ALIGNED / 16     MCG
  |              to avoid chaining maximum sized packets. Push starting
  |              RX descriptor address up to the next cache line boundary.
  |  16-Jan-00   Added support for booting with IP of 0x0                    MKW
  |  15-Mar-00   Updated enetInit() to enable broadcast addresses in the 
  |	       EMAC_RXM register.                                          JWB	
  +-----------------------------------------------------------------------------*/

#include <ppcboot.h>
#include <asm/processor.h>
#include <ppc4xx.h>
#include <commproc.h>
#include <405gp_enet.h>
#include <405_mal.h>
#include <miiphy.h>
#include <net.h>
#include <malloc.h>

#ifdef CONFIG_PPC405GP


#define MAXPACKET       1518
#define ENET_MINPACKET  64

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
| Defines for number of buffers and their starting addresses.
| Buffers are in SDRAM space and must be above 0xA000 to avoid conflict
| with the ROM Monitor's data section and must be below 0xF000, where
| the buffers for an PCI ethernet device (if present) would start.
+-----------------------------------------------------------------------------*/
#define NUM_TX_BUFF 1
#define NUM_RX_BUFF 4
#define TX_BUF_START 0x0000A000
#define RX_BUF_START 0x0000A800

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH  1520
/* #define PKTBUFSRX 2 */

/* static char rxbuf[PKTBUFSRX][ DBUF_LENGTH ]; */
static char *txbuf_ptr;

/*-----------------------------------------------------------------------------+
| Defines for TX and RX descriptors.
| IMPORTANT: The descriptors should be placed in NON-CACHEABLE memory since
|            they are 8 bytes each and must be contiguous. If they were
|            placed in cacheable memory, maintaining software cache coherency
|            may not be possible as a cache flush of a single descriptor could
|            corrupt the other three (in real memory) that are within the same
|            cache line. Here we use the SRAM on the board for the descriptors.
|            The 405GP reference board has 512K of SRAM. The first
|            0x1000 bytes should be reserved for this driver.
+-----------------------------------------------------------------------------*/
#define EMAC_TX_DESCRIPTOR_ADDR 0x00ef0000
#define EMAC_RX_DESCRIPTOR_ADDR (EMAC_TX_DESCRIPTOR_ADDR + \
                                (4 * NUM_TX_BUFF * sizeof(mal_desc_t)))

/* define the number of channels implemented */
#define EMAC_RXCHL      1
#define EMAC_TXCHL      1

/*-----------------------------------------------------------------------------+
| Defines for MAL/EMAC interrupt conditions as reported in the UIC (Universal
| Interrupt Controller).
+-----------------------------------------------------------------------------*/
#define MAL_UIC_ERR ( UIC_MAL_SERR | UIC_MAL_TXDE  | UIC_MAL_RXDE)
#define MAL_UIC_DEF  (UIC_MAL_RXEOB | MAL_UIC_ERR)
#define EMAC_UIC_DEF UIC_ENET

/*-----------------------------------------------------------------------------+
| Global variables. TX and RX descriptors and buffers.
+-----------------------------------------------------------------------------*/
static volatile mal_desc_t *tx;
static volatile mal_desc_t *rx;

/* IER globals */
static unsigned long emac_ier;
static unsigned long mal_ier;


/* Statistic Areas */
#define MAX_ERR_LOG 10
struct emac_stats
{
  int data_len_err;
  int rx_frames;
  int rx;
  int rx_prot_err;
};

static struct stats
{                               /* Statistic Block */
  struct emac_stats emac;
  int int_err;
  short tx_err_log[MAX_ERR_LOG];
  short rx_err_log[MAX_ERR_LOG];
} stats;

static int first_init   = 0;

static int tx_err_index = 0;           /* Transmit Error Index for tx_err_log */
static int rx_err_index = 0;           /* Receive Error Index for rx_err_log */

static int rx_slot = 0;                /* MAL Receive Slot */
static int rx_i_index = 0;             /* Receive Interrupt Queue Index */
static int rx_u_index = 0;             /* Receive User Queue Index */
static int rx_ready[NUM_RX_BUFF];      /* Receive Ready Queue */

static int tx_slot = 0;                /* MAL Transmit Slot */
static int tx_i_index = 0;             /* Transmit Interrupt Queue Index */
static int tx_u_index = 0;             /* Transmit User Queue Index */
static int tx_run[NUM_TX_BUFF];        /* Transmit Running Queue */

static volatile int rx_ptr = -1;

static char             emac_hwd_addr[ENET_ADDR_LENGTH];

static void enet_rcv (unsigned long malisr);

/*-----------------------------------------------------------------------------+
| Prototypes and externals.
+-----------------------------------------------------------------------------*/
void mal_err (unsigned long isr, unsigned long uic, unsigned long mal_def,
              unsigned long mal_errr);
void emac_err (unsigned long isr);


void eth_halt(void)
{
  /* EMAC RESET */
  out32 (EMAC_M0, EMAC_M0_SRST);
}


/*-----------------------------------------------------------------------------+
+----------------------------------------------------------------------------*/
int eth_init (bd_t *bis)
{
  int i;
  unsigned long reg;
  unsigned long msr;
  unsigned long speed;
  unsigned long duplex;
  unsigned mode_reg;

  malloc(10);

  msr = get_msr();
  set_msr(msr & ~(MSR_EE));

  /* MAL RESET */
  mtdcr(malmcr, MAL_CR_MMSR);

  tx_err_index = 0;           /* Transmit Error Index for tx_err_log */
  rx_err_index = 0;           /* Receive Error Index for rx_err_log */

  rx_slot = 0;                /* MAL Receive Slot */
  rx_i_index = 0;             /* Receive Interrupt Queue Index */
  rx_u_index = 0;             /* Receive User Queue Index */

  tx_slot = 0;                /* MAL Transmit Slot */
  tx_i_index = 0;             /* Transmit Interrupt Queue Index */
  tx_u_index = 0;             /* Transmit User Queue Index */

  /* EMAC RESET */
  out32 (EMAC_M0, EMAC_M0_SRST);
  for (i = 0; i < 1000; i++);
  out32 (EMAC_M0, in32 (EMAC_M0) & ~EMAC_M0_SRST);

  speed = miiphy_speed();
  printf("ENET Speed is %d Mbs... \n", (int)speed);
  duplex = miiphy_duplex();
  if( duplex == HALF)
    printf("HALF duplex connection\n");
  else
    printf("FULL duplex connection\n");

  /* set the Mal configuration reg */
  mtdcr(malmcr, MAL_CR_PLBB | MAL_CR_OPBBL | MAL_CR_LEA | MAL_CR_PLBLT_DEFAULT);

  /* set up the TX and RX descriptors */
  tx = (mal_desc_t *) EMAC_TX_DESCRIPTOR_ADDR;
  rx = (mal_desc_t *) EMAC_RX_DESCRIPTOR_ADDR;

  for (i = 0; i < NUM_TX_BUFF; i++)
    {
      tx[i].ctrl = 0;
      tx[i].data_len = 0;
      if (first_init == 0)
        txbuf_ptr = (char *)malloc(DBUF_LENGTH);
      tx[i].data_ptr = txbuf_ptr;
      if ((NUM_TX_BUFF - 1) == i)
        tx[i].ctrl |= MAL_TX_CTRL_WRAP;
      tx_run[i] = -1;
#if 0
      printf("TX_BUFF %d @ 0x%08lx\n", i, (ulong)tx[i].data_ptr);
#endif
    }

  for (i = 0; i < NUM_RX_BUFF; i++)
    {
      rx[i].ctrl = 0;
      rx[i].data_len = 0;
      //      rx[i].data_ptr = (char *) &rx_buff[i];
      rx[i].data_ptr = (char *)NetRxPackets[i];
      if ((NUM_RX_BUFF - 1) == i)
        rx[i].ctrl |= MAL_RX_CTRL_WRAP;
      rx[i].ctrl |=   MAL_RX_CTRL_EMPTY | MAL_RX_CTRL_INTR;
      rx_ready[i] = -1;
#if 0
      printf("RX_BUFF %d @ 0x%08lx\n", i, (ulong)rx[i].data_ptr);
#endif
    }

  memcpy(emac_hwd_addr, bis->bi_enetaddr, ENET_ADDR_LENGTH);

  reg = 0x00000000;

  reg |= emac_hwd_addr[0];           /* set high address */
  reg = reg << 8;
  reg |= emac_hwd_addr[1];

  out32 (EMAC_IAH, reg);

  reg = 0x00000000;
  reg |= emac_hwd_addr[2];           /* set low address  */
  reg = reg << 8;
  reg |= emac_hwd_addr[3];
  reg = reg << 8;
  reg |= emac_hwd_addr[4];
  reg = reg << 8;
  reg |= emac_hwd_addr[5];

  out32 (EMAC_IAL, reg);

  /* setup MAL tx & rx channel pointers */
  mtdcr(maltxctp0r, EMAC_TX_DESCRIPTOR_ADDR);
  mtdcr(malrxctp0r, EMAC_RX_DESCRIPTOR_ADDR);

  /* Reset transmit and receive channels */
  mtdcr(malrxcarr, 0x80000000);     /* 2 channels */
  mtdcr(maltxcarr, 0x80000000);     /* 2 channels */
  
  /* Enable MAL transmit and receive channels */
  mtdcr(maltxcasr, 0x80000000);     /* 1 channel */
  mtdcr(malrxcasr, 0x80000000);     /* 1 channel */

  /* set RX buffer size */
  mtdcr(malrcbs0, ENET_MAX_MTU_ALIGNED / 16);

  /* set transmit enable & receive enable */
  out32 (EMAC_M0, EMAC_M0_TXE | EMAC_M0_RXE);

  /* set receive fifo to 4k and tx fifo to 2k */
  mode_reg = EMAC_M1_RFS_4K | EMAC_M1_TX_FIFO_2K;

  /* set speed */
  if (speed == _100BASET)
    mode_reg = mode_reg | EMAC_M1_MF_100MBPS;
  else
    mode_reg = mode_reg & ~0x00C00000;  /* 10 MBPS */
  if( duplex == FULL)
    mode_reg = mode_reg | 0x80000000;

  out32 (EMAC_M1, mode_reg);

  /* Enable broadcast and indvidual address */
  out32 (EMAC_RXM, EMAC_RMR_BAE | EMAC_RMR_IAE /*|
                                                 EMAC_RMR_ARRP| EMAC_RMR_SFCS |
                                                 EMAC_RMR_SP */);

  /* we probably need to set the tx mode1 reg? maybe at tx time */

  /* set transmit request threshold register */
  out32 (EMAC_TRTR, 0x18000000);  /* 256 byte threshold */

  /* set recieve  low/high water mark register */
  out32 (EMAC_RX_HI_LO_WMARK, 0x0f002000);

  /* Frame gap set */
  out32 (EMAC_I_FRAME_GAP_REG, 0x00000008);

#if 0 /* test-only */
  emac_ip_addr = *srcaddr;
#endif

  if(first_init ==0)
    {
      /*
       * Connect interrupt service routines
       */
      irq_install_handler(9, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(10, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(11, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(12, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(13, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(14, (interrupt_handler_t *)enetInt, NULL);
      irq_install_handler(15, (interrupt_handler_t *)enetInt, NULL);
    }
  
  /* set up interrupt handler */
  /* setup interrupt controler to take interrupts from the MAL &
     EMAC */
  if(first_init ==0)
    mtdcr(uicer, mfdcr(uicer) | MAL_UIC_DEF | EMAC_UIC_DEF);

  /* set the MAL IER ??? names may change with new spec ??? */
  mal_ier = MAL_IER_DE | MAL_IER_NE | MAL_IER_TE | MAL_IER_OPBE |
    MAL_IER_PLBE;
  mtdcr(malier, mal_ier);

  /* Set EMAC IER */
  emac_ier = EMAC_ISR_PTLE |
    EMAC_ISR_BFCS | EMAC_ISR_PTLE |
    EMAC_ISR_ORE | EMAC_ISR_IRE;
  if(speed == _100BASET)
    emac_ier = emac_ier | EMAC_ISR_SYE;

  out32 (EMAC_IER, emac_ier);

  /*  if(first_init ==0)*/ /* test-only */
    set_msr(msr);            /* enable external */

  first_init =1;
  return (0);
}


/*-----------------------------------------------------------------------------+
+-----------------------------------------------------------------------------*/
int eth_send(volatile void *ptr, int len)
{
  struct enet_frame *ef_ptr;
  ulong time_start, time_now;
  unsigned long temp_txm0;


  ef_ptr = (struct enet_frame *) ptr;

  /*-----------------------------------------------------------------------+
    |  Copy in our address into the frame.
    +-----------------------------------------------------------------------*/
  (void) memcpy (ef_ptr->source_addr, emac_hwd_addr, ENET_ADDR_LENGTH);

  /*-----------------------------------------------------------------------+
    | If frame is too long or too short, modify length.
    +-----------------------------------------------------------------------*/
  if (len > ENET_MAX_MTU)
    len = ENET_MAX_MTU;

  //  memcpy ((void *) &tx_buff[tx_slot], (const void *) ptr, len);
  memcpy ((void *) txbuf_ptr, (const void *) ptr, len);

  /*-----------------------------------------------------------------------+
    | set TX Buffer busy, and send it
    +-----------------------------------------------------------------------*/
  tx[tx_slot].ctrl = (MAL_TX_CTRL_LAST |
                      EMAC_TX_CTRL_GFCS | EMAC_TX_CTRL_GP) &
    ~(EMAC_TX_CTRL_ISA | EMAC_TX_CTRL_RSA);
  if ((NUM_TX_BUFF - 1) == tx_slot)
    tx[tx_slot].ctrl |= MAL_TX_CTRL_WRAP;

  tx[tx_slot].data_len = (short) len;
  tx[tx_slot].ctrl |= MAL_TX_CTRL_READY;

  out32 (EMAC_TXM0, in32 (EMAC_TXM0) | EMAC_TXM0_GNP0);

  /*-----------------------------------------------------------------------+
    | poll unitl the packet is sent and then make sure it is OK
    +-----------------------------------------------------------------------*/
  time_start = get_timer (0);
  while (1)
    {
      temp_txm0 = in32(EMAC_TXM0);
      /* loop until either TINT turns on or 3 seconds elapse */
      if ((temp_txm0 & EMAC_TXM0_GNP0) != 0)
        {
          /* transmit is done, so now check for errors */
          /* there is an error, an inerrupt should hapen when we
             return                                    */
          time_now = get_timer (0);
          if ((time_now-time_start) > 3000)
            {
              return(-1);
            }
        } else
          return (0);

    } /* while */
}


/*-----------------------------------------------------------------------------+
| EnetInt.
| EnetInt is the interrupt handler.  It will determine the
| cause of the interrupt and call the apporpriate servive
| routine.
+-----------------------------------------------------------------------------*/
int
enetInt ()
{
  int serviced;
  int rc = -1;                  /* default to not us */
  unsigned long mal_isr;
  unsigned long emac_isr;
  unsigned long mal_rx_eob;
  unsigned long my_uicmsr;


  /* enter loop that stays in interrupt code until nothing to service */
  do
    {
      serviced = 0;

      my_uicmsr = mfdcr(uicmsr);
      if ((my_uicmsr & (MAL_UIC_DEF | EMAC_UIC_DEF)) == 0)
        {                           /* not for us */

          return (rc);
        }


      /* get and clear controller status interrupts */
      /* look at Mal and EMAC interrupts */
      if ((MAL_UIC_DEF & my_uicmsr) != 0)
        {                           /* we have a MAL interrupt */
          mal_isr = mfdcr(malesr);
          /* look for mal error */
          if ((my_uicmsr & MAL_UIC_ERR) !=0)
            {
              mal_err (mal_isr, my_uicmsr,MAL_UIC_DEF,MAL_UIC_ERR);
              serviced = 1;
              rc = 0;
            }
        }
      if ((EMAC_UIC_DEF & my_uicmsr) !=0)
        {                           /* look for EMAC errors */
          emac_isr = in32 (EMAC_ISR);
          if ((emac_ier & emac_isr)!=0)
            {
              emac_err (emac_isr);
              serviced = 1;
              rc = 0;
            }
        }
      if (((emac_ier & emac_isr)!=0) | ((MAL_UIC_ERR & my_uicmsr) != 0))
        {
          mtdcr(uicsr, MAL_UIC_DEF | EMAC_UIC_DEF); /* Clear */
          return(rc);           /* we had errors so get out */
        }


      /* handle MAL RX EOB  interupt from a receive */
      /* check for EOB on valid channels            */
      if ((my_uicmsr & UIC_MAL_RXEOB) !=0)
        {
          mal_rx_eob = mfdcr(malrxeobisr);
          if ((mal_rx_eob & 0x80000000) !=0)
            {                         /* call emac routine for channel 0 */
              /* clear EOB
                 mtdcr(malrxeobisr, mal_rx_eob); */
              enet_rcv (emac_isr);
              /* indicate that we serviced an interrupt */
              serviced = 1;
              rc = 0;
            }
        }
      mtdcr(uicsr, MAL_UIC_DEF | EMAC_UIC_DEF); /* Clear */
    }
  while (serviced);

  if (rc != 0)
    {
    }

  return (rc);
}

/*-----------------------------------------------------------------------------+
|  MAL Error Routine
+-----------------------------------------------------------------------------*/
void mal_err (unsigned long isr, unsigned long uic, unsigned long maldef,
              unsigned long mal_errr)
{
  mtdcr(malesr, isr); /* clear interrupt */
  printf ("MAL error occured.... ISR = %lx UIC = = %lx  MAL_DEF = %lx  MAL_ERR= %lx \n",
          isr, uic, maldef, mal_errr);

#if 0
  eth_init(NULL);
#endif

  mtdcr(malrxcarr, 0x80000000);
  mtdcr(malrxcarr, 0x00000000);
  mtdcr(malrxcasr, 0x80000000);

  mtdcr(maltxcarr, 0x80000000);     /* 2 channels */
  mtdcr(maltxcarr, 0x00000000);
  mtdcr(maltxcasr, 0x80000000);     /* 1 channel */
}

/*-----------------------------------------------------------------------------+
|  EMAC Error Routine
+-----------------------------------------------------------------------------*/
void emac_err (unsigned long isr)
{
  printf ("EMAC error occured.... ISR = %lx\n", isr);
  out32(EMAC_ISR, isr);
}

/*-----------------------------------------------------------------------------+
|  enet_rcv() handles the ethernet receive data
+-----------------------------------------------------------------------------*/
static void enet_rcv (unsigned long malisr)

{
  struct enet_frame *ef_ptr;
  unsigned long data_len;
  unsigned long rx_eob_isr;

  int handled=0;
  int i;

  rx_eob_isr = mfdcr(malrxeobisr);
  if ((0x80000000 >>( EMAC_RXCHL-1)) & rx_eob_isr)
    {
      /* clear EOB */
      mtdcr(malrxeobisr, rx_eob_isr);

      /* EMAC RX done */
      while (1)
        {                           /* do all */
          i = rx_slot;

          if (MAL_RX_CTRL_EMPTY & rx[i].ctrl)
            break;
          rx_slot++;
          if (NUM_RX_BUFF == rx_slot)
            rx_slot = 0;
          handled++;
          data_len = (unsigned long) rx[i].data_len;  /* Get len */
          if (data_len)
            {
              if (data_len > ENET_MAX_MTU)  /* Check len */
                data_len = 0;
              else
                {
                  if (EMAC_RX_ERRORS & rx[i].ctrl)
                    {                     /* Check Errors */
                      data_len = 0;
                      stats.rx_err_log[rx_err_index] = rx[i].ctrl;
                      rx_err_index++;
                      if (rx_err_index == MAX_ERR_LOG)
                        rx_err_index = 0;
                    }                     /* emac_erros         */
                }                       /* data_len < max mtu */
            }                         /* if data_len        */
          if (!data_len)
            {                         /* no data */
              rx[i].ctrl |= MAL_RX_CTRL_EMPTY;  /* Free Recv Buffer */

              stats.emac.data_len_err++;  /* Error at Rx */
            } /* !data_len */
          else
            {
              stats.emac.rx_frames++;
              stats.emac.rx += data_len;
              ef_ptr = (struct enet_frame *) rx[i].data_ptr;

              /*
               * Set rx pointer
               */
              rx_ptr = i;

              //              printf("X");  /* test-only */
              rx[i].ctrl |= MAL_RX_CTRL_EMPTY;  /* Free Recv Buffer */

            }                         /* if data_len */
        }                           /* while */
    }                             /* if EMACK_RXCHL */
}


int eth_rx(void)
{
  int length;
  int i;
  unsigned long msr;

  for (;;)
    {
      if (rx_ptr == -1)
        {
          length = -1;
          break;     /* nothing received - leave for() loop */
        }

      msr = get_msr();
      set_msr(msr & ~(MSR_EE));

      i = rx_ptr;
      length = rx[i].data_len;
      
      /* Pass the packet up to the protocol layers. */
      //      NetReceive(NetRxPackets[rxIdx], length - 4);
      //      NetReceive(NetRxPackets[i], length);
      NetReceive(NetRxPackets[i], length - 4);

      rx_ptr = -1;

      set_msr(msr);                 /* Enable IRQ's */
    }
  return length;
}

#endif /* CONFIG_PPC405GP */
