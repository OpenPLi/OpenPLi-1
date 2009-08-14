#include <lib/gui/ewidget.h>
#include <lib/gui/elabel.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>
#include <lib/dvb/decoder.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

#include "utils.h"
#include "brdlg.h"

#define TS_LEN       188
#define TS_SYNC_BYTE 0x47
#define TS_BUF_SIZE  (TS_LEN * 2048)

#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#define DEMUX_DEVICE "/dev/dvb/card0/demux1"
#define DVR_DEVICE   "/dev/dvb/card0/dvr1"
#else
#include <linux/dvb/dmx.h>
#define DEMUX_DEVICE "/dev/dvb/adapter0/demux0"
#define DVR_DEVICE   "/dev/dvb/adapter0/dvr0"
#endif

BitrateDialog::BitrateDialog(): eWindow(1), startTimer(eApp)
{
   const int dlgWidth = 350;
   int fd = eSkin::getActive()->queryValue("fontsize", 20);
   int dlgHeight = 4 * fd;

	unsigned int tvsystem = 1;
	eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", tvsystem);

   cmove(ePoint(58, ((tvsystem == 2) ? 440 : 490) - dlgHeight));
   cresize(eSize(dlgWidth, dlgHeight));
   setText("Bitrate Viewer v1.4");

   eLabel *lbl = new eLabel(this);
   lbl->move(ePoint(10, 10));
   lbl->resize(eSize(50, fd + 6));
   lbl->setText("kb/s:");

   lbl = new eLabel(this);
   lbl->move(ePoint(60, 10));
   lbl->resize(eSize(70, fd + 6));
   lbl->setText("Current");
   lbl->setAlign(eTextPara::dirRight);

   lbl = new eLabel(this);
   lbl->move(ePoint(140, 10));
   lbl->resize(eSize(60, fd + 6));
   lbl->setText("Min");
   lbl->setAlign(eTextPara::dirRight);

   lbl = new eLabel(this);
   lbl->move(ePoint(210, 10));
   lbl->resize(eSize(60, fd + 6));
   lbl->setText("Ave");
   lbl->setAlign(eTextPara::dirRight);

   lbl = new eLabel(this);
   lbl->move(ePoint(280, 10));
   lbl->resize(eSize(60, fd + 6));
   lbl->setText("Max");
   lbl->setAlign(eTextPara::dirRight);

   lbl = new eLabel(this);
   lbl->move(ePoint(10, 10+fd));
   lbl->resize(eSize(50, fd + 6));
   lbl->setText("Video:");

   lbl = new eLabel(this);
   lbl->move(ePoint(10, 10 + fd * 2));
   lbl->resize(eSize(50, fd + 6));
   lbl->setText("Audio:");

   currVal = new eLabel(this);
   currVal->move(ePoint(70, 10+fd));
   currVal->resize(eSize(60, fd + 6));
   currVal->setAlign(eTextPara::dirRight);

   minVal = new eLabel(this);
   minVal->move(ePoint(140, 10+fd));
   minVal->resize(eSize(60, fd + 6));
   minVal->setAlign(eTextPara::dirRight);

   avrgVal = new eLabel(this);
   avrgVal->move(ePoint(210, 10+fd));
   avrgVal->resize(eSize(60, fd + 6));
   avrgVal->setAlign(eTextPara::dirRight);

   maxVal = new eLabel(this);
   maxVal->move(ePoint(280, 10+fd));
   maxVal->resize(eSize(60, fd + 6));
   maxVal->setAlign(eTextPara::dirRight);

   audioVal = new eLabel(this);
   audioVal->move(ePoint(70, 10 + fd * 2));
   audioVal->resize(eSize(60, fd + 6));
   audioVal->setAlign(eTextPara::dirRight);

   audioType = new eLabel(this);
   audioType->move(ePoint(140, 10 + fd * 2));
   audioType->resize(eSize(120, fd + 6));

   CONNECT(startTimer.timeout, BitrateDialog::Run);
}

BitrateDialog::~BitrateDialog()
{
}

int BitrateDialog::eventHandler(const eWidgetEvent &e)
{
   if (e.type == eWidgetEvent::execBegin)
   {
      startTimer.start(500, true);
      return 0;
   }
   else if (e.type == eWidgetEvent::execDone)
   {
      startTimer.stop();
      return 0;
   }
   return eWindow::eventHandler(e);
}

void BitrateDialog::Run()
{
   unsigned char buf[TS_BUF_SIZE];
   struct pollfd pfd;
#if HAVE_DVB_API_VERSION < 3
   struct dmxPesFilterParams flt;
#else
   struct dmx_pes_filter_params flt;
#endif
   int dmxfd;
   struct timeval tv, first_tv, last_print_tv;
   long d_tim_ms, d_print_ms;
   unsigned long long b_total, b_tot1;
   unsigned long long curr_kb_s, kb_s;
   unsigned long long min_kb_s = 50000ULL, max_kb_s = 0ULL;
   long b;
   int rcHandle;

   startTimer.stop();
   int audio_type = Decoder::current.audio_type;
   int apid = Decoder::current.apid;
   int vpid = Decoder::current.vpid;
   int ac3_type = -1;

   if (audio_type == DECODE_AUDIO_AC3)
   {
      PMT *pmt = eDVB::getInstance()->getPMT();
      for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end() && ac3_type == -1; ++i)
      {
         if ((*i)->stream_type == 6 && (*i)->elementary_PID == apid)
         {
            for (ePtrList<Descriptor>::const_iterator ii((*i)->ES_info); ii != (*i)->ES_info.end(); ++ii)
            {
               if (ii->Tag() == 0x6A)
               {
                  ac3_type = ((AC3Descriptor*)*ii)->AC3_type;
                  break;
               }
            }
         }
      }
   }

   pfd.events = POLLIN | POLLPRI;
   if ((pfd.fd = open(DVR_DEVICE, O_RDONLY | O_NONBLOCK)) < 0)
   {
      DisplayErrorMessage("Cannot open DVR device");
      return;
   }
   if ((dmxfd = open(DEMUX_DEVICE, O_RDWR)) < 0)
   {
      ::close(pfd.fd);
      DisplayErrorMessage("Cannot open DEMUX device");
      return;
   }
   if (apid == -1)
      audioVal->setText("N/A");
   if (vpid == -1)
   {
      currVal->setText("N/A");
      avrgVal->setText("N/A");
      minVal->setText("N/A");
      maxVal->setText("N/A");
   }
   if (apid == -1 && vpid == -1)
   {
      ::close(pfd.fd);
      ::close(dmxfd);
      rcHandle = eRCInput::getInstance()->lock();
      while (1)
      {
         short rccode = GetRCCode(rcHandle);
         if (rccode == RC_HOME || rccode == RC_RED)
            break;
         usleep(150000);
      }
      eRCInput::getInstance()->unlock();
      close(-1);
      return;
   }
   ioctl(dmxfd, DMX_SET_BUFFER_SIZE, sizeof(buf));
   if (apid != -1)
   {
      flt.pid = apid;
      flt.input = DMX_IN_FRONTEND;
      flt.output = DMX_OUT_TS_TAP;
#if HAVE_DVB_API_VERSION < 3
      flt.pesType=DMX_PES_OTHER;
#else
      flt.pes_type=DMX_PES_OTHER;
#endif
      flt.flags = DMX_IMMEDIATE_START;
      if (ioctl(dmxfd, DMX_SET_PES_FILTER, &flt) < 0)
      {
         ::close(pfd.fd);
         ::close(dmxfd);
         DisplayErrorMessage("Cannot set PES filter");
         close(-1);
         return;
      }
      gettimeofday(&first_tv, 0);
      last_print_tv.tv_sec = first_tv.tv_sec;
      last_print_tv.tv_usec = first_tv.tv_usec;

      b_total = b_tot1 = 0;

      while (1)
      {
         int b_len, b_start;
         int timeout = 1000;

         b_len = 0;
         b_start = 0;
         if (poll(&pfd, 1, timeout) > 0)
         {
            if (pfd.revents & POLLIN)
            {
               b_len = read(pfd.fd, buf, sizeof(buf));
               gettimeofday(&tv, 0);
            
               if (b_len >= TS_LEN)
                  b_start = SyncTS(buf, b_len);
               else
                  b_len = 0;

               b = b_len - b_start;
               if (b <= 0)
                  continue;
               b_total += b;
               b_tot1 += b;

               d_print_ms = DeltaTimeMs(&tv, &last_print_tv);
               if (d_print_ms >= 1000)
               {
                  d_tim_ms = DeltaTimeMs(&tv, &first_tv);
                  kb_s = (b_total * 8ULL) / (unsigned long long)d_tim_ms;
                  kb_s = kb_s * 97ULL / 100ULL;
                  kb_s = int(((kb_s / 32.0) + .5)) * 32;
                  // if (audio_type != DECODE_AUDIO_MPEG) kb_s -= 32;
                  eString val;
                  val.sprintf("%llu", kb_s);
                  audioVal->setText(val);
                  switch (audio_type)
                  {
                     case DECODE_AUDIO_MPEG:
                        audioType->setText("MPEG");
                        break;
                     case DECODE_AUDIO_AC3:
                        audioType->setText("AC3");
                        break;
                        /*
                        if ((ac3_type & 7) == 1)
                           audioType->setText("AC3 (1.0)");
                        else if ((ac3_type & 7) == 2 || (ac3_type & 7) == 3)
                           audioType->setText("AC3 (2.0)");
                        else if ((ac3_type & 7) == 4)
                           audioType->setText("AC3 (5.1)");
                        else
                        {
                           val.sprintf("AC3 (0x%x)", ac3_type & 7);
                           audioType->setText(val);
                        }
                        break;
                        */
                     case DECODE_AUDIO_DTS:
                        audioType->setText("DTS");
                        break;
                     case DECODE_AUDIO_AC3_VOB:
                        audioType->setText("AC3/VOB");
                        break;
                  }
                  break;
               }
            }
         }
         else
         {
            audioVal->setText("N/A");
            break;
         }
      }
      ioctl(dmxfd, DMX_STOP);
   }
   if (vpid != -1)
   {
      flt.pid = vpid;
      flt.input = DMX_IN_FRONTEND;
      flt.output = DMX_OUT_TS_TAP;
#if HAVE_DVB_API_VERSION < 3
      flt.pesType=DMX_PES_OTHER;
#else
      flt.pes_type=DMX_PES_OTHER;
#endif
      flt.flags = DMX_IMMEDIATE_START;
      if (ioctl(dmxfd, DMX_SET_PES_FILTER, &flt) < 0)
      {
         ::close(pfd.fd);
         ::close(dmxfd);
         DisplayErrorMessage("Cannot set PES filter");
         close(-1);
         return;
      }
      gettimeofday(&first_tv, 0);
      last_print_tv.tv_sec = first_tv.tv_sec;
      last_print_tv.tv_usec = first_tv.tv_usec;

      b_total = b_tot1 = 0;

      rcHandle = eRCInput::getInstance()->lock();
      while (1)
      {
         int b_len, b_start;
         int timeout = 1000;

         b_len = 0;
         b_start = 0;
         if (poll(&pfd, 1, timeout) > 0)
         {
            if (pfd.revents & POLLIN)
            {
               b_len = read(pfd.fd, buf, sizeof(buf));
               gettimeofday(&tv, 0);
            
               if (b_len >= TS_LEN)
                  b_start = SyncTS(buf, b_len);
               else
                  b_len = 0;

               b = b_len - b_start;
               if (b <= 0)
                  continue;
               b_total += b;
               b_tot1 += b;

               d_print_ms = DeltaTimeMs(&tv, &last_print_tv);
               if (d_print_ms >= 1000)
               {
                  d_tim_ms = DeltaTimeMs(&tv, &first_tv);
                  kb_s = (b_total * 8ULL) / (unsigned long long)d_tim_ms;
                  kb_s = kb_s * 97ULL / 100ULL;
                  curr_kb_s = (b_tot1 * 8ULL) / (unsigned long long)d_print_ms;
                  curr_kb_s = curr_kb_s * 97ULL / 100ULL;
                  b_tot1 = 0;
                  eString val;
                  val.sprintf("%llu", curr_kb_s);
                  currVal->setText(val);
                  val.sprintf("%llu", kb_s);
                  avrgVal->setText(val);
                  if (curr_kb_s < min_kb_s)
                  {
                     min_kb_s = curr_kb_s;
                     val.sprintf("%llu", min_kb_s);
                     minVal->setText(val);
                  }
                  if (curr_kb_s > max_kb_s)
                  {
                     max_kb_s = curr_kb_s;
                     val.sprintf("%llu", max_kb_s);
                     maxVal->setText(val);
                  }
                  last_print_tv.tv_sec = tv.tv_sec;
                  last_print_tv.tv_usec = tv.tv_usec;
               }
            }
         }
         else
         {
            currVal->setText("N/A");
            avrgVal->setText("N/A");
            minVal->setText("N/A");
            maxVal->setText("N/A");
         }
         short rccode = GetRCCode(rcHandle);
         if (rccode == RC_HOME || rccode == RC_RED)
            break;
      }
      ioctl(dmxfd, DMX_STOP);
   }
   else
   {
      rcHandle = eRCInput::getInstance()->lock();
      while (1)
      {
         short rccode = GetRCCode(rcHandle);
         if (rccode == RC_HOME || rccode == RC_RED)
            break;
         usleep(150000);
      }
   }
   ::close(pfd.fd);
   ::close(dmxfd);
   eRCInput::getInstance()->unlock();
   close(-1);
}

unsigned long BitrateDialog::TimevalToMs(const struct timeval *tv)
{
   return(tv->tv_sec * 1000) + ((tv->tv_usec + 500) / 1000);
}

long BitrateDialog::DeltaTimeMs(struct timeval *tv, struct timeval *last_tv)
{
   return TimevalToMs(tv) - TimevalToMs(last_tv);
}

int BitrateDialog::SyncTS(unsigned char *buf, int len)
{
   int  i;

   for (i = 0; i < len; i++)
   {
      if (buf[i] == TS_SYNC_BYTE)
      {
         if ((i+TS_LEN) < len)
         {
            if (buf[i + TS_LEN] != TS_SYNC_BYTE)
               continue;
         }
         break;
      }
   }
   return i;
}

void BitrateDialog::DisplayErrorMessage(const char *msg)
{
   eMessageBox box(msg, _("Error!"), eMessageBox::btOK | eMessageBox::iconError);
   box.show(); box.exec(); box.hide();
   close(-1);
}
