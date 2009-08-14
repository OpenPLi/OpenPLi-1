/*
 * md5check.c - ucode + firmware checker (dbox-II-project)
 *  
 * Homepage: http://dbox2.elxsi.de
 *
 * (C) Copyright 2001
 * Steffen Hehn (McClean) - initial version by derget
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libucodes/libucodes.h>


int main(int argc, char **argv)
{
        char res[60];


        checkFile("/ucodes/avia500.ux", (char*) &res);
        printf("avia500: %s\n", res);

        checkFile("/ucodes/avia600.ux", (char*) &res);
        printf("avia600: %s\n", res);

        checkFile("/ucodes/ucode.bin", (char*) &res);
        printf("ucodes: %s\n", res);

        checkFile("/ucodes/cam-alpha.bin", (char*) &res);
        printf("cam-alpha: %s\n", res);

	return 0;
}
