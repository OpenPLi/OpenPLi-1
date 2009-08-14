#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libmd5sum/libmd5sum.h"

unsigned char md5buffer[16];


int main(int argc, char **argv)
{
        int count;

        if(argc!=2)
        {
                printf("usage:  md5sum <filename>\n\n");
                exit(1);
        }


        if( md5_file(argv[1], 1, (unsigned char*) &md5buffer))
        {
 		exit(-1);
      	}


        for(count=0;count<16;count++)
        {
                printf("%02x", md5buffer[count] );
        }
        printf("\n\n");


        return 1;
}
