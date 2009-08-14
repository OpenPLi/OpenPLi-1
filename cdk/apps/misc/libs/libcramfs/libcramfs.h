#ifndef __libcramfs__
#define __libcramfs__


#ifdef __cplusplus
extern "C" {
#endif


int cramfs_name(char *filename, char *opt_name);
int cramfs_crc(char *filename);


#ifdef __cplusplus
}
#endif


#endif
