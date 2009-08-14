#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define KEY_0		0x5C00
#define KEY_1		0x5C01
#define KEY_2		0x5C02
#define KEY_3		0x5C03
#define KEY_4		0x5C04
#define KEY_5		0x5C05
#define KEY_6		0x5C06
#define KEY_7		0x5C07
#define KEY_8		0x5C08
#define KEY_9		0x5C09
#define KEY_POWER	0x5C0C
#define KEY_UP		0x5C0E
#define KEY_DOWN	0x5C0F
#define KEY_VOLUMEUP	0x5C16
#define KEY_VOLUMEDOWN	0x5C17
#define KEY_HOME	0x5C20
#define KEY_SETUP	0x5C27
#define KEY_MUTE	0x5C28
#define KEY_RED		0x5C2D
#define KEY_RIGHT	0x5C2E
#define KEY_LEFT	0x5C2F
#define KEY_OK		0x5C30
#define KEY_BLUE	0x5C3B
#define KEY_YELLOW	0x5C52
#define KEY_GREEN	0x5C55
#define KEY_HELP	0x5C82

short GetRCCode(int rc)
{
   if (rc == -1)
      return -1;
   short rccode = -1;
   int bytesread = read(rc, &rccode, 2);

   if (bytesread == 2)
   {
      if ((rccode & 0xFF00) == 0x5C00)
      {
         switch(rccode)
         {
            case KEY_UP: rccode = RC_UP; break;
            case KEY_DOWN: rccode = RC_DOWN; break;
            case KEY_LEFT: rccode = RC_LEFT; break;
            case KEY_RIGHT: rccode = RC_RIGHT; break;
            case KEY_OK: rccode = RC_OK; break;
            case KEY_0: rccode = RC_0; break;
            case KEY_1: rccode = RC_1; break;
            case KEY_2: rccode = RC_2; break;
            case KEY_3: rccode = RC_3; break;
            case KEY_4: rccode = RC_4; break;
            case KEY_5: rccode = RC_5; break;
            case KEY_6: rccode = RC_6; break;
            case KEY_7: rccode = RC_7; break;
            case KEY_8: rccode = RC_8; break;
            case KEY_9: rccode = RC_9; break;
            case KEY_RED: rccode = RC_RED; break;
            case KEY_GREEN: rccode = RC_GREEN; break;
            case KEY_YELLOW: rccode = RC_YELLOW; break;
            case KEY_BLUE: rccode = RC_BLUE; break;
            case KEY_VOLUMEUP: rccode = RC_PLUS; break;
            case KEY_VOLUMEDOWN: rccode = RC_MINUS; break;
            case KEY_MUTE: rccode = RC_MUTE; break;
            case KEY_HELP: rccode = RC_HELP; break;
            case KEY_SETUP: rccode = RC_DBOX; break;
            case KEY_HOME: rccode = RC_HOME; break;
            case KEY_POWER: rccode = RC_STANDBY; break;
            default: rccode = -1; break;
         }
      }
   }
   return rccode;
}
