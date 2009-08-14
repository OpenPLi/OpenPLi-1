/* 
TuxTerm v0.2

Written 10/2006 by Seddi
Contact: seddi@ihad.tv / http://www.ihad.tv
*/

#ifdef HAVE_CONFIG_H
        #include "config.h"
#endif
        
#include "main.h"
#include "render.h"
#include "colors.h"

void main2(int thepipe, int rows, int cols);
int g1conversion (int ch);
int bpp;

sigchld_handler (int signum)
{
	printf("Child teminated\n");
	
	//Cleanup
	FTC_Manager_Done(manager);
	FT_Done_FreeType(library);

	//Clear Screen
	memset(lfb, 0, fix_screeninfo.smem_len);
	munmap(lfb, fix_screeninfo.smem_len);
	ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
	close(fb);
	free(lbb);
	exit(0);
}

void setcolors(unsigned short *pcolormap, int offset, int number)
{
	int i,l;
	int j = offset; /* index in global color table */

	for (i = 0; i < number; i++)
	{
		int r = (pcolormap[i] << 12 & 0xF000) >> 8;
		int g = (pcolormap[i] << 8 & 0xF000) >> 8;
		int b = (pcolormap[i] << 4 & 0xF000) >> 8;

#ifdef FB8BIT	
		rd0[j] = r<<8;
		gn0[j] = g<<8;
		bl0[j] = b<<8;
		bgra[j][0] = j;
		bgra[j][1] = j;
#else
		bgra[j][4]=j;
		bgra[j][2]=r;
		bgra[j][1]=g;
		bgra[j][0]=b;
#endif
		// fill color lines for faster boxdrawing
		for (l = 0; l < 720; l++)
		{
#ifdef FB8BIT
			colorline[j][l]=j;
#else
			colorline[j][(l*bpp)+3]=0xFF;
			colorline[j][(l*bpp)+2]=r;
			colorline[j][(l*bpp)+1]=g;
			colorline[j][(l*bpp)+0]=b;
#endif
		}

		
		j++;
	}

#ifdef FB8BIT
	// set 8bit color table
	if (ioctl(fb, FBIOPUTCMAP, &colormap_0) == -1)
			printf("TuxTerm <FBIOPUTCMAP>");	
#endif

}

// Hauptroutine
main(argc,argv)
int argc;
char *argv[];
{

	int ret;
	struct sigaction sa;
	struct sigaction osa;
	sa.sa_handler = sigchld_handler;
	sigfillset (&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	ret = sigaction (SIGCHLD, &sa, &osa);
	if (ret < 0) {
		printf ("Signal handler failed\n");
		return;
	}

	int mypipe[2];
	pid_t pid;
	if (pipe (mypipe)) {
		printf("Pipe failed\n");
		exit(0);
	}
	pid=fork();
	if (pid < (pid_t) 0) {
		printf("Fork failed\n");
		exit(0);
	}

                

	// Defaults
	StartX=60;
	StartY=61;//48+13
	PosX=0;
	PosY=0;

	int rows=28;//28
	int cols=60;//60
	int screenx=600;
	int screeny=448;
	
	// read config
	unsigned char buf[256];
#ifdef FB8BIT
	FILE *file=fopen("/var/tuxbox/config/tuxterm.cfg","r");
#else
	FILE *file=fopen("/etc/tuxbox/tuxterm.cfg","r");
#endif
	if (!file) {
#ifdef FB8BIT
		FILE *file=fopen("/var/tuxbox/config/tuxterm.cfg","w");
#else
		FILE *file=fopen("/etc/tuxbox/tuxterm.cfg","w");
#endif
		fputs("# Tuxterm Config File V0.2\n",file);
		fputs("\n",file);
		fputs("ROWS=28\n",file);
		fputs("COLS=60\n",file);
		fputs("WINX=600\n",file);
		fputs("WINY=448\n",file);
		fclose(file);	
	} else  {
		while (fgets(buf,sizeof(buf),file)) {
			if (sscanf(buf,"ROWS=%d", &rows) == 1) {}
			if (sscanf(buf,"COLS=%d", &cols) == 1) {}
			if (sscanf(buf,"WINX=%d", &screenx) == 1) {}
			if (sscanf(buf,"WINY=%d", &screeny) == 1) {}
		}
	fclose(file);
	}
	
	int spcx=screenx/cols;//10;//10
	int spcy=screeny/rows;//spcx*2;//1.6;//448/rows;//16;//20
	int yoff=spcy/1.23;//13; //- 13
	int yoff2=spcy-yoff-1;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.width = spcx*1.7; desc.height = spcy;//FONTHEIGHT_VERY_SMALL;
#else
	desc.font.pix_width = spcx*1.7; desc.font.pix_height = spcy;//FONTHEIGHT_VERY_SMALL;
#endif

	StartX=(720-(spcx*cols))/2;
	StartY=((576-(spcy*rows))/2)+yoff;

	// Bytes per Pixel
#ifdef FB8BIT
	bpp=1;
#else
	bpp=4;
#endif		

	// Child Process ? Dann Telnet
	if (pid == (pid_t) 0) {main2(mypipe[1],rows,cols);exit(0);}
	printf("Starting Console App\n"); 

	
	// Framebuffer oeffnen
	fb=open("/dev/fb/0", O_RDWR);
	if (fb == -1)
	{
		printf("Framebuffer failed\n");
		return;
	}

	// Framebuffer initialisieren
	if(ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("TuxCom <FBIOGET_VSCREENINFO failed>\n");
		return;
	}

	var_screeninfo_original = var_screeninfo;

	var_screeninfo.xres = 720;
	var_screeninfo.yres = 576;

	/* set variable screeninfo for double buffering */
	var_screeninfo.yres_virtual = 2*var_screeninfo.yres;
	var_screeninfo.xres_virtual = var_screeninfo.xres;
	var_screeninfo.xoffset = var_screeninfo.yoffset = 0;
	var_screeninfo.bits_per_pixel=8*bpp;

	if(ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("TuxTerm <FBIOPUT_VSCREENINFO failed>\n");
		return;
	}
	
	if(ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
	{
		printf("TuxTerm <FBIOGET_FSCREENINFO failed>\n");
		return;
	}

	// set new colormap
	setcolors((unsigned short *)defaultcolors, 0, SIZECOLTABLE);
	
	if(!(lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)))
	{
		printf("Framebuffer: <Speichermapping fehlgeschlagen>\n");
		return;
	}

	// Freetype initialisieren
	if((error = FT_Init_FreeType(&library)))
	{
		printf("Freetype <FT_Init_FreeType fehlgeschlagen. Fehler: 0x%.2X>", error);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_Manager_New(library, 1, 2, 0, &MyFaceRequester, NULL, &manager)))
	{
		printf("Freetype <FTC_Manager_New fehlgeschlagen. Fehler: 0x%.2X>\n", error);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_SBitCache_New(manager, &cache)))
	{
		printf("Freetype <FTC_SBitCache_New fehlgeschlagen. Fehler: 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_Manager_Lookup_Face(manager, FONT, &face)))
	{
		if((error = FTC_Manager_Lookup_Face(manager, FONT2, &face)))
		{
			printf("Freetype <FTC_Manager_Lookup_Face fehlgeschlagen. Fehler: 0x%.2X>\n", error);
			FTC_Manager_Done(manager);
			FT_Done_FreeType(library);
			memset(lfb, 0, fix_screeninfo.smem_len);
			munmap(lfb, fix_screeninfo.smem_len);
			ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
			return;
		}
		else
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			desc.face_id = FONT2;
#else
			desc.font.face_id = FONT2;
#endif
	}
	else
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		desc.face_id = FONT;
#else
		desc.font.face_id = FONT;
#endif


	use_kerning = FT_HAS_KERNING(face);

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.flags = FT_LOAD_MONOCHROME;
#else
	desc.image_type = ftc_image_mono;
#endif

	
	// Backbuffer initialisieren
	if(!(lbb = malloc(var_screeninfo.xres*fix_screeninfo.line_length*bpp)))
	{
		printf("Backbuffer <Speicherbereich kann nicht reserviert werden.>\n");
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	// Screen leeren
#ifdef FB8BIT
	memset(lbb, transp, var_screeninfo.yres*fix_screeninfo.line_length);
#else
	memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
#endif
	memcpy(lfb, lbb, var_screeninfo.yres*fix_screeninfo.line_length);

	// Fenster erzeugen
	RenderBox(-2,-2-yoff,spcx*cols+2,spcy*rows+2-yoff,FILL,bgra[white]);
	RenderBox(0,0-yoff,spcx*cols,spcy*rows-yoff,FILL,bgra[black]);

	int curx=0;
	int cury=0;
	int curaltx=0;
	int curalty=0;
	int scroll_s=1;
	int scroll_e=rows;
	int dlines=-1;
	int g1=0;
	int decawm=1;

	FILE *fp;
	int ch;
	int a_flag=0;
	int autolb=0;
	int z,fr;
	int reverse=0;
	unsigned char chbuf;
	char ch2[2];
	char ansi[50];
	unsigned char color[bpp+1];
	unsigned char tcolor[bpp+1];
	memcpy(color,bgra[white],bpp+1);
	unsigned char bcolor[bpp+1];
	memcpy(bcolor,bgra[black],bpp+1);

	unsigned char bcoloralt[bpp*spcx];
	// Cursor sichern
	memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);


	// Telnet forken und pipe öffnen
	int pidstatus;
	fp = fdopen (mypipe[0], "r");
//	fcntl(fileno(fp), F_SETFL,O_NONBLOCK);
	while(waitpid (pid, &pidstatus, WNOHANG) != pid) {


	ch=fgetc(fp);
//if (ch>-1 && ch < 27) {printf("Control-Code %d\n",ch);}

	if (ch == 7){ //BELL
		/*NIX*/ch=-1;}
	if (ch == 8){ //BS
		curx--;ch=-1;}
	if (ch == 9){ //HT
		/*NIX*/ch=-1;}
	if (ch == 10){ //LF
		// autolinebreak one char before ?
		if(autolb == 1) cury--;
		cury++;ch=-1;}
	if (ch == 11){ //VT
		/*NIX*/ch=-1;}
	if (ch == 12){ //FF
		cury++;ch=-1;}
	if (ch == 13){ //CR
		curx=0;ch=-1;}
	if (ch == 14){ //SO
		g1=1;ch=-1;}
	if (ch == 15){ //SI
		g1=0;ch=-1;}

	autolb=0;

	//Parse Ansi
	if (a_flag == 1) {
		switch (ch) {
			case 91: // [
				a_flag=2;
				break;
			case 68: // D (IND)
				cury++;
				if (cury>rows-1) {
					// Cursor del
					memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);
					// Scroll
					memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
					memcpy(lbb+(fix_screeninfo.line_length*((scroll_s*spcy)-yoff+StartY)), lfb+(fix_screeninfo.line_length*(((scroll_s+1)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					memcpy(lfb+(fix_screeninfo.line_length*((scroll_s*spcy)-yoff+StartY)), lbb+(fix_screeninfo.line_length*(((scroll_s)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					RenderBox(0,(scroll_e-1)*spcy-yoff,spcx*cols,(scroll_e-1)*spcy+yoff2,FILL,bcolor);
					// Print Cursor
					memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
					RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
					cury--;
				}
				break;
				
			case 77: // M (RI)
				cury--;
				if (cury<0) {
					// Cursor del
					memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);
					// Scroll
	 				memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
					memcpy(lbb+(fix_screeninfo.line_length*(((scroll_s+1)*spcy)-yoff+StartY)), lfb+(fix_screeninfo.line_length*((scroll_s*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					memcpy(lfb+(fix_screeninfo.line_length*(((scroll_s+1)*spcy)-yoff+StartY)), lbb+(fix_screeninfo.line_length*(((scroll_s+1)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					RenderBox(0,(scroll_s)*spcy-yoff,spcx*cols,(cury)*spcy+yoff2,FILL,bcolor);
					// Print Cursor
					memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
					RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
					cury++;
				}
				break;

			case 69: // E (NEL)
				cury++;curx=0;
				if (cury>rows-1) {
					// Cursor del
					memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);
					// Scroll
					memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
					memcpy(lbb+(fix_screeninfo.line_length*((scroll_s*spcy)-yoff+StartY)), lfb+(fix_screeninfo.line_length*(((scroll_s+1)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					memcpy(lfb+(fix_screeninfo.line_length*((scroll_s*spcy)-yoff+StartY)), lbb+(fix_screeninfo.line_length*(((scroll_s)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					RenderBox(0,(scroll_e-1)*spcy-yoff,spcx*cols,(scroll_e-1)*spcy+yoff2,FILL,bcolor);
					// Print Cursor
					memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
					RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
					cury--;
				}
				break;
				
			default: // Nix
				break;
		}
	}

	if (a_flag == 2) {
		z=0;
		while (a_flag == 2) {
			ansi[z]=ch;
			ch=-1;
			ch=fgetc(fp);
			if (ch >= 0x41 && ch <= 0x5A || ch >=0x61 && ch <= 0x7A) {a_flag=3;ansi[z+1]=ch;ch=0;}
			z++;
		}
	}

	if (a_flag == 3) {

		if (ansi[z] == 0x6d && z == 1) {
			memcpy(color,bgra[white],bpp+1);
			memcpy(bcolor,bgra[black],bpp+1);
			reverse=0;
		}

		if (ansi[z] == 0x6d && z == 2) {
			switch(ansi[z-1]) {
				case 0x37: //Reverse
					if (reverse == 1) {reverse=0;} else {reverse=1;}
					break;
				case 0x30: // Reset
					memcpy(color,bgra[white],bpp+1);
					memcpy(bcolor,bgra[black],bpp+1);
					reverse=0;
					break;
			}
		}


		if (ansi[z] == 0x6d && ansi[z-2] == 51) { // COLOR
			switch(ansi[z-1]) {
				case 48://black
					memcpy(color,bgra[black],bpp+1);
					break;
				case 49://red
					memcpy(color,bgra[red],bpp+1);
					break;
				case 50://green
					memcpy(color,bgra[green],bpp+1);
					break;
				case 51://yellow
					memcpy(color,bgra[yellow],bpp+1);
					break;
				case 52://blue
					memcpy(color,bgra[blue],bpp+1);
					break;
				case 53://magenta
					memcpy(color,bgra[magenta],bpp+1);
					break;
				case 54://cyan
					memcpy(color,bgra[cyan],bpp+1);
					break;
				case 55://white
					memcpy(color,bgra[white],bpp+1);
					break;
				case 57://default(white)
					memcpy(color,bgra[white],bpp+1);
					break;
				}
		}
		if (ansi[z] == 0x6d && ansi[z-2] == 52) { // BGCOLOR
			switch(ansi[z-1]) {
				case 48://black
					memcpy(bcolor,bgra[black],bpp+1);
					break;
				case 49://red
					memcpy(bcolor,bgra[red],bpp+1);
					break;
				case 50://green
					memcpy(bcolor,bgra[green],bpp+1);
					break;
				case 51://yellow
					memcpy(bcolor,bgra[yellow],bpp+1);
					break;
				case 52://blue
					memcpy(bcolor,bgra[blue],bpp+1);
					break;
				case 53://magenta
					memcpy(bcolor,bgra[magenta],bpp+1);
					break;
				case 54://cyan
					memcpy(bcolor,bgra[cyan],bpp+1);
					break;
				case 55://white
					memcpy(bcolor,bgra[white],bpp+1);
					break;
				case 57://default(black)
					memcpy(bcolor,bgra[black],bpp+1);
					break;
				}
		}
		if (ansi[z] == 0x48 || ansi[z] == 0x66) { // Cursorposition
			if (z == 1) {curx=cury=0;/*printf("Cursor: %d %d\n",curx,cury);*/} else
			{
			curx=ansi[z-1]-48;
			if (ansi[z-2] != 0x3b) {curx=curx+((ansi[z-2]-48)*10);z=z-2;} else {z--;}
			cury=ansi[z-2]-48;
			if (z-2 > 1) {cury=cury+((ansi[z-3]-48)*10);} 
			curx--;cury--;
//			printf("Set-Cursor: %d %d\n",curx,cury);
			}
		}
		if (ansi[z] == 0x72 && ansi[1] != 0x3F) { // Scrolling Region
			if (z == 1) {scroll_s=1;scroll_e=24;} else
			{
			scroll_e=ansi[z-1]-48;
			if (ansi[z-2] != 0x3b) {scroll_e=scroll_e+((ansi[z-2]-48)*10);z=z-2;} else {z--;}
			scroll_s=ansi[z-2]-48;
			if (z-2 > 1) {scroll_s=scroll_s+((ansi[z-3]-48)*10);} 
//			printf("Scroll-Region: %d %d\n",scroll_s,scroll_e);
			}
		}
		if ((ansi[z] == 0x4A && z == 1) || (ansi[z] == 0x4A && ansi[z-1] == 0x30)) { // Clear Screen bottom [J
			RenderBox(curx*spcx,cury*spcy-yoff,spcx*cols,cury*spcy+yoff2,FILL,bcolor);
			RenderBox(0,(cury+1)*spcy-yoff,spcx*cols,(rows-1)*spcy-yoff,FILL,bcolor);
		}
		if (ansi[z] == 0x4A && ansi[z-1] == 0x31) { // Clear Screen top [1J
			RenderBox(0,cury*spcy-yoff,curx*spcx,cury*spcy+yoff2,FILL,bcolor);
			RenderBox(0,0,spcx*cols,cury*spcy-yoff,FILL,bcolor);
		}
		if (ansi[z] == 0x4A && ansi[z-1] == 0x32) { // Clear Screen [2J
			RenderBox(0,0-yoff,spcx*cols,spcy*rows-yoff,FILL,bcolor);
			curx=cury=0;
		}
		if ((ansi[z] == 0x4B && z == 1) || (ansi[z] == 0x4B && ansi[z-1] == 0x30)) { // Clear Line till end [K
			RenderBox(curx*spcx,cury*spcy-yoff,spcx*cols,cury*spcy+yoff2,FILL,bcolor);
		}
		if (ansi[z] == 0x4B && ansi[z-1] == 0x31) { // Clear Line from start [1K
			RenderBox(0,cury*spcy-yoff,curx*spcx,cury*spcy+yoff2,FILL,bcolor);
		}
		if (ansi[z] == 0x4B && ansi[z-1] == 0x32) { // Clear Line [2K
			RenderBox(0,cury*spcy-yoff,spcx*cols,cury*spcy+yoff2,FILL,bcolor);
		}
		if (ansi[z] == 0x4D) { // Delete Line [.M
			if (z == 1) {dlines=1;} else 
			{
			dlines=(ansi[z-1]-48);
			if (z == 3) {dlines=dlines+((ansi[z-2]-48)*10);} 
			}
//			printf("x: %d y: %d DLines: %d\n",curx,cury,dlines);
			// Cursor del
			memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);
			if (cury <= scroll_e-1 && cury >= scroll_s-1 && dlines>0) {
				//Scrolling
				while (dlines > 0) {
					memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
					memcpy(lbb+(fix_screeninfo.line_length*((cury*spcy)-yoff+StartY)), lfb+(fix_screeninfo.line_length*(((cury+1)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					memcpy(lfb+(fix_screeninfo.line_length*((cury*spcy)-yoff+StartY)), lbb+(fix_screeninfo.line_length*(((cury)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					RenderBox(0,(scroll_e-1)*spcy-yoff,spcx*cols,(scroll_e-1)*spcy+yoff2,FILL,bcolor);
					dlines--;
				}
				curx=0;
			}
			// Print Cursor
			memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
			RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
			curaltx=curx;
			curalty=cury;
			dlines=-1;
		}
		if (ansi[z] == 0x4C) { // Insert Line [.L
			if (z == 1) {dlines=1;} else 
			{
			dlines=(ansi[z-1]-48);
			if (z == 3) {dlines=dlines+((ansi[z-2]-48)*10);} 
			}
//			printf("x: %d y: %d ILines: %d\n",curx,cury,dlines);
			// Cursor del
			memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);
			if (cury <= scroll_e-1 && cury >= scroll_s-1 && dlines>0) {
				//Scrolling
				while (dlines > 0) {
					memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
					memcpy(lbb+(fix_screeninfo.line_length*(((cury+1)*spcy)-yoff+StartY)), lfb+(fix_screeninfo.line_length*((cury*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					memcpy(lfb+(fix_screeninfo.line_length*(((cury+1)*spcy)-yoff+StartY)), lbb+(fix_screeninfo.line_length*(((cury+1)*spcy)-yoff+StartY)), (scroll_e-scroll_s)*spcy*fix_screeninfo.line_length);
					RenderBox(0,(cury)*spcy-yoff,spcx*cols,(cury)*spcy+yoff2,FILL,bcolor);
					dlines--;
				}
				curx=0;
			}
			// Print Cursor
			memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
			RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
			curaltx=curx;
			curalty=cury;
			dlines=-1;
		}
		if (ansi[z] == 0x41) { // Cursor up
			if (z == 1) {cury--;} else 
			{
			cury=cury-(ansi[z-1]-48);
			if (z == 3) {cury=cury-((ansi[z-2]-48)*10);} else
				{if (ansi[z-1] == 48) cury--;}
			}
			if (cury<0) {cury=0;}
		}
		if (ansi[z] == 0x42) { // Cursor down
			if (z == 1) {cury++;} else 
			{
			cury=cury+(ansi[z-1]-48);
			if (z == 3) {cury=cury+((ansi[z-2]-48)*10);} else
				{if (ansi[z-1] == 48) cury++;}
			}
			if (cury>(rows-1)) {cury=rows-1;}
		}

		if (ansi[z] == 0x43) { // Cursor right
			if (z == 1) {curx++;} else 
			{
			curx=curx+(ansi[z-1]-48);
			if (z == 3) {curx=curx+((ansi[z-2]-48)*10);} else
				{if (ansi[z-1] == 48) curx++;}
			}
			if (curx>(cols-1)) {curx=cols-1;}
		}
		if (ansi[z] == 0x44) { // Cursor left
			if (z == 1) {curx--;} else 
			{
			curx=curx-(ansi[z-1]-48);
			if (z == 3) {curx=curx-((ansi[z-2]-48)*10);} else
				{if (ansi[z-1] == 48) curx--;}
			}
			if (curx<0) {curx=0;}
		}
		if (ansi[z] == 0x68) { // Autowrap on
			if (ansi[z-1] == 0x37 && ansi[z-2] == 0x3F)
				decawm=1;
		}
		if (ansi[z] == 0x6c) { // Autowrap off
			if (ansi[z-1] == 0x37 && ansi[z-2] == 0x3F)
				decawm=0;
		}


		a_flag=0;
	}


	if (ch == 27) {
		a_flag=1;ansi[0]='\0';}



	// Cursor del
	memcpy(lfb+(curalty*spcy+yoff2+StartY)*fix_screeninfo.line_length+(curaltx*spcx+StartX)*bpp,bcoloralt,(spcx)*bpp);

	if (ch>27 && a_flag==0) {
//	printf("%c\n",ch);

	// Graphics ?
	if (g1 == 1) {ch=g1conversion(ch);}
	if (reverse == 1) {
		RenderBox(curx*spcx,cury*spcy-yoff,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,color);
		RenderChar(ch,curx*spcx,cury*spcy,(curx*spcx)+spcx*2,bcolor);
		} else {
		RenderBox(curx*spcx,cury*spcy-yoff,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bcolor);
		RenderChar(ch,curx*spcx,cury*spcy,(curx*spcx)+spcx*2,color);
		}

	curx++;
	}


	if (curx>(cols-1)) {
		if (decawm == 1) {
			autolb=1;
			curx=0;
			cury++;
		} else {
			curx--;
		}
	
	}

	if (curx<0) {
		curx=0;}

	//Scrolling
	while (cury>(rows-1)) {

	memset(lbb, 0, var_screeninfo.yres*fix_screeninfo.line_length);
	memcpy(lbb+(fix_screeninfo.line_length*(StartY-yoff)), lfb+(fix_screeninfo.line_length*(StartY+spcy-yoff)), (rows*spcy-spcy)*fix_screeninfo.line_length);
	memcpy(lfb+(fix_screeninfo.line_length*(StartY-yoff)), lbb+(fix_screeninfo.line_length*(StartY-yoff)), (rows*spcy-spcy)*fix_screeninfo.line_length);
	RenderBox(0,(rows-1)*spcy-yoff,spcx*cols,(rows-1)*spcy+yoff2,FILL,bcolor);

	cury--;
	}
	
//	printf("Cursor %d %d\n",curx,cury);


	// Print Cursor
	memcpy(bcoloralt,lfb+fix_screeninfo.line_length*(cury*spcy+yoff2+StartY)+(curx*spcx+StartX)*bpp,(spcx)*bpp);
	RenderBox(curx*spcx,cury*spcy+yoff2,curx*spcx+spcx-1,cury*spcy+yoff2,FILL,bgra[white]);	
	curaltx=curx;
	curalty=cury;

	}

	fclose(fp);
	
	//Cleanup
	FTC_Manager_Done(manager);
	FT_Done_FreeType(library);

	//Clear Screen
	memset(lfb, 0, fix_screeninfo.smem_len);
	munmap(lfb, fix_screeninfo.smem_len);
	ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);

	close(fb);
	free(lbb);
	exit(0);
}


//***************************************************
// Telnet Application for Child Process
//***************************************************
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/stat.h>

unsigned char* RCTranslate(unsigned short code, int shift);
unsigned char* URCTranslate(unsigned short code, int shift);

unsigned char* upcase(unsigned char* mixedstr) 
{
	int j;
	for (j=0; j< strlen(mixedstr); ++j)
	{
		mixedstr[j]=toupper(mixedstr[j]);
	} 
	return mixedstr;
}

void main2(int thepipe, int rows, int cols)
{
	int sock, run, run2;
	unsigned char *buf;
	unsigned char ch,ch2;
	struct sockaddr_in server;

	printf("Starting Telnet App\n"); 


//*******************************
	char kbbuf[32];
	int x,x2;
	int shift=0;
	int caps=0;
	unsigned short	code = 0xee;
	struct input_event ev;

	int kbfd,rcfd,usbfd;
	
	// Dream IR Keyboard suchen und oeffnen
	int cnt=0;
	while(1)
	{
		struct stat s;
		char tmp[128];
		sprintf(tmp, "/dev/input/event%d", cnt);
		if (stat(tmp, &s))
			break;
		/* open Remote Control */
		if ((kbfd=open(tmp, O_RDONLY|O_NONBLOCK)) == -1)
		{
			printf("<open dream keyboard control>");
			exit(0);
		}
		if (ioctl(kbfd, EVIOCGNAME(128), tmp) < 0)
			printf("EVIOCGNAME failed");
		if (strstr(upcase(tmp), "DREAMBOX IR KEYBOARD")) {
			printf("dream ir keyboard found: event%d\n",cnt);
			break;}
		close(kbfd);
		kbfd=-1;
		++cnt;
	}

	// USB Keyboard suchen und oeffnen
	cnt=0;
	while(1)
	{
		struct stat s;
		char tmp[128];
		sprintf(tmp, "/dev/input/event%d", cnt);
		if (stat(tmp, &s))
			break;
		/* open Remote Control */
		if ((usbfd=open(tmp, O_RDONLY|O_NONBLOCK)) == -1)
		{
			printf("<open usb keyboard control>");
			exit(0);
		}
		if (ioctl(usbfd, EVIOCGNAME(128), tmp) < 0)
			printf("EVIOCGNAME failed");
		if (strstr(upcase(tmp), "KEYBOARD") && !strstr(upcase(tmp), "DREAMBOX IR KEYBOARD")) {
			printf("usb keyboard found: event%d\n",cnt);
			break;}
		close(usbfd);
		usbfd=-1;
		++cnt;
	}	
	
#ifdef FB8BIT
	// old RC
	rcfd = open("/dev/dbox/rc0", O_RDONLY|O_NONBLOCK);
	unsigned short rccode;
	// RC Buffer leeren
	read(rcfd,buf,32);
#else
	// Dream RC suchen und oeffnen
	cnt=0;
	while(1)
	{
		struct stat s;
		char tmp[128];
		sprintf(tmp, "/dev/input/event%d", cnt);
		if (stat(tmp, &s))
			break;
		/* open Remote Control */
		if ((rcfd=open(tmp, O_RDONLY|O_NONBLOCK)) == -1)
		{
			printf("<open dream rc control>");
			exit(0);
		}
		if (ioctl(rcfd, EVIOCGNAME(128), tmp) < 0)
			printf("EVIOCGNAME failed");
		if (strstr(tmp, "remote control")) {
			printf("dream rc found: event%d\n",cnt);
			break;}
		close(rcfd);
		rcfd=-1;
		++cnt;
	}
#endif
	
//*******************************


	// pipe
	FILE *fp;
	fp = fdopen (thepipe, "w");
	unsigned char tmp[2];

	/* create socket */
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0) {
		printf("open stream socket");
		exit(0);}

	server.sin_family = AF_INET;
	/* set host */
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	/* set port */
	server.sin_port = htons ((unsigned short int) atol("23"));
	/* open connection */

	if(connect(sock,&server,sizeof(server)) < 0) {
		printf("connecting stream socket");
		exit(0);
	}

	fcntl(sock,F_SETFL,O_NONBLOCK);

	write(sock,"\x7F",1); //Start
	
	/*Window Size 60x24 3c*18*/
	//IAC WILL NAWS IAC SB NAWS WIDTH[1] WIDTH[0] HEIGHT[1] HEIGHT[0] IAC SE 
	//255 251 31 255 250 31 x x x x 255 240
	//xFF xFB x1F xFF xFA x1F x00 x3c x00 x18 xFF xF0
	write(sock,"\xFF\xFB\x1F\xFF\xFA\x1F\x00",7);
	sprintf(tmp,"%c",cols);
	write(sock,tmp,1);//\x3C
	write(sock,"\x00",1);
	sprintf(tmp,"%c",rows);
	write(sock,tmp,1);//\x18
	write(sock,"\xFF\xF0",2);

	/* process data */
	while(run=recv(sock,&ch,1,0))
	{

		if (ch == 255) {
			run=recv(sock,&ch,1,0);
			if (ch == 253) { //DO
				run=recv(sock,&ch,1,0);
//				printf("Control: %d\n",ch);
				switch(ch) {
				case 1: // ECHO
//					printf("Echo\n");
					write(sock,"\xFF\xFB\x01",3);
					break;
				case 24:  // Terminal
//					printf("Want Terminaltype\n");
					write(sock,"\xFF\xFB\x18\xFF\xFA\x18\x00VT220\xFF\xF0",14);
					break;
				default: // Everything else
//					printf("???\n");
/* 					write(sock,"\xFF\xFC",2); //WONT
					write(sock,ch,1);*/
					break;
				}
			}
			if (ch == 251) { //WILL
				run=recv(sock,&ch,1,0);
//				printf("Control: %d\n",ch);
				switch(ch) {
				default: // Everything else
//					printf("???\n");
/*					write(sock,"\xFF\xFE",2); //DONT
					write(sock,ch,1);*/
					break;
				}
			}
			if (ch == 252) { //DONT
				run=recv(sock,&ch,1,0);
//				printf("Control: %d\n",ch);
			}
			if (ch == 254) { //WONT
				run=recv(sock,&ch,1,0);
//				printf("Control: %d\n",ch);
			}
				

		} else {
		if (run > 0) {
			fputc(ch,fp);
			//fprintf(fp,"%c",ch);
			fflush(fp);
		}

//**********************************

		x = read( kbfd, &ev, sizeof(struct input_event) );
		if (x > 0) {
		memcpy(&code,&ev.code,sizeof(ev.code));

		if ((code == 0x2A || code == 0x36) && ev.value == 1 && ev.type == EV_KEY)
			shift=1;
		if ((code == 0x2A || code == 0x36) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_LEFTCTRL) && ev.value == 1 && ev.type == EV_KEY)
			shift=2;
		if ((code == KEY_LEFTCTRL) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_RIGHTALT) && ev.value == 1 && ev.type == EV_KEY)
			shift=3;
		if ((code == KEY_RIGHTALT) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_CAPSLOCK) && ev.value == 1 && ev.type == EV_KEY) {
			if (caps == 1) {caps=0;} else {caps=1;}}

		if (shift == 0 && caps == 1) {shift=1;}

		if ((code == 385 /*TV*/) && ev.value == 1 && ev.type == EV_KEY) {
			exit(0);/*Notausstieg*/}


		if  ((ev.value == 1 || ev.value == 2) && ev.type == EV_KEY) {
//			printf("%d\n",code);
			buf=RCTranslate(code,shift);
			if (strlen(buf) > 0)
				write(sock,buf,strlen(buf));
		}
		}
	
		x = read( usbfd, &ev, sizeof(struct input_event) );
		if (x > 0) {
		memcpy(&code,&ev.code,sizeof(ev.code));

		if ((code == 0x2A || code == 0x36) && ev.value == 1 && ev.type == EV_KEY)
			shift=1;
		if ((code == 0x2A || code == 0x36) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) && ev.value == 1 && ev.type == EV_KEY)
			shift=2;
		if ((code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_RIGHTALT) && ev.value == 1 && ev.type == EV_KEY)
			shift=3;
		if ((code == KEY_RIGHTALT) && ev.value == 0 && ev.type == EV_KEY)
			shift=0;
		if ((code == KEY_CAPSLOCK) && ev.value == 1 && ev.type == EV_KEY) {
			if (caps == 1) {caps=0;} else {caps=1;}}

		if (shift == 0 && caps == 1) {shift=1;}

		if  ((ev.value == 1 || ev.value == 2) && ev.type == EV_KEY) {
//			printf("%d\n",code);
			buf=URCTranslate(code,shift);
			if (strlen(buf) > 0)
				write(sock,buf,strlen(buf));
		}		
		}
	
#ifdef FB8BIT
		x=read(rcfd, &rccode, 2);
		if (x > 0) {
			if(rccode == 0x5C0C)	{
				exit(0);/*Notausstieg*/}
		}
#else
		x = read( rcfd, &ev, sizeof(struct input_event) );
		if (x > 0) {
			memcpy(&code,&ev.code,sizeof(ev.code));
			if ((code == KEY_EXIT /*RC EXIT*/) && ev.value == 1 && ev.type == EV_KEY) {
				exit(0);/*Notausstieg*/}
		}
#endif

		if (shift == 1 && caps == 1) {shift=0;}

		ev.value=-1;

//**********************************
	
		}
	}
	close(sock);
	fclose (fp);
}

unsigned char* RCTranslate(unsigned short code, int shift) {

if (shift == 0) { //NORMAL
	switch (code) {
		case KEY_0:	return "0";
		case KEY_1:	return "1";
		case KEY_2:	return "2";
		case KEY_3:	return "3";
		case KEY_4:	return "4";
		case KEY_5:	return "5";
		case KEY_6:	return "6";
		case KEY_7:	return "7";
		case KEY_8:	return "8";
		case KEY_9:	return "9";
		case KEY_A:	return "a";
		case KEY_B:	return "b";
		case KEY_C:	return "c";
		case KEY_D:	return "d";
		case KEY_E:	return "e";
		case KEY_F:	return "f";
		case KEY_G:	return "g";
		case KEY_H:	return "h";
		case KEY_I:	return "i";
		case KEY_J:	return "j";
		case KEY_K:	return "k";
		case KEY_L:	return "l";
		case KEY_M:	return "m";
		case KEY_N:	return "n";
		case KEY_O:	return "o";
		case KEY_P:	return "p";
		case KEY_Q:	return "q";
		case KEY_R:	return "r";
		case KEY_S:	return "s";
		case KEY_T:	return "t";
		case KEY_U:	return "u";
		case KEY_V:	return "v";
		case KEY_W:	return "w";
		case KEY_X:	return "x";
		case KEY_Y:	return "y";
		case KEY_Z:	return "z";
		case KEY_APOSTROPHE:	return "'";
		case KEY_SEMICOLON:	return ";";
		case KEY_LEFTBRACE:	return "[";

		case KEY_MINUS:		return "-";
		case KEY_EQUAL:		return "=";
		case KEY_BACKSPACE:	return "\x7F";

		case KEY_TAB:		return "\x09";
		case KEY_RIGHTBRACE:	return "]";
		case KEY_BACKSLASH:		return "\\";

		case KEY_ENTER:		return "\x0D\x0A";

		case KEY_COMMA:		return ",";
		case KEY_DOT:		return ".";
		case KEY_SLASH:		return "/";

		case KEY_GRAVE:		return "`";

		case KEY_LEFT:		return "\x1BO\x44";
		case KEY_RIGHT:		return "\x1BO\x43";
		case KEY_UP:		return "\x1BO\x41";
		case KEY_DOWN:		return "\x1BO\x42";

		case KEY_F1:		return "^";

		case KEY_F2:		return "\x1BOP";
		case KEY_F3:		return "\x1BOQ";
		case KEY_BACK:		return "\x1BOR";
		case KEY_FORWARD:	return "\x1BOS";
		case KEY_F6:		return "\x1B[15~";
		case KEY_F7:		return "\x1B[17~";
		case KEY_F8:		return "\x1B[18~";
		case KEY_RECORD:	return "\x1B[19~";
		case KEY_STOP:		return "\x1B[20~";
		case KEY_PAUSE:		return "\x1B[21~";
		case KEY_PREVIOUSSONG:	return "\x1B[23~";
		case KEY_REWIND:	return "\x1B[24~";

		case KEY_SPACE:		return " ";
		case KEY_DELETE:	return "\x7F";
		case KEY_ESC:		return "\x1B";

		case KEY_INSERT:	return "\x1B[2~";
			
		case KEY_F11:		return "root\x0D\x0A";
		case KEY_F10:		return "dreambox\x0D\x0A";

		default:		return "";
	}
}

if (shift == 1) { //SHIFT
	switch (code) {
		case KEY_0:	return ")";
		case KEY_1:	return "!";
		case KEY_2:	return "@";
		case KEY_3:	return "#";
		case KEY_4:	return "$";
		case KEY_5:	return "%";
		case KEY_6:	return "^";
		case KEY_7:	return "&";
		case KEY_8:	return "*";
		case KEY_9:	return "(";
		case KEY_A:	return "A";
		case KEY_B:	return "B";
		case KEY_C:	return "C";
		case KEY_D:	return "D";
		case KEY_E:	return "E";
		case KEY_F:	return "F";
		case KEY_G:	return "G";
		case KEY_H:	return "H";
		case KEY_I:	return "I";
		case KEY_J:	return "J";
		case KEY_K:	return "K";
		case KEY_L:	return "L";
		case KEY_M:	return "M";
		case KEY_N:	return "N";
		case KEY_O:	return "O";
		case KEY_P:	return "P";
		case KEY_Q:	return "Q";
		case KEY_R:	return "R";
		case KEY_S:	return "S";
		case KEY_T:	return "T";
		case KEY_U:	return "U";
		case KEY_V:	return "V";
		case KEY_W:	return "W";
		case KEY_X:	return "X";
		case KEY_Y:	return "Y";
		case KEY_Z:	return "Z";
		case KEY_APOSTROPHE:	return "\"";
		case KEY_SEMICOLON:	return ":";
		case KEY_LEFTBRACE:	return "{";

		case KEY_MINUS:		return "_";
		case KEY_EQUAL:		return "+";
		case KEY_BACKSPACE:	return "\x7F";

		case KEY_TAB:		return "\x09";
		case KEY_RIGHTBRACE:	return "}";
		case KEY_BACKSLASH:	return "|";

		case KEY_ENTER:	return "\x0D\x0A";

		case KEY_COMMA:		return "<";
		case KEY_DOT:		return ">";
		case KEY_SLASH:		return "?";

		case KEY_GRAVE:		return "~";

		case KEY_LEFT:		return "\x1BO\x44";
		case KEY_RIGHT:		return "\x1BO\x43";
		case KEY_UP:		return "\x1BO\x41";
		case KEY_DOWN:		return "\x1BO\x42";

		case KEY_F1:		return "°";

		case KEY_F2:		return "\x1BOP";
		case KEY_F3:		return "\x1BOQ";
		case KEY_BACK:		return "\x1BOR";
		case KEY_FORWARD:	return "\x1BOS";
		case KEY_F6:		return "\x1B[15~";
		case KEY_F7:		return "\x1B[17~";
		case KEY_F8:		return "\x1B[18~";
		case KEY_RECORD:	return "\x1B[19~";
		case KEY_STOP:		return "\x1B[20~";
		case KEY_PAUSE:		return "\x1B[21~";
		case KEY_PREVIOUSSONG:	return "\x1B[23~";
		case KEY_REWIND:	return "\x1B[24~";

		case KEY_SPACE:		return " ";
		case KEY_DELETE:	return "\x7F";
		case KEY_ESC:		return "\x1B";

		default:	return "";
	}
}

if (shift == 2) { //CTRL
	switch (code) {
		case KEY_2:	return "\x00";
		case KEY_SPACE:	return "\x00";
		case KEY_A:	return "\x01";
		case KEY_B:	return "\x02";
		case KEY_C:	return "\x03";
		case KEY_D:	return "\x04";
		case KEY_E:	return "\x05";
		case KEY_F:	return "\x06";
		case KEY_G:	return "\x07";
		case KEY_H:	return "\x08";
		case KEY_I:	return "\x09";
		case KEY_J:	return "\x0A";
		case KEY_K:	return "\x0B";
		case KEY_L:	return "\x0C";
		case KEY_M:	return "\x0D";
		case KEY_N:	return "\x0E";
		case KEY_O:	return "\x0F";
		case KEY_P:	return "\x10";
		case KEY_Q:	return "\x11";
		case KEY_R:	return "\x12";
		case KEY_S:	return "\x13";
		case KEY_T:	return "\x14";
		case KEY_U:	return "\x15";
		case KEY_V:	return "\x16";
		case KEY_W:	return "\x17";
		case KEY_X:	return "\x18";
		case KEY_Y:	return "\x19";
		case KEY_Z:	return "\x1A";
		case KEY_3:	return "\x1B";
		case KEY_4:	return "\x1C";
		case KEY_5:	return "\x1D";
		case KEY_6:	return "\x1E";
		case KEY_7:	return "\x1F";
		case KEY_8:	return "\x7F";

		default:	return "";
	}
}

if (shift == 3) { //ALT GR
	switch (code) {
		case KEY_MINUS:		return "\\";
		case KEY_RIGHTBRACE:	return "~";
		case KEY_GRAVE:		return "|";
		case KEY_Q:		return "@";
		case KEY_7:		return "{";
		case KEY_8:		return "[";
		case KEY_9:		return "]";
		case KEY_0:		return "}";

		default:	return "";
	}
}

}

int g1conversion (int ch) {

switch (ch) {
	case 0x6A:	return 0x2518;
	case 0x6B:	return 0x2510;
	case 0x6C:	return 0x250C;
	case 0x6D:	return 0x2514;
	case 0x6E:	return 0x253c;
	case 0x71:	return 0x2500;
	case 0x74:	return 0x251C;
	case 0x75:	return 0x2524;
	case 0x76:	return 0x2534;
	case 0x77:	return 0x252C;
	case 0x78:	return 0x2502;

	default:	return ch;
}

}

unsigned char* URCTranslate(unsigned short code, int shift) {

if (shift == 0) { //NORMAL
	switch (code) {
		case KEY_0:	return "0";
		case KEY_1:	return "1";
		case KEY_2:	return "2";
		case KEY_3:	return "3";
		case KEY_4:	return "4";
		case KEY_5:	return "5";
		case KEY_6:	return "6";
		case KEY_7:	return "7";
		case KEY_8:	return "8";
		case KEY_9:	return "9";
		case KEY_A:	return "a";
		case KEY_B:	return "b";
		case KEY_C:	return "c";
		case KEY_D:	return "d";
		case KEY_E:	return "e";
		case KEY_F:	return "f";
		case KEY_G:	return "g";
		case KEY_H:	return "h";
		case KEY_I:	return "i";
		case KEY_J:	return "j";
		case KEY_K:	return "k";
		case KEY_L:	return "l";
		case KEY_M:	return "m";
		case KEY_N:	return "n";
		case KEY_O:	return "o";
		case KEY_P:	return "p";
		case KEY_Q:	return "q";
		case KEY_R:	return "r";
		case KEY_S:	return "s";
		case KEY_T:	return "t";
		case KEY_U:	return "u";
		case KEY_V:	return "v";
		case KEY_W:	return "w";
		case KEY_X:	return "x";
		case KEY_Y:	return "y";
		case KEY_Z:	return "z";
		case KEY_APOSTROPHE:	return "'";
		case KEY_SEMICOLON:	return ";";
		case KEY_LEFTBRACE:	return "[";

		case KEY_MINUS:		return "-";
		case KEY_EQUAL:		return "=";
		case KEY_BACKSPACE:	return "\x7F";

		case KEY_TAB:		return "\x09";
		case KEY_RIGHTBRACE:	return "]";
		case KEY_BACKSLASH:		return "\\";

		case KEY_ENTER:		return "\x0D\x0A";

		case KEY_COMMA:		return ",";
		case KEY_DOT:		return ".";
		case KEY_SLASH:		return "/";

		case KEY_GRAVE:		return "`";

		case KEY_LEFT:		return "\x1BO\x44";
		case KEY_RIGHT:		return "\x1BO\x43";
		case KEY_UP:		return "\x1BO\x41";
		case KEY_DOWN:		return "\x1BO\x42";

		case KEY_102ND:		return "<";

		case KEY_F1:		return "\x1BOP";
		case KEY_F2:		return "\x1BOQ";
		case KEY_F3:		return "\x1BOR";
		case KEY_F4:	return "\x1BOS";
		case KEY_F5:		return "\x1B[15~";
		case KEY_F6:		return "\x1B[17~";
		case KEY_F7:		return "\x1B[18~";
		case KEY_F8:	return "\x1B[19~";
		case KEY_F9:		return "\x1B[20~";
		case KEY_F10:		return "\x1B[21~";
		case KEY_F11:	return "\x1B[23~";
		case KEY_F12:	return "\x1B[24~";

		case KEY_SPACE:		return " ";
		case KEY_DELETE:	return "\x7F";
		case KEY_ESC:		return "\x1B";

		case KEY_INSERT:	return "\x1B[2~";
			
		default:		return "";
	}
}

if (shift == 1) { //SHIFT
	switch (code) {
		case KEY_0:	return ")";
		case KEY_1:	return "!";
		case KEY_2:	return "@";
		case KEY_3:	return "#";
		case KEY_4:	return "$";
		case KEY_5:	return "%";
		case KEY_6:	return "^";
		case KEY_7:	return "&";
		case KEY_8:	return "*";
		case KEY_9:	return "(";
		case KEY_A:	return "A";
		case KEY_B:	return "B";
		case KEY_C:	return "C";
		case KEY_D:	return "D";
		case KEY_E:	return "E";
		case KEY_F:	return "F";
		case KEY_G:	return "G";
		case KEY_H:	return "H";
		case KEY_I:	return "I";
		case KEY_J:	return "J";
		case KEY_K:	return "K";
		case KEY_L:	return "L";
		case KEY_M:	return "M";
		case KEY_N:	return "N";
		case KEY_O:	return "O";
		case KEY_P:	return "P";
		case KEY_Q:	return "Q";
		case KEY_R:	return "R";
		case KEY_S:	return "S";
		case KEY_T:	return "T";
		case KEY_U:	return "U";
		case KEY_V:	return "V";
		case KEY_W:	return "W";
		case KEY_X:	return "X";
		case KEY_Y:	return "Y";
		case KEY_Z:	return "Z";
		case KEY_APOSTROPHE:	return "\"";
		case KEY_SEMICOLON:	return ":";
		case KEY_LEFTBRACE:	return "{";

		case KEY_MINUS:		return "_";
		case KEY_EQUAL:		return "+";
		case KEY_BACKSPACE:	return "\x7F";

		case KEY_TAB:		return "\x09";
		case KEY_RIGHTBRACE:	return "}";
		case KEY_BACKSLASH:	return "|";

		case KEY_ENTER:	return "\x0D\x0A";

		case KEY_COMMA:		return "<";
		case KEY_DOT:		return ">";
		case KEY_SLASH:		return "?";

		case KEY_GRAVE:		return "~";

		case KEY_LEFT:		return "\x1BO\x44";
		case KEY_RIGHT:		return "\x1BO\x43";
		case KEY_UP:		return "\x1BO\x41";
		case KEY_DOWN:		return "\x1BO\x42";

		case 86:		return ">";
			
		case KEY_F1:		return "\x1BOP";
		case KEY_F2:		return "\x1BOQ";
		case KEY_F3:		return "\x1BOR";
		case KEY_F4:	return "\x1BOS";
		case KEY_F5:		return "\x1B[15~";
		case KEY_F6:		return "\x1B[17~";
		case KEY_F7:		return "\x1B[18~";
		case KEY_F8:	return "\x1B[19~";
		case KEY_F9:		return "\x1B[20~";
		case KEY_F10:		return "\x1B[21~";
		case KEY_F11:	return "\x1B[23~";
		case KEY_F12:	return "\x1B[24~";

		case KEY_SPACE:		return " ";
		case KEY_DELETE:	return "\x7F";
		case KEY_ESC:		return "\x1B";

		default:	return "";
	}
}

if (shift == 2) { //CTRL
	switch (code) {
		case KEY_2:	return "\x00";
		case KEY_SPACE:	return "\x00";
		case KEY_A:	return "\x01";
		case KEY_B:	return "\x02";
		case KEY_C:	return "\x03";
		case KEY_D:	return "\x04";
		case KEY_E:	return "\x05";
		case KEY_F:	return "\x06";
		case KEY_G:	return "\x07";
		case KEY_H:	return "\x08";
		case KEY_I:	return "\x09";
		case KEY_J:	return "\x0A";
		case KEY_K:	return "\x0B";
		case KEY_L:	return "\x0C";
		case KEY_M:	return "\x0D";
		case KEY_N:	return "\x0E";
		case KEY_O:	return "\x0F";
		case KEY_P:	return "\x10";
		case KEY_Q:	return "\x11";
		case KEY_R:	return "\x12";
		case KEY_S:	return "\x13";
		case KEY_T:	return "\x14";
		case KEY_U:	return "\x15";
		case KEY_V:	return "\x16";
		case KEY_W:	return "\x17";
		case KEY_X:	return "\x18";
		case KEY_Y:	return "\x19";
		case KEY_Z:	return "\x1A";
		case KEY_3:	return "\x1B";
		case KEY_4:	return "\x1C";
		case KEY_5:	return "\x1D";
		case KEY_6:	return "\x1E";
		case KEY_7:	return "\x1F";
		case KEY_8:	return "\x7F";

		default:	return "";
	}
}

if (shift == 3) { //ALT GR
	switch (code) {
		case KEY_MINUS:		return "\\";
		case KEY_RIGHTBRACE:	return "~";
		case 86:		return "|";
		case KEY_Q:		return "@";
		case KEY_7:		return "{";
		case KEY_8:		return "[";
		case KEY_9:		return "]";
		case KEY_0:		return "}";

		default:	return "";
	}
}

}

