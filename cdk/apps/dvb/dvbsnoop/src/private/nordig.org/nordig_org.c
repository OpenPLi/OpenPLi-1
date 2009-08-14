/*
$Id: nordig_org.c,v 1.2 2008/09/01 08:06:07 mws Exp $


 DVBSNOOP

 a dvb sniffer  and mpeg2 stream analyzer tool
 http://dvbsnoop.sourceforge.net/

 (c) 2001-2006   Rainer.Scherg@gmx.de  (rasc)


  -- Private Data Structures for:
  -- nordig.org

$Log: nordig_org.c,v $
Revision 1.2  2008/09/01 08:06:07  mws
decode NorDig content protection descriptor

Revision 1.1  2008/08/29 20:00:06  obi
decode NorDig logic(al) channel descriptor (e.g. used by unitymedia)

Revision 1.5  2006/04/04 17:16:54  rasc
finally fix typo in premiere descriptor name

Revision 1.4  2006/01/02 18:24:16  rasc
just update copyright and prepare for a new public tar ball

Revision 1.3  2005/08/10 21:28:19  rasc
New: Program Stream handling  (-s ps)

Revision 1.2  2005/06/29 17:30:38  rasc
some legal notes...

Revision 1.1  2004/11/03 21:01:02  rasc
 - New: "premiere.de" private tables and descriptors (tnx to Peter.Pavlov, Premiere)
 - New: cmd option "-privateprovider <provider name>"
 - New: Private provider sections and descriptors decoding
 - Changed: complete restructuring of private descriptors and sections




*/




#include "dvbsnoop.h"
#include "nordig_org.h"
#include "dvb_descriptor_nordig.h"


static PRIV_DESCR_ID_FUNC pdescriptors[] = {
	{ 0x83, DVB_SI,   descriptor_PRIVATE_NordigORG_LogicChannelDescriptor },
	{ 0xA0, DVB_SI,   descriptor_PRIVATE_NordigORG_ContentProtectionDescriptor },
	{ 0x00,	0,        NULL } // end of table  (id = 0x00, funct = NULL)
};

//
// -- Return private section/descriptor id tables
// -- for this scope
//

void getPrivate_NordigORG ( PRIV_SECTION_ID_FUNC **psect,
		PRIV_DESCR_ID_FUNC **pdesc)
{
   *psect = NULL;
   *pdesc = pdescriptors;
}




