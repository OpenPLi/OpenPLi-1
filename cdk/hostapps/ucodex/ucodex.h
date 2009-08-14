/*
 * $Id: ucodex.h,v 1.1 2002/09/18 07:34:54 obi Exp $
 *
 * extract avia firmware from srec and binary files
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <openssl/md5.h>

struct ucode_s {
	unsigned char name[21];
	size_t size;
	unsigned char md5sum[MD5_DIGEST_LENGTH];
};

struct ucode_type_s {
	unsigned char name[10];
	unsigned char magic[6];
	struct ucode_s ucodes[20];
};

