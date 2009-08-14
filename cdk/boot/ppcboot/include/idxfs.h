#ifndef IDXFS_H
#define IDXFS_H

#define IDXFS_MAGIC 0x19780526
#define IDXFS_VERSION 0x01000100
#define IDXFS_MAX_NAME_LEN 32
#define IDXFS_OFFSET 0x10020000   //offset vom idxfs im Flash 

typedef struct {

  unsigned int Magic;
  unsigned int Version;
  unsigned int FatOffsFirst;

} sIdxFsHdr;

typedef struct {

  unsigned int Size;
  unsigned char Name[IDXFS_MAX_NAME_LEN];
  unsigned int OffsNext;

} sIdxFsFatEntry;

unsigned int idxfs_file_info(unsigned char *mem, unsigned int mem_size, unsigned char *file_name, unsigned int *file_offset, unsigned int *file_size);
void idxfs_dump_info(unsigned char *mem, unsigned int mem_size);

#endif

