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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <ppcboot.h>
#include "mpc8xx.h"
#include <status_led.h>

/*
 * The purpose of this code is to signal the operational status of a
 * target which usually boots over the network; while running in
 * PCBoot, a status LED is blinking. As soon as a valid BOOTP reply
 * message has been received, the LED is turned off. The Linux
 * kernel, once it is running, will start blinking the LED again,
 * with another frequency.
 */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_STATUS_LED

static int status_led_state;
static int status_led_init_done = 0;

static void status_led_init (void)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;

    immr->STATUS_LED_PAR &= ~(STATUS_LED_BIT);
#ifdef STATUS_LED_ODR
    immr->STATUS_LED_ODR &= ~(STATUS_LED_BIT);
#endif
#if (STATUS_LED_ACTIVE == 0)			/* start with LED off */
    immr->STATUS_LED_DAT |=   STATUS_LED_BIT ;
#else
    immr->STATUS_LED_DAT &= ~(STATUS_LED_BIT);
#endif
    immr->STATUS_LED_DIR |=   STATUS_LED_BIT ;
    status_led_state	  = STATUS_LED_BLINKING;
    status_led_init_done  = 1;
}

void status_led_tick (ulong timestamp)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;

    if (status_led_state != STATUS_LED_BLINKING)
	return;

    if (!status_led_init_done)
	status_led_init();

    if ((timestamp % STATUS_LED_PERIOD) == 0) {
	immr->STATUS_LED_DAT ^= STATUS_LED_BIT;
    }
}

void status_led_set (int state)
{
    volatile immap_t *immr = (immap_t *)CFG_IMMR;

    if (!status_led_init_done)
	status_led_init();

    switch (state) {
    default:
	return;
    case STATUS_LED_BLINKING:
	break;
    case STATUS_LED_ON:
#if (STATUS_LED_ACTIVE == 0)
	immr->STATUS_LED_DAT &= ~(STATUS_LED_BIT);
#else
	immr->STATUS_LED_DAT |=   STATUS_LED_BIT ;
#endif
	break;
    case STATUS_LED_OFF:
#if (STATUS_LED_ACTIVE == 0)
	immr->STATUS_LED_DAT |=   STATUS_LED_BIT ;
#else
	immr->STATUS_LED_DAT &= ~(STATUS_LED_BIT);
#endif
	break;
    }
    status_led_state = state;
}

#endif	/* CONFIG_STATUS_LED */
