//
// C++ Interface: retrievalwnd
//
// Description: 
//
//
// Author: Ruud Nabben <r.nabben@gmail.com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RETRIEVALWND_H
#define RETRIEVALWND_H

#include <lib/base/console.h>
#include <lib/gui/eprogress.h>

class RetrievalWnd : public eWindow
{
    public:
        RetrievalWnd(const eString& caption, const eString& cmd, int totalCnt);

    private:
        int eventHandler( const eWidgetEvent &e );

        eLabel *lb_text;
        eProgress *progress;
        eConsoleAppContainer *console;

        int lineNr;
        int numberOfChannels;
        int currentCnt;
        eString command;

        void OnAbortClicked();
        void OnDataAvail(eString data);
        void consoleClosed(int state);
        void setProgressText(const eString& txt );
};

#endif
