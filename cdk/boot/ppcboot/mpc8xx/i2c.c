/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
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

#ifdef CONFIG_I2C

#include <commproc.h>
#include <i2c.h>

// **********************************
// ** DEBUG MACROS
// **********************************

// #define CONFIG_I2C_DEBUG
#define DELAY_US	100000 	// us to wait before checking the I2c

// **********************************
// ** CONSTANTS
// **********************************

#define CPCR_FLAG 0x01
#define I2C_CPCR_CMD ( ( 0<<(15-7) ) | ( 1 << (15-11) ) | CPCR_FLAG )
#define I2C_RX_LEN 128 /* Receive buffer length */
#define I2C_TX_LEN 128 /* Transmit buffer length */
#define TXBD_R 0x8000  /* Transmit buffer ready to send */
#define TXBD_W 0x2000  /* Wrap, last buffer in buffer circle */
#define TXBD_L 0x0800  /* Last, this buffer is the last in this frame */
                       /* This bit causes the STOP condition to be sent */
#define TXBD_S 0x0400  /* Start condition.  Causes this BD to transmit a start */
#define RXBD_E 0x8000  /* Receive buffer is empty and can be used by CPM */
#define RXBD_W 0x2000  /* Wrap, last receive buffer in buffer circle */

// **********************************
// ** VARIABLES
// **********************************

#define CPM_I2C_BASE 0x870 
#ifdef CFG_ALLOC_DPRAM
static unsigned char *rxbuf, *txbuf;
#else
static unsigned char rxbuf[I2C_RX_LEN], *txbuf[I2C_TX_LEN];
#endif

static cbd_t *rxbd, *txbd;  	

volatile i2c8xx_t	*i2c;
volatile cbd_t		*tbdf, *rbdf;
volatile iic_t		*iip;

// **********************************
// ** FUNCTIONS
// **********************************

// Returns the best value of I2BRG to meet desired clock speed of I2C with
// input parameters (clock speed, filter, and predivider value).
// It returns computer speed value and the difference between it and desired
// speed.
static inline int i2c_roundrate (int hz, int speed, int filter, int modval, 
				    int *brgval, int *totspeed)
{
    int moddiv = 1 << (5-(modval & 3)),
	brgdiv,
	div;

    brgdiv = hz / (moddiv * speed);

    *brgval = brgdiv / 2 - 3 - 2*filter ;
    
    if ((*brgval < 0) || (*brgval > 255))
	return -1 ;
    
    brgdiv = 2 * (*brgval + 3 + 2 * filter) ;
    div  = moddiv * brgdiv ;
    *totspeed = hz / div ;
        
    return  0;
}

// Sets the I2C clock predivider and divider to meet required clock speed
static int i2c_setrate (int hz, int speed)
{
    immap_t	*immap = (immap_t *)CFG_IMMR ;
    i2c8xx_t	*i2c	= (i2c8xx_t *)&immap->im_i2c;
    int brgval,
	modval,	// 0-3
	bestspeed_diff = speed,
	bestspeed_brgval=0,
	bestspeed_modval=0,
	bestspeed_filter=0,
	totspeed,
	filter=0;	// Use this fixed value 'cause the filter has a bug!

#if 0
    for (filter = 0; filter < 2; filter++)
#endif
	for (modval = 0; modval < 4; modval++)
	    if (i2c_roundrate (	hz, speed, 
				filter, modval, 
				&brgval, &totspeed) == 0)
	    {
		int diff = speed - totspeed ;
		
		if ((diff >= 0) && (diff < bestspeed_diff))
		{
		    bestspeed_diff 	= diff ;
		    bestspeed_modval 	= modval;
		    bestspeed_brgval 	= brgval;
		    bestspeed_filter 	= filter;
		}
	    }

    i2c->i2c_i2mod |= ((bestspeed_modval & 3) << 1) | (bestspeed_filter << 3); 
    i2c->i2c_i2brg = bestspeed_brgval & 0xff;

#ifdef CONFIG_I2C_DEBUG
    printf("i2mod=%08x i2brg=%08x\n", i2c->i2c_i2mod, i2c->i2c_i2brg);
#endif

    return 1 ;
}

int i2c_setspeed (int speed)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    // Set the I2C BRG Clock division factor from desired i2c rate
    // and current CPU rate (we assume sccr dfbgr field is 0; 
    // divide BRGCLK by 1)

    debug("\n[I2C  ] Setting rate...");

    i2c_setrate (idata->cpu_clk , speed) ;
    
    return ERROR_I2C_NONE ;
}

int i2c_init(void)
{
    immap_t	*immap 	= (immap_t *)CFG_IMMR ;
    volatile cpm8xx_t	*cp;

    /* Get pointer to Communication Processor
     * and to internal registers
     */
    cp = (cpm8xx_t *)&immap->im_cpm ;
    iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];
    i2c = (i2c8xx_t *)&(immap->im_i2c);

    // Disable relocation
    iip->iic_rpbase = 0 ;	

    /* Initialize Port B I2C pins. */
    cp->cp_pbpar |= 0x00000030;
    cp->cp_pbdir |= 0x00000030;
    cp->cp_pbodr |= 0x00000030;
	
    /* Disable interrupts. */
    i2c->i2c_i2mod = 0;
    i2c->i2c_i2cmr = 0;
    i2c->i2c_i2cer = 0xff;

    /* Set I2C controller in master mode */
    i2c->i2c_i2com = 0x01;

    // Set SDMA bus arbitration level to 5 (SDCR)
    immap->im_siu_conf.sc_sdcr = 0x0001 ;

#ifdef CFG_ALLOC_DPRAM
    /* Initialize Tx/Rx parameters.*/
    iip->iic_rbptr = iip->iic_rbase = dpram_alloc_align(sizeof(cbd_t),8) ;
    iip->iic_tbptr = iip->iic_tbase = dpram_alloc_align(sizeof(cbd_t),8) ;

    rxbd = (cbd_t *)((unsigned char *)&cp->cp_dpmem[iip->iic_rbase]);
    txbd = (cbd_t *)((unsigned char *)&cp->cp_dpmem[iip->iic_tbase]);

    // Alloc rx and tx buffers into DPRAM
    rxbuf = (unsigned char *)&cp->cp_dpmem[dpram_alloc(I2C_RX_LEN)] ;
    txbuf = (unsigned char *)&cp->cp_dpmem[dpram_alloc(I2C_TX_LEN)] ;
#else
    {
	ulong base = CPM_I2C_BASE ;
	
	/* Initialize Tx/Rx parameters.*/
	iip->iic_rbptr = iip->iic_rbase = base ; base += sizeof(cbd_t) ;
	iip->iic_tbptr = iip->iic_tbase = base ; base += sizeof(cbd_t) ;

	rxbd = (cbd_t *)((unsigned char *)&cp->cp_dpmem[iip->iic_rbase]);
	txbd = (cbd_t *)((unsigned char *)&cp->cp_dpmem[iip->iic_tbase]);
    }
#endif

#ifdef CONFIG_I2C_DEBUG
    printf("Rxbd  = %08x\n", (int)rxbd);
    printf("Txbd  = %08x\n", (int)txbd);
    printf("Rxbuf = %08x (%d)\n", (int)rxbuf, I2C_RX_LEN);
    printf("Txbuf = %08x (%d)\n", (int)txbuf, I2C_TX_LEN);
#endif

    cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_I2C, CPM_CR_INIT_TRX) | CPM_CR_FLG;
    while (cp->cp_cpcr & CPM_CR_FLG);

    /* Set big endian byte order */
    iip->iic_tfcr = 0x15;
    iip->iic_rfcr = 0x15;

    /* Set maximum receive size. */
    iip->iic_mrblr = I2C_RX_LEN;

    debug("\n[I2C  ] Clearing the buffer memory...");

    // Clear the buffer memory 
    memset ((char *)rxbuf, I2C_RX_LEN, 0);
    memset ((char *)txbuf, I2C_TX_LEN, 0);
    
    debug("\n[I2C  ] Initializing BD's...");

    // Initialize the BD's 
  
    // Rx: Wrap, no interrupt, empty 
    rxbd->cbd_bufaddr = (ulong)rxbuf;
    rxbd->cbd_sc = 0xa800;

    // Tx: Wrap, no interrupt, not ready to send, last 
    txbd->cbd_bufaddr = (ulong)txbuf;
    txbd->cbd_sc = 0x2800;
	
    // Clear events and interrupts
    i2c->i2c_i2cer = 0xff ;
    i2c->i2c_i2cmr = 0 ;

    return ERROR_I2C_NONE ;
}

int i2c_send( unsigned char address,
              unsigned char secondary_address,
              int enable_secondary,
              unsigned short size, unsigned char dataout[] )
{
    unsigned char *buffer = (unsigned char *)txbd->cbd_bufaddr ;
    int i,j;

    if( size > I2C_TX_LEN )  /* Trying to send message larger than BD */
	return ERROR_I2C_LENGTH; 

    debug("\n[I2C  ] Waiting for transmit buffer empty...");
    while( txbd->cbd_sc & TXBD_R ) ; // Loop until previous data sent 
  
    debug("\n[I2C  ] Formatting addresses...");  
    if( enable_secondary ) /* Device has an internal address */
    {
	txbd->cbd_datlen = size + 2;  /* Length of message plus dest addresses */
	buffer[0] = address;
	buffer[0] &= ~(0x01);
	buffer[1] = secondary_address;
	i = 2;
    }
	else
    {
	txbd->cbd_datlen = size + 1;  /* Length of message plus dest address */
	buffer[0] = address;  /* Write destination address to BD */
	buffer[0] &= ~(0x01);  /* Set address to write */
	i = 1;
    }
  
#ifdef CONFIG_I2C_DEBUG
    printf("Length = %d addr[0] = %08x addr[1] = %08x\n", 
	txbd->cbd_datlen, 
	buffer[0],
	buffer[1]);
#endif
  
    /* Copy data to send into buffer */
    debug("\n[I2C  ] Copying data into buffer...");  
    for( j = 0; j < size; i++, j++ )
	buffer[ i ] = dataout[j];
    
    /* Ready to Transmit, wrap, last */    
    debug("\n[I2C  ] Waiting to transmit...");  
    txbd->cbd_sc = txbd->cbd_sc | TXBD_R | TXBD_W | TXBD_L | TXBD_S ;
    
    /* Enable I2C */
    debug("\n[I2C  ] Enabling I2C...");
    i2c->i2c_i2mod |= 1;  

    /* Transmit */
    debug("\n[I2C  ] Transmitting...");  
    i2c->i2c_i2com |= 0x80;

    debug("\n[I2C  ] Waiting for transmit buffer empty...");
    udelay (DELAY_US) ;	// Why without this it doesnt work?
    while( txbd->cbd_sc & TXBD_R );
  
    /* Turn off I2C */
    debug("\n[I2C  ] Turning off I2C...");
    i2c->i2c_i2mod &= (~1);

    return ERROR_I2C_NONE ;
}

int i2c_receive(unsigned char address,
		unsigned char secondary_address,
		int enable_secondary,				
                unsigned short size_to_expect, unsigned char datain[] )
{
    unsigned char *buffer = (unsigned char *)txbd->cbd_bufaddr ;
    int i, j;
  
    if( size_to_expect > I2C_RX_LEN )
	return ERROR_I2C_LENGTH;  /* Expected to receive too much */
  
    /* Turn on I2C */
    i2c->i2c_i2mod |= 0x01;
    
    /* Setup TXBD for destination address */   
    if( enable_secondary )
    {
	txbd->cbd_datlen = 2; 
	buffer[0] = address | 0x00;   /* Write data */
	buffer[1] = secondary_address;  /* Internal address */
	txbd->cbd_sc = TXBD_R;
      
	/* Reset the rxbd */
	rxbd->cbd_sc = RXBD_E | RXBD_W;
  
	/* Begin transmission */
	i2c->i2c_i2com |= 0x80;
    } 
	else
    {
	txbd->cbd_datlen = 1 + size_to_expect;
	buffer[0] = address | 0x01;
    
	/* Buffer ready to transmit, wrap, loop */
	txbd->cbd_sc |= TXBD_R | TXBD_W | TXBD_L;

	/* Reset the rxbd */
	rxbd->cbd_sc = RXBD_E | RXBD_W;
  
	/* Begin transmission */
	i2c->i2c_i2com |= 0x80;
  
	while( txbd->cbd_sc & TXBD_R);  /* Loop until transmit completed */
    }
  
    while( rxbd->cbd_sc & RXBD_E);  /* Wait until receive is finished */
  
    for( i= 0, j = 0; j < size_to_expect; j++, i++ )  /* Copy data to datain[] */
	datain[j] = buffer[i];
  
    /* Turn off I2C */
    i2c->i2c_i2mod &= (~1);

    return ERROR_I2C_NONE ;
}

#endif
