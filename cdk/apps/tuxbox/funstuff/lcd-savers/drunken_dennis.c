
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

/*
  012
  3D4 (D: Dennis)
  567
*/

struct {
  unsigned int   width;
  unsigned int   height;
  unsigned char  pixel_data[32*23];
} bed_buf = {
  31, 22,
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,
  1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,
  1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,1,1,
  1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,1,
  1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
  1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
  1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1}};

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

void draw_screen(int lcd_fd, screen_t s) {
  write(lcd_fd, s, LCD_BUFFER_SIZE);
}

void putpixel(int x, int y, char col, screen_t s) {
  int ofs = (y>>3)*LCD_COLS+x,
    bit = y & 7;
  
  switch (col) {
  case LCD_PIXEL_ON:
    s[ofs] |= 1<<bit;
    break;
  case LCD_PIXEL_OFF:
    s[ofs] &= ~(1<<bit);
    break;
  case LCD_PIXEL_INV:
    s[ofs] ^= 1<<bit;
    break;
  }
}

int main(int argc, char **argv) {
  unsigned char buf;
  int lcdfd,rndfd,x,y;
  int i,delay=1000;
  screen_t screen;
  lcd_pixel pixl;

  memset(screen,0,sizeof(screen));
  
  for(y=0;y<bed_buf.height;y++) {
    for(x=0;x<bed_buf.width;x++) {
      if(bed_buf.pixel_data[(bed_buf.width*y)+x]) putpixel(119-bed_buf.width+x,y,LCD_PIXEL_ON,screen);
    }
  }

  if(argc>1)
    delay=atoi(argv[1]);

  lcdfd=open("/dev/dbox/lcd0",O_RDWR);
  rndfd=open("/dev/urandom",O_RDONLY);
  
  i=LCD_MODE_ASC;
  ioctl(lcdfd,LCD_IOCTL_ASC_MODE,&i);
  ioctl(lcdfd,LCD_IOCTL_CLEAR);
  write(lcdfd,"\nLet's see if\ndrunken Dennis\ncan find his\nway home...\n\n\n",strlen("\nLet's see if\ndrunken Dennis\ncan find his\nway home...\n\n\n"));
  sleep(5);
  i=LCD_MODE_BIN;
  ioctl(lcdfd,LCD_IOCTL_ASC_MODE,&i);
  ioctl(lcdfd,LCD_IOCTL_CLEAR);
  draw_screen(lcdfd,screen);
  read(rndfd,&buf,1);
  pixl.x=buf%119;
  read(rndfd,&buf,1);
  pixl.y=buf%63;
  pixl.v=2;
  while((read(rndfd,&buf,1)) && ((pixl.x<119-bed_buf.width)||(pixl.y>bed_buf.height))){
    buf%=8;
    switch(buf) {
    case 0:
      if ((pixl.x==0) || (pixl.y==0)) break;
      pixl.x--;
      pixl.y--;
      break;
    case 1:
      if(pixl.y==0) break;
      pixl.y--;
      break;
    case 2:
      if((pixl.x==119) || (pixl.y==0)) break;
      pixl.x++;
      pixl.y--;
      break;
    case 3:
      if(pixl.x==0) break;
      pixl.x--;
      break;
    case 4:
      if(pixl.x==119) break;
      pixl.x++;
      break;
    case 5:
      if((pixl.x==0) || (pixl.y==63)) break;
      pixl.x--;
      pixl.y++;
      break;
    case 6:
      if(pixl.y==63) break;
      pixl.y++;
      break;
    case 7:
      if((pixl.x==119) || (pixl.y==63)) break;
      pixl.x++;
      pixl.y++;
    }
    ioctl(lcdfd,LCD_IOCTL_SET_PIXEL,&pixl);
    usleep(delay);
  }
  sleep(3);
  i=LCD_MODE_ASC;
  ioctl(lcdfd,LCD_IOCTL_ASC_MODE,&i);
  ioctl(lcdfd,LCD_IOCTL_CLEAR);
  close(rndfd);
  close(lcdfd);
  return 0;
}
