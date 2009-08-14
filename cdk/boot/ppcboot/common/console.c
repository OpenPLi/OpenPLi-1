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

static int console_setfile(int file, device_t  *dev)
{	
    int error = 0;
    
    if (dev == NULL)
	return -1 ;

    switch (file)
    {
    case stdin:
    case stdout:
    case stderr:
	// Start new device 
	if (dev->start)
	{
	    error = dev->start() ;
	    // If it's not started dont use it
	    if (error < 0)
		break;
	}
	
	// Assign the new device (leaving the existing one started)
	stdio_devices[file] = dev ;
	
	// Update monitor functions (to use the console stuff by other applications)
	switch (file){
	    case stdin:
		bd_ptr->bi_mon_fnc->getc = dev->getc ;
		bd_ptr->bi_mon_fnc->tstc = dev->tstc ;
		break;		
	    case stdout:
		bd_ptr->bi_mon_fnc->putc = dev->putc ;
		bd_ptr->bi_mon_fnc->puts = dev->puts ;
		bd_ptr->bi_mon_fnc->printf = printf ;
		break;		
	}
	break;
    	
    default:	// Invalid file ID
	error = -1 ;
    }
    return error ;
}

//** PPCBOOT INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************

void serial_printf(const char *fmt, ...)
{
	va_list	args;
	uint	i;
	char	printbuffer[CFG_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf(printbuffer, fmt, args);
	va_end(args);

	serial_puts(printbuffer);
}

int	fgetc(int file)
{
    if (file < MAX_FILES)
	return stdio_devices[file]->getc();
    
    return -1 ;
}

int	ftstc(int file)
{
    if (file < MAX_FILES)
	return stdio_devices[file]->tstc();
    
    return -1 ;
}

void	fputc(int file, const char c)
{
    if (file < MAX_FILES)
	stdio_devices[file]->putc(c);
}

void	fputs(int file, const char *s)
{
    if (file < MAX_FILES)
	stdio_devices[file]->puts(s);
}

void fprintf(int file, const char *fmt, ...)
{
	va_list	args;
	uint	i;
	char	printbuffer[CFG_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf(printbuffer, fmt, args);
	va_end(args);

	// Send to desired file
        fputs(file, printbuffer);
}

//** PPCBOOT INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************

int	getc(void)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    if (idata->relocated)
	    // Get from the standard input
	    return fgetc (stdin);
    
    // Send directly to the handler
    return serial_getc();
}

int	tstc(void)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    if (idata->relocated)
	    // Test the standard input
	    return ftstc (stdin);
    
    // Send directly to the handler
    return serial_tstc();
}

void	putc(const char c)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    if (idata->relocated)
	// Send to the standard output
	fputc (stdout, c);
    else    
	// Send directly to the handler
	serial_putc(c);
}

void	puts(const char *s)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    if (idata->relocated)
	// Send to the standard output
	fputs (stdout, s);
    else    
	// Send directly to the handler
	serial_puts(s);
}

void printf(const char *fmt, ...)
{
	va_list	args;
	uint	i;
	char	printbuffer[CFG_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsprintf(printbuffer, fmt, args);
	va_end(args);

	// Print the string
	puts (printbuffer);
}

//** PPCBOOT INIT FUNCTIONS *************************************************

int console_assign (int file, char *devname)
{
    int			flag , i;

    // Check for valid file
    switch(file){
    case stdin:
	flag = DEV_FLAGS_INPUT ;
	break;
    case stdout:
    case stderr:
	flag = DEV_FLAGS_OUTPUT ;
	break;
    default:
	return -1 ;
    }
	
    // Check for valid device name
    
    for(i=1; i<=ListNumItems(devlist); i++)
    {
	device_t	*dev = ListGetPtrToItem (devlist,i) ;
	
	if (strcmp (devname, dev->name) == 0)
	{
	    if (dev->flags & flag)	 
		return console_setfile(file, dev) ;
		
	    return -1 ;
	}
    }
    
    return -1 ;
}

// Called before relocation - use serial functions
void	console_init_f (void)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);

    idata->relocated 		= 0 ;		// Use these pointers before relocation
    idata->bi_mon_fnc.getc   	= serial_getc;	
    idata->bi_mon_fnc.tstc   	= serial_tstc;	
    idata->bi_mon_fnc.putc   	= serial_putc;	
    idata->bi_mon_fnc.puts 	= serial_puts;	
    idata->bi_mon_fnc.printf 	= serial_printf;
}

// Called after the relocation - use desierd console functions
void	console_init_r (void)
{
    init_data_t *idata = (init_data_t *)(CFG_INIT_RAM_ADDR + CFG_INIT_DATA_OFFSET);
    device_t	*inputdev = NULL, *outputdev = NULL ;
    int i, items = ListNumItems(devlist) ;

    // Global pointer to monitor structure (updated by the console stuff)
    bd_ptr->bi_mon_fnc = &idata->bi_mon_fnc ;

    // Scan devices looking for input and output devices
    for (i=1; (i<=items) && ((inputdev == NULL) || (outputdev == NULL)); i++)
    {	
	device_t	*dev = ListGetPtrToItem(devlist, i) ;
	
	if ((dev->flags & DEV_FLAGS_INPUT) && (inputdev==NULL))
	    inputdev = dev ;

	if ((dev->flags & DEV_FLAGS_OUTPUT) && (outputdev==NULL))
	    outputdev = dev ;
    }
	    
    // Initializes output console first
    if (outputdev != NULL)
    {
	console_setfile(stdout, outputdev);
	console_setfile(stderr, outputdev);
    }
	
    // Initializes input console
    if (inputdev != NULL)
	console_setfile(stdin, inputdev);

    // Print informations
    printf("  Input:  ");
    if (stdio_devices[stdin] == NULL)
	printf("No input devices available!\n");
    else
    {
	printf("%s\n", stdio_devices[stdin]->name);
    }
    
    printf("  Output: ");
    if (stdio_devices[stdout] == NULL)
	printf("No output devices available!\n");
    else
	printf("%s\n", stdio_devices[stdout]->name);	
    
    // Setting environment variables
    for (i=0; i<3; i++)
	setenv(stdio_names[i], stdio_devices[i]->name);

    // If nothing usable installed, use only the initial console
    if ((stdio_devices[stdin] == NULL) && (stdio_devices[stdout] == NULL))
    	return ;

    // Set the relocation flag    
    idata->relocated = 1 ;
}
