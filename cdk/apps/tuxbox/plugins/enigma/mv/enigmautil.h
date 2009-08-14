#ifndef __ENIGMAUTIL_H__
#define __ENIGMAUTIL_H__

#include <timer.h>
#include <lib/gui/ewidget.h>

#include "program.h"
#include "channel.h"
#include "enigmacache.h"

enum {
        timerEditDelete = 1,
        timerEditRecord = 2,
        timerEditNGRAB = 3,
        timerEditSwitch = 4
};

//eServiceReferenceDVB getPlayingServiceRef( void );
void showTimerEditWindow( eWidget *parent, int ttType,Channel *ccp, Program *pp, time_t preOffset, time_t postOffset );
void showTimerListWindow( void );

#endif
