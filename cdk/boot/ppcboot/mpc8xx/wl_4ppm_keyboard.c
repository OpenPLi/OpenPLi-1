/*
** Wireless 56Khz 4PPM keyboard interface on SMCx
** ==============================================
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
** Not currently supported. Still under construction.
*/
#include <ppcboot.h>
#include <config.h>
#include <wl_4ppm_keyboard.h>

int	wl_4ppm_keyboard_getc(void)
{
    return serial_getc();
}

int	wl_4ppm_keyboard_tstc(void)
{
    return serial_tstc();
}
