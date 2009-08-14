#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  unsigned char buffer[4];
  unsigned int i, p, t, os, c, r, pe, div;
  unsigned long b;
  
  if (argc!=5)
  {
    printf("usage: showfreq-psk xx xx xx xx\n");
    return 0;
  }
  
  for (i=0; i<4; i++)
    sscanf(argv[i+1], "%x", buffer+i);
/*
       26 25 24 |23 22 21 20 19 18 17 16 | |... 00
       p2 p1 p0  t0 os c0 r2 r1 r0 pe ... freq
       port control
                test mode
                   drive output disable
                      charge pump current
                         divider
                                  predivider
                                      freq
*/
  b=buffer[0]<<24;
  b|=buffer[1]<<16;
  b|=buffer[2]<<8;
  b|=buffer[3];

  p=(b>>24)&7;
  t=!! (b&(1<<23));
  os=!! (b&(1<<22));
  c=!! (b&(1<<21));
  r=(b>>18)&7;
  pe=!! (b&(1<<17));
  div=b&(65536*2-1);
  
  printf("div: %d\n", div);
  printf("p=%d t=%d os=%d c=%d r=%d pe=%d (rest: %x)\n", p, t, os, c, r, pe, b&~((1<<28)-1));
  div*=125000;          // tun->res
  div*=4;
  div-=479500000;        // tun->IFPCoff
  div/=1;               // tun->step
  
  printf("raw freq: %d Hz (%d.%03dMHz)\n", div, div/(1000*1000), (div/1000)%1000);

  printf("high freq: zu viel Hz (%d.%03dMHz)\n", div/(1000*1000)+10600, (div/1000)%1000);
  
  printf("low freq: inf Hz (%d.%03dMHz)\n", div/(1000*1000)+9750, (div/1000)%1000);
  

  return 0;
}