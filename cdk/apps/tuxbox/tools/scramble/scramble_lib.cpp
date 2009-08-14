// scramble.cpp
//
// Converts any text to a scambled text for PPanel

#include <stdio.h>
#include <string>

using namespace std;

unsigned int mask[16]=
{ 89, 52, 178, 9, 56, 86, 114, 5, 29, 98, 156, 209, 243, 73, 198, 101 };

string descramble(string text)
{
   string descrambled;
   char kar;
   int masknr = 0;
   
   for(unsigned int i=0; i<text.length(); i+=2)
   {
      kar = (text[i] - 65) * 16 + (text[i+1] -65);
      
      kar ^= mask[masknr++];
      masknr &= 15;
      
      kar = 128 * (unsigned int)(kar&64)/64+
            64  * (unsigned int)(kar&2)/2+
            32  * (unsigned int)(kar&1)+
            16  * (unsigned int)(kar&4)/4+
            8   * (unsigned int)(kar&32)/32+
            4   * (unsigned int)(kar&128)/128+
            2   * (unsigned int)(kar&8)/8+
                  (unsigned int)(kar&16)/16;
                  
      descrambled += kar;
   }
   
   return descrambled;
}

string scramble(string url)
{
   string scrambled;
   int masknr = 0;
   
   for(unsigned int i=0; i<url.length(); ++i)
   {
      // Bit shuffle
      url[i] = 
         128 * (unsigned int)(url[i]&4)/4+
         64  * (unsigned int)(url[i]&128)/128+
         32  * (unsigned int)(url[i]&8)/8+
         16  * (unsigned int)(url[i]&1)+
         8   * (unsigned int)(url[i]&2)/2+
         4   * (unsigned int)(url[i]&16)/16+
         2   * (unsigned int)(url[i]&64)/64+
               (unsigned int)(url[i]&32)/32;

      // Xor mask
      url[i] ^= mask[masknr++];
      masknr &= 15;

      // Convert to ASCII
      scrambled += (unsigned int)(url[i]&0xf0)/16+65;
      scrambled += (unsigned int)(url[i]&0x0f)+65;
   }   
   
   return scrambled;
}
