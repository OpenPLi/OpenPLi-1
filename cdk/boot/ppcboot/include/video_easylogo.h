/*
** video easylogo
** ==============
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
** This utility is still under construction!
*/

// Dont use print here 'cause video console is not initialized!

#ifndef _EASYLOGO_H_
#define _EASYLOGO_H_

//#define ENABLE_ASCII_BANNERS

typedef struct {
	unsigned char 	*data;
	int		width,
			height,
			bpp,
			pixel_size,
			size;
}	fastimage_t ;

#endif
