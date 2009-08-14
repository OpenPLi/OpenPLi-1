/*
** I2C interface
** =============
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
*/

#ifndef _I2C_H_
#define _I2C_H_

#ifdef CONFIG_MPC8260

#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */

/* This structure keeps track of the bd and buffer space usage. */
typedef struct i2c_state {
	int rx_idx, tx_idx;		/* index to next free rx/tx bd */
	void *rxbd, *txbd;		/* pointer to next free rx/tx bd */
	int tx_space;			/* number of tx bytes left */
	unsigned char *tx_buf;	/* pointer to free tx area */
} i2c_state_t;

/* initialize i2c usage */
void i2c_init(int speed, int slaveaddr);

/* prepare a new io sequence */
void i2c_newio(i2c_state_t *state);

/* schedule a send operation (uses 1 tx bd) */
int i2c_send(i2c_state_t *state,
			  unsigned char address,
              unsigned char secondary_address,
              unsigned int flags,
              unsigned short size, 
			  unsigned char *dataout);

/* schedule a receive operation (uses 1 tx bd, 1 rx bd) */
int i2c_receive(i2c_state_t *state,
				 unsigned char address,
				 unsigned char secondary_address,
				 unsigned int flags,
				 unsigned short size_to_expect, 
				 unsigned char *datain);

/* execute all scheduled operations */
int i2c_doio(i2c_state_t *state);

/* flags for i2c_send() and i2c_receive() */
#define I2CF_ENABLE_SECONDARY	0x01	/* secondary_address is valid */
#define I2CF_START_COND		0x02	/* tx: generate start condition */
#define I2CF_STOP_COND		0x04	/* tx: generate stop condition */

/* return codes */
#define I2CERR_NO_BUFFERS	0x01	/* no more bds or buffer space */
#define I2CERR_MSG_TOO_LONG	0x02	/* tried to send/receive to much data */
#define I2CERR_TIMEOUT		0x03	/* timeout in i2c_doio() */
#define I2CERR_QUEUE_EMPTY	0x04	/* i2c_doio called without send/receive */

#elif defined(CONFIG_CPCI405) || defined(CONFIG_AR405) || defined (CONFIG_WALNUT405)

void i2c_receive(unsigned char address,
		 unsigned short size_to_expect, unsigned char datain[] );
void i2c_send(unsigned char address,
		 unsigned short size_to_send, unsigned char dataout[] );

#else /* !CONFIG_MPC8260 */


int i2c_init(void);
int i2c_send( unsigned char address,
              unsigned char secondary_address,
              int enable_secondary,
              unsigned short size, unsigned char dataout[] );
int i2c_receive(unsigned char address,
		unsigned char secondary_address,
		int enable_secondary,
                unsigned short size_to_expect, unsigned char datain[] );

#endif

#define ERROR_I2C_NONE		0
#define ERROR_I2C_LENGTH	1

#endif
