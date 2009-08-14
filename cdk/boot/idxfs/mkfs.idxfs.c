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
 *
 *
 *
 *
 *	Revision   1.4  2001/06/05 12:30:00  derget 
 *	Some Fixxes
 *
 *	Revision   1.3  2001/04/25 19:41:29  Jolt
 *	Inital Release
 */
#include <stdio.h>
#include "../ppcboot/include/idxfs.h"


#define TARGET_IS_BE 

#define BUFFER_SIZE 1024

#ifdef TARGET_IS_BE  
unsigned short swab16(unsigned short w) { return ( (w>>8)|(w<<8) ); };
unsigned int swab32(unsigned int w) { return ( swab16(w>>16) | (swab16(w)<<16) ); };
#else
#define swab16(X) (X)
#define swab32(X) (X)
#endif


int write_file(FILE *image, char *filename, unsigned char last)
{

  unsigned char buffer[BUFFER_SIZE];
  sIdxFsFatEntry idxfs_fat;
  FILE *idxfs_file;
  unsigned int size;

  idxfs_file = fopen(filename, "rb");
	if (!idxfs_file)
	  {
	    perror(filename);
	    return 0;	
	  }


  fseek(idxfs_file, 0, 2);
  size = ftell(idxfs_file);
  fseek(idxfs_file, 0, 0);

  idxfs_fat.Size = swab32(size);

  strncpy(idxfs_fat.Name, filename, IDXFS_MAX_NAME_LEN);
  
  if (last)
    idxfs_fat.OffsNext = 0;
  else
    idxfs_fat.OffsNext = swab32(ftell(image) + sizeof(sIdxFsFatEntry) + size);
  
  fwrite(&idxfs_fat, sizeof(sIdxFsFatEntry), 1, image);
  
  while ((size = fread(buffer, 1, BUFFER_SIZE, idxfs_file)))
    fwrite(buffer, size, 1, image);
  
  fclose(idxfs_file);

}

int main(int argc, char **argv)
{

  FILE *idxfs_img;
  sIdxFsHdr idxfs_hdr;
  
  if (argc!=2)
  {
    printf("usage: mkfs.idxfs <image.idx>\n");
    printf("kernel , logo-lcd and logo-fb musst be in same dir\n");
    return 0;
  }

  
  idxfs_img = fopen(argv[1], "wb");
  	if(!idxfs_img)
  {
    perror(argv[1]);
    return 0;
  }
 
	
  idxfs_hdr.Magic = swab32(IDXFS_MAGIC);
  idxfs_hdr.Version = swab32(IDXFS_VERSION);
  idxfs_hdr.FatOffsFirst = swab32(sizeof(sIdxFsHdr));
  
  fwrite(&idxfs_hdr, sizeof(sIdxFsHdr), 1, idxfs_img);
  
	
  write_file(idxfs_img, "kernel", 0);
  write_file(idxfs_img, "logo-lcd", 0);
  write_file(idxfs_img, "logo-fb", 1);
  
  fclose(idxfs_img);
  
  return 0;

}

