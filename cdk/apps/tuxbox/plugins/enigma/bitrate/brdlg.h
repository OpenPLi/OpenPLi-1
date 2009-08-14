#ifndef __BRDLG_H__
#define __BRDLG_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/base/ebase.h>
#include <lib/dvb/service.h>
#include <enigma.h>

class BitrateDialog: public eWindow
{
   eLabel *currVal, *avrgVal;
   eLabel *minVal, *maxVal;
   eLabel *audioVal, *audioType;
   eTimer startTimer;
   int SyncTS(unsigned char *buf, int len);
   unsigned long TimevalToMs(const struct timeval *tv);
   long DeltaTimeMs(struct timeval *tv, struct timeval *last_tv);
   int eventHandler(const eWidgetEvent &e);
   void DisplayErrorMessage(const char *msg);
   void Run();

public:
   BitrateDialog();
   ~BitrateDialog();
};

#endif
