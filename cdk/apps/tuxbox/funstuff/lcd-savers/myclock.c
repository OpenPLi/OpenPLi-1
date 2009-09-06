/* myclock by pofis	(11/2001)
	alles was nicht von mir ist, ist von ge0rg (geborgt)
   Ach ja, dieses Programm unterliegt der GPL, also nix klauen ;o)
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>



typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int lcd_fd;

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
	}
}

void draw_screen(screen_t s) {
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

// Erzeugt den Doppelpunkt als Trenner zwischen den Zahlen
void doppelpunkt(int pos,screen_t s) {
	int i,j;

for (i=1;i<=4;i++) for (j=1;j<=4;j++) putpixel(i+10+pos*24-3,j+23,1,s);
for (i=1;i<=4;i++) for (j=1;j<=4;j++) putpixel(i+10+pos*24-3,j+37,1,s);
}

//Erzeugt das Balkenmuster
void balken(int pos,int balken,screen_t s){
int i,j;

if (balken==3||balken==6||balken==7)
{
	//horizontaler balken
	if (balken == 6) for (i=1;i<=14;i++) for (j=1;j<=4;j++) putpixel(i+5+pos*24-3,j+14,1,s);
	if (balken == 7) for (i=1;i<=8;i++) for (j=1;j<=4;j++) putpixel(i+8+pos*24-3,j+30,1,s);
}	if (balken == 3) for (i=1;i<=14;i++) for (j=1;j<=4;j++) putpixel(i+5+pos*24-3,j+46,1,s);
else
{
	//vertikaler balken
	if (balken == 1) for (i=1;i<=4;i++) for (j=1;j<=13;j++) putpixel(i+19+pos*24-3,j+18,1,s);
	if (balken == 2) for (i=1;i<=4;i++) for (j=1;j<=13;j++) putpixel(i+19+pos*24-3,j+33,1,s);
	if (balken == 5) for (i=1;i<=4;i++) for (j=1;j<=13;j++) putpixel(i+1+pos*24-3,j+18,1,s);
	if (balken == 4) for (i=1;i<=4;i++) for (j=1;j<=13;j++) putpixel(i+1+pos*24-3,j+33,1,s);

}

}

//Loescht das Matrix
void clear_clock(screen_t s){
int j,i;

for (j=0; j<=125;j++) for (i=0; i<=84;i++) putpixel(j,i,0,s);


}

void myclock(int h,int m,screen_t s) {


//stunden einer
	if (h==0||h==10||h==20) 
	{ balken(1,1,s); balken(1,2,s); balken(1,3,s); balken(1,4,s); balken(1,5,s); balken(1,6,s);}
	if (h==1||h==11||h==21) 
	{ balken(1,1,s); balken(1,2,s);}
	if (h==2||h==12||h==22) 
	{ balken(1,1,s); balken(1,7,s); balken(1,3,s); balken(1,4,s); balken(1,6,s);}
	if (h==3||h==13||h==23) 
	{ balken(1,1,s); balken(1,2,s); balken(1,3,s); balken(1,7,s); balken(1,6,s);}
	if (h==4||h==14) 
	{ balken(1,1,s); balken(1,2,s); balken(1,7,s); balken(1,5,s);}
	if (h==5||h==15) 
	{ balken(1,2,s); balken(1,3,s); balken(1,5,s); balken(1,6,s); balken(1,7,s);}
	if (h==6||h==16) 
	{ balken(1,7,s); balken(1,2,s); balken(1,3,s); balken(1,4,s); balken(1,5,s); balken(1,6,s);}
	if (h==7||h==17) 
	{ balken(1,1,s); balken(1,2,s); balken(1,6,s);}
	if (h==8||h==18) 
	{ balken(1,1,s); balken(1,2,s); balken(1,3,s); balken(1,4,s); balken(1,5,s); balken(1,6,s); balken(1,7,s);}
	if (h==9||h==19) 
	{ balken(1,1,s); balken(1,2,s); balken(1,3,s); balken(1,7,s); balken(1,5,s); balken(1,6,s);}
//stunden zehner
	if (h>9&&h<20){ balken(0,1,s); balken(0,2,s);}
	if (h>19) 	{ balken(0,1,s); balken(0,7,s); balken(0,3,s); balken(0,4,s); balken(0,6,s);}


//minuten einer 
	if (m==0||m==10||m==20||m==30||m==40||m==50) 
	{ balken(4,1,s); balken(4,2,s); balken(4,3,s); balken(4,4,s); balken(4,5,s); balken(4,6,s);}
	if (m==1||m==11||m==21||m==31||m==41||m==51) 
	{ balken(4,1,s); balken(4,2,s);}
	if (m==2||m==12||m==22||m==32||m==42||m==52) 
	{ balken(4,1,s); balken(4,7,s); balken(4,3,s); balken(4,4,s); balken(4,6,s);}
	if (m==3||m==13||m==23||m==33||m==43||m==53) 
	{ balken(4,1,s); balken(4,2,s); balken(4,3,s); balken(4,7,s); balken(4,6,s);}
	if (m==4||m==14||m==24||m==34||m==44||m==54) 
	{ balken(4,1,s); balken(4,2,s); balken(4,7,s); balken(4,5,s);}
	if (m==5||m==15||m==25||m==35||m==45||m==55) 
	{ balken(4,2,s); balken(4,3,s); balken(4,5,s); balken(4,6,s); balken(4,7,s);}
	if (m==6||m==16||m==26||m==36||m==46||m==56) 
	{ balken(4,7,s); balken(4,2,s); balken(4,3,s); balken(4,4,s); balken(4,5,s); balken(4,6,s);}
	if (m==7||m==17||m==27||m==37||m==47||m==57) 
	{ balken(4,1,s); balken(4,2,s); balken(4,6,s);}
	if (m==8||m==18||m==28||m==38||m==48||m==58) 
	{ balken(4,1,s); balken(4,2,s); balken(4,3,s); balken(4,4,s); balken(4,5,s); balken(4,6,s); balken(4,7,s);}
	if (m==9||m==19||m==29||m==39||m==49||m==59) 
	{ balken(4,1,s); balken(4,2,s); balken(4,3,s); balken(4,7,s); balken(4,5,s); balken(4,6,s);}

	//minuten zehner
	if (m<10)
	{ balken(3,1,s); balken(3,2,s); balken(3,3,s); balken(3,4,s); balken(3,5,s); balken(3,6,s);}
	if (m>9&&m<20){ balken(3,1,s); balken(3,2,s);}
	if (m>19&&m<30){ balken(3,1,s); balken(3,7,s); balken(3,3,s); balken(3,4,s); balken(3,6,s);}
	if (m>29&&m<40){ balken(3,1,s); balken(3,2,s); balken(3,3,s); balken(3,7,s); balken(3,6,s);}
	if (m>39&&m<50){ balken(3,1,s); balken(3,2,s); balken(3,7,s); balken(3,5,s);}
	if (m>=50)	 { balken(3,2,s); balken(3,3,s); balken(3,5,s); balken(3,6,s); balken(3,7,s);}


 }







void init() {
	int i;
	if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
		perror("open(/dev/dbox/lcd0)");
		exit(1);
	}
	clr();

	i = LCD_MODE_BIN;
	if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&i) < 0) {
		perror("init(LCD_MODE_BIN)");
		exit(1);
	}
}







int main(int argc, char *args[]) {
	screen_t screen;
 

        struct timeval tb;
        struct tm *t;
	
      

	
	init();
	while (1) {

  			gettimeofday(&tb, NULL);
			t = localtime(&tb.tv_sec);
			clear_clock(screen);
	                	
			doppelpunkt(2,screen);
			myclock(t->tm_hour,t->tm_min, screen);
			
			//putpixel(10,20,1,screen);
			draw_screen(screen);
			
			sleep(10);
	}
	clr();
	return 0;
}



