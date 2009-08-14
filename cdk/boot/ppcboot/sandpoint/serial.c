/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
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

#include "ns16550.h"

void initialise_ns87308(void);

/*
 * Minimal serial functions needed to use one87308's UARTs
 * as serial console interface.
 */

volatile struct NS16550 *console;

void
serial_init (unsigned long dummy, int baudrate)
{
    int clock_divisor = 115200/baudrate;
    initialise_ns87308();
    console = NS16550_init(0, clock_divisor);
}

void
serial_putc(const char c)
{
    NS16550_putc(console, c);
}

void
serial_puts (const char *s)
{
    while (*s) {
        serial_putc (*s++);
    }
}


int
serial_getc(void)
{
    return NS16550_getc(console);
}

int
serial_tstc(void)
{
    return NS16550_tstc(console);
}

void
serial_setbrg (unsigned long dummy, int baudrate)
{
   int clock_divisor = 115200/baudrate;
   NS16550_reinit(console, clock_divisor);
}
