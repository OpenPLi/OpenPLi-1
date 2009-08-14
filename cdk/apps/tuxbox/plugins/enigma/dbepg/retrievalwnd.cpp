//
// C++ Implementation: retrievalwnd
//
// Description: 
//
//
// Author: Ruud Nabben <r.nabben@gmail.com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "util.h"
#include "retrievalwnd.h"


RetrievalWnd::RetrievalWnd(const eString& caption, const eString& cmd, int totalCnt)
    : eWindow(1), lb_text(0), progress(0), console(0), lineNr(0), numberOfChannels(totalCnt), currentCnt(0), command(cmd)
{
    setWidgetGeom(this, 80, 80, 400, 250);
    setText(caption);
    std::cout << "Creating lbData" << std::endl;
    lb_text = new eLabel(this);
    setWidgetGeom(lb_text, 20, 20, clientrect.width()-40, 30);
    lb_text->show();
    progress = new eProgress(this);
    setWidgetGeom(progress, 20, 50, clientrect.width()-40, 30);
    progress->show();

    eButton *bt_abort = new eButton(this);
    setWidgetGeom(bt_abort, clientrect.width()-170, clientrect.height()-35, 150, 30);
    bt_abort->setText("Abort");
    bt_abort->setShortcut("red");
    bt_abort->setShortcutPixmap("red");
    bt_abort->loadDeco();
    bt_abort->show();
    CONNECT( bt_abort->selected, RetrievalWnd::OnAbortClicked );
}

int RetrievalWnd::eventHandler( const eWidgetEvent &e )
{
    switch(e.type)
    {
        case eWidgetEvent::execBegin:
        {
            //CONNECT(progress->closed, EpgConfigDialog::OnProgressClosed);
            std::cout << "Creating app" << std::endl;
            console = new eConsoleAppContainer(command);
            CONNECT(console->dataAvail, RetrievalWnd::OnDataAvail);
            CONNECT(console->appClosed, RetrievalWnd::consoleClosed);
            break;
        }
        case eWidgetEvent::execDone:
            if (console)
                delete console;
            break;

        default:
            return eWindow::eventHandler( e );
    }
    return 1;
}

void RetrievalWnd::consoleClosed(int state)
{
    if (console)
    {
        delete console;
        console=0;
    }
    accept();
}

void RetrievalWnd::OnDataAvail(eString txt)
{
    //let converter spit out a line so all commands can benefit from feedback
    //Scripts can send a message
    //"#count:" to indicate number of files
    //"#cur:" to indicate current file
    //converter sends "#pos:"
    static unsigned int count = 1;
    static unsigned int cur = 0;
    static int offset = 0;  //to help scripters either count from 1 to count, or from 0 to count -1
    int pos = 0;
    if (txt.find('#') == 0)
    {
        if (txt.find("pos") != eString::npos)
        {
            //update current progress...
            pos = atoi(txt.substr(5).c_str());
            if (pos < 0) pos = 0;
        }
        else if (txt.find("count") != eString::npos)
        {
            count = atoi(txt.substr(7).c_str());
        }
        else if (txt.find("cur") != eString::npos)
        {
            int tmp = atoi(txt.substr(5).c_str());
            if (tmp < 0)
            {
                if (offset == 0 && cur == 0)
                {
                    offset = 1;
                    cur = 0;
                }
                else
                {
                    cur++;
                }
            }
            else
            {
                cur = tmp;
            }
        }
    }
    eString tmp;
    tmp.sprintf("Retrieving file %d of %d", cur+1-offset, count);
    lb_text->setText(tmp);
    progress->setPerc((pos + 100*cur) / count);
}

void RetrievalWnd::OnAbortClicked()
{
    std::cout << "Abort clicked" << std::endl;
    if (console)
    {
        console->kill();
    }
    close(1);
}
