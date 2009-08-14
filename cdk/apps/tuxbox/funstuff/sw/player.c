#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
 FILE *fd;
 char buffer[256];
 char y_offset[]="\n\n\n";
 char x_offset[]="   ";
 int fp=0,pos=0;
 
 if((fd=popen("gunzip -c sw1.txt.gz","r")) == NULL) {
  fprintf(stderr,"pipe-error executing gunzip -c sw1.txt.gz\n");
  return 1;
 }

 while(fgets(buffer,255,fd) != NULL) {
  if (pos == 0) {
   usleep((1000000/15)*fp);
   sscanf(buffer,"%d",&fp);
   printf("\033[;H\033[2J");
   printf("%s",y_offset);
  }
  else {
   printf("%s",x_offset);
   printf("%s",buffer);
  }
  pos++;
  if (pos > 13)
    pos=0;
 }

 pclose(fd);

 return 0;
}
