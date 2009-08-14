/*
 * tuxbox/kernel.h - TuxBox hardware info - kernel
 *
 * Copyright (C) 2003 Bastian Blank <waldi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: kernel.h,v 1.1.2.1 2003/03/09 16:35:45 waldi Exp $
 */

#ifndef TUXBOX_KERNEL_H
#define TUXBOX_KERNEL_H

#ifdef __KERNEL__
#if defined(MODULE) && defined (TUXBOX_MODULE_PARAMETER)
#define TUXBOX_INFO(arg) static tuxbox_##arg##_t tuxbox_##arg; MODULE_PARM(tuxbox_##arg,'i');
#else
#define TUXBOX_INFO(arg) extern tuxbox_##arg##_t tuxbox_##arg;
#endif
#endif /* __KERNEL__ */

#endif /* TUXBOX_KERNEL_H */
