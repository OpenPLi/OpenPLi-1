/*
 * (C) Copyright 2001
 * derget & Jolt
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

#define IDXFSTOOLS

#include <stdio.h>
#include "../ppcboot/include/idxfs.h"
#include "../ppcboot/common/idxfs.c"

int main(void)
{

  FILE *idxfs_img;
  unsigned char *mem;
  unsigned int filesize;

  idxfs_img = fopen("image.idx", "rb");
  fseek(idxfs_img, 0, 2);
  filesize = ftell(idxfs_img);
  fseek(idxfs_img, 0, 0);
  
  mem = (unsigned char *)malloc(filesize);

  if (!mem) {
  
    printf("Not enough memory!\n");

    fclose(idxfs_img);

    return 0;
    
  }
  
  fread(mem, filesize, 1, idxfs_img);
  fclose(idxfs_img);
  
  idxfs_dump_info(mem, filesize);

  free(mem);
  
  return 0;

}

