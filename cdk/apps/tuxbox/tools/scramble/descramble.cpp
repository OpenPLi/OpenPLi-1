// descramble.cpp
//
// Converts scrambled text to plain text for PPanel

#include "scramble_lib.h"

int main(int argc, char *argv[])
{
   string url, scrambled, descrambled;
   
   if(argc != 2)
   {
      printf("Usage: %s \"scrambled_text\"\n", argv[0]);
      return -1;
   }
   else
   {
      url = argv[1];
   }
   
   descrambled = descramble(url);
   printf("Descrambled: %s\n", descrambled.c_str());
   
   scrambled = scramble(descrambled);
   printf("Scrambled: *%s\n", scrambled.c_str());
   
   return 0;
}
