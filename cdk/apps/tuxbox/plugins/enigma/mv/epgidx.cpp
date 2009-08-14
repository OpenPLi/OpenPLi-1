#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//typedef int bool;
//#define false 0
//#define true 1


#define MAX_STRING_LENGTH 20023
#define MAX_CHANNEL_NAME_LENGTH 30
#define MAX_PROGRAM_NAME_LENGTH 100
#define MAX_PROGRAM_DESC_LENGTH 20022
#define CHARSEP 12

bool analyseTimeLine( char *line, time_t *timeP )
{
	*timeP=atol(line);
//	if(sscanf(line,"%ld",timeP))
		return true;
//	return false;
}

bool analyseChannelLine( char * line, unsigned int *id, char *channelName ) 
{
	// Find the space
	char *p = strchr(line, ' ' );

        if ( p ) {
		p++;
		strncpy( channelName, p, MAX_CHANNEL_NAME_LENGTH );
		int nameLength = strlen( channelName );
		while(nameLength >0 && channelName[nameLength-1]=='\n')
			channelName[--nameLength]=0;
		if ( nameLength > 0 )
			return true;
	}
	return false;
}

int main(int argc,char **argv)
{
 FILE *ficin;
 FILE *ficC,*ficE;
 char line[MAX_STRING_LENGTH+2];
        char channelName[MAX_CHANNEL_NAME_LENGTH+2];
	char programName[MAX_PROGRAM_NAME_LENGTH+2];
	char programDesc[MAX_PROGRAM_DESC_LENGTH+2];
	char genreString[MAX_PROGRAM_DESC_LENGTH+2];
	unsigned int channelID;
	time_t startTime;
	time_t durationMinutes;

	bool gotName = false;
	bool gotStart = false;
	bool gotDuration = false;
	bool inChannelFlag = false;
	bool inEntryFlag = false;
  long ficinIdx=0;
  long ficEIdx=0;
  long nbE=0;
  long progIdx=-1;
  //long maxLen=0;
  //long len;
  if(argc >1)
    ficin=fopen(argv[1],"r");
  else
  {
    fprintf(stderr,"syntaxe = %s fichier\n",argv[0]);
    return 1;
  }
  if(!ficin)
  {
    fprintf(stderr,"ouverture fichier %s impossible\n",argv[1]);
    return 1;
  }
  sprintf(line,"%s.idxc",argv[1]);
  ficC=fopen(line,"w");
  sprintf(line,"%s.idxe",argv[1]);
  ficE=fopen(line,"w");
  line[MAX_STRING_LENGTH] = '\0';
  while(fgets(line,MAX_STRING_LENGTH,ficin))
  {
    //len = strlen(line);
    //if(len >maxLen) maxLen=len;
    if ( 		
	( line[1] != ' ' ) &&
	( line[1] != '\0' ) &&
	( line[1] != 10 )
       )
	continue;
    char * realStart=line+2;
    switch( line[0] )
    {
     case 'C':
       analyseChannelLine( realStart, &channelID, channelName ) ;
       inChannelFlag = true;
      break;
     case 'c':
	gotName = false; gotStart = false; gotDuration = false;
	programDesc[0] = '\0';
	fprintf(ficC,"%.*s%c%ld%c%ld\n",MAX_CHANNEL_NAME_LENGTH,channelName,CHARSEP,nbE,CHARSEP,ficEIdx);
	ficEIdx=ftell(ficE);
	nbE = 0;
	break;
     case 'E':
	if ( inChannelFlag ) {
		inEntryFlag = true;
	gotName = false; gotStart = false; gotDuration = false;
	programDesc[0] = '\0';
	genreString[0] = '\0';
	if(strlen(line) >2)
	{
	   while ( *realStart != ' ' && *realStart) realStart++;
	   if(*realStart && analyseTimeLine( ++realStart, &startTime ) )
		gotStart = true;
	   while ( *realStart != ' ' && *realStart) realStart++;
	   if(*realStart && analyseTimeLine( ++realStart, &durationMinutes ) ) {
		durationMinutes /= 60;
		gotDuration = true;
	   }
	 }
	}
	break;
     case 'e':
	{
		if ( inEntryFlag && gotName && gotStart && gotDuration ) {

		time_t durationSeconds = durationMinutes * 60;
		  fprintf(ficE,"%.*s%c%.*s%c%ld%c%ld%c%ld\n"
				  ,MAX_PROGRAM_NAME_LENGTH,programName
				  ,CHARSEP,MAX_PROGRAM_NAME_LENGTH,genreString
				  ,CHARSEP,startTime,CHARSEP,durationSeconds
				  ,CHARSEP,progIdx);
		}	
	}
	inEntryFlag = false;
	progIdx=-1;
	nbE++;

	break;
     case 'T':
	if ( inEntryFlag ) {
		strncpy( programName, realStart, MAX_PROGRAM_NAME_LENGTH );	
		int nameLength = strlen( programName );
		while(nameLength >0 && programName[nameLength-1]=='\n')
			programName[--nameLength]=0;
		if (  programName[0]  > 0 )
			gotName = true;
		}
	break;
     case 'D':
	if ( inEntryFlag ) {		
		progIdx=ficinIdx;
		strncpy( programDesc, realStart, MAX_PROGRAM_DESC_LENGTH );	
		int i;
		for(i=0 ; i<MAX_PROGRAM_DESC_LENGTH ; i++)
		{
		  if(programDesc[i]=='|')programDesc[i]='\n';
		  else if(!programDesc[i]) break;
		}
	}
	break;
     case 'X':
	if ( inEntryFlag ) {		
		if ( analyseTimeLine( realStart, &startTime ) )
			gotStart = true;
	}
	break;
     case 'L':
	if ( inEntryFlag ) {		
		if ( analyseTimeLine( realStart, &durationMinutes ) ) {
			gotDuration = true;
		}
	}
	break;
     case 'S':
	if ( inEntryFlag )
	{
		strncpy( genreString, realStart, MAX_STRING_LENGTH );
		int nameLength = strlen( genreString );
		while(nameLength >0 && genreString[nameLength-1]=='\n')
			genreString[--nameLength]=0;
	}
   default:
	break;
    }
    ficinIdx=ftell(ficin);
  }
  fclose(ficin);
  return 0;
}
