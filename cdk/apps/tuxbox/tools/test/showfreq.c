#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  unsigned char buffer[4];
  unsigned int i, div, r, pe, c, re, rts, p;
  
  if (argc!=5)
  {
    printf("usage: showfreq xx xx xx xx\n");
    return 0;
  }
  
  for (i=0; i<4; i++)
    sscanf(argv[i+1], "%x", buffer+i);

  div=buffer[1]|((buffer[0]&0x7F)<<8)|(((buffer[2]>>5)&3)<<15);
  r=buffer[2]&15;
  pe=(buffer[2]>>4)&1;
  c=buffer[3]>>6;
  re=!!(buffer[3]&32);
  rts=!!(buffer[3]&16);
  p=buffer[3]&15;
  
  printf("div: %d\n", div);
  printf("r=%d pe=%d c=%d re=%d rts=%d p=%d\n", r, pe, c, re, rts, p);
  div*=62500;           // tun->res
  div-=36125000;        // tun->IFPCoff
  div/=1;               // tun->step
  
  printf("freq: %d Hz (%d.%03dMHz)\n", div, div/(1000*1000), (div/1000)%1000);
  
  
  return 0;
}