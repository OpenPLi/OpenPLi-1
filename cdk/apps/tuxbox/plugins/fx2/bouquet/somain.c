/*
** initial coding by fx2
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <colors.h>
#include <draw.h>
#include <fx2math.h>
#include <plugin.h>
#include <rcinput.h>

#include <config.h>

extern	int				debug;
extern	int				doexit;
extern	unsigned short	actcode;
extern	unsigned short	realcode;

static	char	**lines = 0;

typedef struct _Channel
{
	char			*name;
	char			flag;
	char			*line;
} Channel;

static	Channel	*tv_ch=0;
static	Channel	*radio_ch=0;
static	int		num_lines=0;
static	int		num_ch_tv=0;
static	int		num_ch_radio=0;

static	void	SortBouquet( Channel *ch, int num )
{
	int				i;
	int				color;
	int				p1=0;
	int				pos=0;
	int				w=0;
	int				redraw=0;
	int				mode=0;
	int				x;
	int				x2;
	struct timeval	tv;
	Channel			chs;
	char			text[16];

	FBFillRect( 0,0,720,576,BLACK);

	FBDrawString( 200, 32, 48, ch==tv_ch?"TV bouquet":"Radio bouquet",RED,0);

	while( realcode != 0xee )
		RcGetActCode();
	actcode=0xee;

	while( !doexit )
	{
		if ( w )
			FBFillRect( 150, 90, w+150, 44*9+48, BLACK );
		w=0;
		for( i=p1; i<p1+10; i++ )
		{
			sprintf(text,"%d",i+1);
			x2=FBDrawString( 150,90+(i-p1)*44,48,text,BLACK,0);
			FBDrawString( 260-x2,90+(i-p1)*44,48,text,GRAY,0);

			color= ch[i].flag == '0' ? GRAY : GRAY2;
			if ( i== pos )
				color=mode?RED:WHITE;

			x=FBDrawString( 300,90+(i-p1)*44,48,ch[i].name,color,0);
			if ( x > w )
				w=x;
		}
		FBDrawString( 50, 100, 32, "OK select",GRAY,0);
		FBFillRect( 50, 142, 8, 8, BLUE );
		FBDrawString( 62, 130, 32, "hide/show",GRAY,0);
		FBFillRect( 50, 172, 8, 8, RED );
		FBDrawString( 62, 160, 32, "save",GRAY,0);
		FBFillRect( 50, 202, 8, 8, YELLOW );
		FBDrawString( 62, 190, 32, ch==tv_ch?"radio":"tv",GRAY,0);
#ifdef USEX
	FBFlushGrafic();
#endif
		while( realcode != 0xee )
			RcGetActCode();

		redraw=0;
		while( !doexit && !redraw )
		{
			tv.tv_sec = 0;
			tv.tv_usec = 200000;
			select( 0,0,0,0,&tv );
			actcode=0xee;
			RcGetActCode();
			if ( actcode == 0xee )
				continue;
			switch( actcode )
			{
			case RC_UP :
				if ( pos == 0 )
					continue;
				pos--;
				if ( mode )
				{
					memcpy(&chs,ch+pos+1,sizeof(Channel));
					memcpy(ch+pos+1,ch+pos,sizeof(Channel));
					memcpy(ch+pos,&chs,sizeof(Channel));
				}
				if ( pos < p1 )
				{
					p1 -= 9;
					redraw=1;
					if ( p1 < 0 )
						p1 = 0;
				}
				else
				{
					if ( mode )
					{
						FBDrawString( 300,90+(pos+1-p1)*44,48,
							ch[pos].name, BLACK,0);
						FBDrawString( 300,90+(pos-p1)*44,48,
							ch[pos+1].name, BLACK,0);
					}
					color= ch[pos+1].flag == '0' ? GRAY : GRAY2;
					FBDrawString( 300,90+(pos+1-p1)*44,48,ch[pos+1].name,
						color,0);
					color=mode?RED:WHITE;
					FBDrawString( 300,90+(pos-p1)*44,48,ch[pos].name,
						color,0);
				}
				break;
			case RC_DOWN :
				if ( pos == num-1 )
					continue;
				pos++;
				if ( mode )
				{
					memcpy(&chs,ch+pos-1,sizeof(Channel));
					memcpy(ch+pos-1,ch+pos,sizeof(Channel));
					memcpy(ch+pos,&chs,sizeof(Channel));
				}
				if ( pos > p1+9 )
				{
					p1 += 9;
					redraw=1;
					if ( p1 > num-10 )
						p1 = num-10;
				}
				else
				{
					if ( mode )
					{
						FBDrawString( 300,90+(pos-1-p1)*44,48,
							ch[pos].name, BLACK,0);
						FBDrawString( 300,90+(pos-p1)*44,48,
							ch[pos-1].name, BLACK,0);
					}
					color= ch[pos-1].flag == '0' ? GRAY : GRAY2;
					FBDrawString( 300,90+(pos-1-p1)*44,48,ch[pos-1].name,
						color,0);
					color=mode?RED:WHITE;
					FBDrawString( 300,90+(pos-p1)*44,48,ch[pos].name,
						color,0);
				}
				break;
			case RC_BLUE :
				ch[pos].flag = ch[pos].flag == '0' ? '1' : '0';
				break;
			case RC_OK :
				color=mode?WHITE:RED;
				FBDrawString( 300,90+(pos-p1)*44,48,ch[pos].name,
						color,0);
				mode=!mode;
				break;
			case RC_RED :
				doexit=1;
				break;
			case RC_YELLOW :
				doexit=2;
				break;
			}
#ifdef USEX
			FBFlushGrafic();
#endif
			while( realcode != 0xee )
				RcGetActCode();
		}
	}
}

static	void	setup_colors(void)
{
	FBSetColor( YELLOW, 235, 235, 30 );
	FBSetColor( GREEN, 30, 235, 30 );
	FBSetColor( STEELBLUE, 80, 80, 200 );
	FBSetColor( BLUE, 80, 80, 230 );
	FBSetColor( GRAY, 180, 180, 180 );
	FBSetColor( GRAY2, 130, 130, 130 );

	FBSetupColors();
}

static	void	copyname( char *to, char *from )
{
	char	*p;
	int		sz;

	while( *from != '"' )
	{
		if ( *from == '&' )
		{
			p=strchr(from+1,';');
			sz=p-from;
			if ( !strncmp(from,"&amp;",5) )
			{
				*to='-';  to++;
				*to=' ';  to++;
			}
			else if ( !strncmp(from,"&#220;",6) )
			{
				*to='U';  to++;
				*to='E';  to++;
			}
			else
			{
				*to='?';  to++;
			}
			from+=sz;
			from++;
		}
		else
		{
			*to = *from;
			to++;
			from++;
		}
	}
	*to=0;
}

static	void	SaveServices( void )
{
	FILE	*fp;
	Channel	*ch;
	int		i;
	int		n;
	int		nr;
	char	*p;
	char	*p2;
	int		perc;
	int		lp=-1;

	FBDrawString( 62, 160, 32, "save",BLACK,GRAY);
#ifdef USEX
		FBFlushGrafic();
#endif
	fp = fopen( CONFIGDIR "/zapit/services.xml", "w" );
	if ( !fp )
		return;
	FBFillRect( 96, 226, 508, 24, WHITE );
	FBFillRect( 100, 230, 500, 16, GRAY2 );
	for( n=0; n<num_lines; n++ )
	{
		perc=n*500/num_lines;
		if ( perc != lp )
		{
			FBFillRect( 100, 230, perc, 16, GRAY );
#ifdef USEX
			FBFlushGrafic();
#endif
		}
		nr=0;
		ch=tv_ch;
		for( i=0; i<num_ch_tv;i++ )
			if ( tv_ch[i].line == lines[n] )
				break;
		if ( i==num_ch_tv )
		{
			ch=radio_ch;
			for( i=0; i<num_ch_radio;i++ )
				if ( radio_ch[i].line == lines[n] )
					break;
			if ( i == num_ch_radio )
				ch=0;
			else
				nr=i;
		}
		else
			nr=i;
		if ( !ch )
		{
			fprintf(fp,"%s",lines[n]);
			continue;
		}
		p=strstr(lines[n],"serviceType=");
		p+=14;
		*p = ch[nr].flag;
		p2=strstr(lines[n],"channelNR=");
		p2+=11;
		for( p=lines[n]; p!=p2; p++ )
			fprintf(fp,"%c",*p);
		fprintf(fp,"%d",nr+1);
		p=strchr(p,'"');
		for( ; *p; p++ )
			fprintf(fp,"%c",*p);
	}
	fclose(fp);
}

int bouquet_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				t;
	int				r;
	int				i;
	int				idx=0;
	int				cnum;
	int				x;
	Channel			*ch=NULL;
	FILE			*fp;
	char			line[512];
	char			l2[512];
	char			*p;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;

/* load setup */
	fp = fopen( CONFIGDIR "/zapit/services.xml", "r" );
	if ( !fp )
	{
		FBDrawString( 190, 100, 64, "services.xml not found !", RED, 0 );
#ifdef USEX
		FBFlushGrafic();
#endif
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		select( 0, 0, 0, 0, &tv );

		RcClose();
		FBClose();

		return 0;
	}

	while( fgets(line,512,fp) )
	{
		num_lines++;
		if ( strstr(line,"channelNR") )
		{
			p=strstr(line,"serviceType");
			if ( p && (*(p+15) == '0') )
			{
				p+=16;
				if ( *p == '1' )
					num_ch_tv++;
				if ( *p == '2' )
					num_ch_radio++;
				if ( *p == '4' )
					num_ch_tv++;
			}
		}
	}
	fclose(fp);

	lines = (char**)malloc(sizeof(char*)*num_lines);
	tv_ch = (Channel*)malloc(sizeof(Channel)*num_ch_tv);
	radio_ch = (Channel*)malloc(sizeof(Channel)*num_ch_radio);
	for( i=0;i<num_ch_tv;i++)
		tv_ch[i].flag=0;
	for( i=0;i<num_ch_radio;i++)
		radio_ch[i].flag=0;

	fp = fopen( CONFIGDIR "/zapit/services.xml", "r" );
	t=num_ch_tv-1;
	r=num_ch_radio-1;
	for(i=0; i<num_lines;i++)
	{
		fgets( line, 512, fp );
		lines[i] = strdup(line);
		if ( !strstr(line,"channelNR") )
			continue;

		p=strstr(line,"name=");
		copyname(l2,p+6);

		p=strstr(line,"channelNR=");
		cnum=_atoi(p+11);

		if ( cnum )
			idx=cnum-1;

		p=strstr(line,"serviceType");
		if ( !p || ( *(p+15) != '0' ) )
			continue;

		switch( *(p+16) )
		{
		case '1' :
		case '4' :
			ch=tv_ch;
			if ( !cnum || ch[idx].flag )
			{
				idx=t;
				t--;
			}
			break;
		case '2' :
			ch=radio_ch;
			if ( !cnum || ch[idx].flag )
			{
				idx=r;
				r--;
			}
			break;
		default:
			p=0;
			break;
		}
		if ( !p )
			continue;

		ch[idx].name=strdup(l2);
		ch[idx].flag=*(p+14);
		ch[idx].line=lines[i];
	}
	fclose(fp);

	t=0;
	while( doexit != 3 )
	{
		if ( t )
			SortBouquet( radio_ch, num_ch_radio );
		else
			SortBouquet( tv_ch, num_ch_tv );
		t=!t;
		switch( doexit )
		{
		case 2 :
			doexit=0;
			break;
		case 1 :
			SaveServices();
			doexit=3;
			break;
		}
	}

	for( i=0; i<num_ch_tv; i++ )
		free( tv_ch[i].name );
	for( i=0; i<num_ch_radio; i++ )
		free( radio_ch[i].name );
	for( i=0; i<num_lines; i++ )
		free( lines[i] );
	free( lines );
	free( tv_ch );
	free( radio_ch );

/* fx2 */
/* buffer leeren, damit neutrino nicht rumspinnt */
	realcode = RC_0;
	while( realcode != 0xee )
	{
		tv.tv_sec = 0;
		tv.tv_usec = 300000;
		x = select( 0, 0, 0, 0, &tv );		/* 300ms pause */
		RcGetActCode( );
	}

	RcClose();
	FBClose();

	return 0;
}

int plugin_exec( PluginParam *par )
{
	int		fd_fb=-1;
	int		fd_rc=-1;

	for( ; par; par=par->next )
	{
		if ( !strcmp(par->id,P_ID_FBUFFER) )
			fd_fb=_atoi(par->val);
		else if ( !strcmp(par->id,P_ID_RCINPUT) )
			fd_rc=_atoi(par->val);
	}
	return bouquet_exec( fd_fb, fd_rc, -1, 0 );
}
