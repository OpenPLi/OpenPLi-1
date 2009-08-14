// scramble.cpp
//
// Converts any text to a scambled text for PPanel

#include "scramble_lib.h"

using namespace std;

int main(int argc, char *argv[])
{
   string url, scrambled, descrambled;
   
   if(argc != 2)
   {
      printf("Usage: %s \"text\"\n", argv[0]);
      return -1;
   }
   else
   {
      url = argv[1];
   }
   
   scrambled = scramble(url);
   printf("Scrambled: *%s\n", scrambled.c_str());
   
   descrambled = descramble(scrambled);
   printf("Descrambled: %s\n", descrambled.c_str());
   
   return 0;
}
