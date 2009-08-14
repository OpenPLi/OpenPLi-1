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

#ifndef IDXFSTOOLS
#include <ppcboot.h>
#include "mpc8xx.h"
#include "idxfs.h"
#endif

void idxfs_dump_info(unsigned char *mem, unsigned int mem_size)
{

  sIdxFsHdr *hdr;
  sIdxFsFatEntry *fat;
  unsigned int offs;
  
  // Disable sanity checks if = 0
  if (mem_size == 0)
    mem_size = 0xFFFFFFFF;

  if (sizeof(sIdxFsHdr) > mem_size) {
  
    printf("Buffer overflow\n");
  
    return;    
    
  }    

  hdr = (sIdxFsHdr *)mem;

  if (hdr->Magic != IDXFS_MAGIC) {
  
    printf("Bad magic\n");
  
    return;
    
  }

  if (hdr->Version != IDXFS_VERSION) {

    printf("Bad version\n");

    return;
    
  }

  if ((!hdr->FatOffsFirst) || (hdr->FatOffsFirst + sizeof(sIdxFsFatEntry) > mem_size))
    return;
    
  offs = hdr->FatOffsFirst;
  fat = (sIdxFsFatEntry *)&mem[offs];

  while (1) {
  
    printf("Offs->0x%08X Size->0x%08X Next->0x%08X Name->%s\n", offs + sizeof(sIdxFsFatEntry), fat->Size, fat->OffsNext, fat->Name);
    
    if (!fat->OffsNext)
      return;
    
    if (fat->OffsNext + sizeof(sIdxFsFatEntry) > mem_size) {
    
      printf("Buffer overflow\n");
    
      return;
      
    }  
      
    offs = fat->OffsNext;
    fat = (sIdxFsFatEntry *)&mem[offs];  
  
  }
  
}

unsigned int idxfs_file_info(unsigned char *mem, unsigned int mem_size, unsigned char *file_name, unsigned int *file_offset, unsigned int *file_size) 
{

  sIdxFsHdr *hdr = (sIdxFsHdr *)mem;
  sIdxFsFatEntry *fat;
  unsigned int offs;

  // Disable sanity checks if = 0
  if (mem_size == 0)
    mem_size = 0xFFFFFFFF;
  
  if ((!hdr) || (!file_name))
    return 0;
    
  if (sizeof(sIdxFsHdr) > mem_size)
    return 0;    

  if (hdr->Magic != IDXFS_MAGIC)
    return 0;

  if (hdr->Version != IDXFS_VERSION)
    return 0;

  if ((!hdr->FatOffsFirst) || (hdr->FatOffsFirst + sizeof(sIdxFsFatEntry) > mem_size))
    return 0;
    
  offs = hdr->FatOffsFirst;
  fat = (sIdxFsFatEntry *)&mem[offs];
    
  while (1) {
  
    if (strncmp(fat->Name, file_name, IDXFS_MAX_NAME_LEN) == 0) {
    
      if (file_offset)
        *file_offset = offs + sizeof(sIdxFsFatEntry);

      if (file_size)
        *file_size = fat->Size;
	
      return 1;	

    }  
    
    if ((!fat->OffsNext) || (fat->OffsNext + sizeof(sIdxFsFatEntry) > mem_size))
      return 0;
      
    offs = fat->OffsNext;
    fat = (sIdxFsFatEntry *)&mem[offs];  
  
  }
  
  return 0;
  
}

