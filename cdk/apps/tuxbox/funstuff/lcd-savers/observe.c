
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

void error(char *errormsg)
{
	printf("%s\n", errormsg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int lcdfd;
	int i;
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char text[32768];
	char buffer[32768];
	char message[32768];
	char temp[1000];
	char from[1000];
	char *temp2;
	char *msg;

	if(argc!=4)
	{
		printf("%s <nick> <channel ohne #> <server>\n", argv[0]);
		return 1;
	}  
	lcdfd=open("/dev/dbox/lcd0", O_RDWR);
	i=LCD_MODE_ASC;
	ioctl(lcdfd, LCD_IOCTL_ASC_MODE, &i);
	ioctl(lcdfd, LCD_IOCTL_CLEAR);
	// y=8 zeilen und x=15 zeichen

	sprintf(text, "observe by trh\n");
	write(lcdfd, text, strlen(text));

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0)
		error("ERROR opening socket");
	server=gethostbyname(argv[3]);
	printf("connecting to server %s\n", argv[3]);
	if(server==NULL) {
		error("host resolving failed\n");
	}
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	memmove((char*)&serv_addr.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
	serv_addr.sin_port=htons(6667);
	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR: connect");
	sprintf(buffer, "USER %s dbox dbox dbox2_lcd-screensaver\nNICK %s\n", argv[1], argv[1]);
	write(sockfd, buffer, strlen(buffer));
	printf("sent: %s", buffer);
	sleep(1);
	sprintf(buffer, "JOIN #%s\n", argv[2]);
	write(sockfd, buffer, strlen(buffer));
	printf("sent: %s\n", buffer);

	while(1)
	{
		i=read(sockfd, buffer, 255);
		buffer[i]=0;
		printf("%s\n", buffer);
		if(sscanf(buffer, "%s PRIVMSG #%s :%s", from, temp, message)==3)
		{
			sscanf(from, ":%s", temp);
			temp2=strtok((char*)temp, "!");
			
			msg=strstr(buffer+1, ":");
			sprintf(buffer, "<%s>: %s\n", temp2, msg+1);
			write(lcdfd, buffer, strlen(buffer));
		} else
		{
			if(sscanf(buffer, "PING %s", from)==1)
			{
				sprintf(buffer, "PONG %s\n", from);
				write(sockfd, buffer, strlen(buffer));
				printf("%s\n", buffer);
 			}
		}
	}
	return 0;
}
