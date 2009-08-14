/*
 * tuxbox_internal.h - TuxBox hardware info - internal definitions
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
 * $Id: tuxbox_internal.h,v 1.1.2.1 2003/03/09 16:34:30 waldi Exp $
 */

#ifndef TUXBOX_INTERNAL_H
#define TUXBOX_INTERNAL_H

#include <linux/proc_fs.h>

#include <tuxbox/info.h>

extern tuxbox_capabilities_t tuxbox_capabilities;
extern tuxbox_model_t tuxbox_model;
extern tuxbox_submodel_t tuxbox_submodel;
extern tuxbox_vendor_t tuxbox_vendor;

int tuxbox_proc_read (char *buf, char **start, off_t offset, int len, int *eof, void *data);
int tuxbox_proc_create_entry (const char *name, mode_t mode, struct proc_dir_entry *parent, void *data, read_proc_t *read_proc, write_proc_t *write_proc);

int tuxbox_hardware_read (void);

int tuxbox_hardware_proc_create (void);
void tuxbox_hardware_proc_destroy (void);

#endif /* TUXBOX_INTERNAL_H */
