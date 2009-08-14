/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.
*/
/*************************************************************/

/*C PID NAME
E x y z
T title
X time
S type
L length
D descr
e
c	*/
// If localtimeFlag is set, it means we're reading data
// that's in localtime, so we need to convert to GMT

#include "epgui.h"
#include "mv.h"

#define min(x,y) ( (x<y)?x:y )
#define CHARSEP 12


void InputManager::readEPGUIProgramData( char *path, char encoding, int offset, time_t from, time_t to, int maxChannels )
{
 char pathIdx[MAX_PATH_LENGTH+12];
 FILE *fp,*fpe,*fpc;
 char buf[MAX_STRING_LENGTH+2];
 time_t heu,dur;
 long off;
 char cha[MAX_CHANNEL_NAME_LENGTH+2]
	 ,tit[MAX_PROGRAM_NAME_LENGTH+2]
	 ,gen[MAX_PROGRAM_NAME_LENGTH+2]
	 ,chaOld[MAX_CHANNEL_NAME_LENGTH+2]
	 ;
 const char *chaMap;
 char * posvir;
 char * pos0;
 bool inChannelFlag = false;
 bool flagPlus1 = false;
 int nbE;
 short idxFic;
 int first=false;
  for (idxFic=0 ; idxFic < nbDataFiles ; idxFic++)
    if (!strcmp(FileNames[idxFic] , path))
      break;
  if(idxFic >= nbDataFiles)
  {
    if(idxFic >maxDataFiles)
    {
      maxDataFiles +=5;
      pDataFiles=(FILE **)realloc(pDataFiles,maxDataFiles*sizeof(FILE *));
      FileNames=(char **)realloc(FileNames,maxDataFiles*sizeof(char *));
      encodings=(char *)realloc(encodings,maxDataFiles*sizeof(char));
    }
    fp = fopen( path, "r" );
    if ( ! fp )
      return;
    pDataFiles[idxFic]=fp;
    FileNames[idxFic]=(char *)malloc(strlen(path)+1);
    strcpy( FileNames[idxFic], path);
    encodings[idxFic]=encoding;
    nbDataFiles=idxFic+1;
    first=true;
  }
  else
    fp = pDataFiles[idxFic];
  safeStrncpy(pathIdx,path,MAX_PATH_LENGTH+10);
  strcat(pathIdx,".idxe");
  if ( ( ! pathExists(pathIdx))
      || (fileMtime(pathIdx) < fileMtime(path))
	 )
  {
    // lancer l'indexation
    eString pathEpgidx = prefixDir( eString( indexToDir( inputs.storageDir ) ), "epgidx" );
    if( ! pathExists(pathEpgidx))
    {
      pathEpgidx=eString("/media/hdd/mv/epgidx");
      if( ! pathExists(pathEpgidx))
      {
        pathEpgidx=eString("/media/usb/mv/epgidx");
        if( ! pathExists(pathEpgidx))
        {
          pathEpgidx=eString("/var/bin/epgidx");
          if( ! pathExists(pathEpgidx))
          {
            pathEpgidx=eString("/bin/epgidx");
            if( ! pathExists(pathEpgidx))
  		return;
          }
        }
      }
    }
    eString sysCom = pathEpgidx+eString(" ")+eString(path);
    system(sysCom.c_str());
  }
  fpe = fopen( pathIdx, "r" );
  if ( ! fpe )
    return;
  safeStrncpy(pathIdx,path,MAX_PATH_LENGTH+10);
  strcat(pathIdx,".idxc");
  fpc = fopen( pathIdx, "r" );
  if ( ! fpc )
    return;
  cha[MAX_CHANNEL_NAME_LENGTH]=tit[MAX_PROGRAM_NAME_LENGTH]=gen[MAX_PROGRAM_NAME_LENGTH]=0;
  chaOld[0]=cha[0]=tit[0]=gen[0]=0;
 std::list<eServiceReferenceDVB> *servList=ecacheP->getServiceListP();
 std::list<eServiceReferenceDVB>::iterator curService = servList->begin();
 eString eCha,eChaMap;
  if(first)
  {
    while(fgets(buf,MAX_STRING_LENGTH+1,fpc))
    {
      pos0=buf;
      posvir=index(pos0,CHARSEP);
      safeStrncpy(cha,pos0,min((posvir-pos0),MAX_CHANNEL_NAME_LENGTH));
      channelMgrP->addInput( convertToUTF(cha,encoding) );
    }
  }
  for (  ; curService != servList->end() ; curService++ )
  {
    chaMap=getServiceName( *curService ).c_str();
    eChaMap=eString(chaMap);
    convertToUpper(eChaMap);
    fseek(fpc,0L,SEEK_SET);
    // chercher le canal sans mapping
    inChannelFlag=false;
    flagPlus1 = false;
    while(fgets(buf,MAX_STRING_LENGTH+1,fpc))
    {
      pos0=buf;
      posvir=index(pos0,CHARSEP);
      safeStrncpy(cha,(char*)
        pos0,min((posvir-pos0),MAX_CHANNEL_NAME_LENGTH));
      eCha=convertToUTF(cha,encoding);
      convertToUpper(eCha);
      //if(!strcasecmp(cha,chaMap))
      if(!eCha.compare(eChaMap))
      {
	channelMgrP->addMappedName(eCha,eChaMap,true);
	//mylog( eString().sprintf( "chaine trouvee %s: %s", chaMap,cha));
	inChannelFlag=true;
	break;
      }
      //else
	//mylog( eString().sprintf( "chaine diff %s: %s:%s", eChaMap.c_str(),eCha.c_str(),cha));
      // chercher ...+1
      if(  (chaMap[strlen(chaMap)-2]=='+')
             &&(chaMap[strlen(chaMap)-1]=='1')
             && (eChaMap.length()==eCha.length()+2)
	     && !memcmp(eCha.c_str(),eChaMap.c_str(),eCha.length()))
      {
	channelMgrP->addMappedName(eCha,eChaMap,true);
	//mylog( eString().sprintf( "chaine +1 trouvee %s: %s", chaMap,cha));
	inChannelFlag=true;
	flagPlus1=true;
	break;
      }
    }
    if(!inChannelFlag)
    {// chercher avec mapping :
      fseek(fpc,0L,SEEK_SET);
      chaOld[0]=0;
      while(fgets(buf,MAX_STRING_LENGTH+1,fpc))
      {
        pos0=buf;
        posvir=index(pos0,CHARSEP);
        safeStrncpy(cha,(char*)
          pos0,min((posvir-pos0),MAX_CHANNEL_NAME_LENGTH));
        eCha=convertToUTF(cha,encoding);
        convertToUpper(eCha);
//mylog( eString().sprintf( "test %s: %s", chaMap,cha));
	  if( channelMgrP->isMapped(eCha.c_str(),eChaMap.c_str()))
	  {
//mylog( eString().sprintf( "alias %s: %s", chaMap,cha));
	    inChannelFlag=true;
            if(  (chaMap[strlen(chaMap)-2]=='+')
             &&(chaMap[strlen(chaMap)-1]=='1')
   	     && (cha[strlen(cha)-2]!='+'))
	      flagPlus1=true;
            else
	      flagPlus1=false;
	    break;
          }
      }
    }
    if(!inChannelFlag)
    {
	//mylog( eString().sprintf( "chaine pas trouvee %s", eChaMap.c_str()));
	    continue;
    }
    pos0=posvir+1;
    nbE=atol(pos0);
    posvir=index(pos0,CHARSEP);
    pos0=posvir+1;
    off=atol(pos0);
    fseek(fpe,off,SEEK_SET);
    while(fgets(buf,MAX_STRING_LENGTH+1,fpe) && ((nbE--)>0) )
    {
      pos0=buf;
      posvir=index(pos0,CHARSEP);
      safeStrncpy(tit,pos0,min(posvir-pos0,MAX_PROGRAM_NAME_LENGTH));
      pos0=posvir+1;
      posvir=index(pos0,CHARSEP);
      safeStrncpy(gen,pos0,min(posvir-pos0,MAX_PROGRAM_NAME_LENGTH));
      pos0=posvir+1;
      heu=atol(pos0);
      posvir=index(pos0,CHARSEP);
      pos0=posvir+1;
      dur=atol(pos0);
      posvir=index(pos0,CHARSEP);
      pos0=posvir+1;
      off=atol(pos0);
      heu += (time_t)offset;
      if(flagPlus1) heu += 3600;
      //printf("%s:%s:%ld:%ld:%s:%ld\n",cha,tit,heu,off,gen,dur);
      if (
             ( ( heu >= from ) && ( heu <= to ) ) 
           ||( ( (heu+dur) >= from ) && ( (heu+dur) <= to ) )
           ||( ( heu <= from ) && ( (heu+dur) >= to ) )
  	)
      {
  	removeTrailingNewlines( tit );
  
  	struct ProgramData p = { 
  			chaMap, 
  			convertToUTF( tit, encoding ),
  			convertToUTF( "", encoding ),
  			heu, dur,
  			genreDescrToGenreFlags( gen, dur )
				,idxFic,off
  		};
  	/* emit */ receiveData( p );//emit
      }
    }
  }
//  fclose(fp);// closed by ~InputManager()
  fclose(fpe);
  fclose(fpc);
}

bool InputManager::analyseTimeLine( char *line, time_t *timeP )
{
	bool ok = false;

	if ( isNumericString( line ) ) {
		*timeP = (time_t) atoi( line );
		ok = true;
	}

	return ok;
}

// PID NAME
bool InputManager::analyseChannelLine( char * line, unsigned int *id, char *channelName ) 
{
	bool ok = false;

	// Find the space

	char *p = strchr(line, ' ' );

        if ( p ) {
		p++;

		safeStrncpy( channelName, p, MAX_CHANNEL_NAME_LENGTH );
		int nameLength = strlen( channelName );

		removeTrailingNewlines( channelName );
		if ( nameLength > 0 )
			ok = true;
	}

	return ok;
}

void writeEPGUIProgramToFileHandle( FILE *handle, struct ProgramData &p )
{
            fprintf( handle,
                        "E\nT %s\nX %ld\nL %ld\n",
                        p.name.c_str(),
                        p.startTime, ( p.duration / 60 )
                );

                // Only print non-zero descriptions

                if ( p.descr.length() > 0 ) {
                        eString replacedDescr = stringReplace( p.descr, '\n', '\13' );
                        if ( replacedDescr.length() > 0 )
                                fprintf( handle, "D %s\n", replacedDescr.c_str() );
                }

                if ( p.flags & programDataFlagFilm )
                        fprintf( handle, "S FILM\n" );

                fprintf( handle, "e\n" );
}

