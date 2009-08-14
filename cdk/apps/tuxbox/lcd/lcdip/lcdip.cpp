#include <stdio.h>
#include "lcddisplay.h"
#include "rc.h"

CLCDDisplay display;

int pt[3]={1, 10, 100};

int main()
{
	display.draw_string(10, 10, "IP eingeben:");
	FILE* pFile=fopen("/var/lcdip", "r"); // tum lesen "w" zum schreiben
	int update=0;
	update=1;
	
	int curpos=0;
	//int ip[4]={192, 168, 0, 202};
	int ip[4];
	for(int i=0; i< 4; i++) ip[i]=fgetc(pFile);
	fclose(pFile);
	int done=0;
	RCOpen();
	while (!done)
	{
		if (!update)
		{
			int code;
			switch (code=RCGet())
			{
			case RC_LEFT:
				if (curpos)
				{
					curpos--;
					update=1;
				}
				break;
			case RC_RIGHT:
				if (curpos<14)
				{
					curpos++;
					update=1;
				}
				break;
			case RC_UP:
			{
				if ((curpos%4)==3)
					break;
				int i=curpos/4;
				int a=2-(curpos%4);
				int nv=ip[i];
				int s=(nv/pt[a])%10;
				nv-=s*pt[a];
				s++;
				if (s>9)
					s=0;
				nv+=s*pt[a];
				ip[i]=nv; 
				update=1;
				break;
			}
			case RC_0...RC_9:
			{
				if ((curpos%4)==3)
					break;
				int i=curpos/4;
				int a=2-(curpos%4);
				int nv=ip[i];
				int s=(nv/pt[a])%10;
				nv-=s*pt[a];
				s=code;
				nv+=s*pt[a];
				ip[i]=nv; 
				update=1;
				if (curpos<14)
				{
					curpos++;
					if ((curpos%4)==3)
						curpos++;
				} /* else
					done=1;*/ 
				break;
			}
			case RC_DOWN:
			{
				if ((curpos%4)==3)
					break;
				int i=curpos/4;
				int a=2-(curpos%4);
				int nv=ip[i];
				int s=(nv/pt[a])%10;
				nv-=s*pt[a];
				s--;
				if (s<0)
					s=9;
				nv+=s*pt[a];
				ip[i]=nv; 
				update=1;
				break;
			}
			case RC_OK:
				done=1;
				update=1;
				break;
			}
		}
		if (update)
		{
			char ips[16];
			sprintf(ips, "%03d.%03d.%03d.%03d", ip[0], ip[1], ip[2], ip[3]);
			if (done)
			{
				if ((ip[0]>255) || (ip[1]>255) || (ip[2]>255) || (ip[3]>255))
				{
					done=0;
					display.draw_string(0, 30, " UNGÜLTIGE IP! ");
					display.update();
					while (RCGet()!=RC_OK);
				}
			}			
			display.draw_string(0, 30, ips);
			if (!done)
				display.draw_rectangle(curpos*8, 30, curpos*8+7, 37, LCD_PIXEL_INV, LCD_PIXEL_INV);
			display.update();
			update=0;
		}
	}
	RCClose();
	
	char exec[100];
	sprintf(exec, "/sbin/ifconfig eth0 %d.%d.%d.%d up", ip[0], ip[1], ip[2], ip[3]);
	FILE* paFile=fopen("/var/lcdip", "w"); 
	for(int i=0; i< 4; i++) putc(ip[i], paFile);
	fclose(paFile);

	if (!(system(exec)>>8))
	{
		display.draw_string(0, 30, "  IP GESETZT!  ");
		display.update();
	} else
	{
		display.draw_string(0, 30, "    FEHLER!    ");
		display.update();
	}
	return 0;
}
