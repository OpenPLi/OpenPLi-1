#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <dbox/lcd-ks0713.h>

void print_status( int fd )
{
    int val;

    val=0;

    if ( ioctl(fd,LCD_IOCTL_STATUS,&val) < 0 )
        perror("");

    printf("STATUS: %02X = [",val&0xff);

    if (val&LCD_STAT_BUSY)
        printf(" BUSY");
    if (val&LCD_STAT_ADC)
        printf(" ADC");
    if (val&LCD_STAT_ON)
        printf(" ON");
    if (val&LCD_STAT_RESETB)
        printf(" RESETB");
    if (val&8)
        printf(" (8)");
    if (val&4)
        printf(" (4)");
    if (val&2)
        printf(" (2)");
    if (val&1)
        printf(" (1)");

    printf(" ]\n");

    return;
}

void lcd_reset( int fd )
{
    printf("LCD RESET\n");

    if ( ioctl(fd,LCD_IOCTL_RESET,0) < 0 )
        perror("");

    return;
}

void lcd_init( int fd )
{
    printf("LCD INIT\n");

    if ( ioctl(fd,LCD_IOCTL_INIT,0) < 0 )
        perror("");

    return;
}

void lcd_on( int fd, int val )
{
    printf("LCD ON %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_ON,&val) < 0 )
        perror("");

    return;
}

void lcd_eon( int fd, int val )
{
    printf("LCD EON %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_EON,&val) < 0 )
        perror("");

    return;
}

void lcd_reverse( int fd, int val )
{
    printf("LCD REVERSE %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_REVERSE,&val) < 0 )
        perror("");

    return;
}

void lcd_bias( int fd, int val )
{
    printf("LCD BIAS %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_BIAS,&val) < 0 )
        perror("");

    return;
}

void lcd_adc( int fd, int val )
{
    printf("LCD ADC %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_ADC,&val) < 0 )
        perror("");

    return;
}

void lcd_shl( int fd, int val )
{
    printf("LCD SHL %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_SHL,&val) < 0 )
        perror("");

    return;
}

void lcd_powerc( int fd, int val )
{
    printf("LCD POWER CONTROL %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_POWERC,&val) < 0 )
        perror("");

    return;
}

void lcd_res( int fd, int val )
{
    printf("LCD RES %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_SEL_RES,&val) < 0 )
        perror("");

    return;
}

void lcd_sir( int fd, int val )
{
    printf("LCD SIR %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_SIR,&val) < 0 )
        perror("");

    return;
}

void lcd_sirc( int fd, int val )
{
    printf("LCD SIRC %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_SIRC,&val) < 0 )
        perror("");

    return;
}

void lcd_srv( int fd, int val )
{
    printf("LCD SRV %d\n",val);

    if ( ioctl(fd,LCD_IOCTL_SRV,&val) < 0 )
        perror("");

    return;
}

int main(int argc, char **argv)
{
	int fd; // ,fd_sec;
	int pa,col,i;
    lcd_pixel pix;
    int x,y;
	if((fd = open("/dev/dbox/lcd0",O_RDWR)) < 0){
		perror("LCD: ");
		return -1;
	}

    lcd_init(fd);

	return 0;

    print_status(fd);
/*
    pix.v = 2;

    for (i=0;i<0xFFFF;i++) {

    while ( (pix.x=(random()&0xFF)) >= LCD_COLS );
    while ( (pix.y=(random()&0xFF)) >= (LCD_ROWS*8) );

    if ( ioctl(fd,LCD_IOCTL_SET_PIXEL,&pix) < 0 )
     perror("");

    }

//    return 0;
*/


    lcd_on(fd,0);
    sleep(1);
    lcd_on(fd,1);

    sleep(1);
    lcd_eon(fd,1);
    sleep(1);
    lcd_eon(fd,0);

    sleep(1);
    lcd_reverse(fd,1);
    sleep(1);
    lcd_reverse(fd,0);

    sleep(1);
    lcd_bias(fd,0);
    sleep(1);
    lcd_bias(fd,1);

    sleep(1);
    lcd_adc(fd,0);
    sleep(1);
    lcd_adc(fd,1);

    sleep(1);
    lcd_shl(fd,1);
    sleep(1);
    lcd_shl(fd,0);

    // TODO: IDL
    // TODO: SRV

    for(i=0;i<=7;i++) {
        lcd_powerc(fd,i);
        sleep(1);
    }

    for(i=0;i<=7;i++) {
        lcd_res(fd,i);
        sleep(1);
    }

    for(i=0;i<=3;i++) {
        lcd_sir(fd,i);
        sleep(1);
    }

    for(i=0;i<=0x3f;i++) {
        lcd_srv(fd,i);
        sleep(1);
    }

    // spage
    // scolumn

    return 0;
	i=LCD_MODE_BIN;
	if ( ioctl(fd,LCD_IOCTL_ASC_MODE,&i) < 0 )
	 perror("");

	i=LCD_MODE_ASC;
	if ( ioctl(fd,LCD_IOCTL_ASC_MODE,&i) < 0 )
	 perror("");

	col=0;
	if ( ioctl(fd,LCD_IOCTL_SCOLUMN,&col) < 0 )
	 perror("");

	for(pa=0;pa<LCD_ROWS;pa++)
	{
	col=0;
	if ( ioctl(fd,LCD_IOCTL_SCOLUMN,&col) < 0 )
	 perror("");
	if ( ioctl(fd,LCD_IOCTL_SPAGE,&pa) < 0 )
	 perror("");

//	if ( ioctl(fd,LCD_IOCTL_SET_ADDR,&col) < 0 )
//	 perror("");

//		lcd_set_pos( pa, 0 );

	i=pa;
	for(col=0;col<LCD_COLS;col++)
	{
//		i=~i;
		printf("%d - %d\n",pa,col);
		if ( ioctl(fd,LCD_IOCTL_WRITE_BYTE,&i) < 0 )
			perror("");
//			sleep(1);
//			lcd_write_byte( *bp );
		}
  }

		if ( ioctl(fd,LCD_IOCTL_WRITE_BYTE,&i) < 0 )
			perror("");

/*
	fseek(fd,100,SEEK_SET);
	printf("%d\n",ftell(fd));

	fseek(fd,100,SEEK_CUR);
	printf("%d\n",ftell(fd));

	fseek(fd,100,SEEK_END);
	printf("%d\n",ftell(fd));
*/
	close(fd);
}

