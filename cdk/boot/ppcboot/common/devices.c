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
#include <stdarg.h>
#include <malloc.h>
#include <devices.h>

list_t		devlist = 0 ;
device_t	*stdio_devices[] = {NULL,NULL,NULL} ;
char		*stdio_names[MAX_FILES]	= {"stdin", "stdout", "stderr"} ;

#ifdef CONFIG_VIDEO
extern int drv_video_init(void);
#endif
#ifdef CONFIG_WL_4PPM_KEYBOARD
extern int drv_wlkbd_init(void);
#endif

// **************************************************************************
// * SYSTEM DRIVERS
// **************************************************************************

static int drv_system_init (void)
{
    int error, devices = 1 ;
    device_t serdev ;
    
    memset (&serdev, 0, sizeof(serdev));
    
    strcpy(serdev.name, "serial");
    serdev.flags = DEV_FLAGS_OUTPUT | DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
    serdev.putc = serial_putc ;
    serdev.puts = serial_puts ;
    serdev.getc = serial_getc ;
    serdev.tstc = serial_tstc ;

    error = device_register (&serdev);
    
    return (error == 0) ? devices : error ; 
}

// **************************************************************************
// * DEVICES
// **************************************************************************

int device_register (device_t *dev)
{
    ListInsertItem (devlist, dev, LIST_END);    
    return 0 ;
}

int devices_init (void)
{
    // Initialize the list    
    devlist = ListCreate(sizeof(device_t)) ;

    if (devlist == NULL)
    {
	eputs("Cannot initialize the list of devices!\n");
	return -1 ;
    }
    
#ifdef CONFIG_VIDEO
    drv_video_init();
#endif
#ifdef CONFIG_WL_4PPM_KEYBOARD
    drv_wlkbd_init();
#endif
    drv_system_init();

    return ListNumItems(devlist) ;
}

int devices_done (void)
{
    ListDispose(devlist);
    
    return 0 ;
}

