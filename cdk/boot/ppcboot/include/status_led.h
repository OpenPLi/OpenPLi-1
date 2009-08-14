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

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

#ifndef _STATUS_LED_H_
#define	_STATUS_LED_H_

#ifdef CONFIG_STATUS_LED

#if defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L) || \
    defined(CONFIG_TQM855L) || defined(CONFIG_TQM860L)
# ifndef CONFIG_TQM8xxL
#  define CONFIG_TQM8xxL
# endif
#endif

#if defined(CONFIG_TQM8xxL)
# define STATUS_LED_PAR		im_cpm.cp_pbpar
# define STATUS_LED_DIR		im_cpm.cp_pbdir
# define STATUS_LED_ODR		im_cpm.cp_pbodr
# define STATUS_LED_DAT		im_cpm.cp_pbdat

# define STATUS_LED_BIT		0x00000001
# define STATUS_LED_ACTIVE	1		/* LED on for bit == 1 */

# define STATUS_LED_PERIOD	500

#elif defined(CONFIG_ETX094)

# define STATUS_LED_PAR		im_ioport.iop_pdpar
# define STATUS_LED_DIR		im_ioport.iop_pddir
# undef  STATUS_LED_ODR
# define STATUS_LED_DAT		im_ioport.iop_pddat

# define STATUS_LED_BIT		0x00000001
# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0 */

# define STATUS_LED_PERIOD	500

#else
# error Status LED configuration missing
#endif

#define	STATUS_LED_OFF		0
#define STATUS_LED_BLINKING	1
#define STATUS_LED_ON		2

void status_led_tick (unsigned long timestamp);
void status_led_set  (int state);

#endif	/* CONFIG_STATUS_LED */

#endif	/* _STATUS_LED_H_ */
