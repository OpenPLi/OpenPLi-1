/* 
AiO Dreambox Screengrabber v0.71

Written in 10/2006 by Seddi
added png support in 06/2007 by gutemine
added osd only support for 800 in 07/2008 by gutemine

Contact: seddi@ihad.tv / http://www.ihad.tv

This standalone binary will grab the video-picture convert it from
yuv to rgb and resize it, if neccesary, to the same size as the framebuffer.
It also grabs the framebuffer picture in 32Bit, 16Bit or in 8Bit mode with the 
correct colortable from the main graphics memory, because the FBIOGETCMAP is
buggy and didnt give you the correct color map.
Finally it will combine the pixmaps to one final picture by using the framebuffer
alphamap, so you will get the same picture as you can see on your TV Screen.

There are a few command line switches, use "grab -h" to get them listed.

The yuv2rgb and bmp header routines and are completly stolen from enigma, all the 
other routines have been written by myself with public known algorithms (like 
picture resizing, combining with alphamap, ...). A special Thanx to tmbinc for 
the needed information how to find the framebuffer color values.

The png output routine is stolen from fbshot.

Feel free to use the code for your own projects. The bicubic picture resize algorithm 
is able to enlarge and reduce the picture size. So maybe you will find it useful.
*/

#define VERSION "v0.71"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <linux/fb.h>
#include "png.h" 

#define VIDEO_DEV "/dev/video"

#define CLAMP(x)     ((x < 0) ? 0 : ((x > 255) ? 255 : x))

#define RED565(x)    ((((x) >> (11 )) & 0x1f) << 3)
#define GREEN565(x)  ((((x) >> (5 )) & 0x3f) << 2)
#define BLUE565(x)   ((((x) >> (0)) & 0x1f) << 3)

#define YFB(x)    ((((x) >> (10)) & 0x3f) << 2)
#define CBFB(x)  ((((x) >> (6)) & 0xf) << 4)
#define CRFB(x)   ((((x) >> (2)) & 0xf) << 4)
#define BFFB(x)   ((((x) >> (0)) & 0x3) << 6)

unsigned char* upcase(unsigned char* mixedstr) 
{
	int j;
	for (j=0; j< strlen(mixedstr); ++j)
	{
		mixedstr[j]=toupper(mixedstr[j]);
	} 
	return mixedstr;
}

inline unsigned short avg2(unsigned short a, unsigned short b)
{
	return
		(((a & 0xFF) + (b & 0xFF)) >> 1) |
		(((((a>>8) & 0xFF) + ((b>>8) & 0xFF)) >> 1) << 8);
}

struct blasel
{
	int hor, vert;
	char *name;
} subsamplings[]={
	{1, 1, "4:4:4"},
	{2, 1, "4:2:2"},
	{2, 2, "4:2:0"},
	{4, 2, "4:2:0-half"},
	{4, 1, "4:1:1"},
	{4, 4, "4:1:0"}};

int fd_video;
unsigned char frame[1920 * 1080 * 3 + 16]; // video yuv and tmp pixmap for resizing
unsigned char frame2[1920 * 1080 * 3]; // video rgb pixmap
unsigned char frame3[1920 * 1080 * 3]; // framebuffer rgb pixmap
unsigned char frame3t[1920 * 1080]; // framebuffer alpha pixmap
unsigned char frame4[1920 * 1080 * 3]; // destination rgb pixmap

int fb;
unsigned char *lfb;
struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;


int main(int argc, char **argv) {

       printf("AiO Dreambox Screengrabber "VERSION"\n\n");
	
       char filename[128];
       char *commandvalue = NULL;
       int command = 0;
       int index;
       int c,i,x,y;
       int interlace=PNG_INTERLACE_ADAM7;
       int gray=0;
       png_bytep *row_pointers;
       png_structp png_ptr;
       png_infop info_ptr;
       png_text txt_ptr[4];
       int bit_depth=0, color_type;   
       int osdflag = 0;
       int videoflag = 0;
       int bmpflag = 0;
       int pngflag = 0;
     
       opterr = 0;
     
       // enhanced processing of command line switches
       while ((c = getopt (argc, argv, "hvof:")) != -1)
         switch (c)
           {
           case 'f':
	     if (strcmp(optarg,"bmp")==0) {
                bmpflag = 1;
                }
	     if (strcmp(optarg,"png")==0) {
                pngflag = 1;
                }
             if ( (bmpflag + pngflag) < 1) {
             fprintf (stderr, "invalid format -f %s only bmp | png supported\n",optarg);
             exit(0);
             }
             break;
           case 'o':
             osdflag = 1;
             break;
           case 'v':
             videoflag = 1;
             break;
           case 'h':
	     printf("Usage: grab [command] [filename]\n\n");
	     printf("command:\n");
	     printf("-o only grab osd (framebuffer)\n");
	     printf("-v only grab video\n");
	     printf("-f bmp | png output format, bmp is default if not used\n");
	     printf("-h this help screen\n\n");
	     printf("If no command is given the complete picture will be grabbed.\n");
	     printf("If no filename is given the picture will be written to /tmp/screenshot.bmp\n");
	     printf("or if -f png option is used to /tmp/screenshot.png\n");

	     return;
             break;
           case '?':
             if (isprint (optopt))
               fprintf (stderr, "wrong option -%c\n try grab -h for help\n", optopt);
             else
               fprintf (stderr,
                        "unknown option character \\x%x\n try grab -h for help\n",
                        optopt);
             exit(0);
           default:
             exit(0);
           }
     
       // downward command option compatibility 
       if ( (osdflag + videoflag) < 1) {
            osdflag = 1;
            videoflag = 1;
       }
       if ( (pngflag + bmpflag) < 1) {
            bmpflag = 1;
       }
       if ((argc < 2) || (strncmp(argv[argc-1],"-",1)==0) || (strcmp(argv[argc-1],"bmp")==0) || (strcmp(argv[argc-1],"png")==0))
          {
          if ( pngflag == 1) {
	     sprintf(filename,"/tmp/screenshot.png");
             }
          else
             {
	     sprintf(filename,"/tmp/screenshot.bmp");
             }
          }
       else
          {
          sprintf(filename,"%s",argv[argc-1]);
          }
#ifdef DEBUG
       printf("odsflag: %i videoflag: %i bmpflag: %i pngflag: %i filename: %s\n",osdflag,videoflag,bmpflag,pngflag,filename);
#endif


	unsigned short rd[256];
	unsigned short gn[256];
	unsigned short bl[256];
	unsigned short tr[256];
	
	fb=open("/dev/fb/0", O_RDWR);
	if (fb == -1)
	{
		fb=open("/dev/fb0", O_RDWR);
		if (fb == -1)
		{
			printf("Framebuffer failed\n");
			return;
		}
	}

	if(ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
	{
		printf("Framebuffer: <FBIOGET_FSCREENINFO failed>\n");
		return;
	}

	if(ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("Framebuffer: <FBIOGET_VSCREENINFO failed>\n");
		return;
	}

	
	if(!(lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)))
	{
		printf("Framebuffer: <Memmapping failed>\n");
		return;
	}
	
	if ( var_screeninfo.bits_per_pixel == 8) {
		printf("Grabbing 8bit Framebuffer ...\n");
		
		// Read Color Palette directly from the main memory, because the FBIOGETCMAP is buggy on dream and didnt
		// gives you the correct colortable !
		int mem_fd;
		unsigned char *memory;
		if ((mem_fd = open("/dev/mem", O_RDWR) ) < 0) {
			printf("Mainmemory: can't open /dev/mem \n");
			return;
		}

		if(!(memory = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, fix_screeninfo.smem_start-0x1000)))
		{
			printf("Mainmemory: <Memmapping failed>\n");
			return;
		}
		int i2,vulcan;
		//unsigned char buf[256];
		i2=vulcan=0;
				
		// vulcan (DM500/56x0) or pallas (DM70x0) framebuffer ?
		/*FILE *pipe=popen("cat /proc/fb","r");
		while (fgets(buf,sizeof(buf),pipe))
		{
			if (strstr(buf,"vulcan")) {vulcan=1;}
		}
		pclose(pipe);*/
		
		if (strstr(upcase(fix_screeninfo.id),"VULCAN")) {vulcan=1;} // did this work ? not tested yet
		
		if (vulcan == 1) // DM500/5620 stores the colors as a 16bit word with yuv values, so we have to convert :(
		{
			unsigned short yuv;
			for (i=16; i<(256*2)+16; i+=2)
			{
				yuv=memory[i]<<8;
				yuv|=memory[i+1];
			
				rd[i2]=CLAMP((76310*(YFB(yuv)-16) + 104635*(CRFB(yuv)-128))>>16);
				gn[i2]=CLAMP((76310*(YFB(yuv)-16) - 53294*(CRFB(yuv)-128) - 25690*(CBFB(yuv)-128))>>16);
				bl[i2]=CLAMP((76310*(YFB(yuv)-16) + 132278*(CBFB(yuv)-128))>>16);
			
				if (yuv == 0) // transparency is a bit tricky, there is a 2 bit blending value BFFB(yuv), but not really used
				{
					rd[i2]=gn[i2]=bl[i2]=0;
					tr[i2]=0xFF;
				} else
				{
					tr[i2]=0;
				}
				i2++;
			}
		} else // DM70x0 stores the colors in plain rgb values
		{
			for (i=32; i<(256*4)+32; i+=4)
			{
				rd[i2]=memory[i+1];
				gn[i2]=memory[i+2];
				bl[i2]=memory[i+3];
				tr[i2]=0xFF-memory[i];
				i2++;
			}
		}
		
		close(mem_fd);
		
		// Get OSD
		unsigned short color;
	
		for (y=0; y < var_screeninfo.yres; y+=1)
		{
			for (x=0; x < var_screeninfo.xres; x+=1)
			{
			   color=lfb[x+(y*fix_screeninfo.line_length)];
                           if (bmpflag) {
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+0]=bl[color];//b
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+1]=gn[color];//g
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+2]=rd[color];//r
                              }
                           else
                              {
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+0]=rd[color];//r
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+1]=gn[color];//g
			      frame3[(x*3)+(y*var_screeninfo.xres*3)+2]=bl[color];//b
                              }
			   frame3t[x+(y*var_screeninfo.xres)]=0xFF-(tr[color]);//tr
			}
		
		}
	} else if ( var_screeninfo.bits_per_pixel == 16) {
		printf("Grabbing 16bit Framebuffer ...\n");
		unsigned short color;
	
		// Get OSD
		for (y=0; y < var_screeninfo.yres; y+=1)
		{
			int x2=0;
			for (x=0; x < var_screeninfo.xres*2; x+=2)
			{
			   color=lfb[x+(y*fix_screeninfo.line_length)]<<8;
			   color|=lfb[x+(y*fix_screeninfo.line_length)+1];
                           if (bmpflag) {
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+0]=BLUE565(color);//b
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+1]=GREEN565(color);//g
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+2]=RED565(color);//r
                           }
                           else
                           {
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+0]=RED565(color);//r
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+1]=GREEN565(color);//g
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+2]=BLUE565(color);//b
                           }
			   frame3t[x2+(y*var_screeninfo.xres)]=0xFF;//tr - there is no transparency in 16bit mode
			   x2++;
			}
		} 
	} else if ( var_screeninfo.bits_per_pixel == 32) {
		printf("Grabbing 32bit Framebuffer ...\n");
		unsigned short color;
	
		// Get OSD
		for (y=0; y < var_screeninfo.yres; y+=1)
		{
			int x2=0;
			for (x=0; x < var_screeninfo.xres*4; x+=4)
			{
                           if (bmpflag) {
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+0]=lfb[x+(y*fix_screeninfo.line_length)];//b
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+1]=lfb[x+(y*fix_screeninfo.line_length)+1];//g
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+2]=lfb[x+(y*fix_screeninfo.line_length)+2];//r
                           }
                           else
                           {
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+0]=lfb[x+(y*fix_screeninfo.line_length)+2];//r
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+1]=lfb[x+(y*fix_screeninfo.line_length)+1];//g
			      frame3[(x2*3)+(y*var_screeninfo.xres*3)+2]=lfb[x+(y*fix_screeninfo.line_length)];//b
                           }
		   	   frame3t[x2+(y*var_screeninfo.xres)]=lfb[x+(y*fix_screeninfo.line_length)+3];//tr
			   x2++;
			}
		} 
	} 
	close(fb);
	printf("... Framebuffer-Size: %d x %d\n",var_screeninfo.xres,var_screeninfo.yres);

	
	int luma_x,luma_y,t,t1,stride,stride_v,ratio;

//	printf("... ID: %s\n",fix_screeninfo.id);

        if (strstr(upcase(fix_screeninfo.id),"BCM"))
           {
	   printf("... Video-grab: on DM 800 HD not yet supported, forcing -o\n");
           osdflag = 1;
           videoflag = 0;
           }
	if ((strstr(upcase(fix_screeninfo.id),"XILLEON")) || 
	    (strstr(upcase(fix_screeninfo.id),"BCM")))
	{
		// Get Video (xilleon box directly via mpeg decoder)	
		
		unsigned char frame_l[1920 * 576]; // luma frame from video decoder
		unsigned char frame_c[1920 * 288]; // chroma frame from video decoder
		unsigned char luma[1920 * 1080]; // "encoded" or lets say "sorted" luma frame
		unsigned char chroma[1920 * 1080]; // "encoded" or lets say "sorted" chroma frame
		
		int mem_fd;
		unsigned char *memory;
		if ((mem_fd = open("/dev/mem", O_RDONLY) ) < 0) {
			printf("Mainmemory: can't open /dev/mem \n");
			return;
		}

		if(!(memory = (unsigned char*)mmap(0, 1920*1152*6, PROT_READ, MAP_SHARED, mem_fd, 0x6000000)))
		{
			printf("Mainmemory: <Memmapping failed>\n");
			return;
		}

		int offset=1920*1152*5;	// offset for chroma buffer
		
		// grab luma buffer from decoder memory	
		memcpy(frame_l,memory,1920*576); 
		// grab chroma buffer from decoder memory
		memcpy(frame_c,memory+offset,1920*288);

		munmap(memory, 1920*1152*6);
		close(mem_fd);
		
		unsigned char buf[256];
		FILE *pipe=popen("cat /proc/stb/vmpeg/0/xres","r");
		while (fgets(buf,sizeof(buf),pipe))
		{
			sscanf(buf,"%x",&luma_x); 
		}
		pclose(pipe);
		pipe=popen("cat /proc/stb/vmpeg/0/yres","r");
		while (fgets(buf,sizeof(buf),pipe))
		{
			sscanf(buf,"%x",&luma_y); 
		}
		pclose(pipe);
		
		// 16:9 or 4:3
		ratio=0;
		pipe=popen("cat /proc/stb/vmpeg/0/aspect","r");
		while (fgets(buf,sizeof(buf),pipe))
		{
			sscanf(buf,"%x",&ratio); 
		}
		pclose(pipe);
		
		printf("... Video-Size: %d x %d / ",luma_x,luma_y);
		
		if (ratio == 3)
			printf("Aspect-Ratio: 16:9\n");
		else
			printf("Aspect-Ratio: 4:3\n");
		
		stride=var_screeninfo.xres*3;
		stride_v=luma_x*3;
		
		int xtmp,ytmp,t,odd_even,oe2,ysub,xsub;
		int ypart=32;
		int xpart=128;
		t=odd_even=oe2=0;

		// "decode" luma, there are 128x32pixel blocks inside the decoder mem
		for (ysub=0; ysub<18; ysub++) // 1152/32=36 we only need 576 yres, so 18 will be enough
		{
			for (xsub=0; xsub<15; xsub++) // 1920/128=15
			{
				for (ytmp=0; ytmp<ypart; ytmp++)
				{
					for (xtmp=0; xtmp< xpart; xtmp++)
					{
						if (odd_even == 0)
							oe2=0;
						if (odd_even == 1 && xtmp < 64)
							oe2=64;
						if (odd_even == 1 && xtmp >= 64)
							oe2=-64;
						if (xsub*xpart+xtmp+oe2 < luma_x) // we only need 720 xres
							memcpy(luma+((xsub*xpart+oe2))+xtmp+(luma_x*(ytmp+(ysub*ypart))),frame_l+t,1);
						t++;
					}
				}
			}
			odd_even^=1;
		}

		
		
		t=odd_even=oe2=0;
		
		// "decode" chroma, there are 128x32pixel blocks inside the decoder mem
		for (ysub=0; ysub<9; ysub++) // 1152/32=36 we only need 288 yres for chroma (4:2:0 subsampling), so 9 will be enough
		{
			for (xsub=0; xsub<15; xsub++) // 1920/128=15
			{
				for (ytmp=0; ytmp<ypart; ytmp++)
				{
					for (xtmp=0; xtmp< xpart; xtmp++)
					{
						if (odd_even == 0)
							oe2=0;
						if (odd_even == 1 && xtmp < 64)
							oe2=64;
						if (odd_even == 1 && xtmp >= 64)
							oe2=-64;
						if (xsub*xpart+xtmp+oe2 < luma_x) // we only need 720 xres
							memcpy(chroma+((xsub*xpart+oe2))+xtmp+(luma_x*(ytmp+(ysub*ypart))),frame_c+t,1);
						t++;
					}
				}
			}
			odd_even^=1;
		}
		

		int Y, U, V, set;
		set=t=0;
			
		// yuv2rgb conversion (4:2:0)
		for (y=0; y < luma_y; y+=1)
		{
			t=y>>1;
			for (x=0; x < luma_x; x+=1)
			{
			    if (set == 0) 
			    {
			       U=chroma[x+(t*luma_x)]-128;
			       V=chroma[x+(t*luma_x)+1]-128;
			    }
			    set^=1;

			    Y=76310*(luma[x+(luma_x*y)]-16);

                            if (bmpflag){   
			       frame2[x*3+(y*luma_x*3)]=CLAMP((Y + 104635*V)>>16);
			       frame2[x*3+(y*luma_x*3)+1]=CLAMP((Y - 53294*V - 25690*U)>>16);
			       frame2[x*3+(y*luma_x*3)+2]=CLAMP((Y + 132278*U)>>16);
			       }
                            else
                               {
			       frame2[x*3+(y*luma_x*3)]=CLAMP((Y + 132278*U)>>16);
			       frame2[x*3+(y*luma_x*3)+1]=CLAMP((Y - 53294*V - 25690*U)>>16);
			       frame2[x*3+(y*luma_x*3)+2]=CLAMP((Y + 104635*V)>>16);
			       }
			}
		}		
		
		
	} else
	{
		// Get Video (ppc boxes via /dev/video)
		int r;
		
		printf("Grabbing Video ...\n");

		fd_video = open(VIDEO_DEV, O_RDONLY);
		if (fd_video < 0)
		{
			printf("could not open /dev/video");
			return;
		}	  
		  
		// grab picture from videodevice
		r = read(fd_video, frame, 720 * 576 * 3 + 16);
		if (r < 16)
		{
			fprintf(stderr, "read failed\n");
			close(fd_video);
			return;
		}
		close(fd_video);
		
		int *size = (int*)frame;
		luma_x = size[0], luma_y = size[1];
		int chroma_x = size[2], chroma_y = size[3];
		
		int Y, U, V, set, x1 ,y1;
		set=t=x1=y1=0;
		stride=var_screeninfo.xres*3;
		stride_v=luma_x*3;
		
		// 16:9 or 4:3
		ratio=0;
		unsigned char buf[256];
		FILE *pipe=popen("cat /proc/bus/bitstream","r");
		while (fgets(buf,sizeof(buf),pipe))
		{
			sscanf(buf,"A_RATIO: %d",&ratio); 
		}
		pclose(pipe);
		
		printf("... Video-Size: %d x %d / ",luma_x,luma_y);
		
		if (ratio == 3)
			printf("Aspect-Ratio: 16:9\n");
		else
			printf("Aspect-Ratio: 4:3\n");
		
		printf("Converting Video from YUV to RGB color space ...\n");
			
		unsigned char *luma = frame + 16;
		unsigned short *chroma = (unsigned short*)(frame + 16 + luma_x * luma_y);
		
			
		int sub[2] = {luma_x / chroma_x, luma_y / chroma_y};
		
		// get subsampling
		int ssid;
		char *d = "unknown";
		for (ssid = 0; ssid < (int)(sizeof(subsamplings)/sizeof(*subsamplings)); ++ssid) 
		{
			if ((subsamplings[ssid].hor == sub[0]) && (subsamplings[ssid].vert == sub[1]))
			{
				d = subsamplings[ssid].name;
				break;
			}
		}	
		

		// yuv to rgb conversion
		y1=0;t1=1;
		for (y=0; y < luma_y; y++)
		{
			unsigned char line[luma_x * 3];
			for (x = 0; x < luma_x; ++x)
			{
				int l = luma[y * luma_x + x];
				int c = 0x8080;
				switch (ssid)
				{
				case 0: // 4:4:4
					c = chroma[y * chroma_x + x];
					break;
				case 1: // 4:2:2
					if (!(x & 1))
						c = chroma[y * chroma_x + (x >> 1)];
					else
						c = avg2(chroma[y * chroma_x + (x >> 1)], chroma[y * chroma_x + (x >> 1) + 1]);
					break;
				case 2: // 4:2:0
					if (!((x|y) & 1))
						c = chroma[(y >> 1) * chroma_x + (x >> 1)];
					else if (!(y & 1))
						c = avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[(y >> 1) * chroma_x + (x >> 1) + 1]);
					else if (!(x & 1))
						c = avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[((y >> 1) + 1) * chroma_x + (x >> 1)]);
					else
						c = avg2(
							avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[(y >> 1) * chroma_x + (x >> 1) + 1]),
							avg2(chroma[((y >> 1) + 1) * chroma_x + (x >> 1)], chroma[((y >> 1) + 1) * chroma_x + (x >> 1) + 1]));
					break;
				case 3:	// 4:2:0-half
					if (!(((x >> 1)|y) & 1))
						c = chroma[(y >> 1) * chroma_x + (x >> 2)];
					else if (!(y & 1))
						c = avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[(y >> 1) * chroma_x + (x >> 2) + 1]);
					else if (!(x & 2))
						c = avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[((y >> 1) + 1) * chroma_x + (x >> 2)]);
					else
						c = avg2(
							avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[(y >> 1) * chroma_x + (x >> 2) + 1]),
							avg2(chroma[((y >> 1) + 1) * chroma_x + (x >> 2)], chroma[((y >> 1) + 1) * chroma_x + (x >> 2) + 1]));
					break;
				case 4:	// 4:1:1
					if (!((x >> 1) & 1))
						c = chroma[y * chroma_x + (x >> 2)];
					else
						c = avg2(chroma[y * chroma_x + (x >> 2)], chroma[y * chroma_x + (x >> 2) + 1]);
					break;
				case 5:
					if (!((x >> 1) & 1))
						c = chroma[(y >> 2) * chroma_x + (x >> 2)];
					else
						c = avg2(chroma[(y >> 2) * chroma_x + (x >> 2)], chroma[(y >> 2) * chroma_x + (x >> 2) + 1]);
					break;
				}
				
				signed char cr = (c & 0xFF) - 128;
				signed char cb = (c >> 8) - 128;

				l -= 16;
				
				int r, g, b;
				
				r = 104635 * cr + l * 76310;
				g = -25690 * cb - 53294 * cr + l * 76310;
				b = 132278 * cb + l * 76310;
			    	
                                if (bmpflag){
				   line[x * 3 + 2] = CLAMP(r >> 16);
				   line[x * 3 + 1] = CLAMP(g >> 16);
				   line[x * 3 + 0] = CLAMP(b >> 16);
                                }
                                else
                                {
				   line[x * 3 + 2] = CLAMP(b >> 16);
				   line[x * 3 + 1] = CLAMP(g >> 16);
				   line[x * 3 + 0] = CLAMP(r >> 16);
                                }
			}
			memcpy(frame2+(y1*stride_v),line,stride_v);
			y1++;
			
		}
	}
	
	// Resizing Video ?
	if (luma_x != var_screeninfo.xres || luma_y != var_screeninfo.yres || ratio == 3)
	{
		printf("Resizing Video to %d x %d ",var_screeninfo.xres,var_screeninfo.yres);	
		if (ratio == 3)
			printf("and correct aspect ratio (letterbox) ...\n");
		else
			printf("...\n");
		
		memcpy(frame,frame2,stride_v*luma_y);
		
		float fx,fy,tmp_f/*,dpixel*/;
		unsigned int xs,ys,xd,yd,dpixel;
		unsigned int c,tmp_i;
		
		xs=luma_x; // x-resolution source
		ys=luma_y; // y-resolution source
		xd=var_screeninfo.xres; // x-resolution destination
		yd=var_screeninfo.yres; // y-resolution destination
		
		// correct aspect ratio 16:9 ?
		if (ratio == 3)
			yd/=1.33;
		
		// get x scale factor
		fx=(float)(xs-1)/(float)xd;

		// get y scale factor
		fy=(float)(ys-1)/(float)yd;

		unsigned int sx1[xd],sx2[xd],sy1,sy2;
		
		// pre calculating sx1/sx2 for faster resizing
		for (x=0; x<xd; x++) 
		{
			// first x source pixel for calculating destination pixel
			tmp_f=fx*(float)x;
			sx1[x]=(int)tmp_f; //floor()

			// last x source pixel for calculating destination pixel
			tmp_f=(float)sx1[x]+fx;
			sx2[x]=(int)tmp_f;
			if ((float)sx2[x] < tmp_f) {sx2[x]+=1;} //ceil()		
		}
		
		// Scale
		for (y=0; y<yd; y++) 
		{

			// first y source pixel for calculating destination pixel
			tmp_f=fy*(float)y;
			sy1=(int)tmp_f; //floor()

			// last y source pixel for calculating destination pixel
			tmp_f=(float)sy1+fy;
			sy2=(int)tmp_f;
			if ((float)sy2 < tmp_f) {sy2+=1;} //ceil()	
	
			for (x=0; x<xd; x++) 
			{

				/* we are using the precalculated sx1/sx2 for faster resizing
				
				// first x source pixel for calculating destination pixel
				tmp_f=fx*(float)x;
				sx1=(int)tmp_f; //floor()

				// last x source pixel for calculating destination pixel
				tmp_f=(float)sx1+fx;
				sx2=(int)tmp_f;
				if ((float)sx2 < tmp_f) {sx2+=1;} //ceil()
				*/
	
				// we have 3 colors, so we do this for every color
				for (c=0; c<3; c++) 
				{
					// calculationg destination pixel
					tmp_i=0;
					dpixel=0;
			
					for (t1=sy1; t1<sy2; t1++) 
					{
						for (t=sx1[x]; t<=sx2[x]; t++) 
						{
							tmp_i+=(int)frame[(t*3)+c+(t1*stride_v)];
							dpixel++;		
						}
					}
			
					//tmp_f=(float)tmp_i/dpixel;
					//tmp_i=(int)tmp_f;
					//if ((float)tmp_i+0.5 <= tmp_f) {tmp_i+=1;} //round()
					tmp_i=tmp_i/dpixel; // working with integers is not correct, but much faster and +-1 inside the color values doesnt really matter
					
					// writing calculated pixel into destination pixmap
					frame2[(x*3)+c+(y*stride)]=tmp_i;
				}
			}
		}
	
		// correcting aspect ratio 16:9 ? 
		if (ratio == 3)
		{
			memcpy(frame,frame2,stride*yd);
			memset(frame2,0,stride*var_screeninfo.yres);
			memcpy(frame2+((var_screeninfo.yres-yd)>>1)*stride,frame,stride*yd);
		}
	}

	printf("Merge Video with Framebuffer ...\n");
	
	// Combine video and framebuffer pixmap
	for (y=0; y < var_screeninfo.yres; y+=1)
	{
		for (x=0; x < var_screeninfo.xres; x+=1)
		{
			frame4[(x*3)+(y*var_screeninfo.xres*3)] =  ( ( frame2[(x*3)+(y*var_screeninfo.xres*3)] * ( 0xFF-frame3t[x+(y*var_screeninfo.xres)] ) ) + ( frame3[(x*3)+(y*var_screeninfo.xres*3)] * frame3t[x+(y*var_screeninfo.xres)] ) ) >>8;
			frame4[(x*3)+(y*var_screeninfo.xres*3)+1] =  ( ( frame2[(x*3)+(y*var_screeninfo.xres*3)+1] * ( 0xFF-frame3t[x+(y*var_screeninfo.xres)] ) ) + ( frame3[(x*3)+(y*var_screeninfo.xres*3)+1] * frame3t[x+(y*var_screeninfo.xres)] ) ) >>8;
			frame4[(x*3)+(y*var_screeninfo.xres*3)+2] =  ( ( frame2[(x*3)+(y*var_screeninfo.xres*3)+2] * ( 0xFF-frame3t[x+(y*var_screeninfo.xres)] ) ) + ( frame3[(x*3)+(y*var_screeninfo.xres*3)+2] * frame3t[x+(y*var_screeninfo.xres)] ) ) >>8;
		}
		
	}
		

        if (osdflag && !videoflag)
	   {
	   memcpy(frame4,frame3,stride*var_screeninfo.yres);
   	   printf("Only save Framebuffer (-o was given) ...\n");
	   }
	if (videoflag && !osdflag)
	   {
	   memcpy(frame4,frame2,stride*var_screeninfo.yres);
	   printf("Only save Video (-v was given) ...\n");
	   }
	
	// saving picture
     if (pngflag) {
        printf("Saving %s as png ...\n",filename);


/* static int Write_PNG(struct picture * pict, char *filename){ */
  FILE *fd2 = fopen(filename, "wb");
  
  png_ptr = png_create_write_struct(
        	PNG_LIBPNG_VER_STRING, 
        	(png_voidp)NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
        
  if (!png_ptr){
    printf("couldn't create PNG write struct.");
    exit(0);
    }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr){
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    printf("couldn't create PNG info struct.");
    exit(0);
  }

  txt_ptr[0].key="Name";
  txt_ptr[0].text="AiO Dreambox Screengrabber";
  txt_ptr[0].compression=PNG_TEXT_COMPRESSION_NONE;
  txt_ptr[1].key="Date";
  txt_ptr[1].text="Current Date";
  txt_ptr[1].compression=PNG_TEXT_COMPRESSION_NONE;
  txt_ptr[2].key="Hostname";
  txt_ptr[2].text="dm7025";
  txt_ptr[2].compression=PNG_TEXT_COMPRESSION_NONE;
  txt_ptr[3].key="Program";
  txt_ptr[3].text="AiO "VERSION;
  txt_ptr[3].compression=PNG_TEXT_COMPRESSION_NONE;

  png_set_text(png_ptr, info_ptr, txt_ptr, 4);

  png_init_io(png_ptr, fd2);
    
  png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

  row_pointers=(png_bytep*)malloc(sizeof(png_bytep)*var_screeninfo.yres);

  bit_depth=8;
  color_type=PNG_COLOR_TYPE_RGB;
  for (i=0; i<(var_screeninfo.yres); i++)
    row_pointers[i]=frame4+i*3*(var_screeninfo.xres);
  png_set_invert_alpha(png_ptr);
  png_set_IHDR(png_ptr, info_ptr, var_screeninfo.xres, var_screeninfo.yres, 
    bit_depth, color_type, interlace, 
    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(row_pointers);
  fclose(fd2);
  }
  else
  {
  printf("Saving %s as bmp ...\n",filename);

  FILE *fd2 = fopen(filename, "wr");
	
  // writing bmp header
  unsigned char hdr[14 + 40];
  i = 0;
#define PUT32(x) hdr[i++] = ((x)&0xFF); hdr[i++] = (((x)>>8)&0xFF); hdr[i++] = (((x)>>16)&0xFF); hdr[i++] = (((x)>>24)&0xFF);
#define PUT16(x) hdr[i++] = ((x)&0xFF); hdr[i++] = (((x)>>8)&0xFF);
#define PUT8(x) hdr[i++] = ((x)&0xFF);
  PUT8('B'); PUT8('M');
  PUT32((((var_screeninfo.xres * var_screeninfo.yres) * 3 + 3) &~ 3) + 14 + 40);
  PUT16(0); PUT16(0); PUT32(14 + 40);
  PUT32(40); PUT32(var_screeninfo.xres); PUT32(var_screeninfo.yres);
  PUT16(1);
  PUT16(24);
  PUT32(0); PUT32(0); PUT32(0); PUT32(0); PUT32(0); PUT32(0);
#undef PUT32
#undef PUT16
#undef PUT8
  fwrite(hdr, 1, i, fd2);
	
  // writing pixmap
  for (y=var_screeninfo.yres-1; y>=0 ; y-=1) {
    fwrite(frame4+(y*var_screeninfo.xres*3),var_screeninfo.xres*3,1,fd2);
  }
  fclose(fd2);
  }
	
  // Thats all folks 
  printf("... Done !\n");
	
	
}
