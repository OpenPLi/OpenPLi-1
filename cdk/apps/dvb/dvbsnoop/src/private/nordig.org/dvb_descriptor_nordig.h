/*
$Id: dvb_descriptor_nordig.h,v 1.2 2008/09/01 08:06:07 mws Exp $ 


 DVBSNOOP

 a dvb sniffer  and mpeg2 stream analyzer tool
 http://dvbsnoop.sourceforge.net/

 (c) 2001-2006   Rainer.Scherg@gmx.de (rasc)


 -- private DVB Descriptors  nordig.org



$Log: dvb_descriptor_nordig.h,v $
Revision 1.2  2008/09/01 08:06:07  mws
decode NorDig content protection descriptor

Revision 1.1  2008/08/29 20:00:06  obi
decode NorDig logic(al) channel descriptor (e.g. used by unitymedia)

Revision 1.3  2006/04/04 17:16:54  rasc
finally fix typo in premiere descriptor name

Revision 1.2  2006/01/02 18:24:16  rasc
just update copyright and prepare for a new public tar ball

Revision 1.1  2004/11/03 21:01:02  rasc
 - New: "premiere.de" private tables and descriptors (tnx to Peter.Pavlov, Premiere)
 - New: cmd option "-privateprovider <provider name>"
 - New: Private provider sections and descriptors decoding
 - Changed: complete restructuring of private descriptors and sections


*/


#ifndef _NORDIG_DVB_DESCRIPTOR_H
#define _NORDIG_DVB_DESCRIPTOR_H 


void descriptor_PRIVATE_NordigORG_LogicChannelDescriptor (u_char *b);
void descriptor_PRIVATE_NordigORG_ContentProtectionDescriptor (u_char *b);


#endif

