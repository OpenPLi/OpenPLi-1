#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dbox/lcd-ks0713.h>

#define LCD "/dev/dbox/lcd0"

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int main(int argc, char **argv) {
  unsigned char matrix[15*64];
  int lcd_fd;
  screen_t screen;
  
  memset(matrix,0,sizeof(matrix));
  if((lcd_fd=open(LCD,O_RDONLY))<0) {
    perror("Can't open LCD");
    return -1;
  }
  
  if(read(lcd_fd,screen,LCD_BUFFER_SIZE)==LCD_BUFFER_SIZE) {
    int x,y;
    
    for(y=0;y<64;y++) {
      for(x=0;x<120;x++)
	if(argc>=2)
	  printf("%c",(screen[((y/8)*120)+x]>>((y%8)))&1?'#':' ');
	else
	  matrix[(y*15)+(x/8)]|=((screen[((y/8)*120)+x]>>((y%8)))&1)<<(x%8);
      if(argc>=2)
	printf("\n");
    }

    if(argc<2) {
      printf("#define lcd_dump_width 120\n#define lcd_dump_height 64\nstatic unsigned char lcd_dump_bits[]={\n");
      for(y=0;y<64;y++){
	for(x=0;x<15;x++)
	  printf("0x%02X,",matrix[(y*15)+x]);
	printf("\n");
      }
      printf("};");
    }
  }
  else {
    perror("LCD read error");
    return -1;
  }

  close(lcd_fd);

  return 0;
}
