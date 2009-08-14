/***************************************************************************
                          filemgr.cpp  -  description
                             -------------------
    begin                : Sat Jan  3 09:24:05 SAST 2004
    copyright            : (C) 2004 by KingTut
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filemgr.h"

char* strip(char * str)
{
	int length = strlen(str);
	
	for (int i = 0; i < length; ++i)
	{
		if (*str != ' ' && *str != '\t' && *str != '\r' && *str != '\n')
			break;
		++str;
	}

	length = strlen(str);
	
	for (int i = length; i > 0; --i)
	{
		if (str[i - 1] != ' ' && str[i - 1] != '\t' && str[i - 1] != '\r' && str[i - 1] != '\n')
			break;
		str[i - 1] = 0;
	}
	
	return str;
}

int gettag(char *tagName, char *tagValue)
{
  FILE *fh;
  char inLine[100];
  int Hit=0;
  char *tmp;
  int  retCode = -1;

  char tag[100],value[100];

  printf("GetTag Requested -> %s\n",tagName);
  fh = fopen("/tmp/dncast.tmp","r");

  while  (fh && fgets(inLine,100,fh) != NULL)
  {
      Hit = memcmp(&inLine,tagName,strlen(tagName));
      if ( !Hit )
        {
        if (strstr(inLine,"="))
         {

          tmp = strtok(inLine,"=");
          strcpy(tag,tmp);

          tmp = strtok(NULL,"=");
          strcpy(value,tmp);

          printf("Tag -> %s\n",tag);
          printf("Value -> %s\n",value);
          
          strcpy(tagValue, strip(value));
          
          retCode = 1;
          break;
         }
        }  // memory compare succcessfull
  } // loop for entire file

  if (fh)
	 fclose(fh);
 
  return(retCode);
}
