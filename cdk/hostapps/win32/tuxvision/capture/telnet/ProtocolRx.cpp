//
//  DBOXII Capture/Render Filter
//  
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// ProtocolRx.cpp: implementation of the CProtocolRx class.
//
//////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>
#include "ProtocolRx.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void nvt(SOCKET server,unsigned char data);
enum _ansi_state
{
  as_normal,
  as_esc,
  as_esc1
};
static sa = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
char codebuf[256];
unsigned char codeptr;
#define NUM_CODEC 6
typedef void (*LPCODEPROC)(char*);
void ansi_set_screen_attribute(char* buffer);
void ansi_set_position(char* buffer);
void ansi_erase_screen(char* buffer);
void ansi_move_up(char* buffer);
void ansi(SOCKET server,unsigned char data);
void ansi_erase_line(char* buffer);
void ddww_error(SOCKET server,_verb verb,_option option);
void ddww_echo(SOCKET server,_verb verb, _option option);
void ddww_supp(SOCKET server,_verb verb,_option option); //Suppress GA
void ddww_term(SOCKET server,_verb verb,_option option); //Subnegotiate terminal type
void sbproc_term(SOCKET server,unsigned char data);
	

struct 
{
  unsigned char cmd;
  LPCODEPROC proc;
} codec[NUM_CODEC] = {
  {'m',ansi_set_screen_attribute},
  {'H',ansi_set_position},
  {'K',ansi_erase_line},
  {'J',ansi_erase_screen},
  {'A',ansi_move_up},
  {0,0}
};

CProtocolRx::CProtocolRx()
{

}

CProtocolRx::~CProtocolRx()
{

}
#define NUL     0
#define BEL     7
#define BS      8
#define HT      9
#define LF     10
#define VT     11
#define FF     12
#define CR     13
#define SE    240
#define NOP   241
#define DM    242
#define BRK   243
#define IP    244
#define AO    245
#define AYT   246
#define EC    247
#define EL    248
#define GA    249
#define SB    250
#define WILL  251
#define WONT  252
#define DO    253
#define DONT  254
#define IAC   255


int option_error(_verb,_option,int,SOCKET);

typedef void(*LPOPTIONPROC)(SOCKET,_verb,_option);
typedef void(*LPDATAPROC)(SOCKET,unsigned char data);


inline void yesreply(SOCKET server, _verb verb,_option option)
{
  unsigned char buf[3];
  buf[0] = IAC;
  buf[1] = (verb==verb_do)?WILL:(verb==verb_dont)?WONT:(verb==verb_will)?DO:DONT;
  buf[2] = (unsigned char)option;
  send(server,(char*)buf,3,0);
}

inline void noreply(SOCKET server, _verb verb,_option option)
{
  unsigned char buf[3];
  buf[0] = IAC;
  buf[1] = (verb==verb_do)?WONT:(verb==verb_dont)?WILL:(verb==verb_will)?DONT:DO;
  buf[2] = (unsigned char)option;
  send(server,(char*)buf,3,0);
}

inline void askfor(SOCKET server, _verb verb,_option option)
{
  unsigned char buf[3];
  buf[0] = IAC;
  buf[1] = (unsigned char)verb;
  buf[2] = (unsigned char)option;
  send(server,(char*)buf,3,0);
}


void ddww_error(SOCKET server,_verb verb,_option option)
{
#ifdef _DEBUG
  char tmp[256];
  wsprintf(tmp,"Unknown Option Code: %s, %i\n",(verb==verb_do)?"DO":(verb==verb_dont)?"DON'T":(verb==verb_will)?"WILL":"WONT",(int)option);
  OutputDebugString(tmp);
#endif

  switch(verb)
  {
  case verb_will: 
    noreply(server,verb,option);
    break;
  case verb_wont:
    return;
  case verb_do:
    noreply(server,verb,option);
    break;
  case verb_dont:
    return;
  }
}
void ddww_echo(SOCKET server,_verb verb, _option option)
{
  yesreply(server,verb,option);
}


void ddww_supp(SOCKET server,_verb verb,_option option) //Suppress GA
{
  
  int set = 0;

  switch(verb)
  {
  case verb_will: // server wants to suppress GA's
    if(set) break; //don't confirm - already set.
    askfor(server,verb_do,TOPT_SUPP);
    askfor(server,verb_will,TOPT_SUPP);
    askfor(server,verb_do,TOPT_ECHO);
    break;
  case verb_wont: // server wants to send GA's 
    if(!set) break; //don't confirm - already unset.
    askfor(server,verb_dont,TOPT_SUPP);
    askfor(server,verb_wont,TOPT_SUPP);
    break;
  case verb_do:   // server wants me to suppress GA's
    if(set) break;
    askfor(server,verb_do,TOPT_SUPP);
    break;
  case verb_dont: // server wants me to send GA's
    if(!set) break;
    askfor(server,verb_dont,TOPT_SUPP);
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Option TERMINAL-TYPE

void ddww_term(SOCKET server,_verb verb,_option option) //Subnegotiate terminal type
{
  switch(verb)
  {
  case verb_will:
    noreply(server,verb,option); // I don't want terminal info
    break;
  case verb_wont:
    //dat be cool - its not going to send. no need to confirm
    break;
  case verb_do:
    yesreply(server,verb,option); //I'll send it when asked
    break;
  case verb_dont://Ok - I won't
    break;
  }
}

// TERMINAL TYPE subnegotation
enum
{
  SB_TERM_IS = 0,
  SB_TERM_SEND = 1
};

#define NUM_TERMINALS 2

struct
{
  char* name;
  LPDATAPROC termproc;
  //pre requsites.
} terminal[NUM_TERMINALS] = {
  { "NVT", nvt }, 
  { "ANSI", ansi }
};

int term_index = 0;

void sbproc_term(SOCKET server,unsigned char data)
{

  if(data == SB_TERM_SEND)
  {
    if(term_index == NUM_TERMINALS)
      term_index = 0;
    else
      term_index++;
    char buf[16]; //pls limit 
    buf[0] = (char)IAC;
    buf[1] = (char)SB;
    buf[2] = (char)TOPT_TERM;
    buf[3] = (char)SB_TERM_IS;
    lstrcpy(&buf[4],terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].name);
    int nlen = lstrlen(&buf[4]);
    buf[4+nlen] = (char)IAC;
    buf[5+nlen] = (char)SE;
    send(server,buf,4+nlen+2,0);
  }
}

///////////////////////////////////////////////////////////////////////////////

struct
{
  _option option;
  LPOPTIONPROC OptionProc;
  LPDATAPROC DataProc;
}  ol[] = {
  {TOPT_ECHO,   ddww_echo,  NULL},
  {TOPT_SUPP,   ddww_supp,  NULL},
  {TOPT_TERM,   ddww_term,  sbproc_term},
  {TOPT_ERROR,  ddww_error, NULL}
};


void CProtocolRx::TelnetProtcol(SOCKET server,unsigned char code)
{
//These vars are the finite state
  static int state = state_data;
  static _verb verb = verb_sb;
  static LPDATAPROC DataProc = terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].termproc;

//Decide what to do (state based)
  switch(state)
  {
  case state_data:
    switch(code)
    {
    case IAC: state = state_code; break;
    default: DataProc(server,code);
    }
    break;
  case state_code:
    state = state_data;
    switch(code)
    {
    // State transition back to data
    case IAC: 
      DataProc(server,code);
      break;
    // Code state transitions back to data
    case SE:
      DataProc = terminal[(term_index==NUM_TERMINALS)?(NUM_TERMINALS-1):term_index].termproc;
      break;
    case NOP:
      break;
    case DM:
      break;
    case BRK:
      break;
    case IP:
      break;
    case AO:
      break;
    case AYT:
      break;
    case EC:
      break;
    case EL:
      break;
    case GA:
      break;
    // Transitions to option state
    case SB:
      verb = verb_sb;
      state = state_option;
      break;
    case WILL:
      verb = verb_will;
      state = state_option;
      break;
    case WONT:
      verb = verb_wont;
      state = state_option;
      break;
    case DO:
      verb = verb_do;
      state = state_option;
      break;
    case DONT:
      verb = verb_dont;
      state = state_option;
      break;
    }
    break;
  case state_option:
    state = state_data;

    //Find the option entry
    for(
      int i = 0;
      ol[i].option != TOPT_ERROR && ol[i].option != code;
      i++);

    //Do some verb specific stuff
    if(verb == verb_sb)
      DataProc = ol[i].DataProc;
    else
      ol[i].OptionProc(server,verb,(_option)code);
    break;
  }
}
void nvt(SOCKET server,unsigned char data)
{
  switch(data)
  {
  case 0:  //eat null codes.
    break;
  default: //Send all else to the console.
    break;
  }
}
void ansi_set_screen_attribute(char* buffer)
{
  while(*buffer)
  {
    switch(*buffer++)
    {
    case '0': //Normal
      sa = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      break;
    case '1': //Hign Intensity
      sa |= FOREGROUND_INTENSITY;
      break;
    case '4': //Underscore
      break;
    case '5': //Blink.
      sa |= BACKGROUND_INTENSITY;
      break;
    case '7':
      sa = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
      break;
    case '8':
      sa = 0;
      break;
    case '3':
      sa = sa & (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY) |
        (*buffer & 1)?FOREGROUND_RED:0 |
        (*buffer & 2)?FOREGROUND_GREEN:0 |
        (*buffer & 4)?FOREGROUND_BLUE:0;
      if(*buffer)
        buffer++;
      break;
    case '6':
      sa = sa & (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY) |
        (*buffer & 1)?BACKGROUND_RED:0 |
        (*buffer & 2)?BACKGROUND_GREEN:0 |
        (*buffer & 4)?BACKGROUND_BLUE:0;
      if(*buffer)
        buffer++;
      break;
    }
    if(*buffer && *buffer == ';')
      buffer++;
  }
}
void ansi_erase_line(char* buffer)
{
  int act = 0;
  while(*buffer)
  {
    act = (*buffer++) - '0';
  }

}
void ansi_set_position(char* buffer)
{
  COORD pos = {0,0};

  // Grab line
  while(*buffer && *buffer != ';')
    pos.Y = pos.Y*10 + *buffer++ - '0';

  if(*buffer)
    buffer++;

  // Grab y
  while(*buffer && *buffer != ';')
    pos.X = pos.X*10 + *buffer++ - '0';

  (pos.X)?pos.X--:0;
  (pos.Y)?pos.Y--:0;

}
void ansi_erase_screen(char* buffer)
{
  int act = 0;
  while(*buffer)
  {
    act = (*buffer++) - '0';
  }


}
void ansi_move_up(char* buffer)
{
  int cnt = *buffer?0:1;
  while(*buffer)
  {
    cnt = cnt*10 + (*buffer++) - '0';
  }

}
void ansi(SOCKET server,unsigned char data)
{
  static _ansi_state state = as_normal;
  switch( state)
  {
  case as_normal:
    switch(data)
    {
    case 0:  //eat null codes.
      break;
    case 27: //ANSI esc.
      state = as_esc;
      break;
    default: //Send all else to the console.
      break;
    }
    break;
  case as_esc:
    state = as_esc1;
    codeptr=0;
    codebuf[codeptr] = 0;
    break;
  case as_esc1:
    if(data > 64)
    {
      codebuf[codeptr] = 0;
      for(int i=0; codec[i].cmd && codec[i].cmd != data; i++);
      if(codec[i].proc)
        codec[i].proc(codebuf);
#ifdef _DEBUG
      else
      {
        char buf[256];
        wsprintf(buf,"Unknown Ansi code:'%c' (%s)\n",data,codebuf);
        OutputDebugString(buf);
      }
#endif
      state = as_normal;
    }
    else
      codebuf[codeptr++] = data;
    break;
  }
}
