/*
 * tuxbox_core.c - TuxBox hardware info
 *
 * Copyright (C) 2003 Florian Schirmer <jolt@tuxbox.org>
 *                    Bastian Blank <waldi@tuxbox.org>
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
 * $Id: tuxbox_core.c,v 1.2.2.2 2003/03/09 16:34:30 waldi Exp $
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/unistd.h>
#include <linux/init.h>

#include "tuxbox_internal.h"
#include <tuxbox/info.h>

#ifndef CONFIG_PROC_FS
#error Please enable procfs support
#endif

struct proc_dir_entry *proc_bus_tuxbox = NULL;

tuxbox_capabilities_t tuxbox_capabilities;
tuxbox_model_t tuxbox_model;
tuxbox_submodel_t tuxbox_submodel;
tuxbox_vendor_t tuxbox_vendor;

static int tuxbox_proc_create (void);
static void tuxbox_proc_destroy (void);

int tuxbox_proc_read (char *buf, char **start, off_t offset, int len, int *eof, void *data)
{
	int *_data = data;
	return snprintf(buf, len, "%d\n", *_data);
}

int tuxbox_proc_create_entry (const char *name, mode_t mode, struct proc_dir_entry *parent, void *data, read_proc_t *read_proc, write_proc_t *write_proc)
{
	struct proc_dir_entry *entry;

	entry = create_proc_entry (name, mode, parent);

	if (!entry) 
		return -1;

	entry->data = data;
	entry->read_proc = read_proc;
	entry->write_proc = write_proc;
	entry->owner = THIS_MODULE;

	return 0;
}

static int tuxbox_proc_create (void)
{
	if (!proc_bus) {
		printk("tuxbox: /proc/bus does not exist\n");
		return -ENOENT;
	}

	if (!(proc_bus_tuxbox = proc_mkdir ("tuxbox", proc_bus)))
		goto error;

	if (tuxbox_proc_create_entry ("capabilities", 0444, proc_bus_tuxbox, &tuxbox_capabilities, &tuxbox_proc_read, NULL))
		goto error;

	if (tuxbox_proc_create_entry ("model", 0444, proc_bus_tuxbox, &tuxbox_model, &tuxbox_proc_read, NULL))
		goto error;

	if (tuxbox_proc_create_entry ("submodel", 0444, proc_bus_tuxbox, &tuxbox_submodel, &tuxbox_proc_read, NULL))
		goto error;

	if (tuxbox_proc_create_entry ("vendor", 0444, proc_bus_tuxbox, &tuxbox_vendor, &tuxbox_proc_read, NULL))
		goto error;

	return 0;

error:
	printk("tuxbox: Could not create /proc/bus/tuxbox\n");
	return -ENOENT;
}

static void tuxbox_proc_destroy (void)
{
	remove_proc_entry ("capabilities", proc_bus_tuxbox);
	remove_proc_entry ("model", proc_bus_tuxbox);
	remove_proc_entry ("submodel", proc_bus_tuxbox);
	remove_proc_entry ("vendor", proc_bus_tuxbox);

	remove_proc_entry ("tuxbox", proc_bus);
}

int __init tuxbox_init (void)
{
	int ret;

	if (tuxbox_hardware_read ()) {
		printk("tuxbox: Could not read hardware info\n");
		return -ENODEV;
	}

	if ((ret = tuxbox_proc_create ()))
		goto error;
	if ((ret = tuxbox_hardware_proc_create ()))
		goto error;

	return 0;

error:
	tuxbox_hardware_proc_destroy ();
	tuxbox_proc_destroy ();

	return ret;
}

void __exit tuxbox_exit (void)
{
	tuxbox_hardware_proc_destroy ();
	tuxbox_proc_destroy ();
}

module_init(tuxbox_init);
module_exit(tuxbox_exit);

MODULE_AUTHOR("Florian Schirmer <jolt@tuxbox.org>, Bastian Blank <waldi@tuxbox.org>");
MODULE_DESCRIPTION("TuxBox hardware info");
MODULE_LICENSE("GPL");

